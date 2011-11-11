/*
 * BenemMUD v1   
 * BenemMUD is based on DIKUMUD
 */

#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <time.h>

#include "protos.h"


#define MAX_MSGS 99                    /* Max number of messages.          */
#define MAX_MESSAGE_LENGTH 2048     /* that should be enough            */
#define NUM_BOARDS 6

struct message
{
  char *date;
  char *title;
  char *author;
  char *text;
};

struct board
{
  struct message msg[MAX_MSGS+1];
  int number;
};

static struct board_lock_struct
{
  struct char_data *locked_for;
  bool lock;
} board_lock[NUM_BOARDS];


int min_read_level[]   = {  0, 51,  1, 1, 1, 1 };
int min_write_level[]  = {  1, 51,  1, 1, 1, 1 };
int min_remove_level[] = { 51, 51, 51, 1, 1, 1 };

struct board boards[NUM_BOARDS];
struct board *curr_board;
struct message *curr_msg;  
extern struct char_data *character_list;

/* This sets the minimum level needed to read/write/look at these boards
   mainly included to enable the creation of a "wizard-only" board        */

char save_file[NUM_BOARDS][20] =
{ 
  "mortal.board", 
  "wiz.board", 
  "skexie.board",
  "ingannatori.board",
  "lamapietra.board",
  "esploratori.board"
};

    
/* These are the binary files in which to save/load messages */

void board_write_msg(struct char_data *ch, char *arg, int bnum);
int board_display_msg(struct char_data *ch, char *arg, int bnum);
int board_remove_msg(struct char_data *ch, char *arg, int bnum);
void board_save_board( int bnum );
void board_load_board();
int board_show_board(struct char_data *ch, char *arg, int bnum);

/* board.c version 1.2 - Jun 1991 by Twilight.
 *
 * 1.2 changes:

 *   Added a board and message structure took out all pointers in an effort to 
 *   insure integrity in memory. Added differentiation between minimum read 
 *   levels and minimum write/remove levels.
 * 
 * 1.1 changes:
 * 
 *   Major repairs-- now allows multiple boards define at compile-time.  
 *   Set the constants NUM_BOARDS and add the new V-Numbers to the if/then 
 *   structure directl below.  Also you must attach the board procedure in
 *   spec_assign.c as usual.
 * 
 *   Log message removals and restrict them to level 15 and above.
 *   Fixed act message resulting from message removal 
 *   Removed unused procedure "fix_long_desc"
 *   Added a message to inform others in room of a read in progress
 *   Added minimum level check for each board 
 *   (defined in array min_board_level[NUM_BOARDS]

*/

int board( struct char_data *ch, int cmd, char *arg, struct obj_data *obj, 
           int type )
{
  static int has_loaded = 0;
  int bnum = -1;
  int obj_num;

  if (type != EVENT_COMMAND)
    return(FALSE);

  if (!ch->desc)
    return(0); /* or all NPC's will be trapped at the board */
  
  if (!has_loaded)
  {
    board_load_board();
    has_loaded = 1;
  }

  if (!cmd)
    return(FALSE);

  /* Identify which board we're dealing with */
  
  obj_num = (obj->item_number);
  if( obj_num == real_object( 3099 ) )
    bnum = 0;
  else if( obj_num == real_object( 3098 ) )
    bnum = 1;
  else if( obj_num == real_object( 3097 ) )
    bnum = 2;
  else if( obj_num == real_object( 20023 ) )
    bnum = 3;
  else if( obj_num == real_object( 20203 ) )
    bnum = 4;
  else if( obj_num == real_object( 20403 ) )
    bnum = 5;

  switch (cmd) 
  {
  case 15:  /* look */
    return(board_show_board(ch, arg, bnum));
  case 149: /* write */
    board_write_msg(ch, arg, bnum);
    return 1;
  case 63: /* read */
    return(board_display_msg(ch, arg, bnum));
  case 66: /* remove */
    return(board_remove_msg(ch, arg, bnum));
  default:
    return 0;
  }
}


void board_write_msg(struct char_data *ch, char *arg, int bnum) 
{

  int highmessage;
  char buf[MAX_STRING_LENGTH];
  long ct; /* clock time */
  char *tmstr;

  if ( bnum == -1 ) 
  {
    mudlog( LOG_ERROR, 
            "Board special procedure called for non-board object.\r\n");
    send_to_char("La bacheca e` chiusa in questo momento.\n\r", ch);
    return;
  }

  curr_board = &boards[bnum];

  if (GetMaxLevel(ch) < min_write_level[bnum]) 
  {
    send_to_char( "Prendi una penna per scrivere, ma ti rendi conto di non "
                  "essere abbastanza\n\rpotente per lasciare qualcosa di "
                  "intelligente in QUESTA\n\rbacheca\n\r", ch );
    return;
  }

  if( (curr_board->number) > (MAX_MSGS - 1) ) 
  {
    send_to_char("La bacheca e` piena.\n\r", ch);
    return;
  }

  /* Check for locks, return if lock is found on this board */

  if (board_check_locks(bnum, ch))
    return;

  /* skip blanks */

  for(; isspace(*arg); arg++);

  if (!*arg) 
  {
    send_to_char( "Il contenuto della bacheca e` stato memorizzato.\n\r"
                  "Per scrivere un nuovo messaggio usa WRITE seguito da un "
                  "titolo.\n\r", ch);
    return;
  }

  /* Now we're committed to writing a message.  Let's lock the board. */

  board_lock[bnum].lock = 1;
  board_lock[bnum].locked_for = ch;

  /* Lock set */

  highmessage = boards[bnum].number;
  curr_msg = &curr_board->msg[++highmessage];

  if (!(strcmp("Topic",arg))) 
  {
    curr_msg = &curr_board->msg[0];
    free(curr_msg->title);
    free(curr_msg->text);
    free(curr_msg->author);
    free(curr_msg->date);
    (boards[bnum].number)--;
  }
  curr_msg->title = (char *)malloc(strlen(arg)+1);
  strcpy(curr_msg->title, arg);
  curr_msg->author = (char *)malloc(strlen(GET_NAME(ch))+1);
  strcpy(curr_msg->author, GET_NAME(ch));
  ct = time(0);
  tmstr = (char *)asctime(localtime(&ct));
  *(tmstr + strlen(tmstr) - 1) = '\0';
  sprintf(buf,"%.10s",tmstr);
  curr_msg->date = (char *)malloc(strlen(buf)+1);
  strcpy(curr_msg->date, buf);
  send_to_char( "Scrivi il tuo messaggio. Batti '@' quando hai finito.\n\r"
                "Dopo batti nuovamente WRITE (senza argomento) per "
                "memorizarlo.\n\r", ch);
  act("$n inizia a scrivere un messaggio.", TRUE, ch, 0, 0, TO_ROOM);

  /* Take care of free-ing and zeroing if the message text is already
     allocated previously */

  if (curr_msg->text)
    free (curr_msg->text);
  curr_msg->text = 0;

  /* Initiate the string_add procedures from comm.c */

  ch->desc->str = &curr_msg->text;
  ch->desc->max_str = MAX_MESSAGE_LENGTH;
 
   boards[bnum].number +=1; 
  
  if (boards[bnum].number < 0)
    boards[bnum].number = 0;
}

int board_remove_msg(struct char_data *ch, char *arg, int bnum)
{
  int ind, tmessage;
  char buf[256], number[MAX_INPUT_LENGTH];
  
  one_argument(arg, number);
  
  if (!*number || !isdigit(*number))
    return(0);
  
  if( !( tmessage = atoi( number ) ) )
    return(0);
  
  if( bnum == -1 ) 
  {
    mudlog( LOG_ERROR,
            "Board special procedure called for non-board object.\r\n");
    send_to_char("La bacheca e` chiusa, in questo momento.\n\r", ch);
    return 1;
  }

  curr_board = &boards[bnum];

  if(GetMaxLevel(ch) < min_remove_level[bnum]) 
  {
    return FALSE;
#if 0
    send_to_char( "Provi a togliere uno dei foglietti nella bacheca ma ti "
                  "prendi una leggera\n\r",ch);
    send_to_char("scossa. Meglio lasciar perdere.\n\r",ch);
    return 1;
#endif
  }

  if (curr_board->number < 1) 
  {
    send_to_char("La bacheca e` vuota !\n\r", ch);
    return(1);
  }

  if (tmessage < 0 || tmessage > curr_board->number) 
  {
    send_to_char("Quel messaggio esiste solo nella tua immaginazione.\n\r", ch);
    return(1);
  }

  /* Check for board locks, return if lock is found */
  
  if (board_check_locks(bnum, ch))
    return(1);

  ind = tmessage;

  free(curr_board->msg[ind].text);
  free(curr_board->msg[ind].date);
  free(curr_board->msg[ind].author);
  free(curr_board->msg[ind].title);

  for ( ; ind < (curr_board->number) ; ind++ )
    curr_board->msg[ind] = curr_board->msg[ind+1];

/* You MUST do this, or the next message written after a remove will */
/* end up doing a free(curr_board->msg[ind].text) because it's not!! */
/* Causing strange shit to happen, because now the message has a     */
/* To a memory location that doesn't exist, and if THAT message gets */
/* Removed, it will destroy what it's pointing to. THIS is the board */
/* Bug we've been looking for!        -=>White Gold<=-               */

  curr_board->msg[curr_board->number].text = NULL;
  curr_board->msg[curr_board->number].date = NULL;
  curr_board->msg[curr_board->number].author = NULL;
  curr_board->msg[curr_board->number].title = NULL;

  curr_board->number--;

  send_to_char("Messaggio cancellato.\n\r", ch);
  sprintf( buf, "%s ha cancellato il messaggio numero %d.", 
           ch->player.name, tmessage );

  /* Removal message also repaired */

  act(buf, FALSE, ch, 0, 0, TO_ROOM);
  sprintf((buf+strlen(buf)-1)," from board %d.",bnum);
  mudlog( LOG_PLAYERS, buf );  /* Message removals now logged. */

  board_save_board(bnum);
  return(1);
}

char *fix_returns(char *text_string)
{
  char *localbuf;
  int point=0;
  int point2 = 0;

  if (!text_string) 
  {
    CREATE(localbuf,char,2);
    strcpy(localbuf,"\n");
    return(localbuf);
  }

  if (!(*text_string)) 
  {
    CREATE(localbuf,char,strlen("(NULL)")+1);
    strcpy(localbuf,"(NULL)");
    return(localbuf);
  }

  CREATE(localbuf,char,strlen(text_string));

  while(*(text_string+point) != '\0') 
  {
    if (*(text_string+point) != '\r') 
    {
      *(localbuf+point2) = *(text_string+point);
      point2++;
      point++;
    }
    else
      point++;
  }
  *(localbuf + point2) = '\0'; /* You never made sure of null termination */
  return(localbuf);
}
  
void board_save_board( int bnum )
{

  FILE *the_file;
  int ind;
  char *temp_add;

  /* We're assuming the board number is valid since it was passed by
     out own code */

  curr_board = &boards[bnum];

  the_file = fopen(save_file[bnum], "w");

  if (!the_file) 
  {
    mudlog( LOG_ERROR, "Unable to open/create savefile for bulletin board..");
    return;
  }

  fprintf(the_file," %d ", curr_board->number);
  for (ind = 0; ind <= curr_board->number; ind++) 
  {
    curr_msg = &curr_board->msg[ind];
    fwrite_string(the_file,curr_msg->title);
    fwrite_string(the_file,curr_msg->author);
    fwrite_string(the_file,curr_msg->date);
    fwrite_string(the_file,(temp_add = fix_returns(curr_msg->text)));
    free(temp_add);
  }
  fclose(the_file);
  return;
}

void board_load_board()
{

  FILE *the_file;
  int ind;
  int bnum;
  
  memset(boards, 0, sizeof(boards)); /* Zero out the array, make sure no */
                                     /* Funky pointers are left in the   */
                                     /* Allocated space                  */

  for ( bnum = 0 ; bnum < NUM_BOARDS ; bnum++ ) 
  {
    board_lock[bnum].lock = 0;
    board_lock[bnum].locked_for = 0;
  }

  for (bnum = 0; bnum < NUM_BOARDS; bnum++) 
  {
    boards[bnum].number = -1;
    the_file = fopen(save_file[bnum], "r");
    if (!the_file) 
    {
      mudlog( LOG_ERROR, "Can't open message file for board %d.",bnum);
      continue;
    }

    fscanf( the_file, " %d ", &boards[bnum].number);
    if( boards[bnum].number < 0 || boards[bnum].number > MAX_MSGS || 
        feof(the_file)) 
    { 
      mudlog( LOG_ERROR, "Board-message file corrupt, nonexistent, or "
                         "empty (Bnum:%d).", boards[bnum].number );
      boards[bnum].number = -1;
      fclose(the_file);
      continue;
    }

    curr_board = &boards[bnum];

    for (ind = 0; ind <= curr_board->number; ind++) 
    {
      curr_msg = &curr_board->msg[ind];
      curr_msg->title = (char *)fread_string (the_file);
      curr_msg->author = (char *)fread_string (the_file);
      curr_msg->date = (char *)fread_string (the_file);
      curr_msg->text = (char *)fread_string (the_file);
    }
    fclose(the_file);
  }
}

int board_display_msg(struct char_data *ch, char *arg, int bnum)
{
  char number[MAX_INPUT_LENGTH], buffer[MAX_STRING_LENGTH];
  int tmessage;

  one_argument(arg, number);

  if (!*number || !isdigit(*number))
    return(0);

  if (!(tmessage = atoi(number))) 
    return(0);

  curr_board = &boards[bnum];

  if ((boards[bnum].number != -1) &&
      (tmessage >= 0 && tmessage <= curr_board->number) &&
      (GetMaxLevel(ch) < min_read_level[bnum]) &&
      (strcmp(GET_NAME(ch), curr_board->msg[tmessage].author))) ;
  else if ( GetMaxLevel(ch) < min_read_level[bnum] ) 
  {
    send_to_char("Provi a leggere i messaggi nella bacheca, ma\n\r",ch);
    send_to_char("non riesci a comprenderne il significato.\n\r\n\r",ch);
    act( "$n prova a leggere i messaggi, ma sembra confus$b.", TRUE, ch, 0, 0,
         TO_ROOM);
    return(1);
  }

  if (boards[bnum].number == -1) 
  {
    send_to_char( "La bacheca e` vuota!\n\r", ch );
    return(1);
  }
  
  if (tmessage < 0 || tmessage > curr_board->number) 
  {
    send_to_char("Quel messaggio esiste solo nella tua immaginazione.\n\r",ch);
    return(1);
  }

  curr_msg = &curr_board->msg[tmessage];

  sprintf( buffer, "Messaggio %2d (%s): %-15s -- %s", tmessage, curr_msg->date, 
           curr_msg->author, curr_msg->title );
  sprintf( buffer + strlen(buffer), "\n\r----------\n\r%s", 
           (curr_msg->text?curr_msg->text:"(null)"));
  page_string(ch->desc, buffer, 1);
  return(1);
}
                
int board_show_board(struct char_data *ch, char *arg, int bnum)
{
  int i;
  char buf[MAX_STRING_LENGTH], tmp[MAX_INPUT_LENGTH];

  one_argument( arg, tmp );

  if( !*tmp || !isname( tmp, "bacheca bollettino lavagna board bulletin" ) )
    return(0);

  if ((GetMaxLevel(ch) < min_read_level[bnum]) && (bnum !=5)) 
    /* Skip if board 5 (Reimb board) */
  { 
    send_to_char("Provi a leggere i messaggi nella bacheca, ma\n\r",ch);
    send_to_char("non riesci a comprenderne il significato.\n\r\n\r",ch);
    act( "$n prova a leggere i messaggi, ma sembra confus$b.", TRUE, ch, 0, 0,
         TO_ROOM);
    return(1);
  }

  curr_board = &boards[bnum];

  act("$n studia la bacheca.", TRUE, ch, 0, 0, TO_ROOM);

  strcpy(buf, "Questa e` una bacheca. Uso: READ/REMOVE <messg #>, "
              "WRITE <header>\n\rDopo aver scritto il messaggio battete WRITE"
              " senza argomento per memorizzarlo\n\rpermanentemente. " );
  if( boards[bnum].number == -1 )
    strcat(buf, "La bacheca e` vuota.\n\r");
  else 
  {
    if( boards[bnum].number == 0 )
      strcat(buf, "La bacheca e` vuota.\n\r");
    else
      sprintf( buf + strlen(buf), "%s %d messaggi%s in bacheca.\n\r",
               curr_board->number == 1 ? "C'e`" : "Ci sono",
               curr_board->number, curr_board->number == 1 ? "o" : "" );
    sprintf( buf + strlen(buf), "\n\rArgomenti:\n\r%s------------\n\r",
             curr_board->msg[0].text );
    for( i = 1 ; i <= curr_board->number ; i++ ) 
    {
#if 0
      if(((GetMaxLevel(ch) < min_read_level[bnum]) &&
          (strcmp(ch->name, curr_board->msg[i].author))) ||
         (GetMaxLevel(ch) >= min_read_level[bnum]))  
#else
      if( GetMaxLevel( ch ) >= min_read_level[ bnum ] )
#endif
      {          
        sprintf(buf + strlen(buf), "%-2d : %-15s (%s) -- %s\n\r", i , 
                curr_board->msg[i].author, curr_board->msg[i].date,
                curr_board->msg[i].title);
      }
    }
  }
  page_string(ch->desc, buf, 1);
  return(1);
}

/*
int fwrite_string (char *buf, FILE *fl)
{
  return (fprintf(fl, "%s~\n", buf));
}
*/

int board_check_locks (int bnum, struct char_data *ch) {
  
  char buf[MAX_INPUT_LENGTH];
  struct char_data *tmp_char;
  bool found = FALSE;
  if (!board_lock[bnum].lock) 
    return(0);
  
  /* FIRST lets' see if this character is even in the game anymore! -WG-*/
  for (tmp_char = character_list; tmp_char; tmp_char = tmp_char->next)
  {
    if (tmp_char == board_lock[bnum].locked_for)
    {
      found = TRUE;
      break;
    }
  }
  if (!found)
  {
    mudlog( LOG_ERROR, "Board: board locked for a user not in game.");
    board_lock[bnum].lock = 0;
    board_lock[bnum].locked_for = NULL;
    return(0);
  }

  /* Check for link-death of lock holder */

  if (!board_lock[bnum].locked_for->desc) 
  {
    sprintf( buf,"Spingi %s da una parte e ti avvicini alla bacheca.\n\r",
             board_lock[ bnum ].locked_for->player.name);
    send_to_char(buf, ch);
  }

  /* Else see if lock holder is still in write-string mode */

  else if (board_lock[bnum].locked_for->desc->str)
  { /* Lock still holding */
    sprintf( buf, "Provi ad avvicinarti alla bacheca, ma %s ti blocca la "
                  "strada.\n\r", board_lock[ bnum ].locked_for->player.name );
    send_to_char(buf, ch);
    return (1);
  }

  /* Otherwise, the lock has been lifted */

  board_save_board(bnum);
  board_lock[bnum].lock = 0;
  board_lock[bnum].locked_for = 0;
  return(0);
}
  

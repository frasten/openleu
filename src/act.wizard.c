
/*
 * BenemMUD 
 */

#include <ctype.h>
#include <stdio.h>
#include <arpa/telnet.h>
#include <stdlib.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>



#include "protos.h"
#include "cmdid.h"
#include "fight.h"

#ifndef __USE_XOPEN
#define __USE_XOPEN
#endif

#include <unistd.h>

void switch_light(byte why);

/*   external vars  */

extern long SystemFlags;
extern struct weather_data weather_info;
extern char *system_flag_types[];
extern struct zone_data *zone_table;
extern int top_of_zone_table;
#if HASH
extern struct hash_header room_db;
#else
extern struct room_data *room_db[];
#endif
extern struct char_data *character_list;
extern struct descriptor_data *descriptor_list;
extern struct title_type titles[MAX_CLASS][ABS_MAX_LVL];
extern struct index_data *mob_index;
extern struct index_data *obj_index;
extern int top_of_p_table;
extern int top_of_mobt;
extern int top_of_objt;
extern struct int_app_type int_app[26];
extern struct wis_app_type wis_app[26];
extern struct player_index_element *player_table;
extern char *room_bits[];
extern struct str_app_type str_app[];
extern char *spells[];
extern struct spell_info_type spell_info[MAX_SPL_LIST];
extern char *aszLogMessagesType[];
extern char *aszWeaponSpecialEffect[];
extern char *aszWeaponType[];


char EasySummon = 1;


void do_auth(struct char_data *ch, const char *argument, int cmd)
{
  char name[50], word[20], szMessage[ MAX_INPUT_LENGTH + 1 ];
  char buf[ MAX_INPUT_LENGTH + MAX_INPUT_LENGTH ];
  int done=FALSE;
  struct descriptor_data *d;


  if( !IS_PC( ch ) )
    return;

  /* parse the argument */
  /* get char name */
  argument = one_argument(argument, name);
  /*
   * search through descriptor list for player name
   */
  for (d=descriptor_list; d && !done; d = d->next)
  {
    if (d->character)
    {
      if( GET_NAME(d->character) &&
          (str_cmp(GET_NAME(d->character), name) == 0))
      {
        done = TRUE;
        break;
      }
    }
  }

  /*
   * if not found, return error
   */
  if (!d)
  {
    send_to_char("That player was not found.\n\r", ch);
    return;
  }

  if (*argument)
  {
    /* get response (rest of argument) */
    one_argument(argument, word);
    if (str_cmp(word,"yes")==0)
    {
      d->character->generic = NEWBIE_START;
      mudlog( LOG_PLAYERS,"%s has just accepted %s into the game.",
              ch->player.name,name);
      SEND_TO_Q("You have been accepted.  Press enter\n\r", d);
    }
    else if (str_cmp(word,"no")==0)
    {
      SEND_TO_Q("You have been denied.  Press enter\n\r", d);
      mudlog( LOG_PLAYERS,"%s has just denied %s from the game.",
              ch->player.name,name);
      d->character->generic = NEWBIE_AXE;
    }
    else
    {
      only_argument( argument, szMessage );
      SEND_TO_Q( szMessage, d);
      SEND_TO_Q( "\n\r", d);
      sprintf( buf, "Hai mandato '%s' a %.20s\n\r", szMessage, 
               GET_NAME(d->character) );
      send_to_char(buf, ch);
      return;
    }
  }
  else
  {
    send_to_char("Auth[orize] {Yes | No | Message} \n\r", ch);
    return;
  }
  return;
}

void do_imptest(struct char_data *ch, const char *arg, int cmd)
{
char buf[255];

 SET_BIT(ch->player.user_flags,USE_ANSI);
 sprintf(buf,"%s",ansi_parse(arg));  
 send_to_char(buf,ch);

#if 0
  struct char_data *i;
  struct StrHeap *H=0;
  int x = 0;

  if (GetMaxLevel(ch) < IMPLEMENTOR) return;

  H = InitHeap();

  for (i=character_list; i&&x++<100; i=i->next) {
       StringHeap(i->player.short_descr, H);
  } 

  DisplayStringHeap(H, ch, TO_CHAR, TRUE);

#endif

}

void do_passwd(struct char_data *ch, const char *argument, int cmdnum)
{
#if !defined( EMANUELE )
  int player_i, pos;
#else
  char szFileName[ 40 ];
#endif
  char name[30], npasswd[20], pass[20];
  struct char_file_u tmp_store;
  FILE *fl;

  /*  sets the specified user's password. */
  /*  get user's name: */

  argument = one_argument(argument, name);
  argument = one_argument(argument, npasswd);

  /*   Look up character */
#if defined( EMANUELE )
  if( load_char( name, &tmp_store ) )
  {
#else
  if( (player_i = load_char(name, &tmp_store)) > -1)  {
#endif
   
    /*  encrypt new password. */
    if (!*npasswd || strlen(npasswd) > 10) 
    {
      send_to_char("Illegal password\n\r", ch);
      return;
    }

#if defined(CYGWIN)
	 strncpy(pass,npasswd, 10);
#else
    strncpy(pass,(char *)crypt(npasswd, tmp_store.name), 10);
#endif     


    /*  put new password in place of old password */
    *( pass + 10 ) = '\0';
    strcpy( tmp_store.pwd, pass );
    
    /*   save char to file */
#if defined( EMANUELE )
    sprintf( szFileName, "%s/%s.dat", PLAYERS_DIR, lower( name ) );
    if( ( fl = fopen( szFileName, "w+b" ) ) == NULL )
    {
      mudlog( LOG_ERROR, "Cannot open file %s for saving player.", szFileName );
      return;
    }
    fwrite( &tmp_store, sizeof( tmp_store ), 1, fl );
    fclose( fl );
#else

    pos = player_table[player_i].nr;

    if (!(fl = fopen(PLAYER_FILE, "r+")))        
    {
      perror("do_passwd");
      assert(0);
    }

    fseek(fl, pos * sizeof(struct char_file_u), 0);

    fwrite(&tmp_store, sizeof(struct char_file_u), 1, fl);

    fclose(fl);
#endif
  }
  else 
     send_to_char("I don't recognize that name\n\r", ch);
}

void do_setsev(struct char_data *ch, const char *arg, int cmd)
{
  char buf[255];
  char buf2[255];
  unsigned sev;

  arg = one_argument(arg, buf);
  
  if( strlen( buf ) != 0 ) 
  {
    sev = atoi( buf );
    sprintbit( sev, aszLogMessagesType, buf );
    ch->specials.sev = sev;
    sprintf( buf2, "Riceverai i seguenti tipi di messaggi: [%s].\n\r", buf );
    send_to_char( buf2, ch );
    return;
  } 
  else 
  {
    send_to_char( "\n\rCon il comando setsev <numero> puoi cambiare il tipo di messaggi di sistema\n\r", ch );
    send_to_char( "visualizzati. Il numero e` un vettore di bit con il seguente significato:\n\r\n\r", ch );
    send_to_char( " # Nome    Descrizione\n\r", ch );
    send_to_char( " 1 SYSERR  Messaggi di errore grave. Comunicarli a Benem appena possibile.\n\r", ch );
    send_to_char( " 2 CHECK   Messaggi di controllo. Possono essere ignorati.\n\r", ch );
    send_to_char( " 4 PLAYERS Messaggi riguardanti i giocatori (morti ed altri eventi).\n\r", ch );
    send_to_char( " 8 MOBILES Messaggi riguardanti i mobs.\n\r", ch );
    send_to_char( "16 CONNECT Messaggi riguardanti le nuove connessioni.\n\r", ch );
    send_to_char( "32 ERROR   Messaggi di errore non gravi riguardanti il database od altro.\n\r\n\r", ch );
    send_to_char( "Vi prego di tenere attivo almeno SYSERR in modo da individuare eventuali\n\r", ch );
    send_to_char( "bugs.\n\r\n\r", ch );

    sprintbit( ch->specials.sev, aszLogMessagesType, buf );
    sprintf( buf2, "Attualmente ricevi i seguenti tipi di messaggi: [%s].\n\r", 
             buf );
    send_to_char( buf2, ch );
                    
    return;
  }
}


/* Bamfin and bamfout - courtesy of DM from Epic */
void dsearch(char *string, char *tmp)
{
 char *c, buf[255], buf2[255], buf3[255];
 int i, j;

 i = 0;
 while(i == 0) {
    if(strchr(string, '~')==NULL) {
       i = 1;
       strcpy(tmp, string);
    } else {
       c = strchr(string, '~');
       j = c-string;
       switch(string[j+1]) {
         case 'N': strcpy(buf2, "$n"); break;
         case 'H': strcpy(buf2, "$s"); break;
         default:  strcpy(buf2, ""); break;
       }
       strcpy(buf, string);
       buf[j] = '\0';
       strcpy(buf3, (string+j+2));
       sprintf(tmp, "%s%s%s" ,buf, buf2, buf3);
       sprintf(string, tmp);

     }
  }
}

void do_bamfin(struct char_data *ch, const char *arg, int cmd)
{
 char buf[255];
 char *argument;
 int len;

 for (; *arg == ' '; arg++);  /* pass all those spaces */

 if(!*arg) {
   send_to_char("Bamfin <bamf definition>\n\r", ch);
   send_to_char(" Additional arguments can include ~N for where you\n\r", ch);
   send_to_char(" Want your name (if you want your name).  If you use\n\r",ch);
   send_to_char(" ~H, it puts in either his or her depending on your\n\r",ch);
   send_to_char(" sex.  If you use the keyword 'def' for your bamf,\n\r", ch);
   send_to_char(" it turns on the default bamf.  \n\r", ch);
   return;
 }

 if(!strcmp(arg, "def")) {
    REMOVE_BIT(ch->specials.pmask, BIT_POOF_IN);
    free(ch->specials.poofin);
    ch->specials.poofin = 0;
    send_to_char("Ok.\n\r", ch);
    return;
 }

 len = strlen(arg);
 argument = strdup(arg);

 if(len > 150) {
   send_to_char("String too long.  Truncated to:\n\r", ch);
   argument[150] = '\0';
   sprintf(buf, "%s\n\r", argument);
   send_to_char(buf, ch);   
   len = 150;
 }

 if (ch->specials.poofin && len >= (int) strlen(ch->specials.poofin)) {
   free(ch->specials.poofin);
   ch->specials.poofin = (char *)malloc(len+1);
 } else { 
   if (!ch->specials.poofin)
     ch->specials.poofin = (char *)malloc(len+1);
 }

 strcpy(buf, argument);
 dsearch(buf, ch->specials.poofin);
 SET_BIT(ch->specials.pmask, BIT_POOF_IN);
 send_to_char("Ok.\n\r", ch);
 free(argument);
 return;
}

void do_bamfout(struct char_data *ch, const char *arg, int cmd)
{
 char buf[255];
 char *argument;
 int len;

 for (; *arg == ' '; arg++);  /* pass all those spaces */

 if(!*arg) {
   send_to_char("Bamfout <bamf definition>\n\r", ch);
   send_to_char("  Additional arguments can include ~N for where you\n\r", ch);
   send_to_char("  Want your name (if you want your name). If you use\n\r",ch);
   send_to_char("  ~H, it puts in either his or her depending on your\n\r",ch);
   send_to_char("  sex.  If you use the keyword 'def' for your bamf,\n\r", ch);
   send_to_char("  it turns on the default bamf.  \n\r", ch);
   return;
 }

 if(!strcmp(arg, "def")) {
    REMOVE_BIT(ch->specials.pmask, BIT_POOF_OUT);
    free(ch->specials.poofout);
    ch->specials.poofout = 0;
    send_to_char("Ok.\n\r", ch);
    return;
  }

 len = strlen(arg);

 argument = strdup(arg);
 if(len > 150) {
   send_to_char("String too long.  Truncated to:\n\r", ch);
   argument[150] = '\0';
   sprintf(buf, "%s\n\r", argument);
   send_to_char(buf, ch);
   len = 150;
 }

 if (ch->specials.poofout && len >= (int) strlen(ch->specials.poofout)) {
   free(ch->specials.poofout);
   ch->specials.poofout = (char *)malloc(len+1);
 } else if (!ch->specials.poofout) {
   ch->specials.poofout = (char *)malloc(len+1);
 }


 strcpy(buf, argument);
 dsearch(buf, ch->specials.poofout);
 SET_BIT(ch->specials.pmask, BIT_POOF_OUT);
 send_to_char("Ok.\n\r", ch);
 free(argument);
 return;
}

void do_zsave(struct char_data *ch, const char *argument, int cmdnum)
{
  int start_room, end_room, zone;
  char c;
  FILE *fp;

 
  if( IS_NPC( ch ) )
    return;
 
  /*
   *   read in parameters (room #s)
   */
  zone=start_room=end_room = -1;
  sscanf(argument, "%d%c%d%c%d", &zone,&c, &start_room, &c, &end_room);
 
  if ((zone==-1)) {
    send_to_char("Zsave <zone_number> [<start_room> <end_room>]\n\r", ch);
    return;
  }

  if(zone>top_of_zone_table) {
    send_to_char("Invalid zone number\r\n",ch);
    return;
  }
 
/* make some permission checks */
  if( GetMaxLevel(ch)<56 && zone != GET_ZONE(ch) )  {
    send_to_char("Sorry, you are not authorized to save this zone.\n\r", ch);
    return;
  }

  if( !zone_table[zone].start) {
    send_to_char("Sorry, that zone isn't initialized yet\r\n", ch);
    return;
  }

  if( start_room == -1 || end_room == -1) {
    start_room=zone ? (zone_table[zone-1].top + 1) : 0;
    end_room = zone_table[zone].top;
  }

  fp = (FILE *)MakeZoneFile(ch,zone);
  if (!fp) {
    send_to_char("Couldn't make file.. try again later\n\r", ch);
    return;
  }
 
  fprintf(fp,"*Zone %d, rooms %d-%d, last modified by %s\n",
              zone,start_room,end_room,ch->player.name);
 
  SaveZoneFile(fp, start_room, end_room);
  fclose(fp);
 
  send_to_char("Ok\r\n",ch);
 
}
 
void do_zload(struct char_data *ch, const char *argument, int cmdnum)
{
  int zone;
  FILE *fp;
 
  if( IS_NPC( ch ) )
    return;
 
  /*
   *   read in parameters (room #s)
   */
  zone=-1;
  sscanf(argument, "%d", &zone);
 
  if (zone<1) {
    send_to_char("Zload <zone_number>\n\r", ch);
    return;
  }

  if(zone>top_of_zone_table) {
    send_to_char("Invalid zone number\r\n",ch);
    return;
  }
/* make some permission checks */
  if( GetMaxLevel(ch)<56 && zone != GET_ZONE(ch) )  {
    send_to_char("Sorry, you are not authorized to load this zone.\n\r", ch);
    return;
  }
 
  fp = (FILE *)OpenZoneFile(ch,zone);
 
  if (!fp) {
    send_to_char("Couldn't open zone file..\n\r", ch);
    return;
  }

/* oh well, do some things with that zone */ 
  CleanZone(zone);
  LoadZoneFile(fp, zone);
  fclose(fp);
  renum_zone_table(zone);
  zone_table[zone].start=0;
  reset_zone(zone);

  send_to_char("Ok\r\n",ch);
}

void do_zclean(struct char_data *ch, const char *argument, int cmdnum)
{
  int zone=-1;
  struct room_data      *rp;
 
  if( IS_NPC( ch ) )
    return;
 
  sscanf(argument, "%d", &zone);
 
  if (zone<1) {
    send_to_char("Zclean <zone_number> (and don't even think about cleaning Void)\n\r", ch);
    return;
  }
 
/* make some permission checks */
  rp = real_roomp(ch->in_room);
  if( GetMaxLevel(ch)<56 && rp->zone != GET_ZONE(ch) )  {
    send_to_char("Sorry, you are not authorized to clean this zone.\n\r", ch);
    return;
  }
 
  CleanZone(zone);

  send_to_char("4 3 2 1 0, Boom!\r\n",ch);
}

void do_highfive(struct char_data *ch, const char *argument, int cmd)
{
  char buf[80];
  char mess[120];
  struct char_data *tch;


  if (argument)
  {
    only_argument(argument, buf);
    if ((tch = get_char_room_vis(ch,buf)) != 0)
    {
      if( GetMaxLevel(tch) >= DEMIGOD && IS_PC( tch ) && 
          GetMaxLevel(ch)  >= DEMIGOD && IS_PC( ch ) )
      {
       sprintf(mess, "Time stops for a moment as %s and %s high five.\n\r",
            ch->player.name, tch->player.name);
       send_to_all(mess);
      }
      else
      {
       act("$n gives you a high five", TRUE, ch, 0, tch, TO_VICT);
       act("You give a hearty high five to $N", TRUE, ch, 0, tch, TO_CHAR);
       act("$n and $N do a high five.", TRUE, ch, 0, tch, TO_NOTVICT);
      }
    }
    else
    {
      sprintf(buf,"I don't see anyone here like that.\n\r");
      send_to_char(buf,ch);
    }
  }
}


void do_addhost(struct char_data *ch, const char *argument, int command)
{
}

void do_removehost(struct char_data *ch, const char *argument, int command)
{
}

void do_listhosts(struct char_data *ch, const char *argument, int command)
{
}

void do_silence(struct char_data *ch, const char *argument, int cmd)
{
  extern int Silence;
  if( GetMaxLevel(ch) < DEMIGOD || !IS_PC( ch ) )
  {
    send_to_char("You cannot Silence.\n\r",ch);
    return;
  }
  
  if (Silence == 0)
  {
    Silence = 1;
    send_to_char("You have now silenced polyed mobles.\n\r",ch);
    mudlog( LOG_PLAYERS, "%s has stopped Polymophed characters from shouting.",
            GET_NAME( ch ) );
  }
  else
  {
    Silence = 0;
    send_to_char("You have now unsilenced mobles.\n\r",ch);
    mudlog( LOG_PLAYERS, "%s has allowed Polymophed characters to shout.",
            GET_NAME( ch ) );
  }
}
void do_wizlock(struct char_data *ch, const char *argument, int cmd)
{
#if SITELOCK
  const char *test;
  int a, length, b;
  char buf[255];

  extern int numberhosts;
  extern char hostlist[MAX_BAN_HOSTS][30];
  
#endif
  extern int WizLock;


  if ((GetMaxLevel(ch) < DEMIGOD) || (IS_NPC(ch))) {
    send_to_char("You cannot WizLock.\n\r",ch);
    return;
  }


#if SITELOCK
  /* 4 commands */
  /* all,  add (place),  list,  rem  (place) */

   for(test = argument; *test && isspace(*test) ; test++);
   if (!*test) return;
   argument = test;

   /*
   **  get first piece..
   */

   argument = one_argument(argument, buf);
   if (!*buf) {
     send_to_char("Wizlock {all | off | add <host> | rem <host> | list}\n\r", ch);
     return;
   }

   if (str_cmp(buf, "all")==0) {

     if (WizLock) {
       send_to_char("Its already on!\n\r", ch);
     } else {
       send_to_char("WizLock is now on\n\r",ch);
       mudlog( LOG_PLAYERS, "WizLock is now on.");
       WizLock = TRUE;
     }

   } else if (str_cmp(buf, "off")==0) {

     if (!WizLock) {
       send_to_char("Its already off!\n\r", ch);
     } else {
       send_to_char("WizLock is now off\n\r",ch);
       mudlog( LOG_PLAYERS, "WizLock is now off.");
       WizLock = FALSE;
     }

   } else if (str_cmp(buf, "add")==0) {

     argument = one_argument(argument,buf);
     if (!*buf) {
       send_to_char("Wizlock add <host_name>\n\r", ch);
       return;
     }
     length = strlen(buf);
     if ((length <= 3) || (length >= 30)) {
         send_to_char("Host is too long or short, please try again\n\r", ch);
         return;
      } 
      for( a = 0 ; a <= numberhosts-1 ; a++) {
         if (strncmp(hostlist[a],buf,length) == 0) {
            send_to_char("Host is already in database\n\r", ch);
            return;
         }
      }
      strcpy(hostlist[numberhosts],buf);
      mudlog( LOG_PLAYERS, "%s has added host %s to the access denied list.",
              GET_NAME(ch),hostlist[numberhosts]);
      numberhosts++;
      return;

   } else if (str_cmp(buf, "rem")==0) {

      if (numberhosts <= 0) {
         send_to_char("Host list is empty.\n\r", ch);
         return;
      }

      for(test = argument; *test && isspace(*test) ; test++);
      if (!*test) {
        send_to_char("Wizlock rem <host_name>\n\r", ch);
        return;
      }
      argument = test;

      argument = one_argument(argument, buf);

      if (!*buf) {
        send_to_char("Wizlock rem <host_name>\n\r", ch);
        return;
      }

      length = strlen(buf);
      if ((length <= 3) || (length >= 30)) {
         send_to_char("Host length is bad, please try again\n\r", ch);
         return;
      }
 
      for( a = 0 ; a <= numberhosts-1 ; a++) {
         if (strncmp(hostlist[a],buf,length) == 0) {
            for( b = a ; b <= numberhosts ; b++) 
              strcpy(hostlist[b],hostlist[b+1]);
             mudlog( LOG_PLAYERS, 
                     "%s has removed host %s from the access denied list.",
                    GET_NAME(ch),hostlist[numberhosts]);
            numberhosts--;
            return;
         }
      }
      send_to_char("Host is not in database\n\r", ch);
      return;

   } else if (str_cmp(buf, "list")==0) {

      if (numberhosts <= 0) {
         send_to_char("Host list is empty.\n\r", ch);
         return;
      }
      for( a = 0 ; a <= numberhosts-1 ; a++) {
         sprintf(buf,"Host: %s\n",hostlist[a]);
         send_to_char(buf, ch);
      }
      return;

   } else {
     send_to_char("Wizlock {all | add <host> | rem <host> | list}\n\r", ch);
     return;
   }


#else
  if (WizLock) {
    send_to_char("WizLock is now off\n\r",ch);
    mudlog( LOG_PLAYERS, "Wizlock is now off.");
    WizLock = FALSE;
  } else {
    send_to_char("WizLock is now on\n\r",ch);
    mudlog( LOG_PLAYERS, "WizLock is now on.");
    WizLock = TRUE;
  }
#endif
  return;

}

void do_rload(struct char_data *ch, const char *argument, int cmd)
{

   char i;
   int start= -1, end = -2; 

   if (IS_NPC(ch))
     return;
   if (GetMaxLevel(ch) < IMMORTAL) return;

   for (i = 0; *(argument + i) == ' '; i++);
   if (!*(argument + i)) 
     start = end = ch->in_room;
   else
     sscanf(argument,"%d %d", &start, &end);

   if(start==-1) 
     return;      
   if(end==-2) 
     end=start;
   if(end<start) {
     send_to_char("Hey, end room must be >= start room\r\n", ch);
     return;
   }
   for (i=0;start>zone_table[i].top&&i<=top_of_zone_table;i++) 
     ;
   if (i > top_of_zone_table) {
     send_to_char("Strange, start room is outside of any zone.\r\n",ch);
     return;
   }
   if(end>zone_table[i].top) {
     send_to_char("Forget about it, end room is outside of start room zone ;)\r\n",ch);
     return;
   }

   RoomLoad(ch,start,end);
}


void do_rsave(struct char_data *ch, const char *argument, int cmd)
{
   long start= -1, end = -2, i; 

   if (IS_NPC(ch)) return;
   if (GetMaxLevel(ch) < IMMORTAL) return;

   for (i = 0; *(argument + i) == ' '; i++);
   if( !*(argument + i) )
     start = end = ch->in_room;
   else
     sscanf(argument,"%ld %ld", &start, &end);

   if(start==-1)
     return;
   if(end==-2)
     end=start;
   if(end<start) {
     send_to_char("Hey, end room must be >= start room\r\n", ch);
     return;
   }
   
   if (start>WORLD_SIZE || end>WORLD_SIZE) {
                send_to_char("Nope, range to large.\n\r",ch);
                return;
   }
   
   for (i=0;start>zone_table[i].top&&i<=top_of_zone_table;i++)
     ;
   if (i > top_of_zone_table) {
     send_to_char("Strange, start room is outside of any zone.\r\n",ch);
     return;
   }
   if(end>zone_table[i].top) {
     send_to_char("Forget about it, end room is outside of start room zone ;)\r\n",ch);
     return;
   }
      
   RoomSave(ch,start,end);
}


void do_emote( struct char_data *ch, const char *argument, int cmd )
{
  int i;
  char buf[ MAX_INPUT_LENGTH * 2 ];

  if( check_soundproof( ch ) ) 
    return;

  for( i = 0; *(argument + i) == ' '; i++ );

  if( !*( argument + i ) )
    send_to_char( "Si... ma cosa?\n\r", ch );
  else
  {
    if( IS_PC( ch ) && cmd != 177 )
      /* solo se il comando e emote o : se e` , non evidenzia. Cosi` e` 
       * possibile fare gli scherzi :) */
      sprintf( buf, "$c0015$n %.*s", MAX_INPUT_LENGTH, argument + i );
    else
      sprintf( buf, "$n %.*s", MAX_INPUT_LENGTH, argument + i );
    act( buf, FALSE, ch, 0, 0, TO_ROOM );
    if( IS_SET( ch->specials.act,PLR_ECHO ) )
      act( buf, FALSE, ch, 0, 0, TO_CHAR );
    else
      send_to_char("Ok.\n\r",ch);
  }
}



void do_echo(struct char_data *ch, const char *argument, int cmd)
{
  int i;
  char buf[MAX_INPUT_LENGTH];
  
  for (i = 0; *(argument + i) == ' '; i++);

  if( !*(argument + i) && !IS_NPC( ch ) )
  {
    if (IS_SET(ch->specials.act, PLR_ECHO)) 
    {
      send_to_char("echo off\n\r", ch);
      REMOVE_BIT(ch->specials.act, PLR_ECHO);
    } 
    else 
    {
      SET_BIT(ch->specials.act, PLR_ECHO);
      send_to_char("echo on\n\r", ch);
    }
  } 
  else        
  {
    if (IS_IMMORTAL(ch)) 
    {
      sprintf( buf,"%s\n\r", argument + i );
      send_to_room( buf, ch->in_room );
      /* send_to_char("Ok.\n\r", ch); */
    }
  }
}

void do_system(struct char_data *ch, const char *argument, int cmd)
{
  int i;
  char buf[256];
        
  for (i = 0; *(argument + i) == ' '; i++);

  if (!*(argument + i))
    send_to_char("That must be a mistake...\n\r", ch);
  else        
  {
    sprintf(buf,"\n\r%s\n\r", argument + i);
    send_to_all(buf);
  }
}



void do_trans(struct char_data *ch, const char *argument, int cmd)
{
  struct descriptor_data *i;
  struct char_data *victim;
  char buf[100];
  long target;


  only_argument(argument,buf);
  if (!*buf)
    send_to_char("Who do you wich to transfer?\n\r",ch);
  else if (str_cmp("all", buf))
  {
    if (!(victim = get_char_vis_world(ch,buf, NULL)))
      send_to_char("No-one by that name around.\n\r",ch);
    else
    {
      act( "$n disappears in a cloud of mushrooms.", FALSE, victim, 0, 0, 
           TO_ROOM);
      target = ch->in_room;
      char_from_room(victim);
      char_to_room(victim,target);
      act("$n arrives from a puff of smoke.", FALSE, victim, 0, 0, TO_ROOM);
      act("$n has transferred you!",FALSE,ch,0,victim,TO_VICT);
      if( IS_NPC( victim ) )
        victim->lStartRoom = 0;
      do_look(victim,"",15);
      send_to_char("Ok.\n\r",ch);
    }
  }
  else
  { 
    /* Trans All */
    for (i = descriptor_list; i; i = i->next)
    {
      if (i->character != ch && !i->connected)
      {
        victim = i->character;
        act( "$n disappears in a cloud of mushrooms.", FALSE, victim, 0, 0, 
            TO_ROOM);
        target = ch->in_room;
        char_from_room(victim);
        char_to_room(victim,target);
        act("$n arrives from a puff of smoke.", FALSE, victim, 0, 0, TO_ROOM);
        act("$n has transferred you!",FALSE,ch,0,victim,TO_VICT);
        if( IS_NPC( victim ) )
          victim->lStartRoom = 0;
        do_look(victim,"",15);
      }
    }

    send_to_char("Ok.\n\r",ch);
  }
}



void do_at(struct char_data *ch, const char *argument, int cmd)
{
  char command[MAX_INPUT_LENGTH], loc_str[MAX_INPUT_LENGTH];
  int loc_nr, location, original_loc;
  struct char_data *target_mob;
  struct obj_data *target_obj;
  
  half_chop(argument, loc_str, command);
  if (!*loc_str)
  {
    send_to_char("You must supply a room number or a name.\n\r", ch);
    return;
  }
  
  
  if (isdigit(*loc_str))
  {
    loc_nr = atoi(loc_str);
    if (NULL==real_roomp(loc_nr)) 
    {
      send_to_char("No room exists with that number.\n\r", ch);
      return;
    }
    location = loc_nr;
  }
  else if( ( target_mob = get_char_vis( ch, loc_str ) ) ) 
  {
    location = target_mob->in_room;
  }
  else if( ( target_obj = get_obj_vis_world( ch, loc_str, NULL ) ) )
  {
    if (target_obj->in_room != NOWHERE)
      location = target_obj->in_room;
    else
    {
      send_to_char("The object is not available.\n\r", ch);
      return;
    }
  }        
  else
  {
    send_to_char("No such creature or object around.\n\r", ch);
    return;
  }
  
  /* a location has been found. */
  
  original_loc = ch->in_room;
  char_from_room(ch);
  char_to_room(ch, location);
  command_interpreter(ch, command);
  
  /* check if the guy's still there */
  for (target_mob = real_roomp(location)->people; target_mob; target_mob =
       target_mob->next_in_room)
    if (ch == target_mob) {
      char_from_room(ch);
      char_to_room(ch, original_loc);
    }
}

void do_goto(struct char_data *ch, const char *argument, int cmd)
{
  char buf[MAX_INPUT_LENGTH];
  int loc_nr, location, i;
  struct char_data *target_mob, *pers, *v;
  struct obj_data *target_obj;
  
  void do_look(struct char_data *ch, const char *argument, int cmd);
  
  if ((GetMaxLevel(ch) >0 ) && (GetMaxLevel(ch) < LOW_IMMORTAL))
  {
    send_to_char("Huh?\n\r", ch);
    return;
  }

  only_argument(argument, buf);
  if (!*buf)
  {
    send_to_char("You must supply a room number or a name.\n\r", ch);
    return;
  }

  if (isdigit(*buf) && NULL==index(buf, '.'))
  {
    loc_nr = atoi(buf);
    if (NULL==real_roomp(loc_nr))
    {
      if (GetMaxLevel(ch)<CREATOR || loc_nr < 0)
      {
        send_to_char("No room exists with that number.\n\r", ch);
        return;
      }
      else
      {
#if HASH
#else
        if (loc_nr < WORLD_SIZE)
        {
#endif
          send_to_char("You form order out of chaos.\n\r", ch);
          CreateOneRoom(loc_nr);

#if HASH
#else
        }
        else
        {
          send_to_char("Sorry, that room # is too large.\n\r", ch);
          return;
        }
#endif
      }
    }
    location = loc_nr;
  }
  else if( ( target_mob = get_char_vis_world( ch, buf, NULL ) ) )
    location = target_mob->in_room;
  else if( ( target_obj = get_obj_vis_world( ch, buf, NULL ) ) )
  {
    if (target_obj->in_room != NOWHERE)
      location = target_obj->in_room;
    else
    {
      send_to_char("The object is not available.\n\r", ch);
      send_to_char("Try where #.object to nail its room number.\n\r", ch);
      return;
    }
  }
  else
  {
    send_to_char("No such creature or object around.\n\r", ch);
    return;
  }
  
  /* a location has been found. */
  
  
  if (!real_roomp(location))
  {
    mudlog( LOG_SYSERR, "Massive error in do_goto. Everyone Off NOW.");
    return;
  }
  
  if (IS_SET(real_roomp(location)->room_flags, PRIVATE))
  {
    for (i = 0, pers = real_roomp(location)->people; pers; pers =
         pers->next_in_room, i++);
    if (i > 1)
    {
      send_to_char( "There's a private conversation going on in that "
                    "room.\n\r", ch );
      return;
    }
  }
  
  if (IS_SET(ch->specials.act, PLR_STEALTH))
  {
    for (v = real_roomp(ch->in_room)->people; v; v= v->next_in_room)
    {
      if ( (ch != v) && (CAN_SEE(v,ch)) )
      {
        if (!IS_SET(ch->specials.pmask, BIT_POOF_OUT) || !ch->specials.poofout)
          act("$n disappears in a cloud of mushrooms.", 
              FALSE, ch, 0, v, TO_VICT);
        else
        {
          act(ch->specials.poofout, FALSE, ch, 0, v, TO_VICT);
        }
      }
    }
  }
  else
  {
    if( !IS_SET(ch->specials.pmask, BIT_POOF_OUT) || !ch->specials.poofout)
      act("$n disappears in a cloud of mushrooms.", 
          FALSE, ch, 0, 0, TO_ROOM);
    else
      if (*ch->specials.poofout != '!')
        act(ch->specials.poofout, FALSE, ch, 0, 0, TO_ROOM);
      else
        command_interpreter(ch, (ch->specials.poofout+1));
  }
  
  if (ch->specials.fighting)
    stop_fighting(ch);
  char_from_room(ch);
  char_to_room(ch, location);
  
  if( IS_SET( ch->specials.act, PLR_STEALTH ) )
  {
    for( v = real_roomp(ch->in_room)->people; v; v = v->next_in_room )
    {
      if( ch != v && CAN_SEE( v, ch ) )
      {

        if( !IS_SET( ch->specials.pmask, BIT_POOF_IN ) || 
            !ch->specials.poofin )
          act("$n appears with an explosion of rose-petals.", 
              FALSE, ch, NULL, v, TO_VICT );
        else
          act(ch->specials.poofin, FALSE, ch, 0, v, TO_VICT);
      }
    }
  }
  else
  {

    if( !IS_SET( ch->specials.pmask, BIT_POOF_IN ) || !ch->specials.poofin )
      act( "$n appears with an explosion of rose-petals.", 
           FALSE, ch, NULL, NULL, TO_ROOM );
    else if( *ch->specials.poofin != '!' )
      act( ch->specials.poofin, FALSE, ch, NULL, NULL, TO_ROOM );
    else
      command_interpreter( ch, ( ch->specials.poofin + 1 ) );
  }
  do_look(ch, "",15);
}





void do_stat(struct char_data *ch, const char *argument, int cmd)
{
  extern char *spells[];
  struct affected_type *aff;
  char arg1[MAX_STRING_LENGTH];
  char buf[MAX_STRING_LENGTH];
  char buf2[MAX_STRING_LENGTH];
  struct room_data *rm=0;
  struct char_data *k=0;
  struct obj_data  *j=0;
  struct obj_data  *j2=0;
  struct extra_descr_data *desc;
  struct follow_type *fol;
  int i, iVNum;
  int i2;
  int count = 1;
  bool found;
  
  /* for objects */
  extern char *item_types[];
  extern char *wear_bits[];
  extern char *extra_bits[];
  extern char *drinks[];
  
  /* for rooms */
  extern char *dirs[];

  extern char *exit_bits[];
  extern char *sector_types[];
  
  /* for chars */
  extern char *equipment_types[];
  extern char *affected_bits[];
  extern char *affected_bits2[];
  extern char *immunity_names[];
  extern char *special_user_flags[];
  extern char *apply_types[];
  extern char *pc_class_types[];
  extern char *npc_class_types[];
  extern char *action_bits[];
  extern char *player_bits[];
  extern char *position_types[];
  extern char *connected_types[];
  extern char *RaceName[];
  extern struct str_app_type str_app[];
  
  if( !IS_PC(ch) )
    return;
  
  only_argument(argument, arg1);
  
  /* no argument */
  if (!*arg1) 
  {
    send_to_char("Stats on who or what?\n\r",ch);
    return;
  }
  else 
  {
    /* stats on room */
    if (!str_cmp("room", arg1)) 
    {
      rm = real_roomp(ch->in_room);
      sprintf(buf, "Room name: %s, Of zone : %ld. V-Number : %ld, R-number : %ld\n\r",
              rm->name, rm->zone, rm->number, ch->in_room);
      send_to_char(buf, ch);
      
      sprinttype(rm->sector_type,sector_types,buf2);
      sprintf(buf, "Sector type : %s ", buf2);
      send_to_char(buf, ch);
      
      strcpy(buf,"Special procedure : ");
      strcat(buf,(rm->funct) ? "Exists\n\r" : "No\n\r");
      send_to_char(buf, ch);
      
      send_to_char("Room flags: ", ch);
      sprintbit((unsigned long) rm->room_flags,room_bits,buf);
      strcat(buf,"\n\r");
      send_to_char(buf,ch);
      
      send_to_char("Description:\n\r", ch);
      send_to_char(rm->description, ch);
      
      strcpy(buf, "Extra description keywords(s): ");
      if(rm->ex_description) 
      {
        strcat(buf, "\n\r");
        for (desc = rm->ex_description; desc; desc = desc->next) 
        {
          strcat(buf, desc->keyword);
          strcat(buf, "\n\r");
        }
        strcat(buf, "\n\r");
        send_to_char(buf, ch);
      }
      else 
      {
        strcat(buf, "None\n\r");
        send_to_char(buf, ch);
      }
      
      strcpy(buf, "------- Chars present -------\n\r");
      for (k = rm->people; k; k = k->next_in_room)
      {
        if (CAN_SEE(ch,k)) 
        {
          strcat(buf, GET_NAME(k));
          strcat( buf,(!IS_NPC(k) ? "(PC)\n\r" : (!IS_MOB(k) ? "(NPC)\n\r" : 
                                                               "(MOB)\n\r")));
        }
      }
      strcat(buf, "\n\r");
      send_to_char(buf, ch);
      
      strcpy(buf, "--------- Contents ---------\n\r");
      for (j = rm->contents; j; j = j->next_content)
      {
        strcat(buf, j->name);
        strcat(buf, "\n\r");
      }
      strcat(buf, "\n\r");
      send_to_char(buf, ch);
      
      send_to_char("------- Exits defined -------\n\r", ch);
      for (i = 0; i <= 5; i++) 
      {
        if (rm->dir_option[i]) 
        {
          if (rm->dir_option[i]->keyword) 
          {
            sprintf(buf,"Direction %s . Keyword : %s\n\r",
                    dirs[i], rm->dir_option[i]->keyword);
            send_to_char(buf, ch);
          } 
          else 
          {
            sprintf(buf,"Direction %s \n\r",   dirs[i]);
            send_to_char(buf, ch);
          }
          strcpy(buf, "Description:\n\r");
          if(rm->dir_option[i]->general_description)
            strcat(buf, rm->dir_option[i]->general_description);
          else
            strcat(buf,"UNDEFINED\n\r");
          send_to_char(buf, ch);
          sprintbit((unsigned) rm->dir_option[i]->exit_info,exit_bits,buf2);
          sprintf(buf, "Exit flag: %s \n\rKey no: %ld\n\rTo room (R-Number): %ld",
                  buf2, rm->dir_option[i]->key,
                  rm->dir_option[i]->to_room);
          send_to_char(buf, ch);
          sprintf(buf,"\r\n");
          if( rm->dir_option[i]->open_cmd != -1 )
            sprintf(buf," OpenCommand: %ld\r\n",rm->dir_option[i]->open_cmd);
          send_to_char(buf, ch);
          send_to_char( "---------------------------\r\n", ch );
        }
      }
      return;
    }
    /* mobile in world  PERSON! */
    else if( (k = get_char_vis_world( ch, arg1, &count ) ) )
    {
      struct time_info_data ma;
      
      switch(k->player.sex) 
      {
      case SEX_NEUTRAL : 
        strcpy(buf,"$c0015NEUTRAL-SEX"); 
        break;
      case SEX_MALE :
        strcpy(buf,"$c0015MALE");
        break;
      case SEX_FEMALE :
        strcpy(buf,"$c0015FEMALE");
        break;
      default : 
        strcpy(buf,"$c0015ILLEGAL-SEX!!");
        break;
      }
      
      sprintf( buf2, " $c0014%s $c0005- Name : $c0015%s "
                     "$c0005[R-Number $c0015%ld$c0005], "
                     "In room [$c0015%ld$c0005]",
               !IS_NPC(k) ? "PC" : ( !IS_MOB(k) ? "NPC" : "MOB" ),
              GET_NAME(k), k->nr, k->in_room);
      strcat(buf, buf2);
      act(buf,FALSE,ch,0,0,TO_CHAR);
      
      if (IS_MOB(k)) 
      {
        sprintf( buf, "$c0005V-Number [$c0015%d$c0005]", 
                 mob_index[k->nr].iVNum);
        act(buf,FALSE,ch,0,0,TO_CHAR);
      }
      
      strcpy(buf,"$c0005Short description: $c0015");
      strcat(buf, (k->player.short_descr ? k->player.short_descr : "None"));
      act(buf,FALSE,ch,0,0,TO_CHAR);
      
      strcpy(buf,"$c0005Title:$c0015 ");
      strcat(buf, (k->player.title ? k->player.title : "None"));
      act(buf,FALSE,ch,0,0,TO_CHAR);      

      sprintf( buf,"$c0005Long description: $c0015%s",
               k->player.long_descr ? k->player.long_descr : "None" );
      act(buf,FALSE,ch,0,0,TO_CHAR);      
      
      if( !IS_PC( k ) ) 
      {
        strcpy(buf,"$c0005Monster Class:$c0015 ");
        sprinttype(k->player.iClass,npc_class_types,buf2);
      }
      else
      {
        strcpy(buf,"$c0005Class:$c0015 ");
        sprintbit((unsigned)k->player.iClass,pc_class_types, buf2);
      }
      strcat(buf, buf2);
      act(buf,FALSE,ch,0,0,TO_CHAR);
      
      sprintf(buf,"$c0005Level [$c0014%d$c0005/$c0015%d$c0005/$c0014%d$c0005/"
                  "$c0015%d$c0005/$c0014%d$c0005/$c0015%d$c0005/$c0014%d$c0005"
                  "/$c0015%d$c0005/$c0014%d$c0005/$c0015%d$c0005/"
                  "$c0014%d$c0005] Alignment[$c0014%d$c0005]",
              k->player.level[0], k->player.level[1], 
              k->player.level[2], k->player.level[3], 
              k->player.level[4], k->player.level[5],
              k->player.level[6], k->player.level[7],
              k->player.level[8], k->player.level[9],
              k->player.level[10],GET_ALIGNMENT(k));
      act(buf,FALSE,ch,0,0,TO_CHAR);
      
      sprintf( buf,"$c0005Birth : [$c0014%ld$c0005]secs, "
                   "Logon[$c0014%ld$c0005]secs, Played[$c0014%d$c0005]secs", 
               k->player.time.birth,
               k->player.time.logon,
               k->player.time.played);

      act(buf,FALSE,ch,0,0,TO_CHAR);      

      age2(k, &ma);
      
      sprintf(buf, "$c0005Age: [$c0014%d$c0005] Y, [$c0014%d$c0005] M, "
                   "[$c0014%d$c0005] D, [$c0014%d$c0005] H. ",
              ma.year, ma.month, ma.day, ma.hours);

      act(buf,FALSE,ch,0,0,TO_CHAR);

      sprintf( buf,"$c0005Height [$c0014%d$c0005]cm, "
                   "Wgt [$c0014%d$c0005]chili NumAtks[$c0014%.1f$c0005]", 
               GET_HEIGHT(k), ( GET_WEIGHT(k) * 4536 ) / 10000,k->mult_att);

      act(buf,FALSE,ch,0,0,TO_CHAR);
      
      sprintf(buf,"$c0015Stats: $c0005Str:[$c0014%d$c0005/$c0015%d$c0005] "
                  "Int:[$c0014%d$c0005] Ws:[$c0014%d$c0005] "
                  "Dex:[$c0014%d$c0005] Con:[$c0014%d$c0005] "
                  "Ch:[$c0014%d$c0005]",
              GET_STR(k), GET_ADD(k),
              GET_INT(k),
              GET_WIS(k),
              GET_DEX(k),
              GET_CON(k),
              GET_CHR(k));

      act(buf,FALSE,ch,0,0,TO_CHAR);
      
      sprintf(buf,"$c0005Mana:[$c0014%d$c0005/$c0015%d$c0005+$c0011%d$c0005] "
                  "Hit:[$c0014%d$c0005/$c0015%d$c0005+$c0011%d$c0005] "
                  "Move:[$c0014%d$c0005/$c0015%d$c0005+$c0011%d$c0005]",
              GET_MANA(k), mana_limit(k), mana_gain(k),
              GET_HIT(k), hit_limit(k), HitRegain(k),
              GET_MOVE(k), move_limit(k), move_gain(k) );

      act(buf,FALSE,ch,0,0,TO_CHAR);
      
      sprintf(buf,"$c0005AC:[$c0014%d$c0005/$c001510$c0005], "
                  "Coins: [$c0014%d$c0005], Exp: [$c0014%d$c0005]\n\r"
                  "$c0005Hitroll: [$c0014%d$c0005+($c0015%d$c0005)], "
                  "Damroll: [$c0014%d$c0005+($c0015%d$c0005)] "
                  "Spellfail: [$c0014%d$c0005]",
              GET_AC(k),
              GET_GOLD(k),
              GET_EXP(k),
              k->points.hitroll,str_app[STRENGTH_APPLY_INDEX(k)].tohit,
              k->points.damroll,str_app[STRENGTH_APPLY_INDEX(k)].todam,
              k->specials.spellfail);

      act(buf,FALSE,ch,0,0,TO_CHAR);
      
      sprinttype(GET_POS(k),position_types,buf2);
      sprintf( buf,"$c0005Position: $c0014%s$c0005, ",buf2 );
      strcat( buf, "Default position:$c0014" );
      sprinttype((k->specials.default_pos),position_types,buf2);
      strcat(buf, buf2);
      strcat( buf, "$c0005, Fighting: $c0014" );
      strcat( buf, ((k->specials.fighting) ? GET_NAME(k->specials.fighting) :
                    "Nobody") );
      act(buf,FALSE,ch,0,0,TO_CHAR);
      
      if (k->desc) 
      {
        sprinttype(k->desc->connected,connected_types,buf2);
        sprintf( buf,"$c0005Connected: $c0014%s", buf2 );
        act(buf,FALSE,ch,0,0,TO_CHAR);
      }
      
      sprintf(buf,"$c0005Timer [$c0014%d$c0005]", k->specials.timer);
      act(buf,FALSE,ch,0,0,TO_CHAR);
      
      if (IS_NPC(k))         
      {
        strcpy(buf,"$c0005NPC flags:$c0014 ");
        sprintbit(k->specials.act,action_bits,buf2);
      }
      else
      {
        strcpy(buf,"$c0005PC flags:$c0014 ");
        sprintbit(k->specials.act,player_bits,buf2);
      }
      
      strcat(buf, buf2);
      act(buf,FALSE,ch,0,0,TO_CHAR);
      
      if (IS_MOB(k)) 
      {
        strcpy(buf, "$c0005Mobile Special procedure :$c0014 ");
        strcat(buf, (mob_index[k->nr].func ? "Exists " : "None "));

        act(buf,FALSE,ch,0,0,TO_CHAR);
      }
      
      if (IS_NPC(k)) 
      {
        sprintf(buf, "$c0005NPC Bare Hand Damage $c0014%dd$c0015%d$c0005.",
                k->specials.damnodice, k->specials.damsizedice);

        act(buf,FALSE,ch,0,0,TO_CHAR);
      }
      
      sprintf(buf,"$c0005Carried weight: $c0014%d$c0005 etti  "
                  "Carried items: $c0014%d ",
              ( IS_CARRYING_W(k) * 4536 ) / 1000,
              IS_CARRYING_N(k) );

      for(i=0,i2=0;i<MAX_WEAR;i++)
        if (k->equipment[i]) i2++;
      sprintf(buf2,"$c0005Items in equipment: $c0014%d", i2);
      strcat(buf,buf2);

      act(buf,FALSE,ch,0,0,TO_CHAR);
      
      sprintf(buf,"$c0005Apply saving throws: [$c0014%d$c0005] "
                  "[$c0014%d$c0005] [$c0014%d$c0005] [$c0014%d$c0005] "
                  "[$c0014%d$c0005] ",
              k->specials.apply_saving_throw[0],
              k->specials.apply_saving_throw[1],
              k->specials.apply_saving_throw[2],
              k->specials.apply_saving_throw[3],
              k->specials.apply_saving_throw[4]);

      act(buf,FALSE,ch,0,0,TO_CHAR);
      
      sprintf(buf, "$c0005Thirst: $c0014%d$c0005, Hunger: $c0014%d$c0005, "
                   "Drunk: $c0014%d",
              k->specials.conditions[THIRST],
              k->specials.conditions[FULL],
              k->specials.conditions[DRUNK]);

      act(buf,FALSE,ch,0,0,TO_CHAR);
      
      sprintf(buf, "$c0005Master is '$c0014%s$c0005'    ",
              ((k->master) ? GET_NAME(k->master) : "NOBODY"));

      act(buf,FALSE,ch,0,0,TO_CHAR);

      sprintf(buf, "$c0005Followers are:"); 
      act(buf, FALSE, ch,0,0,TO_CHAR);
      for( fol = k->followers; fol; fol = fol->next ) 
      {
        if( fol->follower != NULL )
        {
          sprintf(buf, "$c0014    %s", GET_NAME( fol->follower ) );
          act(buf, FALSE, ch, 0, 0, TO_CHAR);
        }
        else
        {
          mudlog( LOG_ERROR, "fol->follower == NULL in do_stat. Player %s.",
                  GET_NAME( k ) );
        }
      }
      
      /* immunities */
      if (k->M_immune) 
      {
        send_to_char("$c0005Immune to:$c0014",ch); 
        sprintbit(k->M_immune, immunity_names, buf);
        strcat(buf,"\n\r");
        send_to_char(buf,ch);
      }

      /* resistances */
      if (k->immune) 
      {
        send_to_char("$c0005Resistant to:$c0014",ch); 
        sprintbit(k->immune, immunity_names, buf);
        strcat(buf,"\n\r");
        send_to_char(buf,ch);
      }      
     
      /* Susceptible */
      if (k->susc) 
      {
        send_to_char("$c0005Susceptible to:$c0014",ch); 
        sprintbit(k->susc, immunity_names, buf);
        strcat(buf,"\n\r");
        send_to_char(buf,ch);
      }      


      if (k->player.user_flags) 
      {
           send_to_char("$c0005SPECIAL FLAGS:$c0014",ch);
        sprintbit(k->player.user_flags,special_user_flags,buf);
        strcat(buf,"\n\r");
        send_to_char(buf,ch); 
      }

      /*  race, action pointer */
      send_to_char("$c0005Race: $c0014",ch);
      sprinttype((k->race),RaceName,buf2);
      send_to_char(buf2, ch);
      sprintf(buf, "$c0005  Generic value: $c0014%d", k->generic );
      send_to_char(buf, ch);
      sprintf(buf, "$c0005  Action pointer: $c0014%p\n\r", k->act_ptr );
      send_to_char(buf, ch);
      if( IS_NPC( k ) )
      {
        sprintf(buf, "$c0005Start room: $c0014%ld\n\r", k->lStartRoom );
        send_to_char(buf, ch);
      }
      
      /* Showing the bitvector */
      if (k->specials.affected_by) 
      {
        sprintbit((unsigned)k->specials.affected_by,affected_bits,buf);
        send_to_char("$c0005Affected by: $c0014", ch);
        strcat(buf,"\n\r");
        send_to_char(buf, ch);
      }
      
      if (k->specials.affected_by2) 
      {
        sprintbit((unsigned)k->specials.affected_by2,affected_bits2,buf);
        send_to_char("$c0005Affected by2: $c0014", ch);
        strcat(buf,"\n\r");
        send_to_char(buf, ch);      
      }
      
      /* Routine to show what spells a char is affected by */
      if (k->affected) 
      {
        sprintf(buf, "\n\r$c0005Affecting Spells:\n\r$c0015--------------");
        act(buf, FALSE, ch,0,0,TO_CHAR);
        for(aff = k->affected; aff; aff = aff->next) 
        {
          if (aff->type <=MAX_EXIST_SPELL) 
          {
            sprintf( buf, "$c0005Spell : '$c0014%s$c0005'", 
                     spells[aff->type-1]);
            act(buf,FALSE, ch,0,0,TO_CHAR);
          
            sprintf( buf,"     $c0005Modifies $c0014%s $c0005by "
                         "$c0015%ld$c0005 points",
                     apply_types[aff->location], aff->modifier);
            act(buf,FALSE, ch,0,0,TO_CHAR);
          
            sprintf(buf,"     Expires in %3d hours, Bits set ",
                    aff->duration);
            send_to_char(buf,ch);
            if (aff->location != APPLY_BV2)
              sprintbit((unsigned)aff->bitvector,affected_bits,buf);
            else 
              sprintbit((unsigned)aff->bitvector,affected_bits2,buf);
            strcat(buf,"\n\r");
            send_to_char(buf, ch);
          }
          else 
          { 
            /* max spl */
            mudlog( LOG_ERROR,"<%s> had a bogus aff->type act.wizard, do_stat",
                    GET_NAME(k));
            /* log bogus aff->type */
          }
        } /* for aff */
      }
      return;
    }
    /* stat on object */
    else if( ( j = (struct obj_data *)get_obj_vis_world( ch, arg1, &count ) ) )
    {
      iVNum = (j->item_number >= 0) ? obj_index[j->item_number].iVNum : 0;
      sprintf(buf, 
              "Object name: [%s], R-number: [%d], V-number: [%d] Item type: ",
              j->name, j->item_number, iVNum);
      sprinttype(GET_ITEM_TYPE(j),item_types,buf2);
      strcat(buf,buf2); strcat(buf,"\n\r");
      send_to_char(buf, ch);
      sprintf(buf, "Short description: %s\n\rLong description:\n\r%s\n\r",
              ((j->short_description) ? j->short_description : "None"),
              ((j->description) ? j->description : "None") );
      send_to_char(buf, ch);
      if(j->ex_description)
      {
        strcpy(buf, "Extra description keyword(s):\n\r----------\n\r");
        for (desc = j->ex_description; desc; desc = desc->next) 
        {
          strcat(buf, desc->keyword);
          strcat(buf, "\n\r");
        }
        strcat(buf, "----------\n\r");
        send_to_char(buf, ch);
      }
      else 
      {
        strcpy(buf,"Extra description keyword(s): None\n\r");
        send_to_char(buf, ch);
      }
      
      send_to_char("Can be worn on :", ch);
      sprintbit((unsigned)j->obj_flags.wear_flags,wear_bits,buf);
      strcat(buf,"\n\r");
      send_to_char(buf, ch);
      
      send_to_char("Set char bits  :", ch);
      sprintbit((unsigned)j->obj_flags.bitvector,affected_bits,buf);
      strcat(buf,"\n\r");
      send_to_char(buf, ch);
      
      send_to_char("Extra flags: ", ch);
      sprintbit((unsigned)j->obj_flags.extra_flags,extra_bits,buf);
      strcat(buf,"\n\r");
      send_to_char(buf,ch);
      
      sprintf(buf,"Weight: %d, Value: %d, Cost/day: %d, Timer: %d\n\r",
              j->obj_flags.weight,j->obj_flags.cost,
              j->obj_flags.cost_per_day,  j->obj_flags.timer);
      send_to_char(buf, ch);
      
      strcpy(buf,"In room: ");
      if (j->in_room == NOWHERE)
        strcat(buf,"Nowhere");
      else
      {
        sprintf(buf2,"%d",j->in_room);
        strcat(buf,buf2);
      }
      strcat(buf," ,In object: ");
      strcat(buf, (!j->in_obj ? "None" : fname(j->in_obj->name)));
      
      switch (j->obj_flags.type_flag) 
      {
       case ITEM_LIGHT : 
        sprintf(buf, "Colour : [%d]\n\rType : [%d]\n\rHours : [%d]",
                j->obj_flags.value[0],
                j->obj_flags.value[1],
                j->obj_flags.value[2]);
        break;
       case ITEM_SCROLL : 
        sprintf(buf, "Spells : %d, %d, %d, %d",
                j->obj_flags.value[0],
                j->obj_flags.value[1],
                j->obj_flags.value[2],
                j->obj_flags.value[3] );
        break;
       case ITEM_WAND :
       case ITEM_STAFF:
        sprintf(buf, "Level: %d Spell : %d\n\rCharges : %d",
                j->obj_flags.value[0],
                j->obj_flags.value[3],
                j->obj_flags.value[2]);
        break;
       case ITEM_WEAPON :
        sprintf( buf, "Reserved: %d\n\r", j->obj_flags.value[ 0 ] );
#if 0
        sprintbit( j->obj_flags.value[0], aszWeaponSpecialEffect, buf2 );
        strcat( buf, buf2 );
#endif
        sprintf( buf2, "Todam: %dD%d\n\rDamage type: ",
                 j->obj_flags.value[1],
                 j->obj_flags.value[2] );
        strcat( buf, buf2 );
        sprinttype( j->obj_flags.value[3], aszWeaponType, buf2 );
        strcat( buf, buf2 );
        strcat( buf, "\n\r" );
        break;
       case ITEM_FIREWEAPON : 
        sprintf(buf, "Min Strength: %d\n\rMax range: %d\n\r"
                     "Bonus range: %d\n\rType: %d",
                j->obj_flags.value[0],
                j->obj_flags.value[1],
                j->obj_flags.value[2],
                j->obj_flags.value[3] );
        break;
       case ITEM_MISSILE : 
        sprintf(buf, "%% to break: %d\n\rTodam: %dD%d\n\rType : %d",
                j->obj_flags.value[0],
                j->obj_flags.value[1],
                j->obj_flags.value[2],
                j->obj_flags.value[3] );
        break;
       case ITEM_ARMOR :
        sprintf(buf, "AC-apply : [%d]\n\rFull Strength : [%d]",
                j->obj_flags.value[0],
                j->obj_flags.value[1]);
        
        break;
       case ITEM_POTION : 
        sprintf(buf, "Spells : %d, %d, %d, %d",
                j->obj_flags.value[0],
                j->obj_flags.value[1],
                j->obj_flags.value[2],
                j->obj_flags.value[3]); 
        break;
       case ITEM_TRAP :
        sprintf(buf, "Eff type: %d, Dam type: %d, level: %d, charges: %d", 
                j->obj_flags.value[0],
                j->obj_flags.value[1],
                j->obj_flags.value[2],
                j->obj_flags.value[3]);
        break;
       case ITEM_CONTAINER :
        sprintf(buf, "Max-contains : %d\n\rLocktype : %d\n\rCorpse : %s",
                j->obj_flags.value[0],
                j->obj_flags.value[1],
                j->obj_flags.value[3]?"Yes":"No");
        break;
       case ITEM_DRINKCON :
        sprinttype(j->obj_flags.value[2],drinks,buf2);
        sprintf(buf, "Max-contains : %d\n\rContains : %d\n\rPoisoned : %s"
                     "\n\rLiquid : %s",
                j->obj_flags.value[0],
                j->obj_flags.value[1],
                j->obj_flags.value[3]?"Yes":"No",
                buf2);
        break;
       case ITEM_NOTE :
        sprintf(buf, "Tounge : %d",
                j->obj_flags.value[0]);
        break;
       case ITEM_KEY :
        sprintf(buf, "Keytype : %d",
                j->obj_flags.value[0]);
        break;
       case ITEM_FOOD :
        sprintf(buf, "Makes full : %d\n\rPoisoned : %s",
                j->obj_flags.value[0],
                j->obj_flags.value[3]?"Yes":"No");
        break;
       default :
        sprintf(buf,"Values 0-3 : [%d] [%d] [%d] [%d]",
                j->obj_flags.value[0],
                j->obj_flags.value[1],
                j->obj_flags.value[2],
                j->obj_flags.value[3]);
        break;
      }
      send_to_char(buf, ch);
      
      strcpy(buf,"\n\rEquipment Status: ");
      if (!j->carried_by)
        strcat(buf,"NONE");
      else 
      {
        found = FALSE;
        for (i=0;i < MAX_WEAR;i++) {
          if (j->carried_by->equipment[i] == j) {
            sprinttype(i,equipment_types,buf2);
            strcat(buf,buf2);
            found = TRUE;
          }
        }
        if (!found)
          strcat(buf,"Inventory of ");
        else
          strcat( buf, " by " );
        strcat( buf, GET_NAME_DESC( j->carried_by ) );
      }
      send_to_char(buf, ch);
      
      strcpy(buf, "\n\rSpecial procedure : ");
      if (j->item_number >= 0)
        strcat(buf, (obj_index[j->item_number].func ? "exists\n\r" : "No\n\r"));
      else
        strcat(buf, "No\n\r");
      send_to_char(buf, ch);
    
      if( j->contains )
      {  
        strcpy(buf, "Contains :\n\r");
        found = FALSE;
        for(j2=j->contains;j2;j2 = j2->next_content) 
        {
          strcat(buf,fname(j2->name));
          strcat(buf,"\n\r");
          found = TRUE;
        }
        if (!found)
          strcpy(buf,"Contains : Nothing\n\r");
        send_to_char(buf, ch);
      }
      
      send_to_char("Can affect char :\n\r", ch);
      for (i=0;i<MAX_OBJ_AFFECT;i++) 
      {
        if( j->affected[ i ].location )
        {
          sprinttype( j->affected[ i ].location, apply_types, buf2 );
          sprintf( buf, "    Affects : %s By %ld\n\r", 
                   buf2, j->affected[ i ].modifier );
          send_to_char( buf, ch );
        }
      }                        
      return;
    }
    else 
    {
      send_to_char("No mobile or object by that name in the world\n\r", ch);
    }
  }
}

void do_ooedit(struct char_data *ch, const char *argument, int cmd)
{
  char item[80], field[20],parmstr[MAX_STRING_LENGTH];
  char parmstr2[MAX_STRING_LENGTH];

  struct obj_data  *j=0;
  int iVNum;
  
  /* for objects 
  extern char *item_types[];
  extern char *wear_bits[];
  extern char *extra_bits[];
  extern char *drinks[];
  */

  if (IS_NPC(ch))
    return;
 
  argument = one_argument(argument, item);

  if (!*item)
  {
    send_to_char("Oedit what?!? (oedit <item> <field> <num>) Use 'oedit help'.\n\r",ch);
    return;
  }
     

  if (!strcmp(item,"help"))
  {
    send_to_char("Help for Oedit.\n\r"
                 "Command line Parameters OEDIT <NAME> <FIELD> <VALUE>\n\r"
                 "List of Fields :\n\r"
                 "ldesc  = Long Item description | sdesc  = Short description\n\r"
                 "extra  = Extra descriptions*NI*| name   = Item name\n\r"
                 "wflags = wear flags            | afflags= affect flags\n\r"
                 "exflags= extra flags           | weight = item weight\n\r"
                 "cost   = item cost to rent per day\n\r"
                 "value  = Item value if sold    | timer  = item timer\n\r"
                 "type   = item type\n\r"
                 "v0     = value[0] of item      | v1     = value[1] of item\n\r"
                 "v2     = value[2] of item      | v3     = value[3] of item\n\r"
                 "aff1   = special affect 1 (syntax is: oedit aff1 <modifer> <type>)\n\r"
                 "aff2   = special affect 2      | aff3   = special affect 3\n\r"
                 "aff4   = special affect 4      | aff5   = special affect 5\n\r"
                 "\n\rNote: NI = Not implemented.\n\r",ch);
    return;
  } /* End Help! */

  argument = one_argument(argument, field);


  if (!*field)
  {
    send_to_char("Oedit what?!? I need a FIELD! (oedit <item> <field> <num>)\n\r",ch);
    return;
  }
 

  if (!*argument)
  {
    send_to_char("Oedit what?!? I need a <num/change>!(oedit <item> <field> <num>)\n\r",ch);
    return;
  }

  /* object */
    
  if( ( j = (struct obj_data *)get_obj_in_list_vis( ch, item, ch->carrying ) ) ) 
  {
    iVNum = (j->item_number >= 0) ? obj_index[j->item_number].iVNum : 0;

    if (!strcmp(field,"name")) 
    {
      free(j->name);
      for (; isspace(*argument); argument++);
      strcpy(parmstr,argument);
      j->name=strdup(parmstr);
      return;         
    } /* end name */

    if (!strcmp(field,"ldesc")) 
    {
      free(j->description);
      for (; isspace(*argument); argument++);
      strcpy(parmstr,argument);
      j->description = strdup(parmstr);
      return;
    } /* end ldesc */

    if (!strcmp(field,"sdesc")) 
    {
      free(j->short_description);
      for (; isspace(*argument); argument++);
      strcpy(parmstr,argument);
      j->short_description=strdup(parmstr);
      return;         
    } /* end sdesc */

    if (!strcmp(field,"extra"))
    {
      send_to_char("Not able modify extra descriptions (yet).\n\r",ch);
      return;
      /*  j->ex_description */
    } /* end extra desc */

    if (!strcmp(field,"wflags"))        
    {
      argument = one_argument(argument,parmstr);
      j->obj_flags.wear_flags=atol(parmstr);
      return;
    } /* end wear flags */
        

    if (!strcmp(field,"afflags"))        
    {
      argument = one_argument(argument,parmstr);
      j->obj_flags.bitvector=atol(parmstr);
      return;
    } /* end aff flags */
        

    if (!strcmp(field,"exflags"))      
    {
      argument = one_argument(argument,parmstr);
      j->obj_flags.extra_flags=atol(parmstr);
      return;
    } /* end exflags */
        

    if (!strcmp(field,"weight"))
    {
      argument = one_argument(argument,parmstr);
      j->obj_flags.weight=atol(parmstr);
      return;
    } /* end weight */
        
    if (!strcmp(field,"cost"))
    {
      argument = one_argument(argument,parmstr);
      j->obj_flags.cost_per_day=atol(parmstr);
      return;
    } /* end item rent cost */
        
    if (!strcmp(field,"value"))
    {
      argument = one_argument(argument,parmstr);
      j->obj_flags.cost=atol(parmstr);
      return;
    } /* end value of item */

    if (!strcmp(field,"timer"))        
    {
      argument = one_argument(argument,parmstr);
      j->obj_flags.timer=atol(parmstr);
      return;
    } /* end timer */

    if (!strcmp(field,"type"))        
    {
      argument = one_argument(argument,parmstr);
      j->obj_flags.type_flag=atol(parmstr);
      return;
    } /* end type */

    if (!strcmp(field,"v0"))        
    {
      argument = one_argument(argument,parmstr);
      j->obj_flags.value[0]=atol(parmstr);
      return;
    } /* end v0 */
    
    if (!strcmp(field,"v1"))        
    {
      argument = one_argument(argument,parmstr);
      j->obj_flags.value[1]=atol(parmstr);
      return;
    } /* end v1 */

    if (!strcmp(field,"v2"))        
    {
      argument = one_argument(argument,parmstr);
      j->obj_flags.value[2]=atol(parmstr);
      return;
    } /* end v2 */

    if (!strcmp(field,"v3"))        
    {
      argument = one_argument(argument,parmstr);
      j->obj_flags.value[3]=atol(parmstr);
      return;
    } /* end v3 */

    if (!strcmp(field,"aff1"))
    {
      argument  = one_argument(argument,parmstr);
      argument  = one_argument(argument,parmstr2);
      j->affected[0].location=atol(parmstr2);
      j->affected[0].modifier=atol(parmstr);
      return;
    } /* end aff1 */

    if (!strcmp(field,"aff2"))
    {
      argument  = one_argument(argument,parmstr);        
      argument  = one_argument(argument,parmstr2);
      j->affected[1].location=atol(parmstr2);
      j->affected[1].modifier=atol(parmstr);
      return;
    }

    if (!strcmp(field,"aff3"))
    {
      argument  = one_argument(argument,parmstr);        
      argument  = one_argument(argument,parmstr2);
      j->affected[2].location=atol(parmstr2);
      j->affected[2].modifier=atol(parmstr);
      return;
    }

    if (!strcmp(field,"aff4"))
    {
      argument  = one_argument(argument,parmstr);
      argument  = one_argument(argument,parmstr2);
      j->affected[3].location=atol(parmstr2);
      j->affected[3].modifier=atol(parmstr);
      return;
    }

    if (!strcmp(field,"aff5"))
    {
      argument  = one_argument(argument,parmstr);
      argument  = one_argument(argument,parmstr2);
      j->affected[4].location=atol(parmstr2);
      j->affected[4].modifier=atol(parmstr);
      return;
    }         /* end aff5 */

  } 
  else
    send_to_char("You do not have that object.\n\r", ch);

} /* end of object edit */

void do_resetskills( struct char_data *ch, const char *argument, int cmd )
{
  char buf[ 256 ];
  struct char_data *mob;

  if( GetMaxLevel( ch ) < SILLYLORD || IS_NPC( ch ) || cmd != CMD_RESETSKILLS )
    return;

  argument = one_argument( argument, buf );

  if( ( mob = get_char_vis( ch, buf ) ) == NULL )
    send_to_char( "Non c'e` nessuno con quel nome qui.\n\r", ch );
  else if( mob->skills == NULL )
    send_to_char( "Il giocatore non ha skills.\n\r", ch );
  else
  { 
    int i;
    for( i = 0; i < MAX_SKILLS; i++ )
    {  
      if( GetMaxLevel( mob ) < IMPLEMENTOR )
      {
        mob->skills[ i ].learned = 0;
        mob->skills[ i ].flags   = 0;
        mob->skills[ i ].special = 0;
        mob->skills[ i ].nummem  = 0;
      }
      else 
      {
        mob->skills[ i ].learned = 100;
        mob->skills[ i ].flags   = SKILL_KNOWN;
        mob->skills[ i ].special = 1;
        mob->skills[ i ].nummem  = 0;
      }
    }
    send_to_char( "Fatto.\n\r", ch );
  }
}

void do_showskills( struct char_data *ch, const char *argument, int cmd )
{
  char buf[ 256 ];
  struct char_data *mob;
  struct string_block sb;

  if( GetMaxLevel( ch ) < SILLYLORD || IS_NPC( ch ) || cmd != CMD_SHOWSKILLS )
    return;

  argument = one_argument( argument, buf );

  if( ( mob = get_char_vis( ch, buf ) ) == NULL )
    send_to_char( "Non c'e` nessuno con quel nome qui.\n\r", ch );
  else if( mob->skills == NULL )
    send_to_char( "Il giocatore non ha skills.\n\r", ch );
  else
  { 
    int i;
    init_string_block( &sb );
    for( i = 0; i < MAX_EXIST_SPELL; i++ )
    {  
      if( spells[ i ] && *spells[i] != '\n' )
      {
        sprintf( buf, "[%3d] %-30s %3d %-14s %d", i + 1, spells[ i ],
                 mob->skills[ i + 1 ].learned, 
                 how_good( mob->skills[ i + 1 ].learned ),
                 mob->skills[ i + 1 ].flags );
        if( IsSpecialized( mob->skills[ i + 1 ].special ) )
          strcat( buf," (special)" );
        strcat( buf,"\n\r" );
        append_to_string_block( &sb, buf );
      }
    }
    append_to_string_block( &sb, "\n\r" );
    page_string_block( &sb, ch );
    destroy_string_block( &sb );
  }
}

void do_set(struct char_data *ch, const char *argument, int cmd)
{
  char field[100], name[100], parmstr[100];
  struct char_data *mob;
  int parm = 0;
  int parm2 = 0;
  char buf[256];
  unsigned long lparm = 0;


  extern char PeacefulWorks;
  extern char EasySummon;


  if ((GetMaxLevel(ch) < SILLYLORD) || (IS_NPC(ch)))
    return;

  argument = one_argument(argument, field);
  argument = one_argument(argument, name);
  only_argument( argument, parmstr );

  if ((mob = get_char_vis(ch, name)) == NULL) 
  { 
send_to_char(
"@\n\r"
"Usage :@ <field> <user name> <value>\n\r"
"\n\r"
"This is a Implementor command and should be used with care as it can\n\r"
"change any ability/skill/attr of a character. Here is a list of fields,\n\r"
"the value types will differ with each (i.e. number/alpha char)\n\r"
"\n\r"
"aling class exp expadd lev sex race hunger thirst one hit mhit tohit todam\n\r"
"ac bank gold age prac str add saves skills stadd int wis dex con chr pkill\n\r"
"mana start murder stole known specskill zone nodelete specflags pkill\n\r"
"numatks objedit mobedit remaffect gmove height weight position startroom\n\r\n\r"
"Remember, be careful how you use this command!\n\r",ch);
    return;
  }
  
  if( !strcmp( field, "align" ) ) 
  {
    sscanf( parmstr,"%d",&parm );
    GET_ALIGNMENT( mob ) = parm;      
  }
  else if( !strcmp( field, "class" ) )
  {
    sscanf( parmstr, "%d", &parm );
    /*
    ** this will do almost nothing. (hopefully);
    */
    mob->player.iClass = parm;
  } 
  else if (!strcmp(field,"exp")) 
  {
    sscanf( parmstr, "%d", &parm );
    GET_EXP(mob) = parm;
  }
  else if( !strcmp( field, "expadd" ) )
  {
    sscanf( parmstr, "%d", &parm );
    GET_EXP(mob) += parm;
  }
  else if (!strcmp(field,"specflags") )
  {
    sscanf( parmstr, "%d", &parm );
    GET_SPECFLAGS(mob) = parm;
    send_to_char("Changed flags.\n\r",ch);
  }
  else if (!strcmp(field,"zone"))
  {
    sscanf( parmstr, "%d", &parm );
    GET_ZONE(mob) = parm;
    sprintf(buf,"Setting zone access to %d.\n\r",parm);
    send_to_char(buf,ch);
  }
  else if (!strcmp(field,"pkill"))
  {
    if( IS_PC( mob ) )
    {
      if( IS_SET( mob->player.user_flags,RACE_WAR ) )
      {
        REMOVE_BIT(mob->player.user_flags,RACE_WAR);
        sprintf(buf,"Removed PLAYERS KILLING from %s\n\r",GET_NAME(mob));
      }
      else
      {
        SET_BIT( mob->player.user_flags, RACE_WAR );
        sprintf(buf,"Set PLAYERS KILLING to %s\n\r",GET_NAME(mob));
      }
    }
    else
    {
      sprintf( buf, "%s non e` un giocatore.", GET_NAME_DESC( mob ) );
    }
    
    send_to_char(buf,ch);
  }
  else if( !strcmp( field, "aff1" ) )
  {
    if( sscanf( parmstr, "%ld", &lparm ) < 1 )
    {
      sprintf( buf, "Affected_by: %ld\n\r", mob->specials.affected_by );
      send_to_char( buf, ch );
    }
    else
    {
      mob->specials.affected_by = lparm;
      sprintf( buf, "Affected_by changed\n\r" );
      send_to_char( buf, ch );
    }
  }
  else if( !strcmp( field, "aff2" ) )
  {
    if( sscanf( parmstr, "%ld", &lparm ) < 1 )
    {
      sprintf( buf, "Affected_by2: %ld\n\r", mob->specials.affected_by2 );
      send_to_char( buf, ch );
    }
    else
    {
      mob->specials.affected_by2 = lparm;
      sprintf( buf, "Affected_by2 changed\n\r" );
      send_to_char( buf, ch );
    }
  }
  else if (!strcmp(field,"act"))
  {
    sscanf(parmstr,"%d",&parm); 
    mob->specials.act=parm;
  }
  else if (!strcmp(field,"numatks"))
  {
    sscanf(parmstr,"%d",&parm);
    mob->mult_att=parm;
  }
  else if (!strcmp(field,"remaffect"))
  {
    mob->affected=NULL;
    mob->specials.affected_by = 0;
    mob->specials.affected_by2 = 0;
    send_to_char("All affects removed from char!\n\r", ch);
  }
  else if (!strcmp(field, "lev"))
  {
    if( sscanf(parmstr,"%d %d", &parm, &parm2 ) != 2 || parm < 0 || 
        parm2 < 1 || parm2 > MAX_CLASS )
    {
      send_to_char( "Uso: @ lev <name> <newlev> <class>\n\r", ch );
      return;
    }
    if (!IS_NPC(mob)) 
    {
      if ((GetMaxLevel(mob) > GetMaxLevel(ch)) && (ch != mob)) 
      {
        send_to_char(GET_NAME(ch), mob);
        send_to_char(" just tried to change your level.\n\r",mob);
        return;
      }
      else if( GetMaxLevel(mob) < LOW_IMMORTAL &&
               GetMaxLevel(ch) <  IMPLEMENTOR && parm > 50) 
      {
        send_to_char("Thou shalt not create new immortals.\n\r", ch);
      }
    }
    else 
    {
      if( parm >= 0 && parm2 > 0 && parm2 <= MAX_CLASS)
        GET_LEVEL( mob, parm2 - 1 ) = parm;
      return;
    }

    if (parm < 0) 
    {
      send_to_char("bug fix. :-)\n\r", ch);
      return;
    }

    if( parm < GetMaxLevel(ch) || !strcasecmp(GET_NAME(ch), "Benem")) 
    {
      if( GetMaxLevel(ch) >= IMPLEMENTOR) 
      {
        if( parm2 <= MAX_CLASS )
          GET_LEVEL(mob, parm2 - 1 ) = parm;
      }
      else 
      {
        if (parm > DEMIGOD) 
        {
          send_to_char("Sorry, you can't advance past 54th level\n", ch);
          return;
        }
        if (parm2 <= MAX_CLASS)
          GET_LEVEL( mob, parm2 - 1 ) = parm;
      }
    }
  }
  else if (!strcmp(field, "sex")) 
  {
    sscanf(parmstr,"%d",&parm);
    GET_SEX(mob) = parm;
  }
  else if (!strcmp(field, "race")) 
  {
    sscanf(parmstr,"%d",&parm);
    GET_RACE(mob) = parm;
  }
  else if (!strcmp(field, "hunger")) 
  {
    sscanf(parmstr,"%d",&parm);
    GET_COND(mob, FULL) = parm;
  }
  else if (!strcmp(field, "thirst")) 
  {
    sscanf(parmstr,"%d",&parm);
    GET_COND(mob, THIRST) = parm;
  }
  else if (!strcmp(field, "zone")) 
  {
    sscanf(parmstr,"%d",&parm);
    GET_ZONE(mob) = parm;
  }
  else if (!strcmp(field, "hit")) 
  {
    sscanf(parmstr,"%d",&parm);
    GET_HIT(mob) = parm;
  }
  else if (!strcmp(field, "mhit")) 
  {
    if( sscanf(parmstr,"%d",&parm) < 1 )
    {
      sprintf( buf, "Hit points bonus: %d\r\n", mob->points.max_hit );
      send_to_char( buf, ch );
    }
    else
      mob->points.max_hit = parm;
  }
  else if (!strcmp(field, "tohit")) 
  {
    sscanf(parmstr,"%d", &parm);
    GET_HITROLL(mob)=parm;
  }
  else if (!strcmp(field, "todam")) 
  {
    sscanf(parmstr,"%d", &parm);
    GET_DAMROLL(mob)=parm;
  }
  else if (!strcmp(field, "ac")) 
  {
    sscanf(parmstr,"%d", &parm);
    GET_AC(mob)=parm;
  }
  else if (!strcmp(field, "bank")) 
  {
    if( sscanf(parmstr, "%d", &parm) < 1 )
    {
      sprintf( buf, "Bank balance: %d\r\n", GET_BANK( mob ) );
      send_to_char( buf, ch );
    }
    else
      GET_BANK(mob) = parm;
  }
  else if (!strcmp(field, "gold")) 
  {
    sscanf(parmstr, "%d", &parm);
    GET_GOLD(mob) = parm;
  }
  else if (!strcmp(field, "prac")) 
  {
    if( sscanf(parmstr, "%d", &parm) < 1 )
    {
      sprintf( buf, "Practices left: %d\r\n", mob->specials.spells_to_learn );
      send_to_char( buf, ch );
    }
    else
      mob->specials.spells_to_learn = parm;
  }
  else if (!strcmp(field, "age")) 
  {
    sscanf(parmstr, "%d", &parm);
    mob->player.time.birth -= SECS_PER_MUD_YEAR*parm;      
  }
  else if (!strcmp(field, "str")) 
  {
    sscanf(parmstr, "%d", &parm);
    mob->abilities.str = parm;
    mob->tmpabilities.str = parm;
  }
  else if (!strcmp(field, "add")) 
  {
    sscanf(parmstr, "%d", &parm);
    mob->abilities.str_add = parm;
    mob->tmpabilities.str_add = parm;
  }
  else if (!strcmp(field, "saves")) 
  {
    parm = 0;
    parm2 = 0;
    sscanf(parmstr, "%d %d", &parm, &parm2);
    mob->specials.apply_saving_throw[parm] = parm2;
  } 
  else if (!strcmp(field, "skills")) 
  {
    parm = 0;
    parm2 = 0;
    switch( sscanf(parmstr,"%d %d", &parm, &parm2 ) )
    {
      case 1:
        if( parm < 1 || parm > MAX_SKILLS )
          sprintf( buf, "Non esiste uno skill con il numero %d\n\r", parm );
        else if( mob->skills )
          sprintf( buf, "Il valore dello skills %d e` %d\n\r", parm,
                   mob->skills[ parm ].learned );
        else
          sprintf( buf, "%s non ha spazio per gli skills.\n\r", 
                   GET_NAME( mob ) );
        break;
      case 2:
        if( parm < 0 || parm > MAX_SKILLS )
          sprintf( buf, "Non esiste uno skill con il numero %d\n\r", parm );
        else if( parm2 < 0 || parm2 > 100 )
          sprintf( buf, "'%d' non e` un valore valido per lo skill %d",
                   parm2, parm );
        else if( mob->skills )
        {
          mob->skills[parm].learned = parm2;
          sprintf( buf, "Hai posto il valore dello skill %d a %d\n\r", parm,
                   parm2 );
        }
        else
          sprintf( buf, "%s non ha spazio per gli skills.\n\r"
                   "Mandalo alla gilda prima.\n\r", GET_NAME( mob ) );
        break;
      default:
        sprintf( buf, "Uso: @ skills <nome> <skill#> [<valore>]\n\r" );
    }
    send_to_char(buf, ch);
  }
  else if( !strcmp( field, "known") )
  {
    parm = 0;
    parm2 = 0;
    switch( sscanf( parmstr,"%d %d", &parm, &parm2 ) )
    {
      case 1:
        if( parm < 0 || parm > MAX_SKILLS )
          sprintf( buf, "Non esiste uno skill con il numero %d\n\r", parm );
        else if( mob->skills )
          sprintf( buf, "Il valore dei flags dello skills %d e` %d\n\r", parm,
                   mob->skills[ parm ].flags );
        else
          sprintf( buf, "%s non ha spazio per gli skills.\n\r", 
                   GET_NAME( mob ) );
        break;
      case 2:
        if( parm < 0 || parm > MAX_SKILLS )
          sprintf( buf, "Non esiste uno skill con il numero %d\n\r", parm );
        else if( mob->skills )
        {
          mob->skills[parm].flags = parm2;
          sprintf( buf, "Hai posto il valore dei flags dello skill %d a "
                   "%d\n\r", parm, parm2 );
        }
        else
          sprintf( buf, "%s non ha spazio per gli skills.\n\r"
                   "Mandalo alla gilda prima.\n\r", GET_NAME( mob ) );
        break;
      default:
        sprintf( buf, "Uso: @ known <nome> <skill#> [<valore>]\n\r" );
    }
    send_to_char( buf, ch );
  }
  else if( !strcmp( field, "specskill") )
  {
    parm = 0;
    parm2 = 0;
    switch( sscanf( parmstr,"%d %d", &parm, &parm2 ) )
    {
      case 1:
        if( parm < 0 || parm > MAX_SKILLS )
          sprintf( buf, "Non esiste uno skill con il numero %d\n\r", parm );
        else if( mob->skills )
          sprintf( buf, "Il valore di special dello skills %d e` %d\n\r", parm,
                   mob->skills[ parm ].special );
        else
          sprintf( buf, "%s non ha spazio per gli skills.\n\r", 
                   GET_NAME( mob ) );
        break;
      case 2:
        if( parm < 0 || parm > MAX_SKILLS )
          sprintf( buf, "Non esiste uno skill con il numero %d\n\r", parm );
        else if( mob->skills )
        {
          mob->skills[parm].special = parm2;
          sprintf( buf, "Hai posto il valore di special dello skill %d a "
                   "%d\n\r", parm, parm2 );
        }
        else
          sprintf( buf, "%s non ha spazio per gli skills.\n\r"
                   "Mandalo alla gilda prima.\n\r", GET_NAME( mob ) );
        break;
      default:
        sprintf( buf, "Uso: @ specskill <nome> <skill#> [<valore>]\n\r" );
    }
    send_to_char( buf, ch );
  }
  else if( !strcmp( field, "stadd" ) ) 
  {
    sscanf(parmstr, "%d", &parm);
    mob->abilities.str_add = parm;
    mob->tmpabilities.str_add = parm;
  }
  else if (!strcmp(field, "int")) 
  {
    sscanf(parmstr, "%d", &parm);
    mob->abilities.intel = parm;
    mob->tmpabilities.intel = parm;
  }
  else if (!strcmp(field, "wis")) 
  {
    sscanf(parmstr, "%d", &parm);
    mob->abilities.wis = parm;
    mob->tmpabilities.wis = parm;
  }
  else if (!strcmp(field, "dex")) 
  {
    sscanf(parmstr, "%d", &parm);
    mob->abilities.dex = parm;
    mob->tmpabilities.dex = parm;

  }
  else if (!strcmp(field, "con")) 
  {
    sscanf(parmstr, "%d", &parm);
    mob->abilities.con = parm;
    mob->tmpabilities.con = parm;

  }
  else if (!strcmp(field, "chr")) 
  {
    sscanf(parmstr, "%d", &parm);
    mob->abilities.chr = parm;
    mob->tmpabilities.chr = parm;
  }
  else if (!strcmp(field,"nodelete")) 
  {
    if (IS_SET(mob->player.user_flags,NO_DELETE)) 
    {
      send_to_char("Setting no delete flag OFF.\n\r",ch);
      send_to_char("Your no delete flag has been removed.\n\r",mob);
      REMOVE_BIT(mob->player.user_flags,NO_DELETE);
    } 
    else
    {
      send_to_char("Setting no delete flag ON.\n\r",ch);
      send_to_char("You are now flagged as NO DELETE.\n\r",mob);                
      SET_BIT(mob->player.user_flags,NO_DELETE);
    }
  } 
  else if (!strcmp(field,"murder")) 
  {
    if (GetMaxLevel(ch) < GetMaxLevel(mob)) 
    {
      send_to_char("I don't think so.\n\r",ch);
      sprintf(buf,"%s tried to set your murder flag!\n\r", GET_NAME( ch ) );
      send_to_char(buf,mob);
      return;
    }
    else if (IS_SET(mob->player.user_flags,MURDER_1) && IS_PC(mob)) 
    {
      REMOVE_BIT(mob->player.user_flags,MURDER_1);
      send_to_char("Murder flag removed.\n\r",ch);
      if (ch != mob) 
        send_to_char("You have been pardoned for murder!\n\r",mob);
    }
    else 
    {
      SET_BIT(mob->player.user_flags,MURDER_1);
      send_to_char("Murder flag set!\n\r", ch);
      if (mob != ch)
        send_to_char("You have been accused of MURDER!\n\r",mob);
    } 
  }
  else if (!strcmp(field,"objedit")) 
  {
    if (IS_SET(mob->player.user_flags,CAN_OBJ_EDIT) && IS_PC(mob)) 
    {
      REMOVE_BIT(mob->player.user_flags,CAN_OBJ_EDIT);
      if (mob !=ch)
        send_to_char("You can no longer edit objects.\n\r",mob);
      send_to_char("Object edit flag removed.\n\r",ch);
    }
    else /* end objedit was set */
    {
      SET_BIT(mob->player.user_flags,CAN_OBJ_EDIT);
      if (ch!=mob)
        send_to_char("You can now edit objects.\n\r",mob);
      send_to_char("Object edit flag set.\n\r",ch);
    } 
  } /* end objedit */
  else if (!strcmp(field,"mobedit")) 
  {
    if (IS_SET(mob->player.user_flags,CAN_MOB_EDIT) && IS_PC(mob)) 
    {
      REMOVE_BIT(mob->player.user_flags,CAN_MOB_EDIT);
      if (mob !=ch)
        send_to_char("You can no longer edit mobiles.\n\r",mob);
      send_to_char("Mobile edit flag removed.\n\r",ch);
    }
    else /* end mobedit was set */
    {
      SET_BIT(mob->player.user_flags,CAN_MOB_EDIT);
      if (ch!=mob)
        send_to_char("You can now edit mobiles.\n\r",mob);
      send_to_char("Mobile edit flag set.\n\r",ch);
    } 
  } /* end Mobedit */
  else  if (!strcmp(field,"stole")) 
  {
    if (GetMaxLevel(ch) < GetMaxLevel(mob)) 
    {
      send_to_char("I don't think so.\n\r",ch);
      sprintf(buf,"%s tried to set your stole flag!\n\r", GET_NAME( ch ) );
      send_to_char(buf,mob);
      return;
    }
    else if (IS_SET(mob->player.user_flags,STOLE_1) && IS_PC(mob)) 
    {
      REMOVE_BIT(mob->player.user_flags,STOLE_1);
      send_to_char("Thief flag removed.\n\r",ch);
      if (ch != mob) 
        send_to_char("You have been pardoned for robbery!\n\r",mob);
    }
    else 
    {
      SET_BIT(mob->player.user_flags,STOLE_1);
      send_to_char("Thief flag set!\n\r", ch);
      if (mob != ch)
        send_to_char("You have been accused of robbery!\n\r",mob);
    } 
  }
  else if (!strcmp(field, "pkill")) 
  {
    if (PeacefulWorks) 
    {
      PeacefulWorks = FALSE;
      EasySummon = FALSE;
      mudlog( LOG_PLAYERS, "Peaceful rooms and Easy Summon disabled by %s", 
              GET_NAME(ch));
    }
    else 
    {
      PeacefulWorks = TRUE;
      EasySummon = TRUE;
      mudlog( LOG_ERROR, "Peaceful rooms and Easy Summon enabled by %s",
              GET_NAME(ch));
    }
  }
  else if (!strcmp(field, "mana")) 
  {
    sscanf(parmstr, "%d", &parm);
    mob->points.mana     = parm;
    mob->points.max_mana = parm;

  }
  else if (!strcmp(field, "start")) 
  {
    sscanf(parmstr, "%d", &parm);
    mob->specials.start_room = parm;

  }
  else if( !strcmp(field, "gmove") )
  {
    if( sscanf(parmstr, "%d", &parm ) != 1 )
    {
      sprintf( buf, "Il bonus move regain e` uguale a %d\n\r", 
               mob->points.move_gain );
      send_to_char( buf, ch );
    }
    else
    {
      send_to_char( "OK.\n\r", ch );
      mob->points.move_gain = parm;
    }
  }
  else if( !strcmp(field, "height") )
  {
    if( sscanf(parmstr, "%d", &parm ) != 1 )
    {
      sprintf( buf, "L'altezza e` uguale a %d\n\r", 
               GET_HEIGHT( mob ) );
      send_to_char( buf, ch );
    }
    else
    {
      send_to_char( "OK.\n\r", ch );
      GET_HEIGHT( mob ) = parm;
    }
  }
  else if( !strcmp(field, "weight") )
  {
    if( sscanf(parmstr, "%d", &parm ) != 1 )
    {
      sprintf( buf, "Il peso e` uguale a %d\n\r", 
               GET_WEIGHT( mob ) );
      send_to_char( buf, ch );
    }
    else
    {
      send_to_char( "OK.\n\r", ch );
      GET_WEIGHT( mob ) = parm;
    }
  }
  else if( !strcmp( field, "position" ) )
  {
    if( sscanf( parmstr, "%d", &parm ) != 1 )
    {
      sprintf( buf, "La sua posizione e`: %d\n\r", GET_POS( mob ) );
      send_to_char( buf, ch );
    }
    else if( parm < 0 || parm > 9 )
    {
      send_to_char( "Posizione non valida. Eccone l'elenco:\r"
                    "0 Dead\r"
                    "1 Mortally wounded\r"
                    "2 Incapacitated\r"
                    "3 Stunned\r"
                    "4 Sleeping\r"
                    "5 Resting\r"
                    "6 Sitting\r"
                    "7 Fighting\r"
                    "8 Standing\r"
                    "9 Mounted\r", ch );
    }
    else
    {
      send_to_char( "OK.\n\r", ch );
      GET_POS( mob ) = parm;
    }
  }
  else if( !strcmp( field, "startroom" ) )
  {
    if( sscanf( parmstr, "%d", &parm ) != 1 )
    {
      sprintf( buf, "Il numero della stanza di partenza e`: %ld\n\r",
               mob->lStartRoom );
      send_to_char( buf, ch );
    }
    else if( parm < 0 )
    {
      send_to_char( "Il numero della stanza deve essere > di 0.\r\n", ch );
    }
    else
    {
      mob->lStartRoom = parm;
      send_to_char( "OK.\n\r", ch );
    }
  }
  else
  {
    send_to_char("What the did you wanna set?\n\r",ch);
  }
}




void do_shutdow(struct char_data *ch, const char *argument, int cmd)
{
  send_to_char("If you want to shut something down - say so!\n\r", ch);
}



void do_shutdown(struct char_data *ch, const char *argument, int cmd)
{
  extern int mudshutdown, rebootgame;
  char buf[100], arg[MAX_INPUT_LENGTH];
  
  if (IS_NPC(ch))
    return;
  
  one_argument( argument, arg );

  if( GetMaxLevel(ch) < SILLYLORD )
  {
    char szPass[ 20 ];
    FILE *pF = fopen( "shutpass", "r" );
    if( pF != NULL )
    {
      fscanf( pF, "%19s\n", szPass );
      if( *arg && !strcmp( szPass, arg ) )
      {
        int nCount = 0;
        struct descriptor_data *pDesc;
        
        for( pDesc = descriptor_list; pDesc; pDesc = pDesc->next )
          if( pDesc->connected == CON_PLYNG )
            nCount++;
        if( nCount <= 1 )
        {
          sprintf(buf, "Shutdown by %s.\n\r", GET_NAME(ch) );
          send_to_all( buf );
          mudlog( LOG_PLAYERS, "Shutdown by %s.\n\r", GET_NAME(ch) );
          mudshutdown = 1;
        }
      }
      fclose( pF );
    }
  }
  else
  {
    if( !*arg ) 
    {
      sprintf(buf, "Shutdown by %s.\n\r", GET_NAME(ch) );
      send_to_all(buf);
      mudlog( LOG_PLAYERS, "Shutdown by %s.\n\r", GET_NAME(ch) );
      mudshutdown = 1;
    } 
    else if (!str_cmp(arg, "reboot")) 
    {
      sprintf( buf, "Reboot by %s.\n\r", GET_NAME( ch ) );
      send_to_all( buf);
      mudlog( LOG_PLAYERS, "Reboot by %s.\n\r", GET_NAME( ch ) );
      mudshutdown = rebootgame = 1;
    } 
    else
      send_to_char("Go shut down someone your own size.\n\r", ch);
  }
}


void do_snoop(struct char_data *ch, const char *argument, int cmd)
{
  static char arg[MAX_STRING_LENGTH];
  struct char_data *victim;
  
  if (!ch->desc)
    return;
  
  if (IS_NPC(ch))
    return;
  
  only_argument(argument, arg);
  
  if(!*arg)        {
    send_to_char("Snoop who ?\n\r",ch);
    return;
  }
  
  if(!(victim=get_char_vis(ch, arg)))        {
    send_to_char("No such person around.\n\r",ch);
    return;
  }
  
  if(!victim->desc)        {
    send_to_char("There's no link.. nothing to snoop.\n\r",ch);
    return;
  }
  if(victim == ch)        
  {
    send_to_char("Ok, you just snoop yourself.\n\r",ch);
    if(ch->desc->snoop.snooping) 
    {
      if (ch->desc->snoop.snooping->desc)
        ch->desc->snoop.snooping->desc->snoop.snoop_by = 0;
      else 
      {
        mudlog( LOG_CHECK, 
                "caught %s snooping %s who didn't have a descriptor!",
                ch->player.name, ch->desc->snoop.snooping->player.name);
        /* logically.. this person has returned from being a creature? */ 
      }
      ch->desc->snoop.snooping = 0;
    }
    return;
  }
  
  if(victim->desc->snoop.snoop_by) {
    send_to_char("Busy already. \n\r",ch);
    return;
  }
  
  if (GetMaxLevel(victim)>=GetMaxLevel(ch))        {
    send_to_char("You failed.\n\r",ch);
    return;
  }
  
  send_to_char("Ok. \n\r",ch);
  
  if(ch->desc->snoop.snooping)
    if (ch->desc->snoop.snooping->desc)
      ch->desc->snoop.snooping->desc->snoop.snoop_by = 0;
  
  ch->desc->snoop.snooping = victim;
  victim->desc->snoop.snoop_by = ch;
  return;
}



void do_switch(struct char_data *ch, const char *argument, int cmd)
{
  static char arg[80];
  struct char_data *victim;
  
  if (IS_NPC(ch))
    return;
  
  only_argument(argument, arg);
  
  if (!*arg)        
  {
    send_to_char("Switch with who?\n\r", ch);
  }
  else
  {
    if (!(victim = get_char(arg)))
      send_to_char("They aren't here.\n\r", ch);
    else
    {
      if (ch == victim)
      {
        send_to_char("He he he... We are jolly funny today, eh?\n\r", ch);
        return;
      }
      
      if (!ch->desc || ch->desc->snoop.snoop_by || ch->desc->snoop.snooping)
      {
        send_to_char("Mixing snoop & switch is bad for your health.\n\r", ch);
        return;
      }
      
      if( victim->desc || !IS_NPC( victim ) )
      {
        send_to_char( "You can't do that, the body is already in use!\n\r",ch);
      }
      else
      {
        if( GetMaxLevel(victim) > GetMaxLevel(ch) && 
            GetMaxLevel(ch) < IMPLEMENTOR )
        {
          send_to_char("That being is much more powerful than you!\n\r",ch);
          return;
        }

        send_to_char("Ok.\n\r", ch);
        
        ch->desc->character = victim;
        ch->desc->original = ch;
        
        victim->desc = ch->desc;
        ch->desc = 0;
      }
    }
  }
}



void do_return(struct char_data *ch, const char *argument, int cmd)
{
  struct char_data *mob = NULL, *per = NULL;

  void do_snoop(struct char_data *ch, const char *argument, int cmd);
  
  if(!ch->desc)
    return;
  
  if(!ch->desc->original)
  { 
    send_to_char("Arglebargle, glop-glyf!?!\n\r", ch);
    return;
  }
  else
  {
    if (GET_POS(ch) == POSITION_FIGHTING && GetMaxLevel(ch) < LOW_IMMORTAL) 
    {
      send_to_char("Non mentre combatti!\n\r",ch);
      return;
    }

    send_to_char("Ritorni alla tua forma originale.\n\r",ch);

    if( ch->desc->snoop.snoop_by )
    {  /* force the snooper to stop */
      do_snoop(ch->desc->snoop.snoop_by, GET_NAME(ch->desc->snoop.snoop_by), 0);
    }
    
    if (IS_SET(ch->specials.act, ACT_POLYSELF) && cmd) 
    {
      mob = ch;
      per = ch->desc->original;
      
      act("$n riprende la sua forma originale.", TRUE, mob, 0, per, TO_ROOM);

      char_from_room( per );
      char_to_room( per, mob->in_room );

      SwitchStuff( mob, per );
    }
    
    ch->desc->character = ch->desc->original;
    ch->desc->original = 0;
    
    ch->desc->character->desc = ch->desc; 
    ch->desc = 0;
    
    if (IS_SET(ch->specials.act, ACT_POLYSELF) && cmd) 
    {
      extract_char(mob);
    }
  }
}


void do_force(struct char_data *ch, const char *argument, int cmd)
{
  struct descriptor_data *i;
  struct char_data *vict;
  char name[100], to_force[100],buf[100]; 
  
  if( !IS_PC( ch ) && cmd != 0 )
    return;
  
  half_chop(argument, name, to_force);
  
  if (!*name || !*to_force)
    send_to_char("Chi vuoi forzare ed a fare cosa ?\n\r", ch);
  else if (str_cmp("all", name)) 
  {
    if (!(vict = get_char_vis(ch, name)))
      send_to_char( "Non c'e` nessuno con quel nome...\n\r", ch);
    else 
    {
      if( ( GetMaxLevel(ch) <= GetMaxLevel(vict)) && (!IS_NPC(vict))) 
      {
        if (CAN_SEE(ch,vict))
           send_to_char("Oh no, non puoi!\n\r", ch);
      }
      else
      {
        if( !IS_NPC( ch ) && !IS_SET( ch->specials.act, PLR_STEALTH ) )
        {
          sprintf(buf, "$n ti ha obbligat$B a '%s'.", to_force);
          act(buf, FALSE, ch, 0, vict, TO_VICT);
        }
        send_to_char("Ok.\n\r", ch);
        command_interpreter(vict, to_force);
      }
    }
  }
  else 
  { 
    /* force all */
    for (i = descriptor_list; i; i = i->next)
    {
      if (i->character != ch && !i->connected) 
      {
               vict = i->character;
               if ((GetMaxLevel(ch) <= GetMaxLevel(vict)) && (!IS_NPC(vict))) 
        {
          if (CAN_SEE(ch,vict))
            send_to_char("Oh no, non puoi!!\n\r", ch);
        } 
        else 
        {
          if( !IS_NPC( ch ) && !IS_SET( ch->specials.act, PLR_STEALTH ) )
          {
            sprintf(buf, "$n ti ha obbligat$B a '%s'.", to_force);
            act(buf, FALSE, ch, 0, vict, TO_VICT);
          }
          command_interpreter(vict, to_force);
        }
      }
    }
    send_to_char("Ok.\n\r", ch);
  }
}
/*************************************************************************
 *  do_mload porta una creatura in vita.
 *************************************************************************/
void do_mload(struct char_data *ch, const char *argument, int cmd)
{
  struct char_data *mob;
  char num[100];
  int number;
  
  extern int top_of_mobt;
  
  if (IS_NPC(ch))
    return;
  
  only_argument(argument, num);
  if( isdigit(*num))
    number = atoi(num);
  else
    number = -1;
  
  if( number < 0 )
  {
    for( number = 0; number < top_of_mobt; number++ )
      if( isname( num, mob_index[ number ].name ) )
        break;
    if( number >= top_of_mobt )
      number = -1;
  }
  else
  {
    number = real_mobile(number);
  }
  if( number < 0 || number >= top_of_mobt)
  {
    send_to_char("There is no such monster.\n\r", ch);
    return;
  }
  mob = read_mobile(number, REAL);
  char_to_room(mob, ch->in_room);
  
  act( "$n makes a quaint, magical gesture with one hand.", TRUE, ch,
       0, 0, TO_ROOM);
  act("$n has summoned $N from the ether!", FALSE, ch, 0, mob, TO_ROOM);
  act("You bring forth $N from the the cosmic ether.", FALSE, ch, 0, mob, TO_CHAR);
}

/****************************************************************************
 * do_oload crea un oggetto
 ****************************************************************************/
void do_oload(struct char_data *ch, const char *argument, int cmd)
{
  struct obj_data *obj;
  char num[100], buf[100];
  int number;
  
  if (IS_NPC(ch))
    return;
  
  only_argument(argument, num);
  if (isdigit(*num))
    number = atoi(num);
  else
    number = -1;
  
  if (number<0)
  {
    for( number = 0; number < top_of_objt; number++ )
      if( isname(num, obj_index[number].name) )
        break;
    if( number >= top_of_objt )
      number = -1;
  }
  else
  {
    number = real_object(number);
  }
  if ( number < 0 || number >= top_of_objt )
  {
    send_to_char("There is no such object.\n\r", ch);
    return;
  }
    
  if( GetMaxLevel(ch) < IMPLEMENTOR )
  {
    switch( obj_index[number].iVNum )
    {
     case 5021:
      send_to_char( "No.  No more bows!  And don't kill the worm either!\n\r",
                    ch );
      return;
     case 5112:
      send_to_char( "No, no more Ruby rings!  And don't kill for it either!\n\r",
                    ch );
      return;
     case 233:
     case 21150:
     case 30012:
     case 30013:
     case 30014:
     case 30015:
     case 30016:
     case 30017:
     case 30018:
      send_to_char( "When monkeys fly out of Ripper\'s butt.\n\r", ch );
      return;
     case 24:
     case 25:
     case 26:
     case 27:
     case 28:
      send_to_char("Sorry, private items.\n\r", ch);
      return;
     case 1600:
      send_to_char( "Oh no! Basta ebony kris. C'e il kris d'ebano "
                    "ora!\n\r", ch );
      return;
     case 5311:
      send_to_char( "Oh no! Basta mercury's boots. Ci sono gli stivali di "
                    "Mercurio ora!\n\r", ch );
      return;
     case 13702:
      send_to_char( "Oh no! Basta lion skin. C'e la pelle del Leone "
                    "ora!\n\r", ch );
      return;
     case 21113:
     case 21117:
     case 21120:
     case 21121:
     case 21122:
      send_to_char("You can't load this item, sorry.\n\r", ch);
      return;

    }
  }
  if( obj_index[number].iVNum >= 150 && obj_index[number].iVNum < 200 &&
      !isname( "Benem", GET_NAME( ch ) ) )
  {
    send_to_char( "Mi dispiace, ma e` un oggetto privato.\n\r", ch );
    return;
  }
  
    
  obj = read_object(number, REAL);
  obj_to_char(obj, ch);

  if (GetMaxLevel(ch) < BIG_GUY)
  {
    sprintf(buf,"%s loaded %s",GET_NAME(ch),obj->name);
    mudlog( LOG_PLAYERS, buf );
  }

  act("$n esegue un rituale magico.", TRUE, ch, 0, 0, TO_ROOM);
  act("$n ha creato $p!", TRUE, ch, obj, 0, TO_ROOM);
  
  act("Adesso hai $p.", FALSE, ch, obj, 0, TO_CHAR);
}


void purge_one_room(int rnum, struct room_data *rp, int *range)
{
  struct char_data        *ch;
  struct obj_data        *obj;
  extern long room_count;

  if( rnum==0 ||                           /* purge the void?  I think not */
      rnum < range[0] || rnum > range[1])
    return;

  while (rp->people) 
  {
    ch = rp->people;
    send_to_char("A god strikes the heavens making the ground around you erupt into a", ch);
    send_to_char("fluid fountain boiling into the ether.  All that's left is the Void.", ch);
    char_from_room(ch);
    char_to_room(ch, 0);        /* send character to the void */
    do_look(ch, "", 15);
    act("$n tumbles into the Void.", TRUE, ch, 0, 0, TO_ROOM);
  }

  while (rp->contents) 
  {
    obj = rp->contents;
    obj_from_room(obj);
    obj_to_room(obj, 0);        /* send item to the void */
  }

  completely_cleanout_room(rp); /* clear out the pointers */
#if HASH  
  hash_remove(&room_db, rnum);        /* remove it from the database */
#else
  room_remove(room_db, rnum);
#endif
  room_count--;

}


/* clean a room of all mobiles and objects */
void do_purge(struct char_data *ch, const char *argument, int cmd)
{
  struct char_data *vict, *next_v;
  struct obj_data *obj, *next_o;
  
  char name[100];
  
  if (IS_NPC(ch))
    return;
  
  only_argument(argument, name);
  
  if (*name) {  /* argument supplied. destroy single object or char */
    if (strcmp(name, "links")==0 && GetMaxLevel(ch)>= IMPLEMENTOR) {

      struct descriptor_data *d;

      for (d = descriptor_list; d; d = d->next){
        close_socket(d);
      }
      return;
    }
    if( ( vict = get_char_room_vis(ch, name ) ) )
    {
      if ((!IS_NPC(vict) || IS_SET(vict->specials.act, ACT_POLYSELF)) && 
          (GetMaxLevel(ch)<IMPLEMENTOR)) {
        send_to_char("I'm sorry...  I can't let you do that.\n\r", ch);
        return;
      }
        
        act("$n disintegrates $N.", FALSE, ch, 0, vict, TO_NOTVICT);
        
        if (IS_NPC(vict)) {
          extract_char(vict);
        } else {
          if (vict->desc)        {
            close_socket(vict->desc);
            vict->desc = 0;
            extract_char(vict);
          } else {
            extract_char(vict);
          }
        }
      }
      else if( ( obj = get_obj_in_list_vis( ch, name, 
                       real_roomp(ch->in_room)->contents ) ) ) 
      {
        act("$n destroys $p.", FALSE, ch, obj, 0, TO_ROOM);
        extract_obj(obj);
      } else        {
        argument = one_argument(argument, name);
        if (0==str_cmp("room", name)) {
          int        range[2];
          register int i;
          struct room_data *rp;
          if (GetMaxLevel(ch)<IMPLEMENTOR) {
            send_to_char("I'm sorry, I can't let you do that.\n\r", ch);
            return;
          }
          argument = one_argument(argument,name);
          if (!isdigit(*name)) {
            send_to_char("purge room start [end]",ch);
            return;
          }
          range[0] = atoi(name);
          argument = one_argument(argument,name);
          if (isdigit(*name))
            range[1] = atoi(name);
          else
            range[1] = range[0];
          
          if (range[0]==0 || range[1]==0) {
            send_to_char("usage: purge room start [end]\n\r", ch);
            return;
          }
#if HASH
          hash_iterate(&room_db, purge_one_room, range);
#else
          if (range[0]>=WORLD_SIZE || range[1]>=WORLD_SIZE) {
            send_to_char("only purging to WORLD_SIZE\n\r", ch);
            return;
          }
          for (i=range[0];i<=range[1];i++) {
            if ((rp = real_roomp(i)) != 0) {
              purge_one_room(i, rp, range);
            }
          }
#endif
        } else {
          send_to_char("I don't see that here.\n\r", ch);
          return;
        }
      }
      
      send_to_char("Ok.\n\r", ch);
    }  else {   /* no argument. clean out the room */
      if (GetMaxLevel(ch) < DEMIGOD)
        return;
      if (IS_NPC(ch))        {
        send_to_char("You would only kill yourself..\n\r", ch);
        return;
      }
      
      act("$n gestures... You are surrounded by thousands of tiny scrubbing bubbles!", 
          FALSE, ch, 0, 0, TO_ROOM);
      send_to_room("The world seems a little cleaner.\n\r", ch->in_room);

      for (vict = real_roomp(ch->in_room)->people; vict; vict = next_v) {
        next_v = vict->next_in_room;
        if (IS_NPC(vict) && (!IS_SET(vict->specials.act, ACT_POLYSELF)))
          extract_char(vict);
      }
      
      for (obj = real_roomp(ch->in_room)->contents; obj; obj = next_o) {
        next_o = obj->next_content;
        extract_obj(obj);
      }
    }
}



/* Give pointers to the five abilities */
void roll_abilities(struct char_data *ch)
{
  int i, j, k, temp;
  float avg;
  ubyte table[MAX_STAT];
  ubyte rools[4];
  
  for(i=0; i<MAX_STAT; table[i++]=0)  ;
  
  do 
  {
    for(i=0; i<MAX_STAT; i++) 
    {
      
      for(j=0; j<4; j++)
        rools[j] = number(1,6);
    
      temp = (unsigned int)rools[0]+(unsigned int)rools[1]+
             (unsigned int)rools[2]+(unsigned int)rools[3] -
             MIN((int)rools[0], MIN((int)rools[1], 
                                    MIN((int)rools[2],(int)rools[3])));
      
      for(k=0; k<MAX_STAT; k++)
        if (table[k] < temp)
          SWITCH(temp, table[k]);
    }
    for(j=0,avg=0;j<MAX_STAT;j++)
      avg += table[j];
    avg /= j;

  } while(avg<9.0);

  for (i=0;i<MAX_STAT;i++) 
  {
    switch(ch->desc->stat[i]) 
    {
     case 's' :
      ch->abilities.str = table[i];
      break;
     case 'i':
      ch->abilities.intel = table[i];
      break;
     case 'd':
      ch->abilities.dex = table[i];
      break;
     case 'w':
      ch->abilities.wis = table[i];
      break;
     case 'o':
      ch->abilities.con = table[i];
      break;
     case 'h':
      ch->abilities.chr  = table[i];
      break;
     default:
      break;
    }
  }
  
  if (ch->abilities.str == 0)
    ch->abilities.str = 8;
  if (ch->abilities.intel == 0)
    ch->abilities.intel = 8;
  if (ch->abilities.dex == 0)
    ch->abilities.dex = 8;
  if (ch->abilities.wis == 0)
    ch->abilities.wis = 8;
  if (ch->abilities.con == 0)
    ch->abilities.con = 8;
  if (ch->abilities.chr == 0)
    ch->abilities.chr = 8;

  
  ch->abilities.str_add = 0;


  if (GET_RACE(ch) == RACE_ELVEN) 
  {
    ch->abilities.dex++;
    ch->abilities.con--;
  } 

  if (GET_RACE(ch) == RACE_SEA_ELF) 
  {
    ch->abilities.str++;
    ch->abilities.con--;
  } 
  if (GET_RACE(ch) == RACE_WILD_ELF) 
  {
    ch->abilities.str++;
    ch->abilities.intel--;
    ch->abilities.dex++;
    ch->abilities.wis--;
  } 
  if (GET_RACE(ch) == RACE_GOLD_ELF) 
  {
    ch->abilities.intel++;
    ch->abilities.wis--;
    ch->abilities.dex++;
    ch->abilities.con--;
  } 
  else if (GET_RACE(ch) == RACE_DWARF || GET_RACE(ch) == RACE_DARK_DWARF) 
  {
    ch->abilities.con++;
    ch->abilities.dex--;
  } 
  else if (GET_RACE(ch) == RACE_GNOME || GET_RACE(ch) == RACE_DEEP_GNOME) 
  {
    ch->abilities.intel++;
    ch->abilities.wis--;
  } 
  else if (GET_RACE(ch) == RACE_HALFLING || GET_RACE(ch) == RACE_GOBLIN) 
  {
    ch->abilities.dex++;
    ch->abilities.str--;
  }  
  else if (GET_RACE(ch) == RACE_DROW)
  {
    ch->abilities.dex+=2;
    ch->abilities.con--;
    ch->abilities.chr--;
  }
  else if (GET_RACE(ch) == RACE_HALF_OGRE )
  {
    ch->abilities.str++;
    ch->abilities.con++;
    ch->abilities.dex--;
    ch->abilities.intel--;
  }
  else if (GET_RACE(ch) == RACE_HALF_ORC )
  {
    ch->abilities.con++;
    ch->abilities.chr--;
  }
  else if (GET_RACE(ch) == RACE_HALF_GIANT || GET_RACE(ch) == RACE_TROLL)
  {
    ch->abilities.str+=2;
    ch->abilities.con++;
    ch->abilities.dex--;
    ch->abilities.wis--;
    ch->abilities.intel--;
  }    
    


  ch->points.max_hit = HowManyClasses(ch)*10;

  /* race specific hps stuff */
  if (GET_RACE(ch) == RACE_HALF_GIANT || GET_RACE(ch) == RACE_TROLL)
  {
    /* half_giants get +15 hps more at level 1 */
    ch->points.max_hit +=15;
  }

  /* class specific hps stuff */
  if (HasClass(ch, CLASS_BARBARIAN)) 
  {
    ch->points.max_hit +=10; /* give barbs 10 more hps */
  }    

  if (HasClass(ch, CLASS_MAGIC_USER))
  {
    ch->points.max_hit += number(1,4);
  }
  if (HasClass(ch, CLASS_SORCERER))
  {
    ch->points.max_hit += number(1,4);
  }
  if (HasClass(ch, CLASS_CLERIC))
  {
    ch->points.max_hit += number(1,8);
  }
  if (HasClass(ch, CLASS_WARRIOR|CLASS_BARBARIAN|CLASS_PALADIN|CLASS_RANGER))
  {
    ch->points.max_hit += number(1,10);
    if (ch->abilities.str == 18)
      ch->abilities.str_add = number(0,100);
    if (ch->abilities.str > 18 && 
        (GET_RACE(ch) != RACE_HALF_GIANT || GET_RACE(ch) == RACE_TROLL) ) 
      ch->abilities.str_add = number(((ch->abilities.str-18)*10),100);
    else if (ch->abilities.str >18) /* was a half-giant so just make 100 */
      ch->abilities.str_add = 100;         
  }
    
  if (HasClass(ch, CLASS_THIEF|CLASS_PSI))
  {
    ch->points.max_hit += number(1,6);
  }
  if (HasClass(ch, CLASS_MONK))
  {
    ch->points.max_hit += number(1,6); 
  }
  if (HasClass(ch, CLASS_DRUID))
  {
    ch->points.max_hit += number(1,8);
  }
   
  ch->points.max_hit /= HowManyClasses(ch);

 
  ch->tmpabilities = ch->abilities;
}



void do_start(struct char_data *ch)
{
  int r_num;
  struct obj_data *obj;
  
  
  void advance_level(struct char_data *ch, int i);
  
  
  send_to_char("Benevenuto a Lumen et Umbra. Buon divertimento.\n\r",ch);
  ch->specials.start_room = NOWHERE;
  
  StartLevels(ch);
  
  GET_EXP(ch) = 1;
  
  set_title(ch);
  
  roll_abilities(ch);
  /*
   *  This is the old style of determining hit points.  I modified it so that
   *  characters get the standard AD&D + 10 hp to start.
   *        ch->points.max_hit  = 10;  
   */
  
  /* 
   * outfit char with valueless items
   */
    
  if ((r_num = real_object(12)) >= 0)   
  {
    obj = read_object(r_num, REAL);
    obj_to_char(obj,ch);                           /* bread   */ 
    obj = read_object(r_num, REAL);
    obj_to_char(obj,ch);                           /* bread   */
  }

  if ((r_num = real_object(13)) >= 0)   
  {
    obj = read_object(r_num, REAL);
    obj_to_char(obj,ch);                           /* water   */
    obj = read_object(r_num, REAL);
    obj_to_char(obj,ch);                           /* water   */
  }

 
  if (HasClass(ch, 
               CLASS_CLERIC|CLASS_MAGIC_USER|CLASS_SORCERER|CLASS_PSI|
               CLASS_PALADIN|CLASS_RANGER|CLASS_DRUID)) 
    ch->skills[SKILL_READ_MAGIC].learned = 95;

  if (HasClass(ch,CLASS_RANGER|CLASS_PALADIN)) 
  { 
    /* set rangers and pals to good */
    GET_ALIGNMENT(ch) = 1000;
  }  

  SetDefaultLang(ch);  /* the skill */

  /* set default speaking language */
  switch(GET_RACE(ch)) 
  {
   case RACE_DROW:
   case RACE_GOLD_ELF:
   case RACE_WILD_ELF:
   case RACE_SEA_ELF:
   case RACE_ELVEN:
    ch->player.speaks = SPEAK_ELVISH;
    break;
   case RACE_DWARF: 
    ch->player.speaks = SPEAK_DWARVISH;
    break;
   case RACE_GNOME:
    ch->player.speaks = SPEAK_GNOMISH;
    break;
   case RACE_HALFLING:
    ch->player.speaks = SPEAK_HALFLING;
    break;
   case RACE_HALF_ORC:
    ch->player.speaks = SPEAK_ORCISH;
    break;
   case RACE_HALF_GIANT:
    ch->player.speaks = SPEAK_GIANTISH;
    break;
   case RACE_HALF_OGRE:
    ch->player.speaks = SPEAK_OGRE;
    break;
    /* humans, half-elves all speak common so let default get them */                                        
   default:
    ch->player.speaks = SPEAK_COMMON;
    break;
  } /* end race switch */
                

  /* set default to null */
  GET_SPECFLAGS(ch) = 0;                                  
  /* set each user to pause screens */
  SET_BIT(ch->player.user_flags,USE_PAGING);          

  if (IS_SET(ch->player.iClass, CLASS_THIEF))   
  {
    if (GET_RACE(ch) == RACE_HUMAN) 
    {
      ch->skills[SKILL_SNEAK].learned = 10;
      ch->skills[SKILL_HIDE].learned =  5;
      ch->skills[SKILL_STEAL].learned = 15;
      ch->skills[SKILL_BACKSTAB].learned = 10;
      ch->skills[SKILL_PICK_LOCK].learned = 10;
    } 
    else if (GET_RACE(ch) == RACE_ELVEN ||
             GET_RACE(ch) == RACE_GOLD_ELF ||
             GET_RACE(ch) == RACE_WILD_ELF ||
             GET_RACE(ch) == RACE_SEA_ELF )    
    {
      ch->skills[SKILL_SNEAK].learned = 10;
      ch->skills[SKILL_HIDE].learned =  5;
      ch->skills[SKILL_STEAL].learned = 15;
      ch->skills[SKILL_BACKSTAB].learned = 10;
      ch->skills[SKILL_PICK_LOCK].learned = 10;
    } 
    else if (GET_RACE(ch) == RACE_DROW)   
    {
      ch->skills[SKILL_SNEAK].learned = 20;
      ch->skills[SKILL_HIDE].learned =  15;
      ch->skills[SKILL_STEAL].learned = 25;
      ch->skills[SKILL_BACKSTAB].learned = 20;
      ch->skills[SKILL_PICK_LOCK].learned = 5;
    }
    else if (GET_RACE(ch) == RACE_DWARF)    
    {
      ch->skills[SKILL_SNEAK].learned = 10;
      ch->skills[SKILL_HIDE].learned =  5;
      ch->skills[SKILL_STEAL].learned = 15;
      ch->skills[SKILL_BACKSTAB].learned = 10;
      ch->skills[SKILL_PICK_LOCK].learned = 10;
    } 
    else if (GET_RACE(ch) == RACE_HALFLING)    
    {
      ch->skills[SKILL_SNEAK].learned = 10;
      ch->skills[SKILL_HIDE].learned =  5;
      ch->skills[SKILL_STEAL].learned = 15;
      ch->skills[SKILL_BACKSTAB].learned = 10;
      ch->skills[SKILL_PICK_LOCK].learned = 10;
    } 
    else if (GET_RACE(ch) == RACE_GNOME)    
    {
      ch->skills[SKILL_SNEAK].learned = 10;
      ch->skills[SKILL_HIDE].learned =  5;
      ch->skills[SKILL_STEAL].learned = 15;
      ch->skills[SKILL_BACKSTAB].learned = 10;
      ch->skills[SKILL_PICK_LOCK].learned = 10;
    }
    else if (GET_RACE(ch) == RACE_HALF_ELVEN)     
    {
      ch->skills[SKILL_HIDE].learned =  5;
      ch->skills[SKILL_STEAL].learned = 10;
    } 
    
    else if (GET_RACE(ch) == RACE_HALF_OGRE)  {}
    else if (GET_RACE(ch) == RACE_HALF_ORC)  {}
    else if (GET_RACE(ch) == RACE_HALF_GIANT)  {}
  }

  ch->skills[SKILL_BASH].learned = 0;
  ch->skills[SKILL_KICK].learned = 0;
  
  GET_HIT(ch) = GET_MAX_HIT(ch);
  GET_MANA(ch) = GET_MAX_MANA(ch);

  /* ch->points.max_move += GET_CON(ch) + number(1,20) - 9; */

  GET_MOVE(ch) = GET_MAX_MOVE(ch);
  
  GET_COND(ch,THIRST) = 24;
  GET_COND(ch,FULL) = 24;
  GET_COND(ch,DRUNK) = 0;
  
  ch->points.gold = 150;   /* newbies starts with 150 coins */
  
  ch->player.time.played = 0;
  ch->player.time.logon = time(0);
  
}


void do_advance(struct char_data *ch, const char *argument, int cmd)
{
  struct char_data *victim;
  char name[100], level[100], achClass[100];
  int adv, newlevel, lin_class;

  void gain_exp(struct char_data *ch, int gain);

  if (IS_NPC(ch))
    return;

  argument = one_argument(argument, name);
  
  if (*name) 
  {
    if (!(victim = get_char_room_vis(ch, name)))               
    {
      send_to_char("That player is not here.\n\r", ch);
      return;
    }
  }
  else
  {
    send_to_char("Advance who?\n\r", ch);
    return;
  }

  if (IS_NPC(victim))
  {
    send_to_char("NO! Not on NPC's.\n\r", ch);
    return;
  }

  argument = one_argument(argument, achClass);

  if (!*achClass)
  {
    send_to_char("Supply a class: M C W T D K B S P R I\n\r",ch);
    return;
  }

  switch(*achClass)
  {
   case 'M':
   case 'm':
    lin_class = MAGE_LEVEL_IND;
    break;

   case 'T':
   case 't':
    lin_class = THIEF_LEVEL_IND;
    break;
          
   case 'W':
   case 'w':
   case 'F':
   case 'f':
    lin_class = WARRIOR_LEVEL_IND;
    break;
          
   case 'C':
   case 'c':
    lin_class = CLERIC_LEVEL_IND;
    break;
          
   case 'D':
   case 'd':
    lin_class = DRUID_LEVEL_IND;
    break;

   case 'K' :
   case 'k' :
    lin_class = MONK_LEVEL_IND;
    break;

   case 'b' :
   case 'B' :
    lin_class = BARBARIAN_LEVEL_IND;
    break;

   case 'S':
   case 's':
    lin_class = SORCERER_LEVEL_IND;
    break;

   case 'P' :
   case 'p' :
    lin_class = PALADIN_LEVEL_IND;
    break;

   case 'R' :
   case 'r' :
    lin_class = RANGER_LEVEL_IND;
    break;
          
   case 'I' :
   case 'i' :
    lin_class = PSI_LEVEL_IND;
    break;

   default:
    send_to_char("Supply a class: M C W T D K B S P R I\n\r",ch);
    return;
    break;

  }
        
  argument = one_argument(argument, level);

  if (GET_LEVEL(victim, lin_class) == 0)
    adv = 1;
  else if (!*level)
  {
    send_to_char("You must supply a level number.\n\r", ch);
    return;
  }
  else
  {
    if (!isdigit(*level))               
    {
      send_to_char("Third argument must be a positive integer.\n\r",ch);
      return;
    }
    if ((newlevel = atoi(level)) < GET_LEVEL(victim, lin_class))
    {
      send_to_char("Can't dimish a players status (yet).\n\r", ch);
      return;
    }
    adv = newlevel - GET_LEVEL(victim,lin_class);
  }

  if (((adv + GET_LEVEL(victim, lin_class)) > 1) && 
      (GetMaxLevel(ch) < IMPLEMENTOR))
  {
    send_to_char("Thou art not godly enough.\n\r", ch);
    return;
  }

  if ((adv + GET_LEVEL(victim, lin_class)) > IMPLEMENTOR)       
  {
    send_to_char("Implementor is the highest possible level.\n\r", ch);
    return;
  }

  if (((adv + GET_LEVEL(victim, lin_class)) < 1)&&
      ((adv + GET_LEVEL(victim, lin_class)) != 1))
  {
    send_to_char("1 is the lowest possible level.\n\r", ch);
    return;
  }

  send_to_char("You feel generous.\n\r", ch);
  act("$n makes some strange gestures.\n\rA strange feeling comes upon you,"
      "\n\rLike a giant hand, light comes down from\n\rabove, grabbing your "
      "body, that begins\n\rto pulse with coloured lights from inside.\n\rYo"
      "ur head seems to be filled with daemons\n\rfrom another plane as your"
      " body dissolves\n\rinto the elements of time and space itself.\n\rSudde"
      "nly a silent explosion of light snaps\n\ryou back to reality. You fee"
      "l slightly\n\rdifferent.",FALSE,ch,0,victim,TO_VICT);
        
        
  if (GET_LEVEL(victim, lin_class) == 0)
  {
    do_start(victim);
  }
  else
  {
    if (GET_LEVEL(victim, lin_class) < IMPLEMENTOR)
    {

      gain_exp_regardless(victim, 
                          (titles[lin_class]
                           [GET_LEVEL(victim, lin_class)+adv].exp)
                          -GET_EXP(victim), lin_class);

      send_to_char("Character is now advanced.\n\r", ch);
    }
    else
    {
      send_to_char("Some idiot just tried to advance your level.\n\r", victim);
      send_to_char("IMPOSSIBLE! IDIOTIC!\n\r", ch);
    }
  }
}


void do_reroll(struct char_data *ch, const char *argument, int cmd)
{
  send_to_char("Use @ command instead.\n\r", ch);  
}


void do_restore(struct char_data *ch, const char *argument, int cmd)
{
  struct char_data *victim;
  char buf[100];
  int i;

  
  if (cmd == 0) return;
  
  only_argument(argument,buf);
  if (!*buf)
    send_to_char("Who do you wish to restore?\n\r",ch);
  else
    if(!(victim = get_char(buf)))
      send_to_char("No-one by that name in the world.\n\r",ch);
    else 
    {
      GET_MANA(victim) = GET_MAX_MANA(victim);
      GET_HIT(victim) = GET_MAX_HIT(victim);
      GET_MOVE(victim) = GET_MAX_MOVE(victim);
      
      if (IS_NPC(victim))
        return;

      if (victim->player.time.logon + victim->player.time.played<0)
      {
        victim->player.time.logon =0;
        victim->player.time.played=0;    
      }

      if (!IS_IMMORTAL(victim)) {
        GET_COND(victim,THIRST) = 24;
        GET_COND(victim,FULL) = 24;
      } else {
        GET_COND(victim,THIRST) = -1;
        GET_COND(victim,FULL) = -1;
        GET_COND(victim,DRUNK) = -1;
      }
      
      if (IS_IMMORTAL(victim))
      {
        for (i = 0; i < MAX_SKILLS; i++)
        {
          victim->skills[i].learned = 100;
          victim->skills[i].special = 1; /* specialized */
          SET_BIT(victim->skills[i].flags,SKILL_KNOWN);
          victim->skills[i].nummem =99; /* clear memorized */
        }
      }
        
      if (GetMaxLevel(victim) >= GOD)
      {
        victim->abilities.str_add = 100;
        victim->abilities.intel = 25;
        victim->abilities.wis = 25;
        victim->abilities.dex = 25;
        victim->abilities.str = 25;
        victim->abilities.con = 25;
        victim->abilities.chr = 25;
        victim->tmpabilities = victim->abilities;
      }

      /* this should give all immortals all classes and set   */
      /* all levels to the max level they are (i.e. 51 level warrior ) */

      if (GetMaxLevel(victim) > MAX_MORT)
      {
         for (i=0;i<MAX_CLASS;i++)
        {
          if (GET_LEVEL(victim,i) < GetMaxLevel(victim)) 
              GET_LEVEL(victim,i) = GetMaxLevel(victim);
        }/* for */
        for (i=1;i<=CLASS_PSI;i*=2)
        {
          if (!HasClass(victim,i))
            victim->player.iClass +=i;
        } /* for */
      } /* > MAX_MORT */

         
      update_pos( victim );
      send_to_char("Done.\n\r", ch);
      if( !IS_NPC( ch ) && !IS_SET(ch->specials.act, PLR_STEALTH) )
      {
        act("The hand of $N touches you lightly on the forehead, "
            "you have been healed!", FALSE, victim, 0, ch, TO_CHAR);
      }
    }
}




void do_noshout(struct char_data *ch, const char *argument, int cmd)
{
  struct char_data *vict;
  struct obj_data *dummy;
  char buf[MAX_INPUT_LENGTH];
  
  if (IS_NPC(ch))
    return;
  
  only_argument(argument, buf);
  
  if (!*buf)
    if (IS_SET(ch->specials.act, PLR_NOSHOUT))                {
      send_to_char("You can now hear shouts again.\n\r", ch);
      REMOVE_BIT(ch->specials.act, PLR_NOSHOUT);
    }                else                {
      send_to_char("From now on, you won't hear shouts.\n\r", ch);
      SET_BIT(ch->specials.act, PLR_NOSHOUT);
    }
  else if (!generic_find(argument, FIND_CHAR_WORLD, ch, &vict, &dummy))
    send_to_char("Couldn't find any such creature.\n\r", ch);
  else if (IS_NPC(vict))
    send_to_char("Can't do that to a beast.\n\r", ch);
  else if (GetMaxLevel(vict) >= GetMaxLevel(ch))
    act("$E might object to that.. better not.", 0, ch, 0, vict, TO_CHAR);
  else if (IS_SET(vict->specials.act, PLR_NOSHOUT) &&
           (GetMaxLevel(ch) >= SAINT))        {
    send_to_char("You can shout again.\n\r", vict);
    send_to_char("NOSHOUT removed.\n\r", ch);
    REMOVE_BIT(vict->specials.act, PLR_NOSHOUT);
  }        else if (GetMaxLevel(ch) >= SAINT)        {
    send_to_char("The gods take away your ability to shout!\n\r", vict);
    send_to_char("NOSHOUT set.\n\r", ch);
    SET_BIT(vict->specials.act, PLR_NOSHOUT);
  } else {
    send_to_char("Sorry, you can't do that\n\r",ch);
  }
}


void do_nohassle(struct char_data *ch, const char *argument, int cmd)
{
  struct char_data *vict;
  struct obj_data *dummy;
  char buf[MAX_INPUT_LENGTH];
  
  if (IS_NPC(ch))
    return;
  
  only_argument(argument, buf);
  
  if (!*buf)
  {
    if (IS_SET(ch->specials.act, PLR_NOHASSLE))
    {
      send_to_char("You can now be hassled again.\n\r", ch);
      REMOVE_BIT(ch->specials.act, PLR_NOHASSLE);
    }
    else
    {
      send_to_char("From now on, you won't be hassled.\n\r", ch);
      SET_BIT(ch->specials.act, PLR_NOHASSLE);
    }
  }
  else if (!generic_find(argument, FIND_CHAR_WORLD, ch, &vict, &dummy))
    send_to_char("Couldn't find any such creature.\n\r", ch);
  else if (IS_NPC(vict))
    send_to_char("Can't do that to a beast.\n\r", ch);
  else if (GetMaxLevel(vict) > GetMaxLevel(ch))
    act("$E might object to that.. better not.", 0, ch, 0, vict, TO_CHAR);
  else
    send_to_char("The implementor won't let you set this on mortals...\n\r",
                 ch);
  
}


void do_stealth(struct char_data *ch, const char *argument, int cmd)
{
  struct char_data *vict;
  struct obj_data *dummy;
  char buf[MAX_INPUT_LENGTH];
  
  if (IS_NPC(ch))
    return;
  
  only_argument(argument, buf);
  
  if (!*buf)
  {
    
    if (IS_SET(ch->specials.act, PLR_STEALTH))
    {
      send_to_char("STEALTH mode OFF.\n\r", ch);
      REMOVE_BIT(ch->specials.act, PLR_STEALTH);
    }
    else     
    {
      send_to_char("STEALTH mode ON.\n\r", ch);
      SET_BIT(ch->specials.act, PLR_STEALTH);
    }
  }
  else if (!generic_find(argument, FIND_CHAR_WORLD, ch, &vict, &dummy))
    send_to_char("Couldn't find any such creature.\n\r", ch);
  else if (IS_NPC(vict))
    send_to_char("Can't do that to a beast.\n\r", ch);
  else if (GetMaxLevel(vict) > GetMaxLevel(ch))
    act("$E might object to that.. better not.", 0, ch, 0, vict, TO_CHAR);
  else
    send_to_char("The implementor won't let you set this on mortals...\n\r",
                 ch);

}

void print_room(int rnum, struct room_data *rp, struct string_block *sb)
{
  char buf[MAX_STRING_LENGTH];
  int dink,bits, scan;

  extern char *sector_types[];

  if((rp->sector_type < 0) || (rp->sector_type > 9))
  { /* non-optimal */
    rp->sector_type = 0;
  }
  sprintf( buf, "%5ld %4d %-12s %s", rp->number, rnum,
           sector_types[rp->sector_type], (rp->name?rp->name:"Empty"));
  strcat(buf, " [");

  dink=0;
  for (bits=rp->room_flags, scan=0; bits; scan++)
  {
    if (bits & (1<<scan))
    {
      if (dink)
        strcat(buf, " ");
      strcat(buf, room_bits[scan]);
      dink=1;
      bits ^= (1<<scan);
    }
  }
  strcat(buf, "]\n\r");

  append_to_string_block(sb, buf);
}

void print_death_room(int rnum, struct room_data *rp, struct string_block *sb)
{
  if (rp && rp->room_flags&DEATH)
    print_room(rnum, rp, sb);
}

void print_private_room(int rnum, struct room_data *rp, struct string_block *sb)
{
  if (rp && rp->room_flags&PRIVATE)
    print_room(rnum, rp, sb);
}

struct show_room_zone_struct
{
  int        blank;
  int        startblank, lastblank;
  int        bottom, top;
  struct string_block        *sb;
};

void show_room_zone(int rnum, struct room_data *rp,
                           struct show_room_zone_struct *srzs)
{
  char buf[MAX_STRING_LENGTH];

  if (!rp || rp->number<srzs->bottom || rp->number>srzs->top)
    return; /* optimize later*/
  
  if (srzs->blank && (srzs->lastblank+1 != rp->number) )
  {
    sprintf( buf, "rooms %d-%d are blank\n\r", srzs->startblank,
             srzs->lastblank);
    append_to_string_block(srzs->sb, buf);
    srzs->blank = 0;
  }
  if (1==sscanf(rp->name, "%d", &srzs->lastblank) &&
      srzs->lastblank==rp->number)
  {
    if (!srzs->blank)
    {
      srzs->startblank = srzs->lastblank;
      srzs->blank = 1;
    }
    return;
  } 
  else if (srzs->blank)
  {
    sprintf( buf, "rooms %d-%d are blank\n\r", srzs->startblank,
             srzs->lastblank );
    append_to_string_block( srzs->sb, buf );
    srzs->blank = 0;
  }
  
  print_room(rnum, rp, srzs->sb);
}

void do_show(struct char_data *ch, const char *argument, int cmd)
{
  int zone;
  char buf[MAX_STRING_LENGTH], zonenum[MAX_INPUT_LENGTH];
  struct index_data *which_i;
  int bottom = 0, top = 0, topi;
  struct string_block sb;
  
  if (IS_NPC(ch))
    return;
  
  argument = one_argument(argument, buf);
  
  init_string_block(&sb);

  if (is_abbrev(buf, "zones"))
  {
    struct zone_data        *zd;
    int bottom = 0;
    append_to_string_block( &sb, 
                            "# Zone   name                                "
                            "lifespan age     rooms     reset\n\r" );

    for( zone = 0; zone<=top_of_zone_table; zone++ )
    {
      char *mode;

      zd = zone_table+zone;
      switch(zd->reset_mode)
      {
       case 0:
        mode = "never";
        break;
       case 1:
        mode = "ifempty";
        break;
       case 2:
        mode = "always";
        break;
       default:
        if (zd->reset_mode > 2)
        {
          if (IS_SET(zd->reset_mode, ZONE_ALWAYS))
            mode = "#always";
          else if (IS_SET(zd->reset_mode, ZONE_EMPTY))
            mode = "#empty";
          else
            mode = "#never";
          
        }
        else
        {
          mode = "!unknown!";
        }
      }
      sprintf( buf,"%4d %-40.40s %4dm %4dm %6d-%-6d %s\n\r", zone, zd->name,
               zd->lifespan, zd->age, bottom, zd->top, mode );
      append_to_string_block(&sb, buf);
      bottom = zd->top+1;
    }
    
    
  }
  else if( ( is_abbrev(buf, "objects" ) &&
           ( which_i = obj_index ) && ( topi = top_of_objt ) ) ||
           ( is_abbrev(buf, "mobiles" ) &&
           ( which_i = mob_index ) && ( topi = top_of_mobt ) ) ) 
  {
    int                objn;
    struct index_data        *oi;
    
    only_argument(argument, zonenum);
    zone = -1;
    if( sscanf( zonenum,"%i", &zone ) == 1 &&
        ( zone<0 || zone>top_of_zone_table ))
    {
      append_to_string_block(&sb, "That is not a valid zone_number\n\r");
      return;
    }
    if( zone >= 0 )
    {
      bottom = zone ? (zone_table[zone-1].top+1) : 0;
      top = zone_table[zone].top;
    }
    
    append_to_string_block(&sb, "VNUM  rnum count names\n\r");
    for( objn = 0; objn < topi; objn++ ) 
    {
      oi = which_i + objn;
      
      if( ( zone >= 0 && ( oi->iVNum < bottom || oi->iVNum > top ) ) ||
          ( zone < 0 && !isname( zonenum, oi->name ) ) )
        continue; /* optimize later*/
      
      sprintf( buf,"%5d %4d %3d  %s\n\r", oi->iVNum, objn, oi->number, 
               oi->name);
      append_to_string_block(&sb, buf);
    }
  } 
  else if (is_abbrev(buf, "rooms"))
  {
    
    only_argument(argument, zonenum);
    
    append_to_string_block(&sb, "VNUM  rnum type         name [BITS]\n\r");
    if (is_abbrev(zonenum, "death"))
    {
#if HASH
      hash_iterate(&room_db, print_death_room, &sb);
#else
      room_iterate(room_db, CASTVF_IT print_death_room, &sb);
#endif
      
    }
    else if (is_abbrev(zonenum, "private"))
    {
#if HASH
      hash_iterate(&room_db, print_private_room, &sb);
#else
      room_iterate(room_db,CASTVF_IT print_private_room, &sb);
#endif
      
    }
    else if( sscanf(zonenum,"%i", &zone) != 1 ||
             zone<0 || zone>top_of_zone_table)
    {
      append_to_string_block( &sb, 
                              "I need a zone number with this command\n\r" );
      
      
    }
    else
    {
      struct show_room_zone_struct srzs;
      
      srzs.bottom = zone ? (zone_table[zone-1].top+1) : 0;
      srzs.top = zone_table[zone].top;
      
      srzs.blank = 0;
      srzs.sb = &sb;
#if HASH
      hash_iterate(&room_db, show_room_zone, &srzs);
#else
      room_iterate(room_db, CASTVF_IT show_room_zone, &srzs);
#endif
        
      if (srzs.blank)
      {
        sprintf(buf, "rooms %d-%d are blank\n\r", srzs.startblank,
                srzs.lastblank);
        append_to_string_block(&sb, buf);
        srzs.blank = 0;
      }
    }
  } 
  else 
  {
    append_to_string_block(&sb,"Usage:\n\r"
                 "  show zones\n\r"
                 "  show (objects|mobiles) (zone#|name)\n\r"
                 "  show rooms (zone#|death|private)\n\r");
  }
  page_string_block(&sb,ch);
  destroy_string_block(&sb);
}


void do_debug(struct char_data *ch, const char *argument, int cmd)
{
  char        arg[MAX_INPUT_LENGTH];
  int        i;

  i=0;
  one_argument(argument, arg);
  i = atoi(arg);

  if (i<0 || i>2)
  {
    send_to_char("valid values are 0, 1 and 2\n\r", ch);
  } else {
#if DEBUG

/* never had this function, disabled for now */
/*    malloc_debug(i); */
    sprintf(arg, "malloc debug level set to %d\n\r", i);
#else
    sprintf(arg, "Debug level set to %d. May not be implemented\n\r", i);
#endif
    send_to_char(arg, ch);
  }
}

void do_invis(struct char_data *ch, const char *argument, int cmd)
{
  char        buf[MAX_INPUT_LENGTH];
  int        level;

  if (cmd == 242 && !IS_IMMORTAL(ch)) return;

  if (cmd != 242) {
    if (affected_by_spell(ch, SPELL_INVISIBLE)) {
      affect_from_char(ch, SPELL_INVISIBLE);
    }
    REMOVE_BIT(ch->specials.affected_by, AFF_INVISIBLE);
    ch->invis_level = 0;
    return;
  }

    one_argument( argument, buf );
    
    if (!buf[0] == '\0' )
  {
    level = atoi(buf);
    if (level<0) level=0;
    if (level>GetMaxLevel(ch))  level = GetMaxLevel(ch);

      ch->invis_level = level;
      sprintf(buf,"Invis level set to %d.\n\r", level);
      send_to_char(buf, ch);
   
  } else 
    
    if (ch->invis_level>0) 
    {
      ch->invis_level = 0;
      send_to_char("You are now totally visible.\n\r",ch);
    } else 
    {
      ch->invis_level = LOW_IMMORTAL;
      send_to_char("You are now invisible to all but gods.\n\r",ch);
    }

}

void do_create( struct char_data *ch, const char *argument, int cmd)
{
  int i, count, start, end;

  if (!IS_IMMORTAL(ch) || IS_NPC(ch)) {
    return;
  }


  count = sscanf(argument, "%d %d", &start, &end);
  if (count < 2) {
    send_to_char(" create <start> <end>\n\r", ch);
    return;
  }
  if (start > end) {
    send_to_char(" create <start> <end>\n\r", ch);
    return;
  }

  send_to_char("You form much order out of Chaos\n\r", ch);
  for (i = start; i<= end; i++) {
    if (!real_roomp(i))
    CreateOneRoom(i);
  }

}


void CreateOneRoom( int loc_nr)
{
  struct room_data *rp;
  extern int top_of_zone_table;

  char buf[256];

  allocate_room(loc_nr);
  rp = real_roomp(loc_nr);
  bzero(rp, sizeof(*rp));
  
  rp->number = loc_nr;
  if (top_of_zone_table >= 0) {
    int        zone;
    
    for (zone=0;
         rp->number > zone_table[zone].top && zone<=top_of_zone_table;
         zone++);
    if (zone > top_of_zone_table) {
      fprintf(stderr, 
              "Room %ld is outside of any zone.\n", rp->number);
      zone--;
    }
    rp->zone = zone;
  }
  sprintf(buf, "%d", loc_nr);
  rp->name = (char *)strdup(buf);
  rp->description = (char *)strdup("Empty\n");
}

void do_set_log(struct char_data *ch, const char *arg, int cmd)
{
 char name[255];
 struct char_data *victim;
 struct obj_data *dummy;

 if(IS_NPC(ch))
   return;

 only_argument(arg, name);
 if(!*name) {
   send_to_char("Usage:log <character>\n\r", ch);
   return;
 }

 if(!generic_find(arg, FIND_CHAR_WORLD, ch, &victim, &dummy)) {
   send_to_char("No such person in the world.\n\r", ch);
   return;
 }

 if(IS_NPC(victim)) {
   send_to_char("Victim is an NPC.\n\r", ch);
   return;
 }

 if(IS_AFFECTED2(victim, AFF2_LOG_ME)) {
   REMOVE_BIT(victim->specials.affected_by2, AFF2_LOG_ME);
   send_to_char("Log Bit Removed.\n\r", ch);
   return;
 }

 else {
   SET_BIT(victim->specials.affected_by2, AFF2_LOG_ME);
   send_to_char("Log Bit Set.\n\r", ch);
   return;
 }
}

void PulseMobiles(int cmd);

void do_event(struct char_data *ch, const char *arg, int cmd)
{
  int i;
  char buf[255];

  only_argument(arg, buf);
  if(IS_NPC(ch))
    return;

  if (!*buf)
  {
    send_to_char("Event what? (event <mobnum>)\r\n",ch);
    return;
  }

  i = atoi(buf);

  PulseMobiles( i );
}

void do_beep(struct char_data *ch, const char *argument, int cmd)
{
 char buf[255], name[255];
 struct char_data *victim;
 struct obj_data *dummy;

 if(IS_NPC(ch))
   return;

 only_argument(argument, name);
 if(!*name) {
   if(IS_SET(ch->specials.act, PLR_NOBEEP)) {
      send_to_char("Beep now ON.\n\r", ch);
      REMOVE_BIT(ch->specials.act, PLR_NOBEEP);
    }
   else {
      send_to_char("Beep now OFF.\n\r", ch);
      SET_BIT(ch->specials.act, PLR_NOBEEP);
    }
   return;
 }

 if(!generic_find(argument, FIND_CHAR_WORLD, ch, &victim, &dummy)) {
   send_to_char("No such person in the world.\n\r", ch);
   return;
 }

 if(IS_NPC(victim)) {
   send_to_char("Victim is an NPC.\n\r", ch);
   return;
 }

 if(IS_SET(victim->specials.act, PLR_NOBEEP)) {
   sprintf(buf, "%s can not be beeped right now.\n\r", GET_NAME(victim));
   send_to_char(buf, ch);
   return;
 }

 else {
   sprintf(buf, "%c%s is beeping you.\n\r", 7, GET_NAME(ch));
   send_to_char(buf, victim);
   sprintf(buf, "%s has been beeped.\n\r", GET_NAME(victim));
   send_to_char(buf, ch);
   return;
 }
}

void do_cset(struct char_data *ch, const char *arg, int cmd)
{
 char buf[1000], buf1[255], buf2[255], buf3[255], buf4[255];
 int i, radix;
 NODE *n;
 extern struct radix_list radix_head[];
 extern byte HashTable[];
 
 if(IS_NPC(ch))
    return;
 
 arg = one_argument(arg, buf1);
 arg = one_argument(arg, buf2);
 arg = one_argument(arg, buf3);
 arg = one_argument(arg, buf4);
 
 i = atoi(buf4);
 
 if(!strcmp(buf1, "show")) {
    radix = HashTable[ (int)(*buf2) ];
    if(!radix_head[radix].next) {
       send_to_char("Sorry, command not found.\n\r", ch);
       return;
     }
 
    n = SearchForNodeByName(radix_head[radix].next, buf2, strlen(buf2));
    if(!n) {
       send_to_char("Sorry, command not found.\n\r", ch);
       return;
     }
    sprintf(buf, "Name: %s\n\rMinimum Position: %d\n\rMinimum Level: %d\n\rNumber: %d\n\rLog Bit: %s\n\r",
            n->name, n->min_pos, n->min_level, n->number, (n->log ? "On" : "Off"));
    send_to_char(buf, ch);
    return;
  }
 
 else if(!strcmp(buf1, "set")) {
    radix = HashTable[ (int)(*buf2) ];
    if(!radix_head[radix].next) {
       send_to_char("Sorry, command not found.\n\r", ch);
       return;
     }
 
    n = SearchForNodeByName(radix_head[radix].next, buf2, strlen(buf2));
    if(!n) {
       send_to_char("Sorry, command not found.\n\r", ch);
       return;
     }
 
    if(!strcmp(buf3, "level")) {
       if(i < 0 || i > 60) {
          send_to_char("Level must be between 0 and 60.\n\r", ch);
          return;
        }
    
       n->min_level = i;
       send_to_char("Level Changed.\n\r", ch);
       return;
     }
 
    if(!strcmp(buf3, "position")) {
       if(i < 0 || i > 10) {
          send_to_char("Position must be between 0 and 10.\n\r", ch);
          return;
        }
    
       n->min_pos = i;
       send_to_char("Position Changed.\n\r", ch);
       return;
     }
 
  }

 
 else if(!strcmp(buf1, "log")) {
    radix = HashTable[ (int)(*buf2) ];
    if(!radix_head[radix].next) {
       send_to_char("Sorry, command not found.\n\r", ch);
       return;
     }
 
    n = SearchForNodeByName(radix_head[radix].next, buf2, strlen(buf2));
    if(!n) {
       send_to_char("Sorry, command not found.\n\r", ch);
       return;
     }

     if(n->log) {
        send_to_char("Command logging removed.\n\r", ch);
        n->log = 0;
        return;
     }
     else {
        send_to_char("Command logging set.\n\r", ch);
        n->log = 1;
        return;
     } } 
 send_to_char("Usage: # <\"set\" | \"show\" | \"log\"> <cmd> <\"level\" | \"position\"> <level>\n\r", ch);
 return;
}

/* Stolen from Merc21 code. */

void do_disconnect(struct char_data *ch, const char *argument, int cmd )
{
    char arg[255];
    struct descriptor_data *d;
    struct char_data *victim; 

if (IS_NPC(ch))
        return;
        
    one_argument( argument, arg );
    if ( arg[0] == '\0' )
    {
        send_to_char( "Disconnect whom?(discon <name>)\n\r", ch );
        return;
    }
    if(!(victim = get_char(arg))) 
    {
      send_to_char("No-one by that name in the world.\n\r",ch);
      return;
    } 

    if ( victim->desc == NULL )
    {
        act("$N doesn't have a descriptor.", 0, ch, 0, victim, TO_CHAR);
        return;
    }

    for ( d = descriptor_list; d != NULL; d = d->next )
    {
        if ( d == victim->desc )
        {
            close_socket( d );
            send_to_char( "Ok.\n\r", ch );
            return;
        }
    }

    mudlog( LOG_ERROR, "Descriptor not found, do_disconnect");
    send_to_char( "Descriptor not found!\n\r", ch );
    return;
}

/* From Merc21 Code, added by msw */
void do_freeze(struct char_data *ch, const char *argument , int cmd)
{
    char arg[MAX_STRING_LENGTH];
    struct char_data *victim;

if (IS_NPC(ch))
        return;
        
    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
        send_to_char( "Freeze whom?(freeze <name>)\n\r", ch );
        return;
    }

    if(!(victim = get_char(arg))) 
    {
      send_to_char("No-one by that name in the world.\n\r",ch);
      return;
     } 

    if (!IS_PC(victim) && !IS_SET(victim->specials.act,ACT_POLYSELF))
    {
        send_to_char( "Not on NPC's.\n\r", ch );
        return;
    }

  if (GetMaxLevel(victim)>=GetMaxLevel(ch))
    {
     send_to_char("You can't freeze them!\n\r",ch);
     return;
    } else
    {
     if ( IS_SET(victim->specials.act, PLR_FREEZE) )
     {
         REMOVE_BIT(victim->specials.act, PLR_FREEZE);
         send_to_char( "You can play again.\n\r", victim );
         send_to_char( "FREEZE removed.\n\r", ch );
     }
     else
     {
         SET_BIT(victim->specials.act, PLR_FREEZE);
         send_to_char( "You can't do ANYthing!\n\r", victim );
         send_to_char( "FREEZE set.\n\r", ch );
     }
    do_save(victim,"",0);
    return;
   } /* higher than presons level */
}

/* Added by msw, to drain levels of morts/immos */

void do_drainlevel(struct char_data *ch, const char *argument , int cmd)
{
    char arg[MAX_STRING_LENGTH];
    char buf[MAX_STRING_LENGTH];
    int numtolose, i;
    struct char_data *victim;
    

 argument = one_argument( argument, arg );       /* victim name */
 sscanf(argument,"%d",&numtolose);  /* levels to drain */

            
    if ( arg[0] == '\0' )
    {
     send_to_char( "Drain levels from whom? (drain <name> <numbertodrain>)\n\r", ch );
     return;
    }

            
    if(!(victim = get_char(arg))) 
    {
      send_to_char("No-one by that name in the world.\n\r",ch);
      return;
     } 
           
    if ( IS_NPC(victim) )
    {
        send_to_char( "Not on NPC's.\n\r", ch );
        return;
    }
            
  if (GetMaxLevel(victim)>=GetMaxLevel(ch))
    {
                
     send_to_char("You can't drain them!!\n\r",ch);
     sprintf(buf,"%s tried to drain levels from you!\n\r",GET_NAME(ch));
     send_to_char(buf,victim);
     return;
    } else
     {
            
     /* do it here! */

send_to_char(
"You are struck by a black beam from above, it hurts!\n\rThe life force from your body fades and you feel yourself lose\n\rmemories of old times and battles.\n\rThe feeling fades and you shiver at a cold gust of wind.\n\r",victim);

   sprintf(buf,"You drain %d level(s) How Evil!\n\r",numtolose);
   send_to_char(buf,ch);
      
      for (i=0; i<=numtolose-1; i++)
      {
      if (GetMaxLevel(victim) <=1)
      {
        i=numtolose;
        send_to_char("\n\rTried to lower them below 1, can't do that.\n\r",ch);
      } else /* if GetmaxLevel */ 
       {
        drop_level(victim,BestClassBIT(victim),TRUE);     
        send_to_char(".",ch);
       } /* else GetMaxLevel */
      } /* for */
      
     /* all done, save the mess! */
      send_to_char("\n\rOk.\r\n",ch);
      do_save(victim,"",0);
      return;
     }
}

void do_god_interven(struct char_data *ch, const char *argument , int cmd)
{
  char arg[128];
 
  argument= one_argument(argument,arg);
 
  if (!*arg) 
  {
    send_to_char( "Eh? What do you wanna intervene upon?\n\r",ch);
    send_to_char( "interven [type] (Type=portal,summon,astral,kill,logall,"
                  "eclipse,dns,logmob)\n\r\n\r",ch);
    return;
  }

  if (!strcmp("eclipse",arg)) 
  {
    if (IS_SET(SystemFlags,SYS_ECLIPS)) 
    {
      REMOVE_BIT(SystemFlags,SYS_ECLIPS);
      send_to_char( "You part the planets and the sun shines through!\n",ch);
      send_to_outdoor( "The planets return to thier normal orbit, slowly the "
                       "light will returns.\n");
      mudlog( LOG_PLAYERS, "The world is enlightend");
    }
    else 
    {
      SET_BIT(SystemFlags,SYS_ECLIPS);
      weather_info.sunlight=SUN_DARK;
      switch_light(SUN_DARK);
      send_to_char("You summon the planets and force an eclipse!\n",ch);
      send_to_outdoor( "The planets eclipse and hide the sun spreading "
                       "darkness through out the land!\n");
      mudlog( LOG_PLAYERS, "World has been darkened");
    }  
  }
  else if (!strcmp("req",arg)) 
  {
    if (!IS_SET(SystemFlags,SYS_REQAPPROVE)) 
    {
      SET_BIT(SystemFlags,SYS_REQAPPROVE);
      send_to_char("Newbie character approval required.\n\r",ch);
      mudlog( LOG_PLAYERS, "New character approval REQUIRED");
    }
    else 
    {
      REMOVE_BIT(SystemFlags,SYS_REQAPPROVE);
      send_to_char("Newbie character approval REMOVED.\n\r",ch);
      mudlog( LOG_PLAYERS, "New character approval REMOVED");
    }
  }
  else if (!strcmp("color",arg)) 
  {
    if (!IS_SET(SystemFlags,SYS_NOANSI)) 
    {
      SET_BIT(SystemFlags,SYS_NOANSI);
      send_to_char("Color codes disabled world wide.\n\r",ch);
      mudlog( LOG_PLAYERS, "Global colors disabled");
    }
    else 
    {
      REMOVE_BIT(SystemFlags,SYS_NOANSI);
      send_to_char("Color codes enabled for everyone that uses them.\n\r",ch);
      mudlog( LOG_PLAYERS, "Global colors enabled" );
    }
  }
  else if (!strcmp("dns",arg)) 
  {
    if (IS_SET(SystemFlags,SYS_SKIPDNS)) 
    {
      REMOVE_BIT(SystemFlags,SYS_SKIPDNS);
      send_to_char("Domain name searches enabled.\n\r",ch);
      mudlog( LOG_PLAYERS, "DNS Enabled");
    }
    else 
    {
      SET_BIT(SystemFlags,SYS_SKIPDNS);
      send_to_char("Domain name searches Disabled.\n\r",ch);
      mudlog( LOG_PLAYERS, "DNS Disabled");
    }
  }
  else if (!strcmp("portal",arg)) 
  {
    if (IS_SET(SystemFlags,SYS_NOPORTAL)) 
    {
      REMOVE_BIT(SystemFlags,SYS_NOPORTAL);
      send_to_char("You sort out the planes and allow portaling.\n",ch);
      mudlog( LOG_PLAYERS, "Portaling enabled");
    }
    else 
    {
      SET_BIT(SystemFlags,SYS_NOPORTAL);
      send_to_char( "You scramble the planes to make portaling impossible.\n",
                    ch);
      mudlog( LOG_PLAYERS, "Portaling disabled");
    }
  }
  else if (!strcmp("astral",arg)) 
  {
    if (IS_SET(SystemFlags,SYS_NOASTRAL)) 
    {
      REMOVE_BIT(SystemFlags,SYS_NOASTRAL);
      send_to_char("You shift the planes and allow astral travel.\n",ch);
      mudlog( LOG_PLAYERS, "Astral enabled");
    }
    else 
    {
      SET_BIT(SystemFlags,SYS_NOASTRAL);
      send_to_char( "You shift the astral planes and make astral travel "
                    "impossible.\n",ch);
      mudlog( LOG_PLAYERS, "Astral disabled");
    }
  }
  else if (!strcmp("summon",arg)) 
  {
    if (IS_SET(SystemFlags,SYS_NOSUMMON)) 
    {
      REMOVE_BIT(SystemFlags,SYS_NOSUMMON);
      send_to_char("You clear the fog to enable summons.\n",ch);
      mudlog( LOG_PLAYERS, "Summons enabled");
    }
    else 
    {
      SET_BIT(SystemFlags,SYS_NOSUMMON);
      send_to_char( "A magical fog spreads throughout the land making "
                    "summons impossible.\n", ch);
      mudlog( LOG_PLAYERS, "Summons disabled");
    }
  }
  else if (!strcmp("kill",arg)) 
  {
    if (IS_SET(SystemFlags,SYS_NOKILL)) 
    {
      REMOVE_BIT(SystemFlags,SYS_NOKILL);
      send_to_char( "You let the anger lose inside you and the people of the "
                    "land fight.\n",ch);
      mudlog( LOG_PLAYERS, "Killing enabled");
    }
    else 
    {
      SET_BIT(SystemFlags,SYS_NOKILL);
      send_to_char( "You spread thoughts of peace throught the people of "
                    "the land.\n",ch);
      mudlog( LOG_PLAYERS, "Killing disabled");
    }
  }
  else if (!strcmp("logall",arg)) 
  {
    if (IS_SET(SystemFlags,SYS_LOGALL)) 
    {
      REMOVE_BIT(SystemFlags,SYS_LOGALL);
      send_to_char( "You fire the scribe writting the history for poor "
                    "workmanship.\n\r",ch);
      mudlog( LOG_PLAYERS, "Logging all disabled.");
    }
    else 
    {
      SET_BIT(SystemFlags,SYS_LOGALL);
      send_to_char( "You hire a scribe to write the history of the world.\n\r",
                    ch);
      mudlog( LOG_PLAYERS, "Logging all enabled");
    }
  }
  else if (!strcmp("logmob",arg)) 
  {
    if (IS_SET(SystemFlags,SYS_LOGMOB)) 
    {
      REMOVE_BIT(SystemFlags,SYS_LOGMOB);
      send_to_char( "You fire the scribe writting the mobs' history for poor "
                    "workmanship.\n\r",ch);
      mudlog( LOG_PLAYERS, "Logging mobs disabled.");
    }
    else 
    {
      SET_BIT(SystemFlags,SYS_LOGMOB);
      send_to_char("You hire a scribe to write the mobs' history.\n\r",ch);
      mudlog( LOG_PLAYERS, "Logging mobs enabled");
    }
  }
  else
    send_to_char("Godly powers you have, but how do you wanna use them?\n",ch);    
}

void do_nuke(struct char_data *ch, const char *argument , int cmd)
{
#if !defined( EMANUELE )
  int i;
  struct char_file_u ch_st;
  FILE *char_file;
#endif
  struct char_data *victim;
  char buf[254], arg[254];

  if (IS_NPC(ch))
    return;
              
  argument = one_argument( argument, arg );       /* victim name */

            
  if ( arg[0] == '\0' )    
  {
    send_to_char( "Nuke whom?! (nuke <name>)\n\r", ch );
    return;
  }

  if(!(victim = get_char(arg)))     
  {
    send_to_char("No-one by that name in the world.\n\r",ch);
    return;
  } 
           
  if( victim->in_room != ch->in_room) 
  {
    send_to_char("That person is not in the same room as you.\n\r",ch);
    return;        
  }

  if( IS_NPC(victim) )    
  {
    send_to_char( "Not on NPC's.\n\r", ch );
    return;
  }
            
  if( GetMaxLevel( victim ) >= GetMaxLevel( ch ) )   
  {
    send_to_char("You can't nuke them!!\n\r",ch);
    sprintf(buf,"%s tried to nuke you!\n\r",GET_NAME(ch));
    send_to_char(buf,victim);
    return;
  }
  else  
  {
    mudlog( LOG_PLAYERS,"%s just nuked %s!",GET_NAME(ch),GET_NAME(victim));
    act( "$n calls forth the wrath of the gods and destroys $N!", FALSE, ch, 0,
         victim, TO_NOTVICT );
    act( "$n reaches into $N and pulls out a fighting soul!", FALSE, ch, 0,
         victim, TO_NOTVICT );
    act( "$N dies quickly without much a fight.", FALSE, ch, 0, victim,
         TO_NOTVICT );
    act( "$n reaches into your chest and pulls your soul out, you die forever!",
         FALSE, ch, 0, victim, TO_VICT );
    act( "You rip the heart and soul from $N condeming $M to instant death.",
         FALSE, ch, 0, victim, TO_CHAR );
#if defined( EMANUELE )
    sprintf( buf, "rm %s/%s.dat", PLAYERS_DIR, lower( GET_NAME( victim ) ) );
    system( buf );
#else
    for( i = 0; i <= top_of_p_table; i++ )
    {
      if( !str_cmp( ( player_table + i )->name, GET_NAME(victim)))
      {
        free((player_table +i)->name);
        (player_table +i)->name = (char *)malloc(strlen("111111"));
        strcpy((player_table +i)->name, "111111");
        break;
      }
    }
    /* get the structure from player_table[i].nr */
    if (!(char_file = fopen(PLAYER_FILE, "r+"))) 
    {
      perror("Opening player file for updating. act.wizard.c");
      assert(0);
    }
    fseek( char_file, (long) (player_table[i].nr * sizeof(struct char_file_u)),
           0 );

    /* read in the char, change the name, write back */
    fread( &ch_st, sizeof(struct char_file_u), 1, char_file );
    sprintf(ch_st.name,"111111");
    fseek( char_file, (long) (player_table[i].nr * sizeof(struct char_file_u)), 
           0);
    fwrite(&ch_st, sizeof(struct char_file_u), 1, char_file);
    fclose(char_file);
    close_socket(victim->desc);
#endif

    do_purge( ch, GET_NAME( victim ), 0 );
    send_to_char( "Nuked.\n\r", ch );
  }
}

void do_force_rent( struct char_data *ch, const char *argument , int cmd )
{
  char arg[MAX_STRING_LENGTH];
  struct char_data *victim;


  one_argument( argument, arg );

  if( arg[0] == '\0' )    
  {
    send_to_char( "Force rent whom? (forcerent <name>|<alldead>)\n\r", ch );
    return;
  }

  if( !strcmp( arg, "alldead" ) ) 
  {
    for( victim = character_list; victim; victim = victim->next) 
    {
      if( IS_LINKDEAD(victim) && !IS_SET(victim->specials.act,ACT_POLYSELF) )
      {
          if( GetMaxLevel(victim) >= GetMaxLevel(ch) )
        {
          if( CAN_SEE(ch,victim) )
            send_to_char("You can't forcerent them!\n\r",ch);
        }
        else  
        {
          struct obj_cost cost;
     
          if( victim->in_room != NOWHERE )
            char_from_room(victim);

          char_to_room(victim, 4);
          if( victim->desc )
            close_socket(victim->desc);

          victim->desc = 0;
          if( recep_offer(victim, NULL, &cost,TRUE)) 
          {
            cost.total_cost = 100;
                  save_obj(victim, &cost,1);   
          }
          else 
          {
            mudlog( LOG_PLAYERS, 
                    "%s had a failed recp_offer, they are losing EQ!",
                    GET_NAME(victim) );
          }
          extract_char(victim); 
        } /* higher than presons level */
      } /* was linkdead */
    } /* end for */
    return;
  } /* alldead */
   
  if( !( victim = get_char(arg) ) )
  {
    send_to_char("No-one by that name in the world.\n\r",ch);
    return;
  } 

  if( IS_NPC(victim) )    
  {
    send_to_char( "Not on NPC's.\n\r", ch );
    return;
  }

  if (GetMaxLevel(victim)>=GetMaxLevel(ch))    
  {
    send_to_char("You can't forcerent them!\n\r",ch);
    return;
  }
  else    
  {
    struct obj_cost cost;
     
    if (victim->in_room != NOWHERE)
      char_from_room(victim);

    char_to_room(victim, 4);
    if (victim->desc) 
      close_socket(victim->desc);
    victim->desc = 0;
    if (recep_offer(victim, NULL, &cost,TRUE)) 
    {
      cost.total_cost = 100;
      save_obj( victim, &cost,1 );
    }
    else 
    {
      mudlog( LOG_PLAYERS, "%s had a failed recp_offer, they are losing EQ!",
              GET_NAME(victim) );
    }
    extract_char(victim); 
    return;
  } /* higher than presons level */
}

void do_ghost(struct char_data *ch, const char *argument , int cmd) 
{
  extern int plr_tick_count;
  char find_name[80];
#if !defined( EMANUELE )
  int player_i;
#endif
  struct char_file_u tmp_store;
  struct char_data *tmp_ch,*vict;


  one_argument( argument, find_name );

  if ( find_name[0] == '\0' )    
  {
    send_to_char( "Ghost play who?? (ghost <name>)\n\r", ch );
    return;
  }

  if( ( vict = get_char( find_name ) ) )     
  {
    send_to_char("Person is online, cannot control the living.\n\r",ch);
    return;
  } 

#if defined( EMANUELE )
  if( load_char( find_name, &tmp_store ) )
#else
  if( ( player_i = load_char( find_name, &tmp_store ) ) > -1 )
#endif
  {
    CREATE( tmp_ch, struct char_data, 1 );
    clear_char( tmp_ch );
    store_to_char( &tmp_store, tmp_ch );

    reset_char( tmp_ch ); 
    load_char_objs( tmp_ch );
    save_char( tmp_ch, AUTO_RENT );
    tmp_ch->next = character_list;
    character_list = tmp_ch;
    tmp_ch->specials.tick = plr_tick_count++;
    if( plr_tick_count == PLR_TICK_WRAP )
      plr_tick_count = 0;

    char_to_room( tmp_ch, ch->in_room );
    tmp_ch->desc = NULL;

    act( "$n calls forth the soul of $N and they come.", FALSE, ch, 0, tmp_ch,
         TO_ROOM );        
    act( "The soul of $N rises forth from the mortal lands.", FALSE, ch, 0,
         tmp_ch,TO_ROOM );

    act( "You call forth the soul of $N.", FALSE, ch, 0, tmp_ch, TO_CHAR );
    send_to_char( "Be sure to forcerent them when done!\n\r", ch );
  } 
  else
    send_to_char("That person does not exist.\n\r",ch);
}




void do_mforce(struct char_data *ch, const char *argument, int cmd)
{
  struct char_data *vict;
  char name[100], to_force[100],buf[100];
 
  if (IS_NPC(ch) && (cmd != 0))
    return;
 
  half_chop(argument, name, to_force);
 
  if (!*name || !*to_force)
    send_to_char("Who do you wish to force to do what?\n\r", ch);
  else if (str_cmp("all", name)) {
    if (!(vict = get_char_room_vis(ch, name)))
      send_to_char("No-one by that name here..\n\r", ch);
    else {
      if (IS_PC(vict)) {
           send_to_char("Oh no you don't!!\n\r", ch);
        }  else {
        sprintf(buf, "$n has forced you to '%s'.", to_force);
        act(buf, FALSE, ch, 0, vict, TO_VICT);
        send_to_char("Ok.\n\r", ch);
        command_interpreter(vict, to_force);
      }
    }
  } else { /* force all */
    for (vict = real_roomp(ch->in_room)->people; vict; vict=vict->next_in_room) {
      if (vict != ch && !IS_PC(vict)) {
          sprintf(buf, "$n has forced you to '%s'.", to_force);
          act(buf, FALSE, ch, 0, vict, TO_VICT);
          command_interpreter(vict, to_force);
        }
      }
    send_to_char("Ok.\n\r", ch);
  }
}

struct obj_data *clone_obj(struct obj_data *obj)
{
  struct obj_data *ocopy = NULL;
  
  if( obj->item_number >= 0 )
  {
    ocopy = read_object(obj->item_number, REAL);
    /* clear */
    if (ocopy->name)
      free(ocopy->name);
    if (ocopy->short_description)
      free(ocopy->short_description);
    if (ocopy->description)
      free(ocopy->description);
    /* copy */
    if(obj->name)    
      ocopy->name=strdup(obj->name);
    if(obj->short_description)
      ocopy->short_description=strdup(obj->short_description);
    if(obj->description)
      ocopy->description=strdup(obj->description);
  }
  
  return ocopy;
}
 
void clone_container_obj( struct obj_data *to, struct obj_data *obj )
{
  struct obj_data *tmp,*ocopy;
  
  for( tmp = obj->contains; tmp; tmp = tmp->next_content ) 
  {
    if( ( ocopy = clone_obj(tmp) ) != NULL )
    {      
      if(tmp->contains)
        clone_container_obj(ocopy,tmp);
      obj_to_obj(ocopy,to);
    }    
  }
}
 
void do_clone(struct char_data *ch, const char *argument, int cmd)
{
  struct char_data *mob, *mcopy;
  struct obj_data *obj, *ocopy;
  char type[100], name[100],buf[100];
  int j, i, count, where;
 
  if (IS_NPC(ch))
    return;
 
  argument = one_argument(argument, type);
  if( !*type )
  {
    send_to_char("uso: Clone <mob/obj> nome [count]\r\n",ch);
    return;
  }
  argument = one_argument(argument, name);
  if(!*name) 
  {
    send_to_char("uso: Clone <mob/obj> nome [count]\r\n",ch);
    return;
  }
  argument = one_argument(argument, buf);
  if(!*buf)
    count=1;
  else
    count=atoi(buf);
  if(!count || count >20)
  {
    send_to_char("Il numero di cloni deve essere inferiore a 20.\r\n",ch);
    return;
  }
 
  if(is_abbrev(type,"mobile")) 
  {
    if ((mob = get_char_room_vis(ch,name)) == 0) 
    {
      send_to_char("Non riesci a trovare quella creatura.\r\n",ch);
      return;
    }
    if(IS_PC(mob)) 
    {
      send_to_char("Clonare un PC, hahahaha\t\n",ch);
      send_to_char(GET_NAME(ch), mob);
      send_to_char(" ha appena provato a clonarti...", mob);
      return;
    }
    if(mob->nr<0) 
    {
      send_to_char("Non puoi clonarlo\r\n",ch);
      return;
    }
    for(i=0;i<count;i++) 
    {
      mcopy = read_mobile(mob->nr, REAL);
      /* clear */
      if(mcopy->player.name)
        free(mcopy->player.name);
      if(mcopy->player.short_descr)
        free(mcopy->player.short_descr);
      if(mcopy->player.long_descr)
        free(mcopy->player.long_descr);
      if(mcopy->player.description)
        free(mcopy->player.description);
      /* copy */
      if(mob->player.name)
        mcopy->player.name=strdup(mob->player.name);
      if(mob->player.short_descr)
        mcopy->player.short_descr=strdup(mob->player.short_descr);
      if(mob->player.long_descr)
        mcopy->player.long_descr=strdup(mob->player.long_descr);
      if(mob->player.description)
        mcopy->player.description=strdup(mob->player.description);
 
      /* clone EQ equiped */
      if (mob->equipment)
      {
        for (j=0;j<MAX_WEAR;j++) 
        {
          if (mob->equipment[j]) 
          {
            /* clone mob->equipment[j] */
            if( ( ocopy = clone_obj(mob->equipment[j] ) ) != NULL )
            {
              if(mob->equipment[j]->contains) 
              {
                clone_container_obj(ocopy,mob->equipment[j]);
              }
              equip_char(mcopy,ocopy,j);
            }            
          }
        }
      }
      
      /* clone EQ carried */
      if( mob->carrying )
      {        
        for( obj = mob->carrying; obj; obj = obj->next_content )
        {
          if( ( ocopy = clone_obj(obj) ) != NULL )
          {            
            if(obj->contains)
              clone_container_obj(ocopy,obj);
            /* move obj to cloned mobs carrying */
            obj_to_char(ocopy, mcopy);
          }          
        } /* end for */
      }      
 
      /* put */
      char_to_room(mcopy, ch->in_room);
      act("$n ha clonato $N!", FALSE, ch, 0, mob, TO_ROOM);
      act("Hai clonato $N.", FALSE, ch, 0, mob, TO_CHAR);
    } /* end mob clone for */
 
  }
  else if( is_abbrev( type,"object" ) )
  {
    if( ( obj = get_obj_in_list_vis( ch, name, ch->carrying ) ) )
      where=1;
    else if( ( obj = get_obj_in_list_vis( ch, name, 
                      real_roomp( ch->in_room )->contents ) ) )
      where=2;
    else 
    {
      send_to_char("Non riesci a trovare quell'oggetto.\r\n",ch);
      return;
    }
    if(obj->item_number<0) 
    {
      send_to_char("Non puoi clonarlo.\r\n",ch);
      return;
    }
    if( GetMaxLevel(ch) < IMPLEMENTOR )
    {
      switch( obj_index[ obj->item_number ].iVNum )
      {
       case 1600:
        send_to_char( "Oh no! Basta ebony kris. C'e il kris d'ebano "
                      "ora!\n\r", ch );
        return;
       case 5311:
        send_to_char( "Oh no! Basta mercury's boots. Ci sono gli stivali di "
                      "Mercurio ora!\n\r", ch );
        return;
       case 13702:
        send_to_char( "Oh no! Basta lion skin. C'e la pelle del Leone "
                      "ora!\n\r", ch );
        return;
      }
    }
    for( i = 0; i < count; i++ ) 
    {
      ocopy = clone_obj(obj);
      if(obj->contains)
        clone_container_obj(ocopy,obj);
      /* put */
      if(where==1)
        obj_to_char(ocopy, ch);
      else
        obj_to_room(ocopy, ch->in_room);
      act("$n ha clonato $p!", FALSE, ch, obj, 0, TO_ROOM);
      act("Hai clonato $p.", FALSE, ch, obj, 0, TO_CHAR);
    }
  } 
  else 
  {
    send_to_char("uso: Clone <mob/obj> nome [count]\r\n",ch);
    return;
  }
  return;
}
 
void do_viewfile(struct char_data *ch, const char *argument, int cmd)
{
  char namefile[20];
  char bigbuf[32000];
  extern char motd[MAX_STRING_LENGTH];
  extern char wmotd[MAX_STRING_LENGTH];

  /*    extern char titlescreen[MAX_STRING_LENGTH]; */

  only_argument(argument, namefile);
  if(!strcmp(namefile,"bug"))
    file_to_string(BUG_FILE,bigbuf);
  else if(!strcmp(namefile,"idea"))
    file_to_string(IDEA_FILE,bigbuf);
  else if(!strcmp(namefile,"typo"))
    file_to_string(TYPO_FILE,bigbuf);
  else if(!strcmp(namefile,"motd"))
  {
    page_string(ch->desc,motd,0);
    return;
  }
#if 0
  else if(!strcmp(namefile,"title"))
  {
    page_string(ch->desc,titlescreen,0);
    return;
  }
#endif    
  else if(!strcmp(namefile,"wmotd"))
  {
    page_string(ch->desc,wmotd,0);
    return;
  }
  else
  {
    send_to_char("Commands: view <bug|typo|idea|motd|wmotd>.\n\r", ch);
    return;
  }

  page_string(ch->desc,bigbuf,1);
}

void do_osave(struct char_data *ch, const char *argument, int cmd)
{
  FILE *f;
  struct obj_data *obj;
  char oname[128],field[120],buf[254];
  long vnum = -1;
  

  if (IS_NPC(ch) || GetMaxLevel(ch) < LOW_IMMORTAL)
  {
    return;
  }
 
  argument=one_argument(argument,oname);
  if (!*oname)
  {
    send_to_char("Osave <object name> <new_vnum>\n\r",ch);
    return;
  }

  argument=one_argument(argument,field);
  if(!*field)
  {
    send_to_char("osave <object name> <vnum>\n\r", ch);
    return;
  }

  if (!(obj=get_obj_vis_accessible(ch,oname)))
  {
    send_to_char("Hum, I do not know where that is?!?!?\n\r",ch);
    return;
  }
  
  vnum=atoi(field);
  if (vnum <1 || vnum >99999)
  {
    send_to_char("Invalid object number\n\r",ch);
    return;
  }

  /* check for valid VNUM in this zone */    

  if (GetMaxLevel(ch) <= GOD && GET_ZONE(ch)==0)
  {
    send_to_char("Sorry, you do not have access to a zone.\n\r",ch);
    return;
  }
 
  if (GetMaxLevel(ch) <= GOD)
  {
    long start,end;
    start=GET_ZONE(ch) ?(zone_table[GET_ZONE(ch)-1].top+1) : 0;
    end=zone_table[GET_ZONE(ch)].top;
    if (vnum > end)
    {
      send_to_char("VNum is larger than your zone allows.\n\r",ch);
      return;
    }
    if (vnum < start)
    {
      send_to_char("VNUM is smaller than your zone allows.\n\r",ch);
      return;
    }        
  }
  
  sprintf(buf,"objects/%ld",vnum);
  if ((f = fopen(buf,"wt")) == NULL)
  {
    send_to_char("Can't write to disk now..try later.\n\r",ch);
    return;
  }

  write_obj_to_file(obj,f);
  fclose(f);
  
  /* check for valid VNUM period */
  if (real_object(vnum) != -1)
  {
    send_to_char("WARNING: Vnum already in use, OVER-WRITING\n\r",ch);
  }
  
  InsertObject( obj, vnum );
  
  sprintf( buf, "Object %s saved as vnum %ld\n\r", obj->name, vnum);
  mudlog( LOG_PLAYERS, buf );
  send_to_char(buf,ch);
}



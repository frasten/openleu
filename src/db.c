#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <sys/types.h>
#include <dirent.h>
#include <stdlib.h>
#include <errno.h>
#include "protos.h"

#include "gilde.h"
#include "status.h"
#include "fight.h"

#define RENT_INACTIVE 1         /* delete the users rent files after 1 month */

#define NEW_ZONE_SYSTEM
#define killfile "killfile"

#define OBJ_DIR "objects"
#define MOB_DIR "mobiles"

/**************************************************************************
*  declarations of most of the 'global' variables                         *
************************************************************************ */
int no_mail = 0;
int top_of_scripts = 0;
int top_of_world = 0;                 /* ref to the top element of world */
#if HASH
struct hash_header      room_db;
#else
struct room_data        *room_db[WORLD_SIZE];
#endif

struct obj_data  *object_list = 0;    /* the global linked list of obj's */
struct char_data *character_list = 0; /* global l-list of chars          */

struct zone_data *zone_table = NULL;     /* table of reset data             */
int top_of_zone_table = 0;
struct message_list fight_messages[MAX_MESSAGES]; /* fighting messages   */
struct player_index_element *player_table = 0; /* index to player file   */
int top_of_p_table = 0;               /* ref to top of table             */
int top_of_p_file = 0;
long total_bc = 0;
long room_count=0;
long mob_count=0;
long obj_count=0;
long total_mbc=0;
long total_obc=0;

/*
**  distributed monster stuff
*/
int mob_tick_count=0;
char wmotd[MAX_STRING_LENGTH];
char credits[MAX_STRING_LENGTH];      /* the Credits List                */
char news[MAX_STRING_LENGTH];           /* the news                        */
char motd[MAX_STRING_LENGTH];         /* the messages of today           */
char help[MAX_STRING_LENGTH];         /* the main help page              */
char info[MAX_STRING_LENGTH];         /* the info text                   */
char wizlist[MAX_STRING_LENGTH*2];      /* the wizlist                     */
char login[MAX_STRING_LENGTH];


FILE *mob_f,                          /* file containing mob prototypes  */
     *obj_f,                          /* obj prototypes                  */
     *help_fl,                        /* file for help texts (HELP <kwd>)*/
     *wizhelp_fl;                     /* file for wizhelp */

struct index_data *mob_index;         /* index table for mobile file     */
struct index_data *obj_index;         /* index table for object file     */
struct help_index_element *help_index = 0;
struct help_index_element *wizhelp_index = 0;
int top_of_mobt = 0;                  /* top of mobile index table       */
int top_of_objt = 0;                  /* top of object index table       */
int top_of_sort_mobt = 0;
int top_of_sort_objt = 0;
int top_of_alloc_mobt = 0;
int top_of_alloc_objt = 0;
int top_of_helpt;                     /* top of help index table         */
int top_of_wizhelpt;                  /* top of wiz help index table         */

struct time_info_data time_info;        /* the infomation about the time   */
struct weather_data weather_info;       /* the infomation about the weather */

long saved_rooms[WORLD_SIZE];
long number_of_saved_rooms = 0;
extern struct descriptor_data *descriptor_list;
extern struct spell_info_type spell_info[MAX_SPL_LIST];
struct index_data *InsertInIndex( struct index_data *pIndex, char *szName,
                                  int nVNum, int *alloc_top, int *top );
void clean_playerfile(void);
void ReadTextZone( FILE *fl );
int CheckKillFile( int iVNum );

struct script_com *gpComp = NULL;
struct scripts *gpScript_data = NULL;
struct reset_q_type gReset_q = { NULL, NULL };

/*************************************************************************
*  routines for booting the system                                       *
*********************************************************************** */

/* body of the booting system */
void boot_db()
{
  int i;
  extern int no_specials;

  mudlog( LOG_CHECK, "Boot db -- BEGIN.");

  mudlog( LOG_CHECK, "Resetting the game time:");
  reset_time();

  mudlog( LOG_CHECK, "Reading newsfile, credits, help-page, info and motd.");
  file_to_string(NEWS_FILE, news);
  file_to_string(CREDITS_FILE, credits);
  file_to_string(MOTD_FILE, motd);
  file_to_string(WIZ_MOTD_FILE, wmotd);
  file_to_string(HELP_PAGE_FILE, help);
  file_to_string(INFO_FILE, info);
  file_to_string(WIZLIST_FILE, wizlist);
  file_to_string(LOGIN_FILE, login);

  mudlog( LOG_CHECK, "Initializing Script Files.");

  /* some machines are pre-allocation specific when dealing with realloc */
  gpScript_data = (struct scripts *) malloc(sizeof(struct scripts));
  if( gpScript_data == NULL )
  {
    mudlog( LOG_SYSERR, "Cannot allocate memory for gpScript_data" );
    abort();
  }
  CommandSetup();
  InitScripts();
  mudlog( LOG_CHECK, "Opening mobile, object and help files.");
  if (!(mob_f = fopen(MOB_FILE, "r")))
  {
    perror("Opening mob file");
    abort();
  }

  if (!(obj_f = fopen(OBJ_FILE, "r")))
  {
    perror("Opening obj file");
    abort();
  }
  if (!(help_fl = fopen(HELP_KWRD_FILE, "r")))
    mudlog( LOG_ERROR, "   Could not open help file.");
  else
    help_index = build_help_index(help_fl, &top_of_helpt);

  if (!(wizhelp_fl = fopen(WIZ_HELP_FILE, "r")))
    mudlog( LOG_ERROR, "   Could not open wizhelp file.");
  else
    wizhelp_index = build_help_index(wizhelp_fl, &top_of_wizhelpt);

#if CLEAN_AT_BOOT
  mudlog( LOG_CHECK, "Clearing inactive players");
  clean_playerfile();
#endif

  mudlog( LOG_CHECK, "Booting mail system.");
  if (!scan_mail_file())
  {
    mudlog( LOG_ERROR, "   Mail system error -- mail system disabled!");
    no_mail = 1;
  }

  mudlog( LOG_CHECK, "Loading zone table.");
  boot_zones();

  mudlog( LOG_CHECK, "Loading saved zone table.");
  boot_saved_zones();

  mudlog( LOG_CHECK, "Loading rooms.");
  boot_world();

  mudlog( LOG_CHECK, "Loading saved rooms.");
  boot_saved_rooms();

  mudlog( LOG_CHECK, "Generating index tables for mobile and object files.");
  mob_index = generate_indices( mob_f, &top_of_mobt, &top_of_sort_mobt,
                                &top_of_alloc_mobt, MOB_DIR );
  obj_index = generate_indices( obj_f, &top_of_objt, &top_of_sort_objt,
                                &top_of_alloc_objt, OBJ_DIR );

  mudlog( LOG_CHECK, "Renumbering zone table.");
  renum_zone_table(0);

  mudlog( LOG_CHECK, "Generating player index.");
  build_player_index();

  mudlog( LOG_CHECK, "Loading fight messages.");
  load_messages();

  mudlog( LOG_CHECK, "Loading social messages.");
  boot_social_messages();

  mudlog( LOG_CHECK, "Loading pose messages.");
  boot_pose_messages();

  mudlog( LOG_CHECK, "Assigning function pointers:");
  if (!no_specials)
  {
    mudlog( LOG_CHECK, "   Mobiles.");
    assign_mobiles();
    mudlog( LOG_CHECK, "   Objects.");
    assign_objects();
    mudlog( LOG_CHECK, "   Room.");
    assign_rooms();
    mudlog( LOG_CHECK, "   Guilds." );
    BootGuilds();
  }

  mudlog( LOG_CHECK,"   Commands.");
  assign_command_pointers();
  mudlog( LOG_CHECK, "   Spells.");
  assign_spell_pointers();

  mudlog( LOG_CHECK, "Updating characters with saved items:");
  update_obj_file();

#if LIMITED_ITEMS
  PrintLimitedItems();
#endif

  mudlog( LOG_CHECK, "Loading objects for saved rooms.");
  ReloadRooms();

  for (i = 0; i <= top_of_zone_table; i++)
  {
    char  *s;
    int   d,e;
    s = zone_table[i].name;
    d = (i ? (zone_table[i - 1].top + 1) : 0);
    e = zone_table[i].top;
    fprintf( stderr, "Performing boot-time init of %d:%s (rooms %d-%d).\n",
             zone_table[i].num, s, d, e );
    zone_table[i].start = 0;

    if( i == 0 )
    {
      fprintf(stderr, "Performing boot-time reload of static mobs\n" );
      reset_zone(0);
    }

    if( i == 1 )
    {
      mudlog( LOG_CHECK, "Reset of %s\n", s );
      reset_zone(1);
    }
  }

  gReset_q.head = gReset_q.tail = 0;

  mudlog( LOG_CHECK, "Boot db -- DONE.");
}


/* reset the time in the game from file */
void reset_time()
{
  extern unsigned char moontype;
  long beginning_of_time = 650336715;



  struct time_info_data mud_time_passed(time_t t2, time_t t1);

  time_info = mud_time_passed(time(0), beginning_of_time);

  moontype = time_info.day;

  switch(time_info.hours)
  {
   case 0 :
   case 1 :
   case 2 :
   case 3 :
   case 4 :
    {
      weather_info.sunlight = SUN_DARK;
      switch_light(MOON_SET);
      break;
    }
   case 5 :
   case 6 :
    {
      weather_info.sunlight = SUN_RISE;
      switch_light(SUN_RISE);
      break;
    }
   case 7 :
   case 8 :
   case 9 :
   case 10 :
   case 11 :
   case 12 :
   case 13 :
   case 14 :
   case 15 :
   case 16 :
   case 17 :
   case 18 :
    {
      weather_info.sunlight = SUN_LIGHT;
      break;
    }
   case 19 :
   case 20 :
    {
      weather_info.sunlight = SUN_SET;
      break;
    }
   case 21 :
   case 22 :
   case 23 :
   default :
    {
      switch_light(SUN_DARK);
      weather_info.sunlight = SUN_DARK;
      break;
    }
  }

  mudlog( LOG_CHECK,"   Current Gametime: %dH %dD %dM %dY.",
          time_info.hours, time_info.day,
          time_info.month, time_info.year);

  weather_info.pressure = 960;
  if ((time_info.month>=7)&&(time_info.month<=12))
    weather_info.pressure += dice(1,50);
  else
    weather_info.pressure += dice(1,80);

  weather_info.change = 0;

  if (weather_info.pressure<=980)
  {
    if ((time_info.month>=3) && (time_info.month<=14))
      weather_info.sky = SKY_LIGHTNING;
    else
      weather_info.sky = SKY_LIGHTNING;
  }
  else if (weather_info.pressure<=1000)
  {
    if ((time_info.month>=3) && (time_info.month<=14))
      weather_info.sky = SKY_RAINING;
    else
      weather_info.sky = SKY_RAINING;
  }
  else if (weather_info.pressure<=1020)
  {
    weather_info.sky = SKY_CLOUDY;
  }
  else
  {
    weather_info.sky = SKY_CLOUDLESS;
  }
}



/* update the time file */
void update_time()
{
  return;
}



struct wizs
{
  char name[20];
  int level;
};

int intcomp(struct wizs *j, struct wizs *k)
{
  return (k->level - j->level);
}


         /* generate index table for the player file */
void build_player_index()
{
#if defined( EMANUELE )
  DIR *dir;
  struct wizlistgen list_wiz;
  int j, i, center;
  char buf[ 256 ];

  /* might use ABS_MAX_CLASS here some time */
  for(j = 0; j < MAX_CLASS; j++)
    list_wiz.number[j] = 0;

  top_of_p_table = 0;

  if( ( dir = opendir( PLAYERS_DIR ) ) != NULL )
  {
    struct dirent *ent;
    while( ( ent = readdir( dir ) ) != NULL )
    {
      FILE *pFile;
      char szFileName[ 40 ];

      if( *ent->d_name == '.' )
        continue;
      /* ATTENZIONE Inserire controllo sul .dat */

      sprintf( szFileName, "%s/%s", PLAYERS_DIR, ent->d_name );

      if( ( pFile = fopen( szFileName, "r+" ) ) != NULL )
      {
        struct char_file_u Player;

        if( fread( &Player, 1, sizeof( Player ), pFile ) == sizeof( Player ) )
        {
          int max;

          top_of_p_table++;

          for( j = 0, max = 0; j < MAX_CLASS; j++ )
            if( Player.level[ j ] > max )
              max = Player.level[ j ];

          if( max >= IMMORTAL )
          {
            mudlog( LOG_CHECK,
                    "GOD: %s, Levels [%d][%d][%d][%d][%d][%d][%d][%d]",
                    Player.name, Player.level[0], Player.level[1],
                    Player.level[2], Player.level[3], Player.level[4],
                    Player.level[5], Player.level[6], Player.level[7] );

            list_wiz.lookup[max - 51].stuff[list_wiz.number[max - 51]].name =
              (char *)strdup( Player.name );
            list_wiz.lookup[max - 51].stuff[list_wiz.number[max - 51]].title =
              (char *)strdup( Player.title );
            list_wiz.number[max - 51]++;
          }
        }
        fclose( pFile );
      }
    }
  }
#else
  int nr = -1, i;
  struct char_file_u dummy;
  FILE *fl;

  char buf[MAX_STRING_LENGTH*2];

  register int max=0, j;
  int center;

  struct wizlistgen list_wiz;

        /* might use ABS_MAX_CLASS here some time */
  for(j = 0; j < MAX_CLASS; j++)
     list_wiz.number[j] = 0;

  if (!(fl = fopen(PLAYER_FILE, "rb+")))        {
     perror("build player index");
     exit(0);
  }

  for (; !feof(fl);)    {
    fread(&dummy, sizeof(struct char_file_u), 1, fl);
    if (!feof(fl))   /* new record */           {
      /* Create new entry in the list */
      if (nr == -1) {
        CREATE(player_table,
               struct player_index_element, 1);
        nr = 0;
      } else {
        if (!(player_table = (struct player_index_element *)
              realloc(player_table, (++nr + 1) *
                      sizeof(struct player_index_element))))
          {
            perror("generate index");
            exit(0);
          }
      }

      player_table[nr].nr = nr;

      CREATE(player_table[nr].name, char,
             strlen(dummy.name) + 1);
      for( i = 0;
           ( *( player_table[nr].name + i ) = LOWER( *( dummy.name + i ) ) );
           i++);

        for (j=0;j<=ABS_MAX_CLASS;j++)
           if (dummy.level[j] > 60)
              dummy.level[j] = 0;

#if 1
                        /* was 5 */
      for (i = 0; i < MAX_CLASS; i++)
        if (dummy.level[i] >= 51)
        {
          mudlog( LOG_CHECK, "GOD: %s, Levels [%d][%d][%d][%d][%d][%d][%d][%d]",
                  dummy.name, dummy.level[0],dummy.level[1],dummy.level[2],
                  dummy.level[3], dummy.level[4],dummy.level[5],dummy.level[6],
                  dummy.level[7]);

          max = 0;

          /* MAX_CLASS does not work here... */
          for (j=0 ; j < MAX_CLASS; j++)
            if (dummy.level[j] > max)
            {
              max = dummy.level[j];
            }


        list_wiz.lookup[max - 51].stuff[list_wiz.number[max - 51]].name =
          (char *)strdup(dummy.name);
        list_wiz.lookup[max - 51].stuff[list_wiz.number[max - 51]].title =
          (char *)strdup(dummy.title);
        list_wiz.number[max - 51]++;
        break;
      }
#endif


    }
  }


  fclose(fl);

  top_of_p_table = nr;

  top_of_p_file = top_of_p_table;

#endif

  mudlog( LOG_CHECK, "Began Wizlist Generation.");

  sprintf(wizlist, "\033[2J\033[0;0H\n\r\n\r");
  sprintf(buf, "-* Dei degli Dei [%d/2] *-\n\r",list_wiz.number[9]);


  center = (38 - (int) (str_len(buf)/2));
  for( i = 0; i <= center; i++ )
     strcat(wizlist, " ");

  strcat(wizlist, buf);


  for( i = 0; i < list_wiz.number[9]; i++)
  {
    sprintf(buf, "%s %s\n\r", list_wiz.lookup[9].stuff[i].name,
            list_wiz.lookup[9].stuff[i].title);

    center = 38 - (int) (str_len(buf)/2);
    for(j = 0; j <= center; j++)
      strcat(wizlist, " ");
    strcat(wizlist, buf);
  }

  strcat(wizlist, "\n\r\n\r");
  mudlog( LOG_CHECK, "Creator Generated.");

  sprintf(buf, "-* Dei creatori [%d/4] *-\n\r", list_wiz.number[8]);
  center = 38 - (int) (str_len(buf)/2);

  for(i = 0; i <= center; i++)
    strcat(wizlist, " ");
  strcat(wizlist, buf);

  for(i = 0; i < list_wiz.number[8]; i++)
  {
    sprintf(buf, "%s %s\n\r", list_wiz.lookup[8].stuff[i].name,
            list_wiz.lookup[8].stuff[i].title);
    center = 38 - (int) (str_len(buf)/2);
    for(j = 0; j <= center; j++)
      strcat(wizlist, " ");
    strcat(wizlist, buf);
  }

  strcat(wizlist, "\n\r\n\r");
  mudlog( LOG_CHECK, "Implementors Generated.");

  sprintf(buf, "-* Dei del giudizio [%d/6] *-\n\r", list_wiz.number[7]);
  center = 38 - (int) (str_len(buf)/2);

  for(i = 0; i <= center; i++)
    strcat(wizlist, " ");
  strcat(wizlist, buf);

  for(i = 0; i < list_wiz.number[7]; i++)
  {
    sprintf(buf, "%s %s\n\r", list_wiz.lookup[7].stuff[i].name,
            list_wiz.lookup[7].stuff[i].title);
    center = 38 - (int) (str_len(buf)/2);
    for(j = 0; j <= center; j++)
      strcat(wizlist, " ");
    strcat(wizlist, buf);
  }

  strcat(wizlist, "\n\r\n\r");
  mudlog( LOG_CHECK, "Gods of Final Judgement Generated.");

  sprintf(buf, "-* Dei superiori [%d/8] *-\n\r", list_wiz.number[6]);
  center = 38 - (int) (str_len(buf)/2);

  for(i = 0; i <= center; i++)
    strcat(wizlist, " ");
  strcat(wizlist, buf);

  for(i = 0; i < list_wiz.number[6]; i++)
  {
    sprintf(buf, "%s %s\n\r", list_wiz.lookup[6].stuff[i].name,
            list_wiz.lookup[6].stuff[i].title);
    center = 38 - (int) (str_len(buf)/2);
    for(j = 0; j <= center; j++)
      strcat(wizlist, " ");
    strcat(wizlist, buf);
  }

  strcat(wizlist, "\n\r\n\r");


  sprintf(buf, "-* Dei [%d/10] *-\n\r", list_wiz.number[5]);
  center = 38 - (int) (str_len(buf)/2);

  for(i = 0; i <= center; i++)
    strcat(wizlist, " ");
  strcat(wizlist, buf);

  for(i = 0; i < list_wiz.number[5]; i++)
  {
    sprintf(buf, "%s %s\n\r", list_wiz.lookup[5].stuff[i].name,
            list_wiz.lookup[5].stuff[i].title);
    center = 38 - (int) (str_len(buf)/2);
    for(j = 0; j <= center; j++)
      strcat(wizlist, " ");
    strcat(wizlist, buf);
  }

  strcat(wizlist, "\n\r\n\r");


  sprintf(buf, "-* SemiDei [%d/12] *-\n\r", list_wiz.number[4]);
  center = 38 - (int) (str_len(buf)/2);

  for(i = 0; i <= center; i++)
    strcat(wizlist, " ");
  strcat(wizlist, buf);

  for(i = 0; i < list_wiz.number[4]; i++)
  {
    sprintf(buf, "%s %s\n\r", list_wiz.lookup[4].stuff[i].name,
            list_wiz.lookup[4].stuff[i].title);
    center = 38 - (int) (str_len(buf)/2);
    for(j = 0; j <= center; j++)
      strcat(wizlist, " ");
    strcat(wizlist, buf);
  }

  strcat(wizlist, "\n\r\n\r");


  sprintf(buf, "-* Angeli [%d/14] *-\n\r", list_wiz.number[3]);
  center = 38 - (int) (str_len(buf)/2);

  for(i = 0; i <= center; i++)
    strcat(wizlist, " ");
  strcat(wizlist, buf);

  for(i = 0; i < list_wiz.number[3]; i++)
  {
    sprintf(buf, "%s %s\n\r", list_wiz.lookup[3].stuff[i].name,
            list_wiz.lookup[3].stuff[i].title);
    center = 38 - (int) (str_len(buf)/2);
    for(j = 0; j <= center; j++)
      strcat(wizlist, " ");
    strcat(wizlist, buf);
  }

  strcat(wizlist, "\n\r\n\r");


  sprintf(buf, "-* Creatori [%d/30] *-\n\r", list_wiz.number[2]);
  center = 38 - (int) (str_len(buf)/2);

  for(i = 0; i <= center; i++)
    strcat(wizlist, " ");
  strcat(wizlist, buf);

  for(i = 0; i < list_wiz.number[2]; i++)
  {
    sprintf(buf, "%s %s\n\r", list_wiz.lookup[2].stuff[i].name,
            list_wiz.lookup[2].stuff[i].title);
    center = 38 - (int) (str_len(buf)/2);
    for(j = 0; j <= center; j++)
      strcat(wizlist, " ");
    strcat(wizlist, buf);
  }

  strcat(wizlist, "\n\r\n\r");


  sprintf(buf, "-* Santi [%d/50] *-\n\r", list_wiz.number[1]);
  center = 38 - (int) (str_len(buf)/2);

  for(i = 0; i <= center; i++)
    strcat(wizlist, " ");
  strcat(wizlist, buf);

  for(i = 0; i < list_wiz.number[1]; i++)
  {
    sprintf(buf, "%s %s\n\r", list_wiz.lookup[1].stuff[i].name,
            list_wiz.lookup[1].stuff[i].title);
    center = 38 - (int) (str_len(buf)/2);
    for(j = 0; j <= center; j++)
      strcat(wizlist, " ");
    strcat(wizlist, buf);
  }

  strcat(wizlist, "\n\r\n\r");


  sprintf(buf, "-* Immortali [%d/~] *-\n\r", list_wiz.number[0]);
  center = 38 - (int) (str_len(buf)/2);

  for(i = 0; i <= center; i++)
    strcat(wizlist, " ");
  strcat(wizlist, buf);

  for(i = 0; i < list_wiz.number[0]; i++)
  {
    sprintf(buf, "%s %s\n\r", list_wiz.lookup[0].stuff[i].name,
            list_wiz.lookup[0].stuff[i].title);
    center = 38 - (int) (str_len(buf)/2);
    for(j = 0; j <= center; j++)
       strcat(wizlist, " ");
    strcat(wizlist, buf);
  }

  strcat(wizlist, "\n\r\n\r");
  j = 0;
  for(i = 0; i <= 9; i++)
    j += list_wiz.number[i];
  sprintf( buf, "Totale Dei: %d\n\r\n\r", j );
  strcat( wizlist, buf );

  return;
}

void ReplaceInIndex( struct index_data *pIndex, char *szName, int nRNum,
                     int nVNum, int nTop )
{

  if( nRNum < 0 || nRNum >= nTop )
  {
    mudlog( LOG_SYSERR, "Invalid RNum in ReplaceInIndex (db.c)." );
    return;
  }

  pIndex[ nRNum ].iVNum = nVNum;
  pIndex[ nRNum ].pos = -1;
  pIndex[ nRNum ].name = strdup( szName );
  pIndex[ nRNum ].data = NULL;
}


struct index_data *InsertInIndex( struct index_data *pIndex, char *szName,
                                  int nVNum, int *alloc_top, int *top )
{
  if (*top >= *alloc_top)
  {
    if( !(pIndex = (struct index_data*)
                    realloc(pIndex, (*top + 50) * sizeof(struct index_data))))
    {
      perror("load indices");
      assert(0);
    }
    *alloc_top += 50;
  }
  pIndex[ *top ].iVNum = nVNum;
  pIndex[ *top ].pos = -1;
  pIndex[ *top ].name = strdup( szName );
  pIndex[ *top ].number = 0;
  pIndex[ *top ].func = 0;
  pIndex[ *top ].data = NULL;
  (*top)++;
  return pIndex;
}

void InsertObject( struct obj_data *pObj, int nVNum )
{
  int nRNum = real_object( nVNum );
  if( nRNum < 0 )
  {
    obj_index = InsertInIndex( obj_index, pObj->name, nVNum,
                               &top_of_alloc_objt, &top_of_objt );
  }
  else
  {
    ReplaceInIndex( obj_index, pObj->name, nRNum, nVNum, top_of_objt );
  }
}

void InsertMobile( struct char_data *pMob, int nVNum )
{
  int nRNum = real_mobile( nVNum );
  if( nRNum < 0 )
  {
    obj_index = InsertInIndex( mob_index, GET_NAME( pMob ), nVNum,
                               &top_of_alloc_mobt, &top_of_mobt );
  }
  else
  {
    ReplaceInIndex( mob_index, GET_NAME( pMob ), nRNum, nVNum, top_of_mobt );
  }
}


void read_object_to_memory( int nVNum)
{
  int i = real_object( nVNum );
  if( i >= 0 )
    obj_index[ i ].data = (void *)read_object( i, REAL );
}

/* generate index table for object or monster file */
struct index_data *generate_indices( FILE *fl, int *top, int *sort_top,
                                     int *alloc_top, char *dirname )
{
  FILE *f;
  DIR *dir;
  struct index_data *index;
  struct dirent *ent;
  long i = 0, di = 0, vnum, j;
  long bc=2000;
  long dvnums[2000];               /* guess 2000 stored objects is enuff */
  char buf[82], tbuf[128];

  /* scan main obj file */
  rewind(fl);
  for (;;)
  {
    if (fgets(buf, sizeof(buf), fl))
    {
      if (*buf == '#')
      {
        if (!i)                                          /* first cell */
          CREATE(index, struct index_data, bc);
        else if (i >= bc)
        {
          if (!(index = (struct index_data*)
                realloc(index, (i + 50) * sizeof(struct index_data))))
          {
            perror("load indices");
            assert(0);
          }
          bc += 50;
        }
        sscanf(buf, "#%d", &index[i].iVNum );
        sprintf(tbuf,"%s/%d",dirname,index[i].iVNum);
        if((f=fopen(tbuf,"rt"))==NULL)
        {
          index[i].pos = ftell(fl);
          index[i].name = (index[i].iVNum < 99999) ? fread_string(fl) :
                                                       strdup("omega");
        }
        else
        {
          index[i].pos = -1;
          fscanf(f, "#%*d\n");
          index[i].name = (index[i].iVNum < 99999) ? fread_string(f) :
                                                       strdup("omega");
          dvnums[di++] = index[i].iVNum;
          fclose(f);
        }
        index[i].number = 0;
        index[i].func = 0;
        index[i].data = NULL;
        i++;
      }
      else
      {
        if (*buf == '%' && buf[ 1 ] == '%' )        /* EOF */
          break;
      }
    }
    else
    {
      fprintf(stderr,"generate indices");
      assert(0);
    }
  }
  *sort_top = i;
  *alloc_top = bc;
  *top = *sort_top;
  /* scan for directory entrys */
  if((dir=opendir(dirname))==NULL)
  {
    mudlog( LOG_ERROR ,"unable to open index directory %s",dirname);
    return(index);
  }
  while((ent=readdir(dir)) != NULL)
  {
    if(*ent->d_name=='.')
      continue;
    vnum=atoi(ent->d_name);
    if(vnum == 0)
      continue;
    /* search if vnum was already sorted in main database */
    for(j=0;j<di;j++)
      if(dvnums[j] == vnum)
        break;
    if(dvnums[j] == vnum)
      continue;
    sprintf(buf,"%s/%s",dirname, ent->d_name);
    if((f=fopen(buf,"rt")) == NULL)
    {
      mudlog( LOG_ERROR, "Can't open file %s for reading\n",buf);
      continue;
    }
    if (!i)
      CREATE(index, struct index_data, bc);
    else if (i >= bc)
    {
      if (!(index = (struct index_data*)
            realloc(index, (i + 50) * sizeof(struct index_data))))
      {
        perror("load indices");
        assert(0);
      }
      bc += 50;
    }
    fscanf(f, "#%*d\n");
    index[i].iVNum = vnum;
    index[i].pos = -1;
    index[i].name = (index[i].iVNum<99999)?fread_string(f):strdup("omega");
    index[i].number = 0;
    index[i].func = 0;
    index[i].data = NULL;
    fclose(f);
    i++;
  }
  *alloc_top = bc;
  *top = i;
  return(index);
}


void cleanout_room(struct room_data *rp)
{
  int   i;
  struct extra_descr_data *exptr, *nptr;

  free(rp->name);
  free(rp->description);
  for (i=0; i<6; i++)
    if (rp->dir_option[i]) {
      free(rp->dir_option[i]->general_description);
      free(rp->dir_option[i]->keyword);
      free (rp->dir_option[i]);
      rp->dir_option[i] = NULL;
    }

  for (exptr=rp->ex_description; exptr; exptr = nptr) {
    nptr = exptr->next;
    free(exptr->keyword);
    free(exptr->description);
    free(exptr);
  }
}

void completely_cleanout_room(struct room_data *rp)
{
  struct char_data      *ch;
  struct obj_data       *obj;

  while (rp->people) {
    ch = rp->people;
    act("The hand of god sweeps across the land and you are swept into the Void.", FALSE, NULL, NULL, NULL, TO_VICT);
    char_from_room(ch);
    char_to_room(ch, 0);        /* send character to the void */
  }

  while (rp->contents) {
    obj = rp->contents;
    obj_from_room(obj);
    obj_to_room(obj, 0);        /* send item to the void */
  }

  cleanout_room(rp);
}

void load_one_room( FILE *fl, struct room_data *rp )
{
  char chk[ 161 ];
  int   bc=0;
  long int     tmp;

  struct extra_descr_data *new_descr;

  bc = sizeof(struct room_data);

  rp->name = fread_string(fl);
  if (rp->name && *rp->name)
    bc += strlen(rp->name);
  rp->description = fread_string(fl);
  if (rp->description && *rp->description)
    bc += strlen(rp->description);

  if (top_of_zone_table >= 0)
  {
    int zone;
    fscanf(fl, " %*d ");

    /* OBS: Assumes ordering of input rooms */

    for( zone=0;
         rp->number > zone_table[zone].top && zone<=top_of_zone_table;
         zone++)
      ;
    if (zone > top_of_zone_table)
    {
      fprintf(stderr, "Room %ld is outside of any zone.\n", rp->number);
      exit( 1 );
    }
    rp->zone = zone;
  }
  tmp = fread_number( fl );
  rp->room_flags = tmp;
  tmp = fread_number( fl );
  rp->sector_type = tmp;

  if (tmp == -1)
  {
    tmp = fread_number( fl );
    rp->tele_time = tmp;
    tmp = fread_number( fl );
    rp->tele_targ = tmp;
    tmp = fread_number( fl );
    rp->tele_mask = tmp;
    if( IS_SET( TELE_COUNT, rp->tele_mask ) )
    {
      tmp = fread_number( fl );
      rp->tele_cnt = tmp;
    }
    else
    {
      rp->tele_cnt = 0;
    }
    tmp = fread_number( fl );
    rp->sector_type = tmp;
  }
  else
  {
    rp->tele_time = 0;
    rp->tele_targ = 0;
    rp->tele_mask = 0;
    rp->tele_cnt  = 0;
  }

  if( tmp == SECT_WATER_NOSWIM || tmp == SECT_UNDERWATER )
  {
    /* river */
    /* read direction and rate of flow */
    tmp = fread_number( fl );
    rp->river_speed = tmp;
    tmp = fread_number( fl );
    rp->river_dir = tmp;
  }

  if( rp->room_flags & TUNNEL )
  {
    /* read in mobile limit on tunnel */
    tmp = fread_number( fl );
    rp->moblim = tmp;
  }

  rp->funct = 0;
  rp->light = 0; /* Zero light sources */

  for (tmp = 0; tmp <= 5; tmp++)
    rp->dir_option[tmp] = 0;

  rp->ex_description = 0;

  while( fscanf( fl, " %160s \n", chk ) == 1 )
  {
    switch( *chk )
    {
     case 'D':
      setup_dir(fl, rp->number, atoi(chk + 1));
      bc += sizeof(struct room_direction_data);
      break;
     case 'E': /* extra description field */

      CREATE(new_descr,struct extra_descr_data,1);
      bc += sizeof(struct extra_descr_data);

      new_descr->keyword = fread_string(fl);
      if (new_descr->keyword && *new_descr->keyword)
        bc += strlen(new_descr->keyword);
      else
        fprintf(stderr, "No keyword in room %ld\n", rp->number);

      new_descr->description = fread_string(fl);
      if( new_descr->description && *new_descr->description)
        bc += strlen(new_descr->description);
      else
        fprintf(stderr, "No desc in room %ld\n", rp->number);

      new_descr->next = rp->ex_description;
      rp->ex_description = new_descr;
      break;
     case 'L':
      rp->szWhenBrightAtNight = fread_string( fl );
      rp->szWhenBrightAtDay = fread_string( fl );
      break;
     case 'S':   /* end of current room */

#if BYTE_COUNT
      if (bc >= 1000)
        fprintf(stderr, "Byte count for this room[%d]: %d\n",rp->number,  bc);
#endif
      total_bc += bc;
      room_count++;

      if( IS_SET( rp->room_flags, SAVE_ROOM ) )
      {
        saved_rooms[ number_of_saved_rooms ] = rp->number;
        number_of_saved_rooms++;
      }
      else
      {
        FILE *fp;
        char buf[255];

        sprintf( buf, "world/%ld", rp->number );
        fp = fopen( buf, "r" );
        if( fp )
        {
          saved_rooms[ number_of_saved_rooms ] = rp->number;
          number_of_saved_rooms++;
          fclose( fp );
        }
      }
      return;
     case 'C':
      /* Commento, non deve fare nulla. Il tutto deve stare su una sola
       * linea. */
      break;
     default:
      mudlog( LOG_ERROR, "unknown auxiliary code `%s' in room load of #%ld",
              chk, rp->number);
      break;
    }
  }
}



/* load the rooms */
void boot_world()
{
  FILE *fl;
  long lVNum, last;
  struct room_data      *rp;


#if HASH
  init_hash_table(&room_db, sizeof(struct room_data), 2048);
#else
  init_world(room_db);
#endif
  character_list = 0;
  object_list = 0;

  if (!(fl = fopen(WORLD_FILE, "r")))
  {
    perror("fopen");
    mudlog( LOG_ERROR, "boot_world: could not open world file.");
    assert(0);
  }




  last = 0;
  while (1==fscanf(fl, " #%ld\n", &lVNum))
  {
    allocate_room(lVNum);
    /* do we need to to_of_world++ in here somewhere? msw */
    rp = real_roomp(lVNum);
    if (rp)
      bzero(rp, sizeof(*rp));
    else
    {
      fprintf(stderr, "Error, room %ld not in database!(%ld)\n",
              lVNum, last);
      assert(0);
    }

    rp->number = lVNum;
    load_one_room(fl, rp);
    last = lVNum;
  }

  fclose(fl);
}





void allocate_room(long room_number)
{
  if (room_number>top_of_world)
    top_of_world = room_number;
#if HASH
  hash_find_or_create(&room_db, room_number);
#else
  room_find_or_create(room_db, room_number);
#endif
}






/* read direction data */
void setup_dir(FILE *fl, long room, int dir)
{
  long tmp;
  struct room_data      *rp, dummy;

  rp = real_roomp(room);

  if (!rp) {
    rp = &dummy;            /* this is a quick fix to make the game */
    dummy.number = room;   /* stop crashing   */
  }

  CREATE(rp->dir_option[dir], struct room_direction_data, 1);

  rp->dir_option[dir]->general_description = fread_string(fl);
  rp->dir_option[dir]->keyword = fread_string(fl);

  rp->dir_option[dir]->exit_info = fread_number( fl );

  rp->dir_option[dir]->key = fread_number( fl );

  rp->dir_option[dir]->to_room = fread_number( fl );

  tmp = -1;
  fscanf(fl, " %ld ", &tmp);
  rp->dir_option[dir]->open_cmd = tmp;

}

void boot_saved_zones()
{
        DIR *dir;
        FILE *fp;
        struct dirent *ent;
        char    buf[80];
        long     zone;

  if ((dir = opendir("zones")) == NULL) {
    mudlog( LOG_ERROR, "Unable to open zones directory.\n");
    return;
  }

  while((ent = readdir(dir)) != NULL) {
    if(*ent->d_name=='.') continue;
    zone=atoi(ent->d_name);
    if(!zone || zone>top_of_zone_table) {
      continue;
    }
    sprintf(buf,"zones/%s",ent->d_name);
    if((fp=fopen(buf,"rt")) == NULL) {
      mudlog( LOG_ERROR, "Can't open file %s for reading\n",buf);
      continue;
    }
    mudlog( LOG_CHECK, "Loading saved zone %ld:%s",zone,zone_table[zone].name);
    LoadZoneFile(fp,zone);
    fclose(fp);
  }
}

void boot_saved_rooms()
{
  DIR *dir;
  FILE *fp;
  struct dirent *ent;
  char buf[ 80 ];
  struct room_data *rp;
  long rooms = 0, vnum;

  if ((dir = opendir("rooms")) == NULL)
  {
    mudlog( LOG_ERROR, "Unable to open rooms directory.\n");
    return;
  }

  while((ent = readdir(dir)) != NULL)
  {
    if(*ent->d_name=='.')
      continue;
    vnum=atoi(ent->d_name);
    if(!vnum || vnum>top_of_world)
      continue;
    sprintf(buf,"rooms/%s",ent->d_name);
    if((fp=fopen(buf,"rt")) == NULL)
    {
      mudlog( LOG_ERROR, "Can't open file %s for reading\n",buf);
      continue;
    }
    while (!feof(fp))
    {
      fscanf(fp, "#%*d\n");
      if( ( rp = real_roomp( vnum ) ) == NULL )
      {  /* empty room */
        rp = (struct room_data *)malloc( sizeof( struct room_data ) );
        if( rp )
          bzero(rp, sizeof(struct room_data));
#if HASH
        room_enter(&room_db, vnum, rp);
#else
        room_enter(room_db, vnum, rp);
#endif
      }
      else
      {
        cleanout_room(rp);
      }
      rp->number = vnum;
      load_one_room( fp, rp );
    }
    fclose(fp);
    rooms++;
  }
  if(rooms)
  {
    mudlog( LOG_CHECK, "Loaded %ld rooms",rooms);
  }
}


#define LOG_ZONE_ERROR(ch, type, zone, cmd) {\
        mudlog( LOG_ERROR, \
                "error in zone %s cmd %ld (%c) resolving %s number", \
                zone_table[zone].name, cmd, ch, type); \
                  }

void renum_zone_table(int spec_zone)
{
  long zone, comm,start,end;
  struct reset_com *cmd;

  if(spec_zone)
  {
    start=end=spec_zone;
  }
  else
  {
    start=0;
    end=top_of_zone_table;
  }

  for (zone = start; zone <= end; zone++)
  {
    for (comm = 0; zone_table[zone].cmd[comm].command != 'S'; comm++)
    {
      switch((cmd = zone_table[zone].cmd +comm)->command)
      {
       case 'M':
        cmd->arg1 = real_mobile(cmd->arg1);
        if (cmd->arg1<0)
          LOG_ZONE_ERROR('M', "mobile", zone, comm);
        if( cmd->arg3 < 0 || real_roomp( cmd->arg3 ) == NULL )
          LOG_ZONE_ERROR('M', "room", zone, comm);
        break;
       case 'C':
        cmd->arg1 = real_mobile(cmd->arg1);
        if (cmd->arg1<0)
          LOG_ZONE_ERROR('C', "mobile", zone, comm);
        break;
       case 'O':
        cmd->arg1 = real_object(cmd->arg1);
        if(cmd->arg1<0)
          LOG_ZONE_ERROR('O', "object", zone, comm);
        if (cmd->arg3 != NOWHERE)
        {
          /*cmd->arg3 = real_room(cmd->arg3);*/
          if( cmd->arg3 < 0 || real_roomp( cmd->arg3 ) == NULL )
            LOG_ZONE_ERROR('O', "room", zone, comm);
        }
        break;
       case 'G':
        cmd->arg1 = real_object(cmd->arg1);
        if(cmd->arg1<0)
          LOG_ZONE_ERROR('G', "object", zone, comm);
        break;
       case 'E':
        cmd->arg1 = real_object(cmd->arg1);
        if(cmd->arg1<0)
          LOG_ZONE_ERROR('E', "object", zone, comm);
        break;
       case 'P':
        cmd->arg1 = real_object(cmd->arg1);
        if(cmd->arg1<0)
          LOG_ZONE_ERROR('P', "object", zone, comm);
        cmd->arg3 = real_object(cmd->arg3);
        if(cmd->arg3<0)
          LOG_ZONE_ERROR('P', "object", zone, comm);
        break;
       case 'D':
        /*cmd->arg1 = real_room(cmd->arg1);*/
        if( cmd->arg1 < 0 || real_roomp( cmd->arg1 ) == NULL )
          LOG_ZONE_ERROR('D', "room", zone, comm);
        break;
      }
    }
  }
}


/* load the zone table and command tables */
void boot_zones()
{

  FILE *fl;
  int zon = 0, cmd_no = 0, expand, tmp, bc=100, cc = 22, znumber;
  char *check, buf[81];

  if (!(fl = fopen(ZONE_FILE, "r")))
  {
    perror("boot_zones");
    assert(0);
  }

  for (;;)
  {
    fscanf(fl, " #%d\n",&znumber);
    check = fread_string(fl);

    if (*check == '$')
      break;            /* end of file */

    /* alloc a new zone */

    if (!zon)
      CREATE(zone_table, struct zone_data, bc);
    else if (zon >= bc)
    {
      if (!(zone_table = (struct zone_data *) realloc(zone_table,
                                     (zon + 10) * sizeof(struct zone_data))))
      {
        perror("boot_zones realloc");
        assert(0);
      }
      bc += 10;
    }
    zone_table[zon].num = znumber;
    zone_table[zon].name = check;
    fscanf(fl, " %d ", &zone_table[zon].top);
    fscanf(fl, " %d ", &zone_table[zon].lifespan);
    fscanf(fl, " %d ", &zone_table[zon].reset_mode);

    /* read the command table */

    /*
     * new code to allow the game to be 'static' i.e. all the mobs are saved
     * in one big zone file, and restored later.
     */

    cmd_no = 0;

    if (zon == 0)
      cc = 20;

    for (expand = 1;;)
    {
      if (expand)
      {
        if (!cmd_no)
          CREATE(zone_table[zon].cmd, struct reset_com, cc);
        else if (cmd_no >= cc)
        {
          cc += 5;
          if (!(zone_table[zon].cmd =
                  (struct reset_com *) realloc(zone_table[zon].cmd,
                                       (cc * sizeof(struct reset_com)))))
          {
            perror("reset command load");
            assert(0);
          }
        }
      }

      expand = 1;

      fscanf(fl, " "); /* skip blanks */
      fscanf(fl, "%c",
             &zone_table[zon].cmd[cmd_no].command);

      if (zone_table[zon].cmd[cmd_no].command == 'S')
        break;

      if (zone_table[zon].cmd[cmd_no].command == '*')
      {
        expand = 0;
        fgets(buf, 80, fl); /* skip command */
        continue;
      }

      fscanf(fl, " %d %d %d",
             &tmp,
             &zone_table[zon].cmd[cmd_no].arg1,
             &zone_table[zon].cmd[cmd_no].arg2);

      zone_table[zon].cmd[cmd_no].if_flag = tmp;

      if( zone_table[zon].cmd[cmd_no].command == 'M' ||
          zone_table[zon].cmd[cmd_no].command == 'O' ||
          zone_table[zon].cmd[cmd_no].command == 'C' ||
          zone_table[zon].cmd[cmd_no].command == 'E' ||
          zone_table[zon].cmd[cmd_no].command == 'P' ||
          zone_table[zon].cmd[cmd_no].command == 'D')
        fscanf(fl, " %d", &zone_table[zon].cmd[cmd_no].arg3 );

      if( zone_table[zon].cmd[cmd_no].command == 'O')
        fscanf(fl, " %d", &zone_table[zon].cmd[cmd_no].arg4 );

      fgets(buf, 80, fl);       /* read comment */
      cmd_no++;
    }
    zon++;
    if( zon == 1 )
    {
      /* fix the cheat */
/*      if (fl != tmp_fl && fl != 0)
        fclose(fl);
      fl = tmp_fl;*/
    }

  }
  top_of_zone_table = --zon;
  free(check);
  fclose(fl);
}


/*************************************************************************
*  procedures for resetting, both play-time and boot-time                *
*********************************************************************** */


/* read a mobile from MOB_FILE */
struct char_data *read_mobile(int nr, int type)
{
  int i;
  long tmp, tmp2, tmp3, bc=0;
  struct char_data *mob;
  char letter;

  extern int mob_tick_count;
  extern long mob_count;

  i = nr;
  if( type == VIRTUAL )
  {
    if( ( nr = real_mobile( nr ) ) < 0 )
    {
      mudlog( LOG_ERROR, "Mobile (V) %d does not exist in database.", i );
      return NULL;
    }
  }

  fseek(mob_f, mob_index[nr].pos, 0);

  CREATE(mob, struct char_data, 1);

  if (!mob)
  {
    mudlog( LOG_SYSERR, "Cannot create mob?! db.c read_mobile");
    return(FALSE);
  }

  bc = sizeof(struct char_data);
  clear_char(mob);

  mob->specials.last_direction = -1;  /* this is a fix for wander */

  /***** String data *** */

  mob->player.name = fread_string(mob_f);
  if( mob->player.name )
    bc += strlen( mob->player.name );
  mob->player.short_descr = fread_string(mob_f);
  if( mob->player.short_descr )
    bc += strlen(mob->player.short_descr);
  mob->player.long_descr = fread_string(mob_f);
  if( mob->player.long_descr )
    bc += strlen(mob->player.long_descr);
  mob->player.description = fread_string(mob_f);
  if( mob->player.description )
    bc += strlen( mob->player.description );
  mob->player.title = 0;

  /* *** Numeric data *** */

  mob->mult_att = 1.0;
  mob->specials.spellfail = 101;

  mob->specials.act = fread_number( mob_f );
  SET_BIT(mob->specials.act, ACT_ISNPC);
  if( IS_SET( mob->specials.act, ACT_POLYSELF ) )
  {
    mudlog( LOG_ERROR, "ACT_POLYSELF bit set in mob #%d.",
            mob_index[nr].iVNum );
    REMOVE_BIT( mob->specials.act, ACT_POLYSELF );
  }

  mob->specials.affected_by = fread_number( mob_f );

  mob->specials.alignment = fread_number( mob_f );

  mob->player.iClass = CLASS_WARRIOR;

  fscanf(mob_f, " %c ", &letter);

  if (letter == 'S')
  {
    fscanf(mob_f, "\n");

    tmp = fread_number( mob_f );
    GET_LEVEL(mob, WARRIOR_LEVEL_IND) = tmp;

    mob->abilities.str   =  MIN( 10 + number( 0, MAX( 1, tmp / 5 ) ), 18 );
    mob->abilities.intel =  MIN( 10 + number( 0, MAX( 1, tmp / 5 ) ), 18 );
    mob->abilities.wis   =  MIN( 10 + number( 0, MAX( 1, tmp / 5 ) ), 18 );
    mob->abilities.dex   =  MIN( 10 + number( 0, MAX( 1, tmp / 5 ) ), 18 );
    mob->abilities.con   =  MIN( 10 + number( 0, MAX( 1, tmp / 5 ) ), 18 );
    mob->abilities.chr   =  MIN( 10 + number( 0, MAX( 1, tmp / 5 ) ), 18 );


    mob->points.hitroll = 20 - fread_number( mob_f );

    tmp = fread_number( mob_f );

    if (tmp > 10 || tmp < -10)
      tmp /= 10;

    mob->points.armor = 10 * tmp;

    fscanf(mob_f, " %ldd%ld+%ld ", &tmp, &tmp2, &tmp3);
    mob->points.max_hit = dice(tmp, tmp2)+tmp3;
    mob->points.hit = mob->points.max_hit;

    fscanf(mob_f, " %ldd%ld+%ld \n", &tmp, &tmp2, &tmp3);
    mob->points.damroll = tmp3;
    mob->specials.damnodice = tmp;
    mob->specials.damsizedice = tmp2;

    mob->points.mana = 10;
    mob->points.max_mana = 10;


    mob->points.move = 50;
    mob->points.max_move = 50;

    tmp = fread_number( mob_f );
    if (tmp == -1)
    {
      mob->points.gold = fread_number( mob_f );
      GET_EXP(mob) = fread_number( mob_f );
      GET_RACE(mob) = fread_number( mob_f );
      if(IsGiant(mob))
        mob->abilities.str += number(1,4);
      if(IsSmall(mob))
        mob->abilities.str -= 1;
    }
    else
    {
      mob->points.gold = tmp;
      GET_EXP(mob) = fread_number( mob_f );
    }
    mob->specials.position = fread_number( mob_f );

    mob->specials.default_pos = fread_number( mob_f );

    tmp = fread_number( mob_f );
    if (tmp < 3)
    {
      mob->player.sex = tmp;
      mob->immune = 0;
      mob->M_immune = 0;
      mob->susc = 0;
    }
    else if (tmp < 6)
    {
      mob->player.sex = tmp - 3;
      mob->immune = fread_number( mob_f );
      mob->M_immune = fread_number( mob_f );
      mob->susc = fread_number( mob_f );
    }
    else
    {
      mob->player.sex = 0;
      mob->immune = 0;
      mob->M_immune = 0;
      mob->susc = 0;
    }

    fscanf(mob_f,"\n");

    mob->player.iClass = 0;

    mob->player.time.birth = time(0);
    mob->player.time.played     = 0;
    mob->player.time.logon  = time(0);
    mob->player.weight = 200;
    mob->player.height = 198;

    for (i = 0; i < 3; i++)
      GET_COND(mob, i) = -1;

    for (i = 0; i < 5; i++)
      mob->specials.apply_saving_throw[i] =
                                MAX(20-GET_LEVEL(mob, WARRIOR_LEVEL_IND), 2);
  }
  else if( letter == 'A' || letter == 'N' || letter == 'B' || letter == 'L' )
  {
    if( letter == 'A' || letter == 'B' || letter == 'L' )
    {
      mob->mult_att = (float)fread_number( mob_f );

#if 0
      /*  read in types: */
      for (i=0;i<mob->mult_att && i < 10; i++)
      {
         mob->att_type[i] = fread_number( mob_f );
      }
#endif
    }

    fscanf(mob_f, "\n");

    tmp = fread_number( mob_f );
    GET_LEVEL(mob, WARRIOR_LEVEL_IND) = tmp;

    mob->abilities.str   =  MIN( 10 + number( 0, MAX( 1, tmp / 5 ) ), 18 );
    mob->abilities.intel =  MIN( 10 + number( 0, MAX( 1, tmp / 5 ) ), 18 );
    mob->abilities.wis   =  MIN( 10 + number( 0, MAX( 1, tmp / 5 ) ), 18 );
    mob->abilities.dex   =  MIN( 10 + number( 0, MAX( 1, tmp / 5 ) ), 18 );
    mob->abilities.con   =  MIN( 10 + number( 0, MAX( 1, tmp / 5 ) ), 18 );
    mob->abilities.chr   =  MIN( 10 + number( 0, MAX( 1, tmp / 5 ) ), 18 );

    mob->points.hitroll = 20 - fread_number( mob_f );

    mob->points.armor = 10 * fread_number( mob_f );

    tmp = fread_number( mob_f );
    mob->points.max_hit = dice(GET_LEVEL(mob, WARRIOR_LEVEL_IND), 8)+tmp;
    mob->points.hit = mob->points.max_hit;

    fscanf(mob_f, " %ldd%ld+%ld \n", &tmp, &tmp2, &tmp3);
    mob->points.damroll = tmp3;
    mob->specials.damnodice = tmp;
    mob->specials.damsizedice = tmp2;

    mob->points.mana = 10;
    mob->points.max_mana = 10;

    mob->points.move = 50;
    mob->points.max_move = 50;

    tmp = fread_number( mob_f );
    if (tmp == -1)
    {
      mob->points.gold = fread_number( mob_f );
      tmp = fread_number( mob_f );
      if (tmp >= 0)
        GET_EXP(mob) = (DetermineExp(mob, tmp)+mob->points.gold);
      else
        GET_EXP(mob) = -tmp;
      GET_RACE(mob) = fread_number( mob_f );
      if(IsGiant(mob))
        mob->abilities.str += number(1,4);
      if(IsSmall(mob))
        mob->abilities.str -= 1;
    }
    else
    {
      mob->points.gold = tmp;

      /* this is where the new exp will come into play */
      tmp = fread_number( mob_f );
      if (tmp >= 0)
        GET_EXP(mob) = (DetermineExp(mob, tmp)+mob->points.gold);
      else
        GET_EXP(mob) = -tmp;
    }

    mob->specials.position = fread_number( mob_f );

    mob->specials.default_pos = fread_number( mob_f );

    tmp = fread_number( mob_f );
    if( tmp < 3 )
    {
      mob->player.sex = tmp;
      mob->immune = 0;
      mob->M_immune = 0;
      mob->susc = 0;
    }
    else if( tmp < 6 )
    {
      mob->player.sex = tmp - 3;
      mob->immune = fread_number( mob_f );
      mob->M_immune = fread_number( mob_f );
      mob->susc = fread_number( mob_f );
    }
    else
    {
      mob->player.sex = 0;
      mob->immune = 0;
      mob->M_immune = 0;
      mob->susc = 0;
    }

    /* read in the sound string for a mobile */
    if (letter == 'L')
    {
      mob->player.sounds = fread_string(mob_f);
      if (mob->player.sounds && *mob->player.sounds)
        bc += strlen(mob->player.sounds);

      mob->player.distant_snds = fread_string(mob_f);
      if (mob->player.distant_snds && *mob->player.distant_snds)
        bc += strlen(mob->player.distant_snds);
    }
    else
    {
      mob->player.sounds = 0;
      mob->player.distant_snds = 0;
    }

    if( letter == 'B' )
    {
      SET_BIT(mob->specials.act, ACT_HUGE);
    }

    mob->player.iClass = 0;

    mob->player.time.birth = time(0);
    mob->player.time.played     = 0;
    mob->player.time.logon  = time(0);
    mob->player.weight = 200;
    mob->player.height = 198;

    for (i = 0; i < 3; i++)
      GET_COND(mob, i) = -1;

    for (i = 0; i < 5; i++)
      mob->specials.apply_saving_throw[ i ] =
        MAX( 20 - GET_LEVEL( mob, WARRIOR_LEVEL_IND ), 2 );
  }
  else
  {  /* The old monsters are down below here */
    fscanf(mob_f, "\n");

    fscanf(mob_f, " %ld ", &tmp);
    mob->abilities.str = tmp;

    fscanf(mob_f, " %ld ", &tmp);
    mob->abilities.intel = tmp;

    fscanf(mob_f, " %ld ", &tmp);
    mob->abilities.wis = tmp;

    fscanf(mob_f, " %ld ", &tmp);
    mob->abilities.dex = tmp;

    fscanf(mob_f, " %ld \n", &tmp);
    mob->abilities.con = tmp;


    fscanf(mob_f, " %ld ", &tmp);
    fscanf(mob_f, " %ld ", &tmp2);

    mob->points.max_hit = number(tmp, tmp2);
    mob->points.hit = mob->points.max_hit;

    fscanf(mob_f, " %ld ", &tmp);
    mob->points.armor = 10*tmp;

    fscanf(mob_f, " %ld ", &tmp);
    mob->points.mana = tmp;
    mob->points.max_mana = tmp;

    fscanf(mob_f, " %ld ", &tmp);
    mob->points.move = tmp;
    mob->points.max_move = tmp;

    fscanf(mob_f, " %ld ", &tmp);
    mob->points.gold = tmp;

    fscanf(mob_f, " %ld \n", &tmp);
    GET_EXP(mob) = tmp;

    fscanf(mob_f, " %ld ", &tmp);
    mob->specials.position = tmp;

    fscanf(mob_f, " %ld ", &tmp);
    mob->specials.default_pos = tmp;

    fscanf(mob_f, " %ld ", &tmp);
    mob->player.sex = tmp;

    mob->player.iClass = fread_number( mob_f );

    fscanf(mob_f, " %ld ", &tmp);
    GET_LEVEL(mob, WARRIOR_LEVEL_IND) = tmp;

    mob->abilities.chr = MIN( 10 + number( 0, MAX( 1, tmp / 5 ) ), 18 );

    fscanf(mob_f, " %ld ", &tmp);
    mob->player.time.birth = time(0);
    mob->player.time.played     = 0;
    mob->player.time.logon  = time(0);

    fscanf(mob_f, " %ld ", &tmp);
    mob->player.weight = tmp;

    fscanf(mob_f, " %ld \n", &tmp);
    mob->player.height = tmp;

    for (i = 0; i < 3; i++)
    {
      fscanf(mob_f, " %ld ", &tmp);
      GET_COND(mob, i) = tmp;
    }
    fscanf(mob_f, " \n ");

    for (i = 0; i < 5; i++)
    {
      fscanf(mob_f, " %ld ", &tmp);
      mob->specials.apply_saving_throw[i] = tmp;
    }

    fscanf(mob_f, " \n ");

    /* Set the damage as some standard 1d4 */
    mob->points.damroll = 0;
    mob->specials.damnodice = 1;
    mob->specials.damsizedice = 6;

    /* Calculate THAC0 as a formular of Level */
    mob->points.hitroll = MAX(1, GET_LEVEL(mob,WARRIOR_LEVEL_IND)-3);
  }

  mob->tmpabilities = mob->abilities;

  for (i = 0; i < MAX_WEAR; i++) /* Initialisering Ok */
    mob->equipment[i] = 0;

  mob->nr = nr;

  mob->desc = 0;

  if (!IS_SET(mob->specials.act, ACT_ISNPC))
    SET_BIT(mob->specials.act, ACT_ISNPC);

  mob->generic = 0;
  mob->commandp = 0;
  mob->commandp2 = 0;
  mob->waitp = 0;

  /* Check to see if associated with a script, if so, set it up */
  if(IS_SET(mob->specials.act, ACT_SCRIPT))
    REMOVE_BIT(mob->specials.act, ACT_SCRIPT);

  for(i = 0; i < top_of_scripts; i++)
  {
    if(gpScript_data[i].iVNum == mob_index[nr].iVNum)
    {
      SET_BIT(mob->specials.act, ACT_SCRIPT);
      mob->script = i;
      break;
    }
  }

  /* insert in list */

  mob->next = character_list;
  character_list = mob;

#if LOW_GOLD
  if (mob->points.gold >= 10)
    mob->points.gold /= 5;
  else if (mob->points.gold > 0)
    mob->points.gold = 1;
#endif

  if( mob->points.gold > GET_LEVEL( mob, WARRIOR_LEVEL_IND ) * 1500 )
    mudlog( LOG_MOBILES, "%s has gold > level * 1500 (%d)",
            mob->player.short_descr, mob->points.gold );

  /* set up things that all members of the race have */
  SetRacialStuff(mob);

  /* change exp for wimpy mobs (lower) */
  if (IS_SET(mob->specials.act, ACT_WIMPY))
    GET_EXP(mob) -= GET_EXP(mob)/10;

  /* change exp for agressive mobs (higher) */
  if (IS_SET(mob->specials.act, ACT_AGGRESSIVE))
  {
    GET_EXP(mob) += GET_EXP(mob)/10;
    /* big bonus for fully aggressive mobs for now */
    if (!IS_SET(mob->specials.act, ACT_WIMPY)||
        IS_SET(mob->specials.act, ACT_META_AGG))
      GET_EXP(mob) += (GET_EXP(mob)/2);
  }

  /* set up distributed movement system */

  mob->specials.tick = mob_tick_count++;

  if( mob_tick_count == TICK_WRAP_COUNT )
    mob_tick_count=0;

  mob_index[ nr ].number++;

#if BYTE_COUNT
  fprintf(stderr,"Mobile [%d]: byte count: %d\n", mob_index[nr].iVNum, bc);
#endif

  total_mbc += bc;
  mob_count++;

  mudlog( LOG_CHECK | LOG_SILENT,
          "Loaded mob %s (ADDR: %p, magic %d, next %p, #mobs %ld).",
          GET_NAME_DESC( mob ), mob, mob->nMagicNumber, mob->next, mob_count );

#if 0
  /* tell the spec_proc (if there is one) that we've been born */
  if( mob_index[ mob->nr ].func )
    ( *mob_index[ mob->nr ].func )( mob, 0, "", mob, EVENT_BIRTH );
#endif

  return(mob);
}


void clone_obj_to_obj(struct obj_data *obj, struct obj_data *osrc)
{
  struct extra_descr_data *new_descr, *tmp_descr;
  int i;

  if(osrc->name)
    obj->name = strdup(osrc->name);
  if(osrc->short_description)
    obj->short_description = strdup(osrc->short_description);
  if(osrc->description)
    obj->description = strdup(osrc->description);
  if(osrc->action_description)
    obj->action_description = strdup(osrc->action_description);

    /* *** numeric data *** */

  obj->obj_flags.type_flag    = osrc->obj_flags.type_flag;
  obj->obj_flags.extra_flags  = osrc->obj_flags.extra_flags;
  obj->obj_flags.wear_flags   = osrc->obj_flags.wear_flags;
  obj->obj_flags.value[0]     = osrc->obj_flags.value[0];
  obj->obj_flags.value[1]     = osrc->obj_flags.value[1];
  obj->obj_flags.value[2]     = osrc->obj_flags.value[2];
  obj->obj_flags.value[3]     = osrc->obj_flags.value[3];
  obj->obj_flags.weight       = osrc->obj_flags.weight;
  obj->obj_flags.cost         = osrc->obj_flags.cost;
  obj->obj_flags.cost_per_day = osrc->obj_flags.cost_per_day;

  /* *** extra descriptions *** */

  obj->ex_description = 0;

  if(osrc->ex_description)
  {
    for(tmp_descr=osrc->ex_description;tmp_descr;tmp_descr=tmp_descr->next)
    {
      CREATE(new_descr, struct extra_descr_data, 1);
      new_descr->nMagicNumber = EXDESC_VALID_MAGIC;
      if(tmp_descr->keyword)
        new_descr->keyword = strdup(tmp_descr->keyword);
      if(tmp_descr->description)
        new_descr->description = strdup(tmp_descr->description);
      new_descr->next = obj->ex_description;
      obj->ex_description = new_descr;
    }
  }

  for( i = 0 ; i < MAX_OBJ_AFFECT ; i++)
  {
    obj->affected[i].location = osrc->affected[i].location;
    obj->affected[i].modifier = osrc->affected[i].modifier;
  }

  if( osrc->szForbiddenWearToChar )
    obj->szForbiddenWearToChar = strdup( osrc->szForbiddenWearToChar );
  if( osrc->szForbiddenWearToRoom )
    obj->szForbiddenWearToRoom = strdup( osrc->szForbiddenWearToRoom );
}

int read_obj_from_file(struct obj_data *obj, FILE *f)
{
  int i,tmp;
  long bc = 0L;
  char chk[ 161 ];
  struct extra_descr_data *new_descr;

  SetStatus( "Entrato in read_obj_from_file", NULL );

  obj->name = fread_string(f);
  if (obj->name )
  {
    bc += strlen(obj->name);
  }
  obj->short_description = fread_string(f);
  if( obj->short_description )
  {
    bc += strlen(obj->short_description);
  }
  obj->description = fread_string(f);
  if( obj->description )
  {
    bc += strlen(obj->description);
  }
  obj->action_description = fread_string(f);
  if( obj->action_description )
  {
    bc += strlen(obj->action_description);
  }

  /* *** numeric data *** */

  SetStatus( "Reading numeric data in read_obj_from_file", NULL );

  obj->obj_flags.type_flag = fread_number( f );
  obj->obj_flags.extra_flags = fread_number( f );
  obj->obj_flags.wear_flags = fread_number( f );
  obj->obj_flags.value[0] = fread_number( f );
  obj->obj_flags.value[1] = fread_number( f );
  obj->obj_flags.value[2] = fread_number( f );
  obj->obj_flags.value[3] = fread_number( f );
  obj->obj_flags.weight = fread_number( f );
  obj->obj_flags.cost = fread_number( f );
  obj->obj_flags.cost_per_day = fread_number( f );

  SetStatus( "Reading Extra description in read_obj_from_file", NULL );
  /* *** extra descriptions *** */

  obj->ex_description = 0;

  while( fscanf( f, " %160s \n", chk ) == 1 && *chk == 'E' )
  {
    CREATE(new_descr, struct extra_descr_data, 1);
    new_descr->nMagicNumber = EXDESC_VALID_MAGIC;
    bc += sizeof(struct extra_descr_data);
    new_descr->keyword = fread_string(f);
    if( new_descr->keyword )
      bc += strlen(new_descr->keyword);
    new_descr->description = fread_string(f);
    if( new_descr->description )
      bc += strlen(new_descr->description);

    new_descr->next = obj->ex_description;
    obj->ex_description = new_descr;
  }

  SetStatus( "Reading affect in read_obj_from_file", NULL );

  for( i = 0 ; (i < MAX_OBJ_AFFECT) && (*chk == 'A') ; i++)
  {
    fscanf(f, " %d ", &tmp);
    obj->affected[i].location = tmp;
    fscanf(f, " %d \n", &tmp);
    obj->affected[i].modifier = tmp;
    if( fscanf(f, " %160s \n", chk) != 1 )
    {
      i++;
      break;
    }
  }

  for (;(i < MAX_OBJ_AFFECT);i++)
  {
    obj->affected[i].location = APPLY_NONE;
    obj->affected[i].modifier = 0;
  }

  SetStatus( "Reading forbidden string in read_obj_from_file", NULL );

  if( *chk == 'P' )
  {
    obj->szForbiddenWearToChar = fread_string( f );
    obj->szForbiddenWearToRoom = fread_string( f );
    fscanf( f, " %160s \n", chk );
  }
  else
  {
    obj->szForbiddenWearToChar = NULL;
    obj->szForbiddenWearToRoom = NULL;
  }

  SetStatus( "Returning from read_obj_from_file", NULL );

  return bc;
}

void write_obj_to_file(struct obj_data *obj, FILE *f)
{
  int         i;
  struct extra_descr_data *descr;


  fprintf(f,"#%d\n", obj->item_number >= 0 ?
                     obj_index[obj->item_number].iVNum : 0 );
  fwrite_string(f, obj->name);
  fwrite_string(f, obj->short_description);
  fwrite_string(f, obj->description);
  fwrite_string(f, obj->action_description);

  fprintf(f,"%d %ld %ld\n", obj->obj_flags.type_flag,
          obj->obj_flags.extra_flags, obj->obj_flags.wear_flags);
  fprintf(f,"%d %d %d %d\n", obj->obj_flags.value[0], obj->obj_flags.value[1],
          obj->obj_flags.value[2], obj->obj_flags.value[3]);
  fprintf(f,"%d %d %d\n", obj->obj_flags.weight,
          obj->obj_flags.cost, obj->obj_flags.cost_per_day);

  /* *** extra descriptions *** */
  if(obj->ex_description)
    for(descr=obj->ex_description;descr;descr=descr->next)
  {
    fprintf(f,"E\n");
    fwrite_string(f,descr->keyword);
    fwrite_string(f,descr->description);
  }

  for( i = 0 ; i < MAX_OBJ_AFFECT ; i++)
  {
    if(obj->affected[i].location!=APPLY_NONE)
      fprintf(f,"A\n%d %ld\n",obj->affected[i].location,
              obj->affected[i].modifier);
  }

  if( obj->szForbiddenWearToChar )
  {
    fprintf( f, "P\n" );
    fwrite_string( f, obj->szForbiddenWearToChar );
    fwrite_string( f, obj->szForbiddenWearToRoom );
  }

}

/* read an object from OBJ_FILE */
struct obj_data *read_object(int nr, int type)
{
  FILE *f;
  struct obj_data *obj;
  int i;
  long bc;
  char buf[100];

  extern long obj_count;
  extern long total_obc;

  SetStatus( "read_object start", NULL );
  i = nr;
  if( type == VIRTUAL )
  {
    SetStatus( "before real_object", NULL );
    nr = real_object( nr );
  }
  if( nr < 0 || nr >= top_of_objt )
  {
    mudlog( LOG_ERROR, "Object (V) %d does not exist in database.", i );
    return NULL;
  }

  SetStatus( "before CREATE object", NULL );

  CREATE(obj, struct obj_data, 1);
  bc = sizeof(struct obj_data);

  SetStatus( "before clear_object", NULL );
  clear_object(obj);

  if(obj_index[nr].data == NULL)
  {
    /* object haven't data structure */
    if(obj_index[nr].pos == -1)
    {
      /* object in external file */
      sprintf(buf,"%s/%d",OBJ_DIR,obj_index[nr].iVNum);
      if((f = fopen(buf,"rt"))==NULL)
      {
        mudlog( LOG_ERROR, "can't open object file for object %d",
                obj_index[nr].iVNum);
        free(obj);
        return(0);
      }
      fscanf( f, "#%*d \n" );
      SetStatus( "before read_obj_from_file 1", NULL );
      read_obj_from_file(obj, f);
      fclose(f);
    }
    else
    {
      if( fseek(obj_f, obj_index[nr].pos, 0) == 0 )
      {
        SetStatus( "before read_obj_from_file 2", NULL );
        read_obj_from_file(obj, obj_f);
      }
      else
      {
        mudlog( LOG_ERROR,
                "Cannot seek obj file at %l for obj n. %d(%d) in "
                "read_object (%s).", obj_index[nr].pos, nr,
                obj_index[nr].iVNum, __FILE__ );
        free( obj );
        return NULL;
      }
    }
  }
  else
  {
    SetStatus( "before clone_obj_to_obj", NULL );
    /* data for object present */
    clone_obj_to_obj(obj, (struct obj_data *)obj_index[nr].data);
  }

  SetStatus( "before inzializing object", NULL );

  obj->in_room = NOWHERE;
  obj->next_content = 0;
  obj->carried_by = 0;
  obj->equipped_by = 0;
  obj->eq_pos = -1;
  obj->in_obj = 0;
  obj->contains = 0;
  obj->item_number = nr;
  obj->in_obj = 0;

  obj->next = object_list;
  object_list = obj;

  obj_index[nr].number++;

  obj_count++;
#if BYTE_COUNT
  fprintf(stderr, "Object [%d] uses %d bytes\n", obj_index[nr].iVNum, bc);
#endif
  total_obc += bc;

  SetStatus( "ending read_object", NULL );

  return (obj);
}




#define ZO_DEAD  999

/* update zone ages, queue for reset if necessary, and dequeue when possible */
void zone_update()
{
  int i;
  struct reset_q_element *update_u, *temp, *tmp2;


  /* enqueue zones */

  for (i = 0; i <= top_of_zone_table; i++)
  {
    if (zone_table[i].start)
    {
      if (zone_table[i].age < zone_table[i].lifespan &&
          zone_table[i].reset_mode)
      {

        (zone_table[i].age)++;
      }
      else if (zone_table[i].age < ZO_DEAD && zone_table[i].reset_mode)
      {

        /* enqueue zone */

        CREATE(update_u, struct reset_q_element, 1);

        update_u->zone_to_reset = i;
        update_u->next = 0;

        if (!gReset_q.head)
          gReset_q.head = gReset_q.tail = update_u;
        else
        {
          gReset_q.tail->next = update_u;
          gReset_q.tail = update_u;
        }

        zone_table[i].age = ZO_DEAD;
      }
    }
  }

  /* dequeue zones (if possible) and reset */

  for (update_u = gReset_q.head; update_u; update_u = tmp2)
  {
    if (update_u->zone_to_reset > top_of_zone_table)
    {

      /*  this may or may not work */
      /*  may result in some lost memory, but the loss is not signifigant
       *   over the short run
       */
      update_u->zone_to_reset = 0;
      update_u->next = 0;
    }
    tmp2 = update_u->next;

    if (IS_SET(zone_table[update_u->zone_to_reset].reset_mode, ZONE_ALWAYS) ||
       (IS_SET(zone_table[update_u->zone_to_reset].reset_mode, ZONE_EMPTY) &&
               is_empty(update_u->zone_to_reset)))
    {
      SetStatus( "Before reset_zone", NULL );
      reset_zone(update_u->zone_to_reset);
      SetStatus( "After reset_zone", NULL );
      /* dequeue */

      if (update_u == gReset_q.head)
        gReset_q.head = gReset_q.head->next;
      else
      {
        for (temp = gReset_q.head; temp->next != update_u; temp = temp->next)
          ;

        if (!update_u->next)
          gReset_q.tail = temp;

        temp->next = update_u->next;
      }
      free(update_u);
    }
  }
}

#if 0
typedef struct tagZoneCommand
{
  int nCmdNo;


} ZoneCommand;

void ExecuteZoneCommand( ZoneCommand *pZC, NumberType NT )
{

}
#endif

#define ZCMD zone_table[zone].cmd[cmd_no]

/* execute the reset command table of a given zone */

/* I have gotten a memory out of bounds on this function, not sure where the */
/* problem came from... need to look for it, could possibly be a pointer */
/* going out of range or a variable not getting assigned. msw */

void reset_zone(int zone)
{
  int cmd_no, nLastCmd = TRUE, i;
  char buf[256];
  struct char_data *pMob = NULL;
  struct char_data *pMaster = NULL;
  struct obj_data *pObj, *pCont;
  struct room_data      *rp;
  FILE *fl;
  static int done = FALSE;
  struct char_data *pLastMob = 0;
  struct obj_data *pLastCont = 0;

  if( zone == 0 && !done )
  {
    done = TRUE;

    for( i = 0; i < WORLD_SIZE; i += 1000 )
    {
      sprintf( buf, "world/mobs.%d", i );
      fl = fopen(buf, "r");
      if( !fl )
      {
        mudlog( LOG_ERROR, "Unable to load scratch zone file for update.");
        return;
      }
      ReadTextZone( fl );
      fclose( fl );
    }
    return;
  }



  {
    char *s;
    int d, e;
    s = zone_table[zone].name;
    d = (zone ? (zone_table[zone - 1].top + 1) : 0);
    e = zone_table[zone].top;
    if( zone_table[zone].start == 0 )
      sprintf( buf, "Run time initialization of zone %s (%d), rooms (%d-%d)",
               s, zone, d, e );
    else
      sprintf( buf, "Run time reset of zone %s (%d), rooms (%d-%d)",
               s, zone, d, e );

    mudlog( LOG_CHECK, buf);
  }

  if( !zone_table[zone].cmd )
    return;

  for( cmd_no = 0; ; cmd_no++ )
  {
    if (ZCMD.command == 'S')
      break;

    if( nLastCmd || ZCMD.if_flag <= 0 )
    {
      switch( ZCMD.command )
      {
       case 'M': /* read a mobile */
        SetStatus( "Command M", NULL );
        rp = real_roomp( ZCMD.arg3 );
        if( ( ZCMD.arg2 == 0 || mob_index[ ZCMD.arg1 ].number < ZCMD.arg2 ) &&
            !fighting_in_room( ZCMD.arg3 ) &&
            !CheckKillFile( mob_index[ZCMD.arg1].iVNum ) &&
            ( pMob = read_mobile(ZCMD.arg1, REAL) ) != NULL &&
            rp != NULL )
        {
          pLastMob = pMaster = pMob;
          pMob->specials.zone = zone;
          char_to_room( pMob, ZCMD.arg3 );
          if( IS_SET( pMob->specials.act, ACT_SENTINEL ) )
            pMob->lStartRoom = ZCMD.arg3;

          if( GET_RACE( pMob ) > RACE_GNOME &&
              !strchr( zone_table[ zone ].races, GET_RACE( pMob ) ) )
          {
            zone_table[ zone ].races[ strlen( zone_table[ zone ].races ) ] =
              GET_RACE( pMob );
          }

          nLastCmd = TRUE;
        }
        else
        {
          pLastMob = pMaster = pMob = NULL;
          nLastCmd = FALSE;
        }
        if( rp == NULL )
          mudlog( LOG_ERROR, "Cannot find room #%d", ZCMD.arg3);
        break;

      case 'C': /* read a mobile.  Charm them to follow prev. */
        SetStatus( "Command C", NULL );
        if( ( ZCMD.arg2 != 0 || mob_index[ ZCMD.arg1 ].number < ZCMD.arg2 ) &&
            !CheckKillFile( mob_index[ ZCMD.arg1 ].iVNum ) && pMaster &&
            ( pMob = read_mobile( ZCMD.arg1, REAL ) ) != NULL )
        {
          pLastMob = pMob;
          pMob->specials.zone = zone;

          if( GET_RACE( pMob ) > RACE_GNOME &&
              !strchr( zone_table[ zone ].races, GET_RACE( pMob ) ) )
            zone_table[ zone ].races[ strlen( zone_table[ zone ].races ) ] =
                  GET_RACE( pMob );

          char_to_room( pMob, pMaster->in_room );
          /* add the charm bit to the dude.  */
          add_follower( pMob, pMaster );
          SET_BIT( pMob->specials.affected_by, AFF_CHARM );
          SET_BIT( pMob->specials.act, ZCMD.arg3 );
          nLastCmd = TRUE;
        }
        else
        {
          pLastMob = pMob = NULL;
          nLastCmd = FALSE;
        }
        break;

      case 'Z':  /* set the last mobile to this zone */
        SetStatus( "Command Z", NULL );
        if( pLastMob )
        {
          pLastMob->specials.zone = ZCMD.arg1;

          if( GET_RACE( pLastMob ) > RACE_GNOME &&
              !strchr( zone_table[ ZCMD.arg1 ].races, GET_RACE( pLastMob ) ) )
            zone_table[ZCMD.arg1].races[strlen(zone_table[ZCMD.arg1].races)] =
                  GET_RACE( pLastMob);
        }
        break;

      case 'O': /* read an object */
        SetStatus( "Command O", NULL );
        pObj = NULL;
        nLastCmd = FALSE;
        if( ZCMD.arg1 >= 0 && ( ZCMD.arg2 == 0 ||
                                obj_index[ ZCMD.arg1 ].number < ZCMD.arg2 ) )
        {
          if( ZCMD.arg3 >= 0 && ( ( rp = real_roomp( ZCMD.arg3 ) ) != NULL ) )
          {
            if( ZCMD.arg4 == 0 || ObjRoomCount( ZCMD.arg1, rp ) < ZCMD.arg4 )
            {
              if( ( pObj = read_object( ZCMD.arg1, REAL ) ) != NULL )
              {
                obj_to_room( pObj, ZCMD.arg3 );
                nLastCmd = TRUE;
                if( ITEM_TYPE( pObj ) == ITEM_CONTAINER )
                  pLastCont = pObj;
              }
            }
          }
          else
          {
            mudlog( LOG_ERROR, "Cannot find room #%d", ZCMD.arg3);
          }
        }
        break;

      case 'P': /* object to object */
        SetStatus( "Command P", NULL );
        if( ZCMD.arg1 >= 0 && ( ZCMD.arg2 == 0 ||
                                obj_index[ ZCMD.arg1 ].number < ZCMD.arg2 ) &&
            ( pCont = get_obj_num( ZCMD.arg3 ) ) != NULL &&
            ( pObj = read_object( ZCMD.arg1, REAL ) ) != NULL )
        {
          obj_to_obj( pObj, pCont );
          nLastCmd = TRUE;
        }
        else
        {
          pObj = pCont = NULL;
          nLastCmd = FALSE;
        }
        break;

      case 'G': /* obj_to_char */
        SetStatus( "Command G", NULL );
        if( ZCMD.arg1 >= 0 && ( ZCMD.arg2 == 0 ||
                                obj_index[ ZCMD.arg1 ].number < ZCMD.arg2 ) &&
            pLastMob && ( pObj = read_object( ZCMD.arg1, REAL ) ) != NULL )
        {
          obj_to_char( pObj, pLastMob );
          if( ITEM_TYPE( pObj ) == ITEM_CONTAINER )
            pLastCont = pObj;
        }
        break;

      case 'H': /* hatred to char */
        SetStatus( "Command H", NULL );
        if( pLastMob )
          AddHatred( pLastMob, ZCMD.arg1, ZCMD.arg2 );
        break;

      case 'F': /* fear to char */
        SetStatus( "Command F", NULL );
        if( pLastMob )
          AddFears( pLastMob, ZCMD.arg1, ZCMD.arg2 );
        break;

      case 'E': /* object to equipment list */
        SetStatus( "Command E", NULL );
        if( ZCMD.arg1 >= 0 && ( ZCMD.arg2 == 0 ||
                                obj_index[ZCMD.arg1].number < ZCMD.arg2 ) &&
            pLastMob && ( pObj = read_object( ZCMD.arg1, REAL ) ) != NULL )
        {
          if( !pLastMob->equipment[ ZCMD.arg3 ] )
          {
            equip_char( pLastMob, pObj, ZCMD.arg3);
            if( ITEM_TYPE( pObj ) == ITEM_CONTAINER )
              pLastCont = pObj;
          }
          else
          {
            mudlog( LOG_ERROR, "eq error - zone %d, cmd %d, item %d, mob %d, "
                               "loc %d\n", zone, cmd_no,
                    obj_index[ ZCMD.arg1 ].iVNum,
                    mob_index[ pLastMob->nr ].iVNum, ZCMD.arg3 );
          }
        }
        break;

      case 'D': /* set state of door */
        SetStatus( "Command D", NULL );
        rp = real_roomp( ZCMD.arg1 );
        if( rp && rp->dir_option[ZCMD.arg2] )
        {
          if( !IS_SET( rp->dir_option[ZCMD.arg2]->exit_info, EX_ISDOOR ) )
          {
            mudlog( LOG_ERROR, "Door error - zone %d, cmd %d, loc %d (fixed)",
                    zone, cmd_no, ZCMD.arg1 );
            SET_BIT( rp->dir_option[ZCMD.arg2]->exit_info, EX_ISDOOR );
          }
          switch (ZCMD.arg3)
          {
          case 0:
            REMOVE_BIT(rp->dir_option[ZCMD.arg2]->exit_info, EX_LOCKED);
            REMOVE_BIT(rp->dir_option[ZCMD.arg2]->exit_info, EX_CLOSED);
            break;
          case 1:
            SET_BIT(rp->dir_option[ZCMD.arg2]->exit_info, EX_CLOSED);
            REMOVE_BIT(rp->dir_option[ZCMD.arg2]->exit_info, EX_LOCKED);
            break;
          case 2:
            SET_BIT(rp->dir_option[ZCMD.arg2]->exit_info, EX_LOCKED);
            SET_BIT(rp->dir_option[ZCMD.arg2]->exit_info, EX_CLOSED);
            break;
          }
        }
        else
        {
          /* that exit doesn't exist anymore */
          mudlog( LOG_ERROR, "Exit error - zone %d, cmd %d, loc %d",
                   zone, cmd_no, ZCMD.arg1 );
        }
        break;

      default:
        mudlog( LOG_ERROR, "Undefd cmd in reset table; zone %d cmd# %d\n\r",
                 zone, cmd_no );
        break;
      }
    }
    else
      nLastCmd = FALSE;
  }

  zone_table[zone].age = 0;
  zone_table[zone].start = 1;

}

#undef ZCMD

/* for use in reset_zone; return TRUE if zone 'nr' is free of PC's  */
int is_empty(int zone_nr)
{
  struct descriptor_data *i;

  for (i = descriptor_list; i; i = i->next)
    if (!i->connected)
      if (real_roomp(i->character->in_room)->zone == zone_nr)
        return(0);

  return(1);
}





/*************************************************************************
*  stuff related to the save/load player system                                                           *
*********************************************************************** */

/* Load a char, TRUE if loaded, FALSE if not */
int load_char(char *name, struct char_file_u *char_element)
{
  FILE *fl;

#if defined( EMANUELE )
  char szFileName[ 41 ];

  sprintf( szFileName, "%s/%s.dat", PLAYERS_DIR, lower( name ) );
  if( ( fl = fopen( szFileName, "r" ) ) != NULL )
  {
    fread( char_element, sizeof( struct char_file_u ), 1, fl );
    fclose( fl );
    /*
     **  Kludge for ressurection
     */
    char_element->talks[2] = FALSE; /* they are not dead */
    return TRUE;
  }
  else
    return FALSE;
#else
  int player_i;
  int find_name(char *name);

  if( ( player_i = find_name( name ) ) >= 0)
  {
    if (!(fl = fopen(PLAYER_FILE, "r")))
    {
      perror("Opening player file for reading. (db.c, load_char)");
      assert(0);
    }

    fseek(fl, (long) (player_table[player_i].nr *
                      sizeof(struct char_file_u)), 0);

    fread(char_element, sizeof(struct char_file_u), 1, fl);
    fclose(fl);
    /*
     **  Kludge for ressurection
     */
    char_element->talks[2] = FALSE; /* they are not dead */
    return(player_i);
  }
  else
    return(-1);
#endif
}




/* copy data from the file structure to a char struct */
void store_to_char(struct char_file_u *st, struct char_data *ch)
{
  int i;
  int max;


  GET_SEX(ch) = st->sex;
  ch->player.iClass = st->iClass;


  for( i = MAGE_LEVEL_IND; i < MAX_CLASS; i++ )
    ch->player.level[i] = st->level[i];

  /* to make sure all levels above the normal are 0 */
  for (i=MAX_CLASS;i<=ABS_MAX_CLASS;i++)
    ch->player.level[i] = 0;



  GET_RACE(ch)  = st->race;

  ch->player.short_descr = 0;
  ch->player.long_descr = 0;

  if (*st->title)
  {
    CREATE(ch->player.title, char, strlen(st->title) + 1);
    strcpy(ch->player.title, st->title);
  }
  else
    GET_TITLE(ch) = 0;

  if (*st->description)
  {
    CREATE(ch->player.description, char, strlen(st->description) + 1);
    strcpy(ch->player.description, st->description);
  }
  else
    ch->player.description = 0;


  ch->player.hometown = st->hometown;

  ch->player.time.birth = st->birth;

  ch->player.time.played = st->played;

  ch->player.time.logon  = time(0);

  for (i = 0; i <= MAX_TOUNGE - 1; i++)
    ch->player.talks[i] = st->talks[i];

  ch->player.weight = st->weight;
  ch->player.height = st->height;

  ch->abilities = st->abilities;
  ch->tmpabilities = st->abilities;
  ch->points = st->points;


  SpaceForSkills(ch);

  if( IS_IMMORTAL( ch ) )
    max = 100;
  else if( HowManyClasses(ch) >= 3 )
    max = 81;
  else if( HowManyClasses(ch) == 2 )
    max = 86;
  else
    max = 95;

  for (i = 0; i <= MAX_SKILLS - 1; i++)
  {
    ch->skills[i].flags   = st->skills[i].flags;
    ch->skills[i].special = st->skills[i].special;
    ch->skills[i].nummem  = st->skills[i].nummem;
    ch->skills[i].learned = MIN(st->skills[i].learned, max);
  }


  ch->specials.spells_to_learn = st->spells_to_learn;
  ch->specials.alignment    = st->alignment;

  ch->specials.act          = st->act;
  ch->specials.carry_weight = 0;
  ch->specials.carry_items  = 0;
  ch->specials.pmask        = 0;
  ch->specials.poofin       = 0;
  ch->specials.poofout      = 0;
  ch->specials.group_name   = 0;
  ch->points.armor          = 100;
  ch->points.hitroll        = 0;
  ch->points.damroll        = 0;
  ch->specials.affected_by  = st->affected_by;
  ch->specials.affected_by2  = st->affected_by2;
  ch->specials.start_room     = st->startroom;

  ch->player.speaks = st->speaks;
  ch->player.user_flags = st->user_flags;

  ch->player.extra_flags    = st->extra_flags;

  CREATE(GET_NAME(ch), char, strlen(st->name) +1);
  strcpy(GET_NAME(ch), st->name);


  for(i = 0; i <= 4; i++)
    ch->specials.apply_saving_throw[i] = 0;

  for(i = 0; i <= 2; i++)
    GET_COND(ch, i) = st->conditions[i];

  /* Add all spell effects */
  for(i=0; i < MAX_AFFECT; i++)
    if (st->affected[i].type)
      affect_to_char(ch, &st->affected[i]);

  ch->in_room = st->load_room;

  ch->term = 0;

  /* set default screen size */
  ch->size = 25;

  affect_total(ch);

  ch->nMagicNumber = CHAR_VALID_MAGIC;
} /* store_to_char */




/* copy vital data from a players char-structure to the file structure */
void char_to_store(struct char_data *ch, struct char_file_u *st)
{
  int i;
  struct affected_type *af;
  struct obj_data *char_eq[MAX_WEAR];

  /* Unaffect everything a character can be affected by */

  for(i=0; i<MAX_WEAR; i++)
  {
    if (ch->equipment[i])
      char_eq[i] = unequip_char(ch, i);
    else
      char_eq[i] = 0;
  }

  for(af = ch->affected, i = 0; i<MAX_AFFECT; i++)
  {
    if (af)
    {
      st->affected[i] = *af;
      st->affected[i].next = 0;
      /* subtract effect of the spell or the effect will be doubled */
      affect_modify( ch, st->affected[i].location,
                    st->affected[i].modifier,
                    st->affected[i].bitvector, FALSE);
      af = af->next;
    }
    else
    {
      st->affected[i].type = 0;  /* Zero signifies not used */
      st->affected[i].duration = 0;
      st->affected[i].modifier = 0;
      st->affected[i].location = 0;
      st->affected[i].bitvector = 0;
      st->affected[i].next = 0;
    }
  }

  if ((i >= MAX_AFFECT) && af && af->next)
    mudlog( LOG_CHECK, "WARNING: OUT OF STORE ROOM FOR AFFECTED TYPES!!!");



  ch->tmpabilities = ch->abilities;

  st->birth      = ch->player.time.birth;
  st->played     = ch->player.time.played;
  st->played    += (long) (time(0) - ch->player.time.logon);
  st->last_logon = time(0);

  ch->player.time.played = st->played;
  ch->player.time.logon = time(0);

  st->hometown = ch->player.hometown;
  st->weight   = GET_WEIGHT(ch);
  st->height   = GET_HEIGHT(ch);
  st->sex      = GET_SEX(ch);
  st->iClass    = ch->player.iClass;

  for (i=MAGE_LEVEL_IND; i< MAX_CLASS; i++)
    st->level[i]    = ch->player.level[i];

  st->race            = GET_RACE(ch);

  ch->specials.charging=0;  /* null it out to be sure. */
  ch->specials.charge_dir=-1; /* null it out */

  st->abilities       = ch->abilities;
  st->points          = ch->points;
  st->alignment       = ch->specials.alignment;
  st->spells_to_learn = ch->specials.spells_to_learn;
  st->act             = ch->specials.act;
  st->affected_by     = ch->specials.affected_by;
  st->affected_by2    = ch->specials.affected_by2;
                          /* do not store group_name */
  st->startroom       = ch->specials.start_room;
  st->extra_flags     = ch->player.extra_flags;


  st->speaks = ch->player.speaks;
  st->user_flags = ch->player.user_flags;

  st->points.armor   = 100;
  st->points.hitroll =  0;
  st->points.damroll =  0;

  if (GET_TITLE(ch))
    strcpy(st->title, GET_TITLE(ch));
  else
    *st->title = '\0';

  if (ch->player.description)
    strcpy(st->description, ch->player.description);
  else
    *st->description = '\0';


  for (i = 0; i <= MAX_TOUNGE - 1; i++)
    st->talks[i] = ch->player.talks[i];

  for (i = 0; i <= MAX_SKILLS - 1; i++)
  {
    st->skills[i] = ch->skills[i];
    st->skills[i].flags   = ch->skills[i].flags;
    st->skills[i].special = ch->skills[i].special;
    st->skills[i].nummem  = ch->skills[i].nummem;
  }

  strcpy(st->name, GET_NAME(ch) );

  for(i = 0; i <= 4; i++)
    st->apply_saving_throw[i] = ch->specials.apply_saving_throw[i];

  for(i = 0; i <= 2; i++)
    st->conditions[i] = GET_COND(ch, i);

  for(af = ch->affected, i = 0; i<MAX_AFFECT; i++)
  {
    if (af)
    {
      /* Add effect of the spell or it will be lost */
      /* When saving without quitting               */
      affect_modify( ch, st->affected[i].location,
                         st->affected[i].modifier,
                    st->affected[i].bitvector, TRUE);
      af = af->next;
    }
  }

  for(i=0; i<MAX_WEAR; i++)
  {
    if (char_eq[i])
      equip_char(ch, char_eq[i], i);
  }

  affect_total(ch);
} /* Char to store */




/* create a new entry in the in-memory index table for the player file */
int create_entry(char *name)
{
  int i;

  if (top_of_p_table == -1)     {
    CREATE(player_table, struct player_index_element, 1);
    top_of_p_table = 0;
  }  else
    if (!(player_table = (struct player_index_element *)
          realloc(player_table, sizeof(struct player_index_element) *
                  (++top_of_p_table + 1))))
      {
        perror("create entry");
        assert(0);
      }

  CREATE(player_table[top_of_p_table].name, char , strlen(name) + 1);

  /* copy lowercase equivalent of name to table field */
  for( i = 0;
       ( *(player_table[top_of_p_table].name + i) = LOWER( *( name + i ) ) );
       i++ );

  player_table[top_of_p_table].nr = top_of_p_table;

  return (top_of_p_table);
}



/* write the vital data of a player to the player file */
void save_char(struct char_data *ch, sh_int load_room)
{
  struct char_file_u st;
  FILE *fl;
  char szFileName[ 200 ];

  char mode[4];
  int expand;
  struct char_data *tmp;

  if( IS_NPC( ch ) && !IS_SET( ch->specials.act, ACT_POLYSELF ) )
    return;

  if (IS_NPC(ch))
  {
    if (!ch->desc)
      return;
    tmp = ch->desc->original;
    if (!tmp)
      return;
  }
  else
  {
    if (!ch->desc)
      return;
    tmp = 0;
  }

  if( ( expand = (ch->desc->pos > top_of_p_file) ) )
  {
    strcpy(mode, "a");
    top_of_p_file++;
  }  else
    strcpy(mode, "r+");

  if (!tmp)
    char_to_store(ch, &st);
  else
    char_to_store(tmp, &st);

  st.load_room = load_room;

  strcpy( st.pwd, ch->desc->pwd );

#if defined( EMANUELE )
  sprintf( szFileName, "%s/%s.dat", PLAYERS_DIR,
           tmp ? lower( tmp->player.name ) : lower( ch->player.name ) );
  if( ( fl = fopen( szFileName, "w+b" ) ) == NULL )
  {
    mudlog( LOG_ERROR, "Cannot open file %s for saving player.", szFileName );
    return;
  }
  fwrite( &st, sizeof( struct char_file_u ), 1, fl );
  fclose( fl );
#else
  if (!(fl = fopen(PLAYER_FILE, mode))) {
    perror("save char");
    assert(0);
  }

  if (!expand)
    fseek(fl, ch->desc->pos * sizeof(struct char_file_u), 0);

  fwrite(&st, sizeof(struct char_file_u), 1, fl);

  fclose(fl);
#endif

}

/* for possible later use with qsort */
int compare(struct player_index_element *arg1, struct player_index_element
        *arg2)
{
  return (str_cmp(arg1->name, arg2->name));
}

/************************************************************************
*  procs of a (more or less) general utility nature                     *
********************************************************************** */

int fwrite_string (FILE *fl, char *buf)
{
  if(buf)
    return (fprintf(fl, "%s~\n", buf));
  else
    return (fprintf(fl, "~\n"));
}

char *fread_string(FILE *f1)
{
  char buf[ MAX_STRING_LENGTH ];
  int i = 0, tmp;

  SetStatus( "In fread_string", NULL );

  buf[ 0 ] = '\0';

  while( i < MAX_STRING_LENGTH - 3 )
  {
    if( ( tmp = fgetc(f1) ) == EOF )
    {
      mudlog( LOG_ERROR, "Error '%s' reading file in fread_string",
               strerror( errno ) );
      break;
    }

    if(tmp == '~')
    {
      break;
    }

    buf[i++] = (char)tmp;
    if (buf[i-1] == '\n')
      buf[i++] = '\r';
  }

  if( i >= MAX_STRING_LENGTH - 3 )
  {
    /* We filled the buffer */
    mudlog( LOG_ERROR, "File too long (fread_string).");
    while( ( tmp = fgetc( f1 ) ) != EOF )
      if( tmp == '~' )
        break;
  }

  buf[ i ] = '\0';

  fgetc( f1 );

  char *pReturnString = NULL;

  if( strlen( buf ) )
  {

    SetStatus( "Prima di duplicare la stringa", NULL );
    pReturnString = (char *)strdup(buf);

    SetStatus( "Ritorno da fread_string", NULL );

    if( pReturnString == NULL )
      mudlog( LOG_ERROR, "Errore nel duplicare la stringa %s", buf );
  }

  return pReturnString;
}

/****************************************************************************
 * Legge un numero dl file puntata da pFIle. Se il numero contiene il
 * carattere | le due pozioni di numero vengono addizionate. Ad esempio
 * 4|128 diventa 132. Molto utile per i flags.
 ****************************************************************************/
long fread_number( FILE *pFile )
{
  long number;
  bool sign;
  char c;

  do
  {
    c = getc( pFile );
  } while( isspace(c) );

  number = 0;

  sign = FALSE;
  if( c == '+' )
  {
    c = getc( pFile );
  }
  else if ( c == '-' )
  {
    sign = TRUE;
    c = getc( pFile );
  }

  if( !isdigit(c) )
  {
    mudlog( LOG_ERROR, "Fread_number: bad format ? char %c", c );
    ungetc( c, pFile );
    return 0;
  }

  while( isdigit(c) )
  {
    number = number * 10 + c - '0';
    c = getc( pFile );
  }

  if( sign )
    number = 0 - number;

  if( c == '|' )
    number += fread_number( pFile );
  else if ( c != ' ' )
    ungetc( c, pFile );

  return number;
}

void fwrite_flag( FILE *pFile, unsigned long ulFlags )
{
  unsigned long ulBit = 1;
  short bPrimaVolta = TRUE;

  while( ulFlags )
  {
    if( ulFlags & 1 )
    {
      if( !bPrimaVolta )
        fprintf( pFile, "|" );
      else
        bPrimaVolta = FALSE;

      fprintf( pFile, "%lu", ulBit );
    }
    ulBit *= 2;
    ulFlags >>= 1;
  }
}




/* release memory allocated for a char struct */
void free_char(struct char_data *ch)
{
  struct affected_type *af, *pNext = NULL;
  int i;
  if( ch->nMagicNumber != CHAR_VALID_MAGIC )
  {
    mudlog( LOG_SYSERR,
            "Characters char %s with uncorrect magic number in free_char!",
            GET_NAME_DESC( ch ) );
    return;
  }

  mudlog( LOG_CHECK, "Freeing char %s (ADDR: %p, magic %d)",
          GET_NAME_DESC( ch ), ch, ch->nMagicNumber );

  if( GET_NAME( ch ) )
  {
    free(GET_NAME(ch));
    GET_NAME( ch ) = NULL;
  }
  if( ch->player.title )
  {
    free(ch->player.title);
    ch->player.title = NULL;
  }
  if( ch->player.short_descr )
  {
    free(ch->player.short_descr);
    ch->player.short_descr = NULL;
  }
  if( ch->player.long_descr )
  {
    free(ch->player.long_descr);
    ch->player.long_descr = NULL;
  }
  if( ch->player.description )
  {
    free(ch->player.description);
    ch->player.description = NULL;
  }
  if( ch->player.sounds )
  {
    free( ch->player.sounds );
    ch->player.sounds = NULL;
  }
  if( ch->player.distant_snds )
  {
    free(ch->player.distant_snds);
    ch->player.distant_snds = NULL;
  }
  if( ch->specials.A_list )
  {
    for (i=0;i<10;i++)
    {
      if( GET_ALIAS( ch, i ) )
      {
        free( GET_ALIAS( ch, i ) );
        GET_ALIAS( ch, i ) = NULL;
      }
    }
    free( ch->specials.A_list );
    ch->specials.A_list = NULL;
  }

  for( af = ch->affected; af; af = pNext )
  {
    pNext = af->next;
    affect_remove( ch, af );
  }


  if( ch->skills )
  {
    free( ch->skills );
    ch->skills = NULL;
  }
  if( ch->nMagicNumber != CHAR_FREEDED_MAGIC )
  {
    ch->nMagicNumber = CHAR_FREEDED_MAGIC;
    free( ch );
  }
}







/* release memory allocated for an obj struct */
void free_obj(struct obj_data *obj)
{
  struct extra_descr_data *pExDescr, *next_one;

  if( !obj )
  {
    /* bug fix, msw */
    mudlog( LOG_SYSERR, "!obj in free_obj, db.c");
    return;
  }
#if defined( EMANUELE )
  free(obj->name);
  obj->name = NULL;
  free(obj->description);
  obj->description = NULL;
  free(obj->short_description);
  obj->short_description = NULL;
  free(obj->action_description);
  obj->action_description = NULL;

  for( pExDescr = obj->ex_description; pExDescr; pExDescr = next_one )
  {
    if( pExDescr->nMagicNumber == EXDESC_VALID_MAGIC )
    {
      next_one = pExDescr->next;
      pExDescr->nMagicNumber = EXDESC_FREED_MAGIC;
      free( pExDescr->keyword );
      pExDescr->keyword = NULL;
      free( pExDescr->description );
      pExDescr->description = NULL;
      free( pExDescr );
    }
    else
    {
      next_one = NULL;
      mudlog( LOG_SYSERR,
              "Invalid extra description freeing object in free_obj (db.c)" );
    }
  }
  obj->ex_description = NULL;

  free( obj->szForbiddenWearToChar );
  obj->szForbiddenWearToChar = NULL;
  free( obj->szForbiddenWearToRoom );
  obj->szForbiddenWearToRoom = NULL;
#else

  if (obj->name && *obj->name)    /* msw, bug fix */
    free(obj->name);
  if(obj->description && *obj->description)
    free(obj->description);
  if(obj->short_description && *obj->short_description)
    free(obj->short_description);
  if(obj->action_description && *obj->action_description)
    free(obj->action_description);

  for( pExDescr = obj->ex_description ; pExDescr != 0; pExDescr = next_one )
  {
    next_one = pExDescr->next;
    if( pExDescr->keyword )
      free( pExDescr->keyword );
    if( pExDescr->description )
      free( pExDescr->description );
    free( pExDescr );
  }

  if( obj->szForbiddenWearToChar )
    free( obj->szForbiddenWearToChar );
  if( obj->szForbiddenWearToRoom )
    free( obj->szForbiddenWearToRoom );
#endif

  free(obj);
}






/* read contents of a text file, and place in buf */
int file_to_string(char *name, char *buf)
{
  FILE *fl;
  char tmp[100];

  *buf = '\0';

  if (!(fl = fopen(name, "r")))
  {
    sprintf(tmp,"[%s] file-to-string",name);
    perror(tmp);
    *buf = '\0';
    return(-1);
  }

  do
  {
    fgets(tmp, 99, fl);

    if (!feof(fl))
    {
      if (strlen(buf) + strlen(tmp) + 2 > MAX_STRING_LENGTH)
      {
        mudlog( LOG_ERROR,
                "fl->strng: string too big (db.c, file_to_string)");
        *buf = '\0';
        fclose(fl);
        return(-1);
      }

      strcat(buf, tmp);
      *(buf + strlen(buf) + 1) = '\0';
      *(buf + strlen(buf)) = '\r';
    }
  } while (!feof(fl));

  fclose(fl);

  return(0);
}


void ClearDeadBit(struct char_data *ch)
{

  FILE *fl;
  struct char_file_u st;

#if defined( EMANUELE )
  char szFileName[ 40 ];
  sprintf( szFileName, "%s/%s.dat", PLAYERS_DIR, lower( GET_NAME( ch ) ) );
  if( ( fl = fopen( szFileName, "r+" ) ) != NULL )
  {
    fread( &st, sizeof( st ), 1, fl );

    /* this is a serious kludge, and must be changed before multiple
       languages can be implemented */
    if( st.talks[ 2 ] )
    {
      st.talks[2] = 0;  /* fix the 'resurrectable' bit */
      fseek( fl, 0, 0 );
      fwrite( &st, sizeof( st ), 1, fl );
      ch->player.talks[2] = 0;  /* fix them both */
    }
    fclose(fl);
  }
  else
  {
    mudlog( LOG_ERROR, "Cannot open file %s.dat for player %s.",
            lower( GET_NAME( ch ) ), GET_NAME( ch ) );
  }
#else

  fl = fopen(PLAYER_FILE, "r+");
  if (!fl) {
    perror("player file");
    exit(0);
  }

  fseek(fl, ch->desc->pos * sizeof(struct char_file_u), 0);
  fread(&st, sizeof(struct char_file_u), 1, fl);
  /*
   **   this is a serious kludge, and must be changed before multiple
   **   languages can be implemented
   */
  if (st.talks[2]) {
    st.talks[2] = 0;  /* fix the 'resurrectable' bit */
    fseek(fl, ch->desc->pos * sizeof(struct char_file_u), 0);
    fwrite(&st, sizeof(struct char_file_u), 1, fl);
    ch->player.talks[2] = 0;  /* fix them both */
  }
  fclose(fl);
#endif

}

/* clear some of the the working variables of a char */
void reset_char(struct char_data *ch)
{
  struct affected_type *af;
  extern struct dex_app_type dex_app[];

  int i;

  for (i = 0; i < MAX_WEAR; i++) /* Initializing */
    ch->equipment[i] = 0;

  ch->followers = 0;
  ch->master = 0;
  ch->carrying = 0;
  ch->next = 0;

  ch->immune = 0;
  ch->M_immune = 0;
  ch->susc = 0;
  ch->mult_att = 1.0;

  if (!GET_RACE(ch))
    GET_RACE(ch) = RACE_HUMAN;

  if ((ch->player.iClass == 3) && (GET_LEVEL(ch, THIEF_LEVEL_IND)))
  {
    ch->player.iClass = 8;
    send_to_char("Setting your class to THIEF only.\n\r", ch);
  }

  for (i=0;i<MAX_CLASS;i++)
  {
    if (GET_LEVEL(ch, i) > BIG_GUY)
    {
      GET_LEVEL(ch,i) = 51;
    }
  }

  SET_BIT(ch->specials.act, PLR_ECHO);

  ch->hunt_dist = 0;
  ch->hatefield = 0;
  ch->fearfield = 0;
  ch->hates.clist = 0;
  ch->fears.clist = 0;

  /* AC adjustment */
  GET_AC(ch) = 100;

  GET_HITROLL(ch)=0;
  GET_DAMROLL(ch)=0;

  ch->next_fighting = 0;
  ch->next_in_room = 0;
  ch->specials.fighting = 0;
  ch->specials.position = POSITION_STANDING;
  ch->specials.default_pos = POSITION_STANDING;
  ch->specials.carry_weight = 0;
  ch->specials.carry_items = 0;
  ch->specials.spellfail = 101;

  if (GET_HIT(ch) <= 0)
    GET_HIT(ch) = 1;
  if (GET_MOVE(ch) <= 0)
    GET_MOVE(ch) = 1;
  if (GET_MANA(ch) <= 0)
    GET_MANA(ch) = 1;

  ch->points.max_mana = 0;
  ch->points.max_move = 0;

  if (IS_IMMORTAL(ch))
  {
    GET_BANK(ch) = 0;
    GET_GOLD(ch) = 100000;
  }

  if (GET_BANK(ch) > GetMaxLevel(ch)*100000)
  {
    mudlog( LOG_PLAYERS, "%s has %d coins in bank.", GET_NAME(ch),
            GET_BANK(ch));
  }
  if (GET_GOLD(ch) > GetMaxLevel(ch)*100000)
  {
    mudlog( LOG_PLAYERS, "%s has %d coins.", GET_NAME(ch), GET_GOLD(ch));
  }

  /* rimettiamo a posto le condizioni di affamato od assetato in modo che
   * qualche bug non tolga la necessita di bere o di mangiare al PC >:) */
  if( !IS_IMMORTAL( ch ) )
  {
    if( GET_COND( ch, DRUNK ) < 0 )
      GET_COND( ch, DRUNK ) = 0;
    if( GET_COND( ch, FULL ) < 0 )
      GET_COND( ch, FULL ) = 0;
    if( GET_COND( ch, THIRST ) < 0 )
      GET_COND( ch, THIRST ) = 0;
  }

  /*
   * Class specific Stuff
   */

  ClassSpecificStuff(ch);


  if (HasClass(ch, CLASS_MONK))
  {
    GET_AC(ch) -= MIN(150, (GET_LEVEL(ch, MONK_LEVEL_IND)*5));
    ch->points.max_move += GET_LEVEL(ch, MONK_LEVEL_IND);
  }

  /*
   * racial stuff
   */
  SetRacialStuff(ch);

  /*
   * update the affects on the character.
   */

  ch->specials.sev = LOG_SYSERR | LOG_ERROR | LOG_CONNECT;

  for(af = ch->affected; af; af=af->next)
  {
    affect_modify(ch, af->location, af->modifier, af->bitvector, TRUE);
  }

  if (!HasClass(ch, CLASS_MONK))
  {
    GET_AC(ch) += dex_app[ (int)GET_DEX(ch) ].defensive;
  }


  /* could add barbarian double dex bonus here.... ... Nah! */

  if (GET_AC(ch) > 100)
      GET_AC(ch) = 100;


  /*
   * clear out the 'dead' bit on characters
   */
  if (ch->desc)
    ClearDeadBit(ch);
  /*
   * Clear out berserk flags case there was a crash in a fight
   */
  if (IS_SET(ch->specials.affected_by2,AFF2_BERSERK))
  {
    REMOVE_BIT(ch->specials.affected_by2,AFF2_BERSERK);
  }
  /*
   * Clear out MAILING flags case there was a crash
   */
  if (IS_SET(ch->specials.act,PLR_MAILING))
    REMOVE_BIT(ch->specials.act,PLR_MAILING);

  /*
   * Clear out objedit flags
   */
  if (IS_SET(ch->player.user_flags,CAN_OBJ_EDIT))
    REMOVE_BIT(ch->player.user_flags,CAN_OBJ_EDIT);
  /*
   * Clear out group/order/AFK flags
   */

  REMOVE_BIT( ch->specials.affected_by, AFF_GROUP );
  if (IS_SET(ch->specials.affected_by2,AFF2_CON_ORDER))
    REMOVE_BIT(ch->specials.affected_by2,AFF2_CON_ORDER);
  if (IS_AFFECTED2(ch,AFF2_AFK))
    REMOVE_BIT(ch->specials.affected_by2,AFF2_AFK);

  /*
   * Remove bogus flags on mortals
   */

  if( IS_SET(ch->specials.act,PLR_NOHASSLE) &&
      GetMaxLevel(ch) < LOW_IMMORTAL)
  {
    REMOVE_BIT(ch->specials.act,PLR_NOHASSLE);
  }

  /* check spells and if lower than 95 remove special flag */
  if( !IS_IMMORTAL( ch ) )
  {
    for( i = 0; i < MAX_SKILLS - 1; i++ )
    {
      if( ch->skills[i].learned < 95 ||
          !IS_SET(ch->skills[i].flags,SKILL_KNOWN))
        ch->skills[i].special = 0;
    }
  }


  SetDefaultLang(ch);


  if( !strcmp(GET_NAME(ch),"Benem"))
  {
    GET_LEVEL(ch,0) = 60;
  }

  if( !strcmp(GET_NAME(ch),"Nemrac"))
  {
    GET_LEVEL(ch,11) = 59;
  }

  if( !strcmp(GET_NAME(ch),"Xilo"))
  {
    GET_LEVEL(ch,11) = 59;
  }

  /* this is to clear up bogus levels on people that where here before */
  /* these classes where made... */


  if (!HasClass(ch,CLASS_MAGIC_USER))
    ch->player.level[0] = 0;
  if (!HasClass(ch,CLASS_CLERIC))
    ch->player.level[1] = 0;
  if (!HasClass(ch,CLASS_THIEF))
    ch->player.level[3] =0;
  if (!HasClass(ch,CLASS_WARRIOR))
    ch->player.level[2] = 0;
  if (!HasClass(ch,CLASS_DRUID))
    ch->player.level[4] = 0;
  if (!HasClass(ch,CLASS_MONK))
    ch->player.level[5] = 0;
  if (!HasClass(ch,CLASS_BARBARIAN))
    ch->player.level[6] = 0;
  if (!HasClass(ch,CLASS_SORCERER))
    ch->player.level[7] = 0;
  if (!HasClass(ch,CLASS_PALADIN))
    ch->player.level[8]=0;
  if (!HasClass(ch,CLASS_RANGER))
    ch->player.level[9]=0;
  if (!HasClass(ch,CLASS_PSI))
    ch->player.level[10]=0;


#if 0
  /*
   * Fix problem with Sorcerer learned spells
   */
  if (HasClass(ch,CLASS_SORCERER))
  {
    for (i=0;i<MAX_SKILLS-1;i++)
    {
      if (IS_SET(ch->skills[i].flags,SKILL_KNOWN)
          && !IS_SET(ch->skills[i].flags,SKILL_KNOWN_CLERIC)
          && !IS_SET(ch->skills[i].flags,SKILL_KNOWN_SORCERER) )
        SET_BIT(ch->skills[i].flags,SKILL_KNOWN_SORCERER);
    } /* for */
  }
#endif

}               /* end */



/* clear ALL the working variables of a char and do NOT free any space alloc'ed*/
void clear_char(struct char_data *ch)
{
  memset(ch, '\0', sizeof(struct char_data));

  ch->in_room = NOWHERE;
  ch->specials.was_in_room = NOWHERE;
  ch->specials.position = POSITION_STANDING;
  ch->specials.default_pos = POSITION_STANDING;
  GET_AC(ch) = 100; /* Basic Armor */
  ch->size = 25;
  ch->nMagicNumber = CHAR_VALID_MAGIC;
}


void clear_object(struct obj_data *obj)
{
  memset(obj, '\0', sizeof(struct obj_data));

  obj->item_number = -1;
  obj->in_room      = NOWHERE;
  obj->eq_pos       = -1;
}




/* initialize a new character only if class is set */
void init_char(struct char_data *ch)
{
  int i;

  /* *** if this is our first player --- he be God *** */

  if (top_of_p_table < 0)
  {

    mudlog( LOG_CHECK, "Building FIRST CHAR, setting up IMPLEMENTOR STATUS!") ;

    GET_EXP(ch) = 24000000;
    GET_LEVEL(ch,0) = IMPLEMENTOR;
    ch->points.max_hit = 1000;

    /* set all levels */

    for (i=0;i<MAX_CLASS;i++)
    {
      if (GET_LEVEL(ch,i) < GetMaxLevel(ch))
        GET_LEVEL(ch,i) = GetMaxLevel(ch);
    }/* for */

    /* set all classes */
    for (i=1;i<=CLASS_PSI;i*=2)
    {
      if (!HasClass(ch,i))
        ch->player.iClass +=i;
    } /* for */

  } /* end implmentor setup */

  set_title(ch);

  ch->player.short_descr = 0;
  ch->player.long_descr = 0;
  ch->player.description = 0;

  ch->player.hometown = number(1,4);

  ch->player.time.birth = time(0);
  ch->player.time.played = 0;
  ch->player.time.logon = time(0);

  SET_BIT( ch->player.user_flags, USE_PAGING );

  for (i = 0; i < MAX_TOUNGE; i++)
    ch->player.talks[i] = 0;

  GET_STR(ch) = 9;
  GET_INT(ch) = 9;
  GET_WIS(ch) = 9;
  GET_DEX(ch) = 9;
  GET_CON(ch) = 9;
  GET_CHR(ch) = 9;

  /* make favors for sex */
  switch( GET_RACE(ch) )
  {
   case RACE_HUMAN:
    if (ch->player.sex == SEX_MALE)
    {
      ch->player.weight = number(120,180);
      ch->player.height = number(160,200);
    }
    else
    {
      ch->player.weight = number(100,160);
      ch->player.height = number(150,180);
    }
    break;

   case RACE_DWARF:
   case RACE_GNOME:
   case RACE_DARK_DWARF:
   case RACE_DEEP_GNOME:
    if (ch->player.sex == SEX_MALE)
    {
      ch->player.weight = number(120,180);
      ch->player.height = number(100,150);
    }
    else
    {
      ch->player.weight = number(100,160);
      ch->player.height = number(100,150);
    }
    break;

   case RACE_HALFLING:
    if (ch->player.sex == SEX_MALE)
    {
      ch->player.weight = number(70,120);
      ch->player.height = number(80,120);
    }
    else
    {
      ch->player.weight = number(60,110);
      ch->player.height = number(70,115);
    }
    break;

   case RACE_ELVEN:
   case RACE_DROW:
   case RACE_GOLD_ELF:
   case RACE_WILD_ELF:
   case RACE_SEA_ELF:
    if (ch->player.sex == SEX_MALE)
    {
      ch->player.weight = number(100,150);
      ch->player.height = number(160,200);
    }
    else
    {
      ch->player.weight = number(80,230);
      ch->player.height = number(150,180);
    }
    break;

   case RACE_HALF_ELVEN:
    if (ch->player.sex == SEX_MALE)
    {
      ch->player.weight = number(110,160);
      ch->player.height = number(140,180);
    }
    else
    {
      ch->player.weight = number(90,150);
      ch->player.height = number(140,170);
    }
    break;

   case RACE_HALF_OGRE:
    if (ch->player.sex == SEX_MALE)
    {
      ch->player.weight = number(200,400);
      ch->player.height = number(200,230);
    }
    else
    {
      ch->player.weight = number(180,350);
      ch->player.height = number(190,220);
    }
    break;

   case RACE_HALF_ORC:
    if (ch->player.sex == SEX_MALE)
    {
      ch->player.weight = number(120,180);
      ch->player.height = number(160,200);
    }
    else
    {
      ch->player.weight = number(100,160);
      ch->player.height = number(150,180);
    }
    break;

   case RACE_HALF_GIANT:
    if (ch->player.sex == SEX_MALE)
    {
      ch->player.weight = number(300,900);
      ch->player.height = number(300,400);
    }
    else
    {
      ch->player.weight = number(250,800);
      ch->player.height = number(290,350);
    }
    break;

   case RACE_ORC:
    if (ch->player.sex == SEX_MALE)
    {
      ch->player.weight = number(140,200);
      ch->player.height = number(150,190);
    }
    else
    {
      ch->player.weight = number(120,180);
      ch->player.height = number(140,170);
    }
    break;

   case RACE_GOBLIN:
    if (ch->player.sex == SEX_MALE)
    {
      ch->player.weight = number(60,90);
      ch->player.height = number(130,160);
    }
    else
    {
      ch->player.weight = number(60,90);
      ch->player.height = number(120,150);
    }
    break;

   case RACE_TROLL:
    if (ch->player.sex == SEX_MALE)
    {
      ch->player.weight = number(100,200);
      ch->player.height = number(120,170);
    }
    else
    {
      ch->player.weight = number(90,180);
      ch->player.height = number(110,160);
    }
    break;

   default:
    if (ch->player.sex == SEX_MALE)
    {
      ch->player.weight = number(120,180);
      ch->player.height = number(160,200);
    }
    else
    {
      ch->player.weight = number(100,160);
      ch->player.height = number(150,180);
    }
  }

  ch->points.mana = GET_MAX_MANA(ch);
  ch->points.hit = GET_MAX_HIT(ch);
  ch->points.move = GET_MAX_MOVE(ch);

  ch->points.armor = 100;

  if (!ch->skills)
    SpaceForSkills(ch);

  for (i = 0; i <= MAX_SKILLS - 1; i++)
  {
    if (GetMaxLevel(ch) <IMPLEMENTOR)
    {
      ch->skills[i].learned = 0;
      ch->skills[i].flags   = 0;
      ch->skills[i].special = 0;
      ch->skills[i].nummem  = 0;
    }
    else
    {
      ch->skills[i].learned = 100;
      ch->skills[i].flags   = 0;
      ch->skills[i].special = 1;
      ch->skills[i].nummem  = 0;
    }
  }

  ch->specials.affected_by = 0;
  ch->specials.spells_to_learn = 0;

  for (i = 0; i < 5; i++)
    ch->specials.apply_saving_throw[i] = 0;

  for (i = 0; i < 3; i++)
    GET_COND(ch, i) = (GetMaxLevel(ch) > GOD ? -1 : 24);
}

struct room_data *real_roomp( long lVNum )
{
#if HASH
  return hash_find( &room_db, lVNum );
#else
  return( ( lVNum < WORLD_SIZE && lVNum > -1 ) ? room_db[ lVNum ] : 0 );
#endif
}

/* returns the real number of the monster with given virtual number */
int real_mobile(int iVNum)
{
  int bot, top, mid;

  bot = 0;
  top = top_of_sort_mobt - 1;

  /* perform binary search on mob-table */
  for (;;)
  {
    mid = (bot + top) / 2;

    if( (mob_index + mid)->iVNum == iVNum )
      return(mid);
    if( bot >= top )
    {
      /* start unsorted search now */
      for( mid = top_of_sort_mobt; mid < top_of_mobt; mid++ )
        if( (mob_index + mid)->iVNum == iVNum )
          return( mid );
      return( -1 );
    }
    if( (mob_index + mid)->iVNum > iVNum )
      top = mid - 1;
    else
      bot = mid + 1;
  }
}






/* returns the real number of the object with given virtual number */
int real_object( int nVNum )
{
  long bot, top, mid;

  bot = 0;
  top = top_of_sort_objt - 1;

  /* perform binary search on obj-table */
  for(;;)
  {
    mid = (bot + top) / 2;

    if( (obj_index + mid)->iVNum == nVNum )
      return(mid);
    if( bot >= top )
    {
        /* start unsorted search now */
      for( mid = top_of_sort_objt; mid < top_of_objt; mid++ )
        if( (obj_index + mid )->iVNum == nVNum )
          return( mid );
      return( -1 );
    }
    if( (obj_index + mid)->iVNum > nVNum )
      top = mid - 1;
    else
      bot = mid + 1;
  }
}

int ObjRoomCount(int nr, struct room_data *rp)
{
  struct obj_data *o;
  int count = 0;

  for( o = rp->contents; o; o = o->next_content )
  {
    if( o->item_number == nr )
    {
      count++;
    }
  }
  return( count );
}

int str_len(char *buf)
{
  int i = 0;
  while( buf[ i ] != '\0' )
    i++;
  return(i);
}


int load()
{
  return(0);
}

void gr()
{
  return;
}

int workhours()
{
  return(0);
}

void reboot_text(struct char_data *ch, char *arg, int cmd)
{
  struct char_data *p;
  int i;

  if(IS_NPC(ch))
    return;

  mudlog( LOG_CHECK, "Rebooting Essential Text Files.");

  file_to_string(NEWS_FILE, news);
  file_to_string(CREDITS_FILE, credits);
  file_to_string(MOTD_FILE, motd);
  file_to_string("wizmotd", wmotd);
  mudlog( LOG_CHECK, "Initializing Scripts.");
  InitScripts();

  /* jdb -- you don't appear to re-install the scripts after you
   * reset the script db
   */

  for (p = character_list;p;p=p->next)
  {
    for(i = 0; i < top_of_scripts; i++)
    {
      if(gpScript_data[i].iVNum == mob_index[p->nr].iVNum)
      {
        SET_BIT(p->specials.act, ACT_SCRIPT);
        mudlog( LOG_CHECK, "Setting SCRIPT bit for mobile %s, file %s.",
                GET_NAME(p), gpScript_data[i].filename );
        p->script = i;
        break;
      }
    }
  }
  return;
}


void InitScripts()
{
 char buf[255], buf2[255];
 FILE *f1, *f2;
 int i, count;
 struct char_data *mob;

 if(!gpScript_data)
    top_of_scripts = 0;

 /* what is ths for?  turn off all the scripts ??? */
 /* -yes, just in case the script file was removed, saves pointer probs */

  for(mob = character_list; mob; mob = mob->next)
  {
    if(IS_MOB(mob) && IS_SET(mob->specials.act, ACT_SCRIPT))
    {
      mob->commandp = 0;
      REMOVE_BIT(mob->specials.act, ACT_SCRIPT);
    }
  }

  if(!(f1 = fopen("scripts.dat", "r")))
  {
    mudlog( LOG_ERROR, "Unable to open file \"scripts.dat\".");
    return;
  }

  if(gpScript_data)
  {
    int i = 0;
    for(;i < top_of_scripts; i++)
    {
      free(gpScript_data[i].script);
      free(gpScript_data[i].filename);
    }
    free(gpScript_data);
    top_of_scripts = 0;
  }


  gpScript_data = NULL;
  gpScript_data = (struct scripts *)malloc(sizeof(struct scripts));

  while(1)
  {
    if(fgets(buf, 254, f1) == NULL)
      break;

    if(buf[strlen(buf) - 1] == '\n')
       buf[strlen(buf) - 1] = '\0';

    sscanf(buf, "%s %d", buf2, &i);

    sprintf(buf, "scripts/%s", buf2);
    if(!(f2 = fopen(buf, "r")))
    {
      mudlog( LOG_ERROR, "Unable to open script \"%s\" for reading.", buf2);
    }
    else
    {

      gpScript_data = (struct scripts *) realloc(gpScript_data,
                               (top_of_scripts + 1) * sizeof(struct scripts));

      count = 0;
      while(!feof(f2))
      {
        fgets(buf, 254, f2);
        if(buf[strlen(buf) - 1] == '\n')
          buf[strlen(buf) - 1] = '\0';
        /* you really don't want to do a lot of reallocs all at once */
        if (count==0)
        {
          gpScript_data[top_of_scripts].script =
                      (struct foo_data *) malloc( sizeof( struct foo_data ) );
        }
        else
        {
          gpScript_data[top_of_scripts].script =
           (struct foo_data *) realloc(gpScript_data[top_of_scripts].script,
                                       sizeof(struct foo_data) * (count + 1));
        }
        gpScript_data[top_of_scripts].script[count].line =
                            (char *) malloc(sizeof(char) * (strlen(buf) + 1));

        strcpy(gpScript_data[top_of_scripts].script[count].line, buf);

        count++;
      }

      gpScript_data[top_of_scripts].iVNum = i;
      gpScript_data[top_of_scripts].filename =
                           (char *) malloc((strlen(buf2) + 1) * sizeof(char));
      strcpy(gpScript_data[top_of_scripts].filename, buf2);
      mudlog( LOG_CHECK, "Script %s assigned to mobile %d.", buf2, i);
      top_of_scripts++;
      fclose(f2);
    }
  }

  if(top_of_scripts)
    mudlog( LOG_CHECK, "%d scripts assigned.", top_of_scripts);
  else
    mudlog( LOG_CHECK, "No scripts found to assign.");

  fclose(f1);
}

int CheckKillFile( int iVNum )
{
  FILE *f1;
  char buf[255];
  int i;

  if(!(f1 = fopen(killfile, "r")))
  {
    mudlog( LOG_ERROR, "Unable to find killfile.");
    exit(0);
  }

  while(fgets(buf, 254, f1) != NULL)
  {
    sscanf(buf, "%d", &i);
    if(i == iVNum)
    {
      fclose(f1);
      return(1);
    }
  }

  fclose(f1);
  return(0);
}

void ReloadRooms()
{
  int i;

  for( i = 0; i < number_of_saved_rooms; i++ )
    load_room_objs( saved_rooms[ i ] );
}


void SaveTheWorld()
{
#if SAVEWORLD

  static int ctl=0;
  char cmd, buf[80];
  int i, j, arg1, arg2, arg3;
  struct char_data *p;
  struct obj_data *o;
  struct room_data *room;
  FILE *fp;

  if (ctl == WORLD_SIZE) ctl = 0;

  sprintf(buf, "world/mobs.%d", ctl);
  fp = (FILE *)fopen(buf, "w");  /* append */

  if (!fp)
  {
    mudlog( LOG_ERROR, "Unable to open zone writing file.");
    return;
  }

  i = ctl;
  ctl += 1000;

  for (; i< ctl; i++) {
    room = real_roomp(i);
    if (room && !IS_SET(room->room_flags, DEATH)) {
      /*
       *  first write out monsters
       */
      for (p = room->people; p; p = p->next_in_room) {
        if (!IS_PC(p)) {
          cmd = 'M';
          arg1 = MobVnum(p);
          arg2 = mob_index[p->nr].number;
          arg3 = i;
          Zwrite(fp, cmd, 0, arg1, arg2, arg3, 0, p->player.short_descr);
          fprintf(fp, "Z 1 %d 1\n", p->specials.zone);

          /* save hatreds && fears */
          if (IS_SET(p->hatefield, HATE_SEX))
            fprintf(fp, "H 1 %d %d -1\n", OP_SEX, p->hates.sex);
          if (IS_SET(p->hatefield, HATE_RACE))
            fprintf(fp, "H 1 %d %d -1\n", OP_RACE, p->hates.race);
          if (IS_SET(p->hatefield, HATE_GOOD))
            fprintf(fp, "H 1 %d %d -1\n", OP_GOOD, p->hates.good);
          if (IS_SET(p->hatefield, HATE_EVIL))
            fprintf(fp, "H 1 %d %d -1\n", OP_EVIL, p->hates.evil);
          if (IS_SET(p->hatefield, HATE_CLASS))
            fprintf(fp, "H 1 %d %d -1\n", OP_CLASS, p->hates.iClass);
          if (IS_SET(p->hatefield, HATE_VNUM))
            fprintf(fp, "H 1 %d %d -1\n", OP_VNUM, p->hates.vnum);

          if (IS_SET(p->fearfield, FEAR_SEX))
            fprintf(fp, "H 1 %d %d -1\n", OP_SEX, p->fears.sex);
          if (IS_SET(p->fearfield, FEAR_RACE))
            fprintf(fp, "H 1 %d %d -1\n", OP_RACE, p->fears.race);
          if (IS_SET(p->fearfield, FEAR_GOOD))
            fprintf(fp, "H 1 %d %d -1\n", OP_GOOD, p->fears.good);
          if (IS_SET(p->fearfield, FEAR_EVIL))
            fprintf(fp, "H 1 %d %d -1\n", OP_EVIL, p->fears.evil);
          if (IS_SET(p->fearfield, FEAR_CLASS))
            fprintf(fp, "H 1 %d %d -1\n", OP_CLASS, p->fears.iClass);
          if (IS_SET(p->fearfield, FEAR_VNUM))
            fprintf(fp, "H 1 %d %d -1\n", OP_VNUM, p->fears.vnum);

          for (j = 0; j<MAX_WEAR; j++) {
            if (p->equipment[j]) {
              if (p->equipment[j]->item_number >= 0) {
                cmd = 'E';
                arg1 = ObjVnum(p->equipment[j]);
                arg2 = obj_index[p->equipment[j]->item_number].number;
                arg3 = j;
                strcpy(buf, p->equipment[j]->short_description);
                Zwrite(fp, cmd,1,arg1, arg2, arg3, 0, buf);
                RecZwriteObj(fp, p->equipment[j]);
              }
            }
          }
          for (o = p->carrying; o; o=o->next_content) {
            if (o->item_number >= 0) {
              cmd = 'G';
              arg1 = ObjVnum(o);
              arg2 = obj_index[o->item_number].number;
              arg3 = 0;
              strcpy(buf, o->short_description);
              Zwrite(fp, cmd, 1, arg1, arg2, arg3, 0, buf);
              RecZwriteObj(fp, o);
            }
          }
        }
      }
    }
  }
  fprintf(fp, "S\n");
  fclose(fp);

#endif
}

void ReadTextZone( FILE *fl)
{
  char c, buf[255], count=0, last_cmd=1;
  int i, j, k, tmp, zone=0, rcount;
  struct char_data *mob = NULL, *master = NULL, *pLastMob = NULL;
  struct room_data *rp;
  struct obj_data *obj, *obj_to;

  while (1)
  {
    count++;
    fscanf(fl, " "); /* skip blanks */
    fscanf(fl, "%c", &c);


    if (c == 'S' || c == EOF)
      break;

    if (c == '*')
    {
      fgets(buf, 80, fl); /* skip command */
      continue;
    }

    fscanf(fl, " %d %d %d", &tmp, &i, &j);
    if (c == 'M' || c == 'O' || c == 'C' || c == 'E' || c == 'P' || c == 'D')
      fscanf(fl, " %d", &k);

    if( c == 'O' )
      fscanf( fl, " %d", &rcount );

    fgets(buf, 80, fl);/* read comment */
    if( last_cmd || tmp <= 0 )
    {
      switch( c )
      {
      case 'M': /* read a mobile */
        i = real_mobile( i );
        rp = real_roomp( k );
        if( ( j == 0 || mob_index[ i ].number < j ) &&
            !CheckKillFile( mob_index[ i ].iVNum ) &&
            ( mob = read_mobile(i, REAL) ) != NULL &&
            rp != NULL )
        {
          char_to_room( mob, k );

          if( IS_SET( mob->specials.act, ACT_SENTINEL ) )
            mob->lStartRoom = k;

          last_cmd = TRUE;
          master = pLastMob = mob;
        }
        else
        {
          master = pLastMob = mob = NULL;
          last_cmd = FALSE;
        }
        if( rp == NULL )
          mudlog( LOG_ERROR, "Cannot find room #%d", k );
        break;

      case 'C': /* read a mobile.  Charm them to follow prev. */
        i = real_mobile(i);
        if( ( j == 0 || mob_index[i].number < j ) &&
            !CheckKillFile( mob_index[i].iVNum ) && master &&
            ( mob = read_mobile( i, REAL ) ) != NULL )
        {
          pLastMob = mob;
          char_to_room( mob, master->in_room );
          /* add the charm bit to the dude.  */
          add_follower( mob, master );
          SET_BIT( mob->specials.affected_by, AFF_CHARM );
          SET_BIT( mob->specials.act, k );
          last_cmd = TRUE;
        }
        else
        {
          last_cmd = FALSE;
          mob = pLastMob = NULL;
        }
        break;

      case 'Z':  /* set the last mobile to this zone */
        if( pLastMob )
        {
          pLastMob->specials.zone =i;

          if( GET_RACE( pLastMob ) > RACE_GNOME &&
              !strchr( zone_table[ i ].races, GET_RACE( pLastMob ) ) )
            zone_table[i].races[strlen(zone_table[i].races)] =
                GET_RACE( pLastMob );
        }
        break;

      case 'O': /* read an object */
        obj = NULL;
        last_cmd = FALSE;
        i = real_object(i);
        if( i >= 0 && ( j == 0 || obj_index[i].number < j ) )
        {
          if( k > 0 && ( rp = real_roomp( k ) ) != NULL )
          {
            if( rcount == 0 || ObjRoomCount( i, rp ) < rcount )
            {
              if( ( obj = read_object( i, REAL ) ) != NULL )
              {
                obj_to_room(obj, k);
                last_cmd = TRUE;
              }
            }
          }
          else
          {
            mudlog( LOG_ERROR, "Cannot find room #%d", k);
          }
        }
        break;

      case 'P': /* object to object */
        i = real_object(i);
        if( i >= 0 && ( j == 0 || obj_index[ i ].number < j ) &&
            ( obj_to = get_obj_num(k) ) != NULL &&
            ( obj = read_object( i, REAL ) ) != NULL )
        {
          obj_to_obj(obj, obj_to);
          last_cmd = 1;
        }
        else
        {
          obj = obj_to = NULL;
          last_cmd = 0;
        }
        break;

      case 'G': /* obj_to_char */
        i = real_object(i);
        if( i >= 0 && ( j == 0 || obj_index[i].number < j ) && pLastMob &&
            ( obj = read_object(i, REAL) ) != NULL )
          obj_to_char( obj, pLastMob );
        break;

      case 'H': /* hatred to char */
        if( pLastMob )
          AddHatred( pLastMob, i, j );
        break;

      case 'F': /* fear to char */
        if( pLastMob )
          AddFears( pLastMob, i, j );
        break;

      case 'E': /* object to equipment list */
        i = real_object(i);
        if( i >= 0 && ( j == 0 || obj_index[i].number < j ) && pLastMob &&
            ( obj = read_object(i, REAL) ) != NULL )
        {
          if( !pLastMob->equipment[k] )
          {
            equip_char( pLastMob, obj, k);
          }
          else
          {
            mudlog( LOG_ERROR, "eq error - zone %d, item %d, mob %d, loc %d",
                    zone, obj_index[ i ].iVNum,
                    mob_index[ pLastMob->nr ].iVNum, k );
          }
        }
        break;

      case 'D': /* set state of door */
        rp = real_roomp( i );
        if( rp && rp->dir_option[ j ] )
        {
          if( !IS_SET( rp->dir_option[ j ]->exit_info, EX_ISDOOR ) )
          {
            mudlog( LOG_ERROR, "Door error - zone %d, loc %d (fixed)",
                    zone, i );
            SET_BIT( rp->dir_option[ j ]->exit_info, EX_ISDOOR );
          }

          switch (k)
          {
          case 0:
            REMOVE_BIT(rp->dir_option[j]->exit_info, EX_LOCKED);
            REMOVE_BIT(rp->dir_option[j]->exit_info, EX_CLOSED);
            break;
          case 1:
            SET_BIT(rp->dir_option[j]->exit_info, EX_CLOSED);
            REMOVE_BIT(rp->dir_option[j]->exit_info, EX_LOCKED);
            break;
          case 2:
            SET_BIT(rp->dir_option[j]->exit_info, EX_LOCKED);
            SET_BIT(rp->dir_option[j]->exit_info, EX_CLOSED);
            break;
          }
        }
        else
        {
          /* that exit doesn't exist anymore */
          mudlog( LOG_ERROR, "Exit error - zone %d, loc %d",
                  zone, i );
        }
        break;

      default:
        break;
      }
    }
  }
}

int ENomeValido( char *pchNome )
{
  if( pchNome )
  {
    while( strlen( pchNome ) && !isalpha( pchNome[ strlen( pchNome ) - 1 ] ) )
      pchNome[ strlen( pchNome ) - 1 ] = 0;

    if( strlen( pchNome ) )
    {
      while( pchNome )
      {
        if( !isalpha( *pchNome ) )
          return FALSE;
        pchNome++;
      }
      return TRUE;
    }
  }
  return FALSE;
}

void ConvertPlayerFile( void )
{
  char buf[ 160 ];

  FILE *pPlayersFile;

  if( ( pPlayersFile = fopen( PLAYER_FILE, "rb" ) ) != NULL )
  {
    mudlog( LOG_CHECK, "Players file found. Converting..." );
    while( !feof( pPlayersFile ) )
    {
      struct char_file_u stChar;
      int nReaden;
      if( ( nReaden = fread( &stChar, 1, sizeof( stChar ), pPlayersFile ) ) ==
          sizeof( stChar ) )
      {
        FILE *pCharFile;
        char szFileName[ 40 ];

        if( !ENomeValido( stChar.name ) )
        {
          sprintf( szFileName, "%s/%s.dat", PLAYERS_DIR, lower( stChar.name ) );
          if( ( pCharFile = fopen( szFileName, "w+" ) ) != NULL )
          {
            fwrite( &stChar, sizeof( stChar ), 1, pCharFile );
            fclose( pCharFile );
          }
          else
          {
            mudlog( LOG_ERROR, "Cannot create file %s.", szFileName );
          }
        }
        else
        {
          mudlog( LOG_ERROR, "Invalid name '%s'. Discarded", stChar.name );
        }
      }
      else
      {
        mudlog( LOG_ERROR, "Letti %d caratteri invece di %d del player file.",
                nReaden, sizeof( stChar ) );
      }
    }
    fclose( pPlayersFile );
    sprintf( buf, "mv %s %s.converted", PLAYER_FILE, PLAYER_FILE );
    system( buf );
    mudlog( LOG_CHECK, "Conversion done." );
  }
}

void clean_playerfile()
{
  struct junk
  {
    struct char_file_u dummy;
    bool AXE;
  };

  struct junk grunt;

  time_t timeH;
  char buf[80];
#if !defined( EMANUELE )
  FILE *f,*f2;
#endif
  int j, max, num_processed, num_deleted, num_demoted, ones;

  DIR *dir;

  num_processed = num_deleted = num_demoted = ones = 0;
  timeH=time(0);

#if defined( EMANUELE )
  ConvertPlayerFile();

  if( ( dir = opendir( PLAYERS_DIR ) ) != NULL )
  {
    struct dirent *ent;
    while( ( ent = readdir( dir ) ) != NULL )
    {
      FILE *pFile;
      char szFileName[ 40 ];

      if( *ent->d_name == '.' )
        continue;
      /* ATTENZIONE Inserire controllo sul .dat */

      sprintf( szFileName, "%s/%s", PLAYERS_DIR, ent->d_name );

      if( ( pFile = fopen( szFileName, "r+" ) ) != NULL )
      {
        grunt.AXE = FALSE;

        if( fread( &grunt.dummy, 1, sizeof( grunt.dummy ), pFile ) ==
            sizeof( grunt.dummy ) )
        {
          num_processed++;

          for( j = 0, max = 0; j < MAX_CLASS; j++ )
            if( grunt.dummy.level[ j ] > max )
              max = grunt.dummy.level[ j ];

          if( max < LOW_IMMORTAL )
          {
            j = 1;
            if( max > 15 )
              j++;
            if( max > 30 )
              j++;
            if( max > 45 )
              j++;

#if CHECK_RENT_INACTIVE

             /* Purge rent files! after inactivity of 1 month */
            if( !grunt.AXE && timeH - grunt.dummy.last_logon >
                (long) RENT_INACTIVE * ( SECS_PER_REAL_DAY * 30 ) )
            {
              mudlog( LOG_PLAYERS,
                      "Purging rent file for %s, inactive for %d month.",
                       grunt.dummy.name, RENT_INACTIVE );
              sprintf( buf, "rm %s/%s", RENT_DIR, lower( grunt.dummy.name ) );
              system( buf );
            }
#endif
            if( !grunt.AXE && timeH - grunt.dummy.last_logon >
                (long) j * ( SECS_PER_REAL_DAY * 30 ) &&
                !IS_SET( grunt.dummy.user_flags, NO_DELETE ) )
            {
              num_deleted++;
              grunt.AXE = TRUE;
              mudlog( LOG_PLAYERS, "%s deleted after %d months of inactivity.",
                       grunt.dummy.name, j );
            }

            /* even the no_deletes get deleted after a time */
            if( IS_SET( grunt.dummy.user_flags, NO_DELETE ) )
            {
              if( timeH - grunt.dummy.last_logon >
                  (long) ( j * 2 ) * ( SECS_PER_REAL_DAY * 30 ) )
              {
                num_deleted++;
                grunt.AXE = TRUE;
                mudlog( LOG_PLAYERS,
                        "%s deleted after %d months of inactivity (NO_DELETE).",
                         grunt.dummy.name,j );
              }
            }
          }
          else if( max > LOW_IMMORTAL )
          {
            /* delete people with levels greater than BIG_GUY */
            if( max > BIG_GUY )
            {
              num_deleted++;
              grunt.AXE = TRUE;
              mudlog( LOG_PLAYERS, "%s deleted (TOOHIGHLEVEL).",
                      grunt.dummy.name );
            }
            else if( timeH - grunt.dummy.last_logon >
                     (long) SECS_PER_REAL_DAY * 30 )
            {
              num_demoted++;
              mudlog( LOG_PLAYERS,
                      "%s demoted from %d to %d due to inactivity.",
                      grunt.dummy.name, max, max - 1 );
              grunt.dummy.last_logon = timeH; /* so it doesn't happen twice */
              max--;
              max = MAX( 51, max ); /* should not be necessary */
              for( j = 0; j < MAX_CLASS; j++ )
                grunt.dummy.level[ j ] = max;

              rewind( pFile );
              fwrite( &grunt.dummy, sizeof( grunt.dummy ), 1, pFile );
            }
          }
        }
        else
        {
          mudlog( LOG_ERROR, "Error reading file %s.", szFileName );
        }

        fclose( pFile );

        if( grunt.AXE )
        {
          sprintf( buf, "rm %s/%s.dat", PLAYERS_DIR, lower( grunt.dummy.name ) );
          system( buf );
          sprintf( buf, "rm %s/%s", RENT_DIR, lower( grunt.dummy.name ) );
          system( buf );
          sprintf( buf,"rm %s/%s.aux", RENT_DIR, grunt.dummy.name );
          system( buf );
        }
      }
      else
      {
        mudlog( LOG_ERROR, "Error opening file %s.", szFileName );
      }
    }
  }
  else
  {
    mudlog( LOG_ERROR, "Error opening dir %s.", PLAYERS_DIR );
  }



#else

  if (!(f = fopen(PLAYER_FILE, "rb+")))
  {
    perror("clean player file");
    exit(0);
  }

  if (!(f2 = fopen("temp", "w+")))
  {
    perror("clean player file");
    exit(0);
  }


  for(; !feof( f ); )
  {
    fread( &grunt.dummy, sizeof( struct char_file_u ), 1, f );
    if( !feof( f ) )
    {           /* we have someone */
      num_processed++;
      grunt.AXE = FALSE;
      if(!str_cmp(grunt.dummy.name,"111111"))
      {
        mudlog( LOG_PLAYERS,"%s was deleted (111111 name hopefully).",
                grunt.dummy.name);
        ones++;
        num_deleted++;
        grunt.AXE = TRUE;
      }
      else
      {
        for( j = 0, max = 0; j < MAX_CLASS; j++ )
          if( grunt.dummy.level[ j ] > max )
            max = grunt.dummy.level[ j ];

        if( max < LOW_IMMORTAL )
        {
          j = 1;
          if( max > 15 )
            j++;
          if( max > 30 )
            j++;
          if( max > 45 )
            j++;


#if CHECK_RENT_INACTIVE
#ifndef STRANGE_WACK

           /* Purge rent files! after inactivity of 1 month */
          if( !grunt.AXE && timeH - grunt.dummy.last_logon >
                            (long) RENT_INACTIVE * ( SECS_PER_REAL_DAY * 30 ) )
          {
            char uname[50];

            sprintf(uname,"%s",grunt.dummy.name);
            uname[0]=tolower(uname[0]);

            muflog( LOG_PLAYERS,
                    "Purging rent file for %s, inactive for %d month.",
                    uname, RENT_INACTIVE );
            sprintf( buf, "rm %s/%s", RENT_DIR, uname );
            system( buf );
            sprintf( buf,"rm %s/%s.aux", RENT_DIR, grunt.dummy.name );
            system( buf );
          }

#endif
#endif

#ifndef STRANGE_WACK
          if( !grunt.AXE && timeH - grunt.dummy.last_logon >
                            (long) j * ( SECS_PER_REAL_DAY * 30 ) &&
              !IS_SET( grunt.dummy.user_flags, NO_DELETE ) )
          {
            num_deleted++;
            grunt.AXE = TRUE;
            mudlog( LOG_PLAYERS, "%s deleted after %d months of inactivity.",
                     grunt.dummy.name, j );
          }  /* even the no_deletes get deleted after a time */

          if( IS_SET( grunt.dummy.user_flags, NO_DELETE ) )
          {
            if( timeH - grunt.dummy.last_logon >
                (long) ( j * 2 ) * ( SECS_PER_REAL_DAY * 30 ) )
            {
              num_deleted++;
              grunt.AXE = TRUE;
              mudlog( LOG_PLAYERS,
                      "%s deleted after %d months of inactivity (NO_DELETE).",
                       grunt.dummy.name,j );
            }
          }
#endif
        }
        else if(max > LOW_IMMORTAL)
        {
#ifndef STRANGE_WACK
          /* delete people with levels greater than BIG_GUY */
          if( max > BIG_GUY )
          {
            num_deleted++;
            grunt.AXE = TRUE;
            mudlog( LOG_PLAYERS, "%s deleted after %d months of inactivity."
                          "(TOOHIGHLEVEL)",
                     grunt.dummy.name, j );
          }
          else if( timeH - grunt.dummy.last_logon >
                   (long) SECS_PER_REAL_DAY * 30 )
          {
            num_demoted++;
            mudlog( LOG_PLAYERS, "%s demoted from %d to %d due to inactivity.",
                     grunt.dummy.name, max, max - 1 );
            grunt.dummy.last_logon = timeH; /* so it doesn't happen twice */
            max--;
            max = MAX( 51, max ); /* should not be necessary */
                        /* 6 */
            for( j = 0; j < MAX_CLASS; j++ )
              grunt.dummy.level[ j ] = max;

          }
#endif
        } /* level < LOW_IMMORT */
#ifdef STRANGE_WACK

        /* used to clear up corrupted players files, bogus chars in the
         * name... etc.. */

        for( i = 0; i < strlen( grunt.dummy.name ); i++ )
        {
          if( !( toupper( grunt.dummy.name[ i ] ) >= 'A' &&
                 toupper( grunt.dummy.name[ i ] ) <= 'Z' ) )
          {
            mudlog( LOG_PLAYERS, "%s was deleted (strange name).",
                    grunt.dummy.name );
            grunt.AXE = 1;
            break;
          }
        }

        /* used to clear chars with bogus levels and such */
        if( !grunt.AXE )
        {
          for( i = 0; i < MAX_CLASS; i++ )
          {
            if( grunt.dummy.level[ i ] < 0 )
            {
              mudlog( LOG_PLAYERS, "%s was deleted (strange levels).",
                      grunt.dummy.name );
              grunt.AXE=1;
              break;
            }
          }
        }

#endif
        if( !grunt.AXE )
          fwrite( &grunt.dummy, sizeof( struct char_file_u ), 1, f2 );
        else
        {
          char uname[50];

          sprintf(uname,"%s",grunt.dummy.name);
          uname[0]=tolower(uname[0]);
          sprintf( buf, "rm %s/%s", RENT_DIR, uname );
          system( buf );
          sprintf( buf,"rm %s/%s.aux", RENT_DIR, grunt.dummy.name );
          system( buf );
        }
      }
    }
  }
#endif


  mudlog( LOG_CHECK, "-- %d characters were processed.", num_processed);
  mudlog( LOG_CHECK, "-- %d characters were deleted.  ", num_deleted);
#if !defined( EMANUELE )
  mudlog( LOG_CHECK, "-- %d of these were allread deleted. (11111s)", ones);
#endif
  mudlog( LOG_CHECK, "-- %d gods were demoted due to inactivity.", num_demoted);
#if !defined( EMANUELE )
  sprintf( buf, "mv %s %s.bak", PLAYER_FILE, PLAYER_FILE);
  system(buf);
  sprintf(buf,"mv temp %s", PLAYER_FILE);
  system(buf);
#endif
  mudlog( LOG_CHECK, "Cleaning done.");
}

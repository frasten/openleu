#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

#include "protos.h"
#include "rhyodin.h"
#include "cmdid.h"
#include "charlist.h"
#include "fight.h"

extern struct time_info_data time_info;
extern struct index_data *mob_index;
extern struct index_data *obj_index;
extern struct char_data *character_list;


#define START_ROOM      21276
#define END_ROOM        21333
#define GhostSoldier    21138
#define GhostLieutenant 21139
  /* Must be a unique identifier for this mob type, or we lose */
#define Identifier      "gds"

/****************************************************************************
  Il capitano dei fantasmi ad una certa ora summona una certa quantita` di 
  soldati fantasma.  Se combatte un mob charmato, attacca il padrone.
  Chiama altri fantasmi se attaccato.
****************************************************************************/

int keystone( struct char_data *ch, int cmd, char *arg, struct char_data *mob, 
              int type )
{
  struct char_data *ghost, *t, *master;
  int i;
                                  
  if( cmd || !AWAKE( ch ) ) 
    return(FALSE);
  
  if( time_info.hours == 22 ) 
  {
    if( !( ghost = get_char_vis_world( ch, Identifier, 0 ) ) ) 
    {
      act( "$c0015La voce lamentosa di $c0005$n$c0015 si alza nella notte",
           FALSE, ch, 0, 0, TO_ROOM );
      act( "$c0005$n$c0015 dice 'Sveglia miei soldati! E` la nostra ora.!'",
           FALSE, ch, 0, 0, TO_ROOM );
      act( "Un brivido di terrore corre lungo la tua schiena.",
           FALSE, ch, 0, 0, TO_ROOM );
      for( i = START_ROOM;i < END_ROOM; ++i )
      {
        if( number( 0, 2 ) == 0 ) 
        {
          ghost = read_mobile( GhostSoldier, VIRTUAL );
          char_to_room( ghost, i );
        }
        else if( number( 0, 7 ) == 0 ) 
        {
          ghost = read_mobile( GhostLieutenant, VIRTUAL );
          char_to_room( ghost, i);
        }
      }
      for( t = character_list; t; t = t->next )
        if( real_roomp( ch->in_room)->zone == real_roomp( t->in_room )->zone )
          act( "Senti un raccapricciante lamento che ti riempie di terrore!",
               FALSE, t, 0, 0, TO_CHAR );
    }
  }

  if( ch->specials.fighting ) 
  {
    if( IS_NPC( ch->specials.fighting ) &&
        !IS_SET( ( ch->specials.fighting )->specials.act, ACT_POLYSELF ) )
    {
      if( ( master = ( ch->specials.fighting )->master ) && 
          CAN_SEE( ch, master ) ) 
      {
        stop_fighting( ch );
        hit( ch, master, TYPE_UNDEFINED );
        return TRUE;
      }
    }
    if( GET_POS( ch ) == POSITION_FIGHTING ) 
    {
      FighterMove( ch );
    } 
    else 
    {
      StandUp( ch );
    }
    CallForGuard( ch, ch->specials.fighting, 3, OUTPOST );
  }
  return FALSE;
}

/****************************************************************************
  I soldati fantasma spariscono la mattina. Se attaccati da un mob charmato
  attaccano il padrone.
  Se non combattono attaccano il giocatore piu` buono nella stanza.
  Chiama altri fantasmi se attaccato.
****************************************************************************/

int ghostsoldier( struct char_data *ch, int cmd, char *arg, 
                  struct char_data *mob, int type)
{
  struct char_data *tch, *good, *master;
  int max_good;
  int (*gs)( struct char_data *, int, char *, struct char_data *, int );
  int (*gc)( struct char_data *, int, char *, struct char_data *, int );

  gs = ghostsoldier;
  gc = keystone;

  if (cmd) return(FALSE);

  if( time_info.hours > 4 && time_info.hours < 22 ) 
  {
    act("$n si dissolve lentamente nel mattino.", FALSE, ch, 0, 0, TO_ROOM);
    extract_char(ch);
    return(TRUE);
  }

  max_good = -1001;
  good = 0;
  
  for( tch = real_roomp( ch->in_room )->people; tch; tch = tch->next_in_room )
  {
    if (!(mob_index[tch->nr].func == CASTVF gs) && /* Another ghost soldier? */
        !(mob_index[tch->nr].func == CASTVF gc) && /* The ghost captain? */
        (GET_ALIGNMENT(tch) > max_good) &&  /* More good than prev? */
        !IS_IMMORTAL(tch) &&                /* A god? */
        (GET_RACE(tch) >= 4))               /* Attack only npc races */ 
    {
      max_good = GET_ALIGNMENT(tch);
      good = tch;
    }
  }

  /* What is a ghost Soldier doing in a peaceful room? */
  if (check_peaceful(ch, ""))
    return FALSE;
  
  if( good ) 
  {
    if (!check_soundproof(ch))
      act( "$c0009[$c0015$N]$c0009 ti attacca con un urlo spaventoso!", 
           FALSE, good, 0, ch, TO_CHAR );
    hit( ch, good, TYPE_UNDEFINED );
    return TRUE;
  }

  if( ch->specials.fighting ) 
  {
    if( IS_NPC( ch->specials.fighting ) &&
        !IS_SET( ( ch->specials.fighting )->specials.act,ACT_POLYSELF ) )
    {
      if( ( master = ( ch->specials.fighting )->master ) && 
          CAN_SEE( ch, master ) ) 
      {
        stop_fighting( ch );
        hit( ch, master, TYPE_UNDEFINED );
        return TRUE;
      }
    }
    if( GET_POS( ch ) == POSITION_FIGHTING ) 
    {
      FighterMove( ch );
    } 
    else 
    {
      StandUp( ch );
    }
    CallForGuard( ch, ch->specials.fighting, 3, OUTPOST );
  }
  return FALSE;
}

char *quest_one[] = 
{
  "'Il secondo oggetto che devi trovare e` l'anello di Tlanic.",
  "Tlanic era un guerriero elfo che lascio` Rhyodin cinque anni",
  "dopo Lorces. Porto` il suo anello con se` come portafortuna.",
  "Egli parti` per cercare Lorces, i suoi amici e per trovare,",
  "se possibile, una via verso il nord. Anche lui ha fallito.",
  "Riportami l'anello per ulteriori istruzioni.'",
  "\n"  
};

char *quest_two[] = 
{
  "'Devi sapere che molte lune dalla partenza di Tlanic, suo fratello",
  "Evistar ando` in sua ricerca.",
  "Evistar, al contrario del fratello, non era un grande",
  "guerriero, ma era il segugio piu` bravo di tutta la sua gente.",
  "Per aiutarlo nella sua imprese gli fu` dato un calice mai",
  "vuoto. Portami quel calice magico se vuoi entrare nel reame ",
  "di Rhyodin.'",
  "\n"
};

char *quest_three[] = 
{
  "'Purtroppo, nemmeno Evistar fece mai ritorno, e passarono anni prima che",
  "un potente mago decise di cercare la via verso il nord.",
  "Il suo nome era C*zarnak. Si crede che sia morto nelle caverne",
  "come gli altri. Indossava un cerchio incantato intorno alla testa.",
  "Riportamelo e ti diro` di piu`.'",
  "\n"
};

char *necklace[] = 
{
  "'Mi hai riportato tutti gli oggetti degli eroi morti nella",
  "ricerca della via di uscita dal reame.",
  "In piu` hai trovato la via attraverso le montagne, provando la",
  "tua abilita` nell'orientamento e nella cartografia.",
  "Tu sei degno di essere un ambasciatore nel mio regno.",
  "Prendi questa collana e non perderla!",
  "Dalla al guardiano del cancello che ti lasciera` passare e",
  "ti dara` la giusta ricompensa.'",
  "\n"
};

char *nonecklace[] = 
{
  "'Mi hai riportato tutti gli oggetti degli eroi morti nella",
  "ricerca della via di uscita dal reame di Rhyodin.",
  "In piu` hai trovato la via attraverso le montagne, provando la",
  "tua abilita` nell'orientamento e nella cartografia.",
  "Tu sei degno di essere un ambasciatore nel mio regno.",
  "Il compito finale e` di trovare la collana della saggezza.",
  "Il guardiano del cancello la riconoscera` e ti lasciera`",
  "passare dandoti la giusta ricompensa.'",
  "\n"
};

char *quest_intro[] = 
{
  "'Vuoi conoscere la storia di Rhyodin ? Bene. Devi sapere",
  "che Rhyodin e` un reame a sudest dell'Alpes Oppidum.",
  "Un giorno, l'unica grande strada sotterranea che collegava",
  "il regno con il resto del mondo, fu interrotta da una frana.",
  "Lorces, il Paladino, fu il primo che cerco` una strada",
  "alternativa ma, purtroppo, falli`.",
  "Il primo passo per poter entrere nel regno di Rhyodin e` di",
  "cercare e riportarmi il suo scudo.'",
  "\n"
};

void SayQuest( struct char_data *pCh, char *apchQuest[] )
{
  int i;
  char buf[ 100 ];
  
  act( "$c0015[$c0005$n$c0015] dice:", TRUE, pCh, 0, 0, TO_ROOM );
  for( i = 0 ; *apchQuest[ i ] != '\n' ; i++ ) 
  {
    sprintf( buf,"$c0015%.90s", apchQuest[ i ] );
    act( buf, TRUE, pCh, 0, 0, TO_ROOM );
  }
}

int Valik( struct char_data *ch, int cmd, char *arg, struct char_data *mob, 
           int type )
{
  
#define Valik_Wandering   0
#define Valik_Meditating  1
#define Valik_Qone        2
#define Valik_Qtwo        3
#define Valik_Qthree      4
#define Shield            21113
#define Ring              21120
#define Chalice           21121
#define Circlet           21117
#define Necklace          21122
#define Med_Chambers      21324
  
  char obj_name[80], vict_name[80], buf[MAX_INPUT_LENGTH];
  struct obj_data *obj;
  static short valik_dests[] =
  {
    104, 1638, 7902, 13551, 19244, 21325
#if 0
    16764, 17330, 25239
#endif
  };
  
  mob->lStartRoom = 0;

  if( !AWAKE( mob ) )
    return FALSE;

  if( type == EVENT_TICK )
  {
    if( mob->specials.fighting )
    {
      if( IS_NPC( mob->specials.fighting ) &&
         !IS_SET( ( mob->specials.fighting )->specials.act, ACT_POLYSELF ) )
      {
        struct char_data *master;
        if( ( master = (mob->specials.fighting)->master ) && 
            CAN_SEE( mob, master ) )
        {
          stop_fighting( mob );
          hit( mob, master, TYPE_UNDEFINED );
          return TRUE;
        }
      }
    }
    else
    {
      if( mob->generic ==  Valik_Meditating )
      {
        if( time_info.hours < 22 && time_info.hours > 5) 
        {
          do_stand( mob, "", -1 );
          do_say( mob, "Forse oggi sara` un giorno diverso.", 0 );
          act( "$n svanisce lentamente nel mattino.", FALSE, mob, 0, 0, 
               TO_ROOM );
          char_from_room( mob );
          char_to_room( mob, valik_dests[ number( 0, 5 ) ] );
          act( "Il mondo si distorce, si dissolve e si riforma.", FALSE, mob, 
               0, 0, TO_ROOM );
          mob->generic = Valik_Wandering;
          return FALSE;
        }
        else 
        {
          struct char_data *vict;
          for( vict = real_roomp( mob->in_room )->people; vict; 
               vict = vict->next_in_room )
          {
            if( !IS_NPC( vict ) && GetMaxLevel( vict ) < LOW_IMMORTAL &&
                CAN_SEE( mob, vict ) && number( 0, 3 ) == 0 )
            {
              act( "$n interrompe bruscamente i suoi esercizi di meditazione.", 
                   FALSE, mob, 0, 0, TO_ROOM);
              do_stand( mob, "", -1 );
              hit( mob, vict, TYPE_UNDEFINED );
              return TRUE;
            }
          }
        }
      }
      else if( mob->generic == Valik_Wandering )
      {
        if( time_info.hours > 21 ) 
        {
           if( !check_soundproof( ch ) )
            do_say( mob, "E` tempo di meditare.", 0 );
          act( "$c0015$n sparisce in un lampo di luce!", FALSE, mob, 0, 0,
               TO_ROOM );
          char_from_room( mob );
          char_to_room( mob, Med_Chambers );
          act( "La realta` si distorce e gira intorno a te!", FALSE, mob, 0, 0,
               TO_ROOM );
          sprintf( buf, "close mahogany" );
          command_interpreter( mob, buf );
          do_rest( mob, "", -1 );
          mob->generic = Valik_Meditating;
          return FALSE;
        }
      }
    }
  }
  else if( type == EVENT_COMMAND )
  {
    if( cmd == CMD_ASK )
    { 
      arg = one_argument( arg, vict_name );
      if( !*vict_name || get_char_room_vis( ch, vict_name ) != mob )
        return FALSE;
      else 
      {
        if( ch->generic == Valik_Wandering &&
            ( strstr( arg, "ambasciator" ) || strstr( arg, "storia" ) ) && 
            strstr( arg, "Rhyodin" ) )
        {
          SayQuest( mob, quest_intro );
        }
        else if( ch->generic == Valik_Meditating )
        {
          act( "$n e` troppo impegnat$b a meditare per darti ascolto", TRUE,
               mob, 0, ch, TO_VICT );
        }
        else
        {
          act( "$n non sembra interessat$b alle domande di $N.", TRUE, mob, 0,
               ch, TO_NOTVICT );
          act( "$n non sembra interessat$b alle tue domande.", TRUE, mob, 0,
               ch, TO_VICT );
        }
        return TRUE;
      }
    }
    else if( cmd == CMD_TELL )
    {
      arg = one_argument( arg, vict_name );
      if( !*vict_name || get_char_room_vis( ch, vict_name ) != mob )
        return FALSE;
      else 
      {
        act( "$n non sembra interessat$b alle chiacchiere di $N.", TRUE, mob, 0,
             ch, TO_NOTVICT );
        act( "$n non sembra interessat$b alle tue chiacchiere.", TRUE, mob, 0,
             ch, TO_VICT );
        return TRUE;
      }
    }
    else if( cmd == CMD_GIVE )
    {
      arg = one_argument( arg, obj_name );
      if( !*obj_name || 
          !( obj = get_obj_in_list_vis( ch, obj_name, ch->carrying ) ) )
        return FALSE;
      only_argument( arg, vict_name );
      if( !*vict_name || get_char_room_vis( ch, vict_name ) != mob )
        return FALSE;
        
      act( "Dai $p a $N.", TRUE, ch, obj, mob, TO_CHAR );
      act( "$n da` $p a $N.", TRUE, ch, obj, mob, TO_ROOM );
      
      switch( mob->generic )
      {
      case Valik_Meditating:
        act( "$n e` troppo impegnat$b a meditare per accettare regali.",
             TRUE, mob, 0, ch, TO_CHAR );
        break;
      case Valik_Wandering:
        /* Take it, in either case */
        obj_from_char( obj );
        obj_to_char( obj, mob );
        if( obj->item_number>=0 && 
            obj_index[ obj->item_number ].iVNum == Shield ) 
        {
          do_say( mob, "Lo scudo di Lorces!", 0 );
          SayQuest( mob,  quest_one );
          mob->generic = Valik_Qone;
        }
        else 
        {
          act( "$N prende $p e si inchina in ringraziamento.'",
               FALSE, ch, obj, mob, TO_CHAR );
          act( "$N prende $p da $n e si inchina in ringraziamento.'",
               FALSE, ch, obj, mob, TO_ROOM );
        }
        break;
      case Valik_Qone:
        if( obj->item_number >= 0 && 
            obj_index[ obj->item_number ].iVNum == Ring ) 
        {
          do_say( mob, "Mi hai portato l'anello di Tlanic.", 0 );
          obj_from_char( obj );
          obj_to_char( obj, mob );
          SayQuest( mob, quest_two );
          mob->generic = Valik_Qtwo;
        }
        else 
        {
          do_say( mob, "Non e` quello che cerco.", 0 );
          act( "$N ti restituisce $p.", TRUE, ch, obj, mob, TO_CHAR );
          act( "$N restituisce $p a $n.", TRUE, ch, obj, mob, TO_ROOM );
        }
        break;
      case Valik_Qtwo:
        if( obj->item_number >= 0 &&
            obj_index[ obj->item_number ].iVNum == Chalice ) 
        {
          do_say( mob, "Mi hai portato il calice di Evistar.", 0 );
          obj_from_char( obj );
          obj_to_char( obj, mob );
          SayQuest( mob, quest_three );
          mob->generic = Valik_Qthree;
        }
        else 
        {
          do_say( mob, "Non e` quello che cerco.", 0 );
          act( "$N ti restituisce $p.", TRUE, ch, obj, mob, TO_CHAR );
          act( "$N restiruisce $p a $n.", TRUE, ch, obj, mob, TO_ROOM );
        }
        break;
      case Valik_Qthree:
        if( obj->item_number >= 0 &&
            obj_index[ obj->item_number ].iVNum == Circlet ) 
        {
          do_say( mob, "Mi hai portato il cerchio di C*zarnak.", 0 );
          obj_from_char( obj );
          obj_to_char( obj, mob );
          if( mob->equipment[ WEAR_NECK_1 ] && 
              mob->equipment[WEAR_NECK_1]->item_number >= 0 &&
              obj_index[mob->equipment[WEAR_NECK_1]->item_number].iVNum == 
              Necklace )
          {
            SayQuest( mob, necklace );
            act( "$N mette la Collana della Saggezza nelle tue mani.",
                 TRUE, ch, 0, mob, TO_CHAR );
            act( "$N mette la Collana della Saggezza nelle mani di $n.",
                 TRUE, ch, 0, mob, TO_ROOM );
            obj_to_char( unequip_char( mob, WEAR_NECK_1 ), ch );
          }
          else 
          {
            SayQuest( mob, nonecklace );
          }
          mob->generic = Valik_Wandering;
        }
        else 
        {
          do_say( mob, "Non e` quello che cerco.", 0 );
          act( "$N ti restituisce $p.", TRUE, ch, obj, mob, TO_CHAR );
          act( "$N restituisce $p a $n.", TRUE, ch, obj, mob, TO_ROOM );
        }
        break;
      default:
        mudlog( LOG_SYSERR, "Unexpected generic for %s", 
                GET_NAME_DESC( mob ) );
        mob->generic = Valik_Wandering;
        break;
      }
      return TRUE;
    }
  }
  else if( type == EVENT_DEATH )
  {
  }
  
  return magic_user( ch, cmd, arg, mob, type );
}

struct GuardianListNames 
{
  char **names;
  short num_names;
};

int IsInGuardianList( struct GuardianListNames *pGList, char *pchName )
{
  int i;
  for( i = 0; i < pGList->num_names; i++ )
  {
    if( !strcmp( pGList->names[ i ], pchName ) ) 
      return TRUE;
  }
  return FALSE;
}

void InsertInGuardianList( struct GuardianListNames *pGList, char *pchName )
{
  pGList->num_names++;
  pGList->names = (char **) realloc( pGList->names, 
                                     pGList->num_names * sizeof( char * ) );
  pGList->names[ pGList->num_names - 1 ] = 
                                      (char *) malloc( strlen( pchName ) + 1 );
  strcpy( pGList->names[ pGList->num_names - 1 ], pchName );
}

struct GuardianListNames *NewGuardianList()
{
  struct GuardianListNames *pNewList;
  pNewList = 
     (struct GuardianListNames *) malloc( sizeof( struct GuardianListNames ) );
  if( pNewList )
  {
    memset( pNewList, 0, sizeof( struct GuardianListNames ) );
    pNewList->names = (char **) malloc( sizeof( char * ) );
    pNewList->num_names = 0;
  }
  else
    mudlog( LOG_SYSERR, "Problemi nell'allocare memoria in NewGuardianList"
                        "(rhyodin.c)." );
  return pNewList;
}

void ShowGuardianList( struct GuardianListNames *pGList, 
                       struct char_data * pCh )
{
  int i;
  for( i = 0; i < pGList->num_names; i++ )
  {
    send_to_char( pGList->names[ i ], pCh );
    send_to_char( " ", pCh );
  }
  send_to_char( "\n\r", pCh );
}

void FreeGuardianList( struct GuardianListNames *pGList )
{
  int i;
  if( pGList )
  {
    for( i = 0; i < pGList->num_names; i++ )
    {
      if( pGList->names[ i ] )
        free( pGList->names[ i ] );
    }
    if( pGList->names )
      free( pGList->names );
    free( pGList );
  }
  else
    mudlog( LOG_SYSERR, "pGList == NULL in FreeGuardianList(rhyodin)" );
}

int guardian( struct char_data *ch, int cmd, char *arg, struct char_data *mob, 
              int type)
{
#define RHYODIN_FILE "rhyodin"
#define Necklace 21122
#define RHYODIN_PREMIO 5000000

  FILE *pass;
  struct char_data *master;
  struct obj_data *obj;
  struct room_data *rp;
  struct follow_type *fol;
  char player_name[80], obj_name[80], name[15];
  struct GuardianListNames *pGList;

  if( mob->generic == -1 )
    return FALSE;  /* ci deve essere stato qualche problema */

  if( mob->act_ptr == NULL )
  {
    mudlog( LOG_CHECK, "Inizializing Rhyodin guardian list" );
    /* Open the file, read the names into an array in the act pointer */
    if( !( pass = fopen( RHYODIN_FILE, "r" ) ) ) 
    {
      mudlog( LOG_ERROR, "Rhyodin access file unreadable or non-existant");
      mob->generic = -1;
      return FALSE;
    }

    mob->act_ptr = pGList = NewGuardianList();

    while( 1 == fscanf( pass, " %s", name ) ) 
    {
      InsertInGuardianList( pGList, name );
    }
    
    fclose( pass );
    mob->generic = 1;
    mudlog( LOG_CHECK, "Initialized." );
  }
  else 
    pGList = (struct GuardianListNames *) mob->act_ptr;

  if( type == EVENT_TICK ) 
  {
    if( mob->specials.fighting ) 
    {
      if( IS_NPC( mob->specials.fighting ) &&
         !IS_SET( (mob->specials.fighting)->specials.act, ACT_POLYSELF ) )
      {
        if( (master = (mob->specials.fighting)->master) && CAN_SEE(mob, master)) 
        {
          stop_fighting( mob );
          hit( mob, master, TYPE_UNDEFINED);
          return TRUE;
        }
      }
      if( GET_POS(mob) == POSITION_FIGHTING ) 
      {
        FighterMove( mob );
      } 
      else 
      {
        StandUp( mob );
      }
    }
    return FALSE;
  }
  else if( type == EVENT_COMMAND )
  {
    if( cmd == CMD_GIVE )
    {
      arg = one_argument( arg, obj_name );
      if( !*obj_name || 
          !( obj = get_obj_in_list_vis( ch, obj_name, ch->carrying ) ) )
        return FALSE;
      only_argument( arg, player_name );
      if( !*player_name || get_char_room_vis( ch, player_name ) != mob )
        return FALSE;
      
      act( "Dai $p a $N.", TRUE, ch, obj, mob, TO_CHAR );
      act( "$n da` $p a $N.", TRUE, ch, obj, mob, TO_ROOM );
        
      if( obj->item_number >= 0 &&
          obj_index[ obj->item_number ].iVNum == Necklace )
      {
          
        act( "$n guarda perplesso $p.",
             FALSE, mob, obj, 0, TO_ROOM);
        if( !check_soundproof( mob ) ) 
        {
          act( "$c0015[$c0005$n$c0015] dice:",
               FALSE, mob, 0, 0, TO_ROOM );
          act( "$c0015AH! Quel ringoglionito di Valik!",
               FALSE, mob, 0, 0, TO_ROOM );
          act( "$c0015Gliel'avro` detto mille volte, ma lui NO, non ci sente.",
               FALSE, mob, 0, 0, TO_ROOM );
          act( "$c0015Mi dispiace, ma il regno di Rhyodin e` scomparso da tempo.",
               FALSE, mob, 0, 0, TO_ROOM );
          act( "$c0015Valik continua a chiedere a voi avventurieri di cercare la",
               FALSE, mob, 0, 0, TO_ROOM );
          act( "$c0015strada di collegamento fra Rhyodin ed il mondo esterno.",
               FALSE, mob, 0, 0, TO_ROOM );
          act( "$c0015Ma questa strada non serve piu`.",
               FALSE, mob, 0, 0, TO_ROOM );
          act( "$c0015Il popolo di Rhyodin, isolato dal resto del mondo, si e` estinto.",
               FALSE, mob, 0, 0, TO_ROOM );
          act( "$c0015Finche` un giorno, un grande Dio, vedendo il vecchio reame cosi`",
               FALSE, mob, 0, 0, TO_ROOM );
          act( "$c0015malridotto, preso da malinconia ha deciso di distruggerlo.",
               FALSE, mob, 0, 0, TO_ROOM );
          act( "$c0015Dietro questo cancello troverete solo luoghi a voi familiari.",
               FALSE, mob, 0, 0, TO_ROOM );
          act( "$c0015Mi dispiace. Tuttavia, fate tesoro dell'esperienza acquisita.'",
               FALSE, mob, 0, 0, TO_ROOM );
        }
          
          /* Take it away */
        act( "$p brilla nella mani di $n e sparisce.",
             FALSE, mob, obj, 0, TO_ROOM);
        obj_from_char( obj );
        extract_obj( obj );
          
        if( !IS_NPC( ch ) ) 
        {
          if( !( pass = fopen( RHYODIN_FILE, "a" ) ) ) 
          {
            mudlog( LOG_ERROR, "Couldn't open file for writing permanent "
                               "Rhyodin passlist.");
            mob->generic = -1;
            return FALSE;
          }
          
          if( !IsInGuardianList( pGList, GET_NAME( ch ) ) )
          {
            /* Go to the end of the file and write the character's name */
            fprintf( pass, " %s", GET_NAME( ch ) );
            InsertInGuardianList( pGList, GET_NAME( ch ) );
            gain_exp( ch, RHYODIN_PREMIO );
            for( fol = ch->followers; fol ;fol = fol->next ) 
            {
              if( ch->in_room == fol->follower->in_room && 
                  IS_PC( fol->follower ) )
              {
                if( !IsInGuardianList( pGList, GET_NAME( fol->follower ) ) )
                {
                  fprintf( pass, " %s", GET_NAME( fol->follower ) );
                  InsertInGuardianList( pGList, GET_NAME( fol->follower ) );
                  gain_exp( fol->follower, RHYODIN_PREMIO );
                }
              }
            }
            fprintf( pass, "\n" );
          }
          fclose( pass );
        }
        return(TRUE);
      }
    }
    else if( cmd == CMD_SOUTH && !IS_NPC(ch) ) 
    {
      /* Trying to move south, check against namelist */
      if( IsInGuardianList( pGList, GET_NAME( ch ) ) )
      {
        if( real_roomp( ch->in_room ) && EXIT( ch, 2 )->to_room != NOWHERE ) 
        {
          if( ch->specials.fighting ) 
            return(FALSE);
          act( "$N ti scorta attraverso il cancello.",
               FALSE, ch, 0, mob, TO_CHAR);
          act( "$N scorta $n attraverso il cancello.",
               FALSE, ch, 0, mob, TO_ROOM);
          rp = real_roomp(ch->in_room);
          char_from_room(ch);
          char_to_room(ch,rp->dir_option[2]->to_room);
          do_look(ch, "\0", 15);
          /* Follower stuff again */
          if(ch->followers) 
          {
            act( "$N dice 'Se sono con te possono passare anche gli altri.'",
                 FALSE, ch, 0, mob, TO_CHAR);
            for(fol = ch->followers ; fol ;fol = fol->next) 
            {
              if( fol->follower->specials.fighting ) 
                continue;
              if( real_roomp( fol->follower->in_room ) &&
                  EXIT(fol->follower,2)->to_room != NOWHERE &&
                  GET_POS( fol->follower ) >= POSITION_STANDING ) 
              {
                char_from_room(fol->follower);
                char_to_room(fol->follower,rp->dir_option[2]->to_room);
                do_look(fol->follower, "\0", 15);
              }
            }
          }
          return TRUE;
        }
      }
    }
    else if( cmd == CMD_PLAYER )
    {
      send_to_char( "Questo e` l'elenco dei giocatori che hanno risolto la "
                    "quest:\n\r", ch );
      ShowGuardianList( pGList, ch );
    }
  }
  else if( type == EVENT_DEATH )
  {
    if( pGList )
      FreeGuardianList( pGList );
    mob->act_ptr = pGList = NULL;
  }
  return(FALSE);
}

/***************************************************************************
  web_slinger viene attaccata ad un mob che lancia ragnatele appicicose.
  Viene attaccata al ragno nella cava dei trigloditi.
****************************************************************************/

int web_slinger( struct char_data *ch, int cmd, char *arg, 
                 struct char_data *mob, int type )
{
  struct char_data *vict;

  if( ( cmd || !AWAKE(ch) ) || IS_AFFECTED( ch, AFF_BLIND ) )
    return(FALSE);

  if( ( GET_POS(ch) > POSITION_STUNNED ) &&
      ( GET_POS(ch) < POSITION_FIGHTING ) ) 
  {
    StandUp(ch);
    return(TRUE);
  }
  
  /* Find a dude to to evil things upon ! */
  vict = ch->specials.fighting;
  
  if( !vict )
    vict = FindVictim( ch );
  
  if( !vict ) 
    return(FALSE);

  if( number( 0, 3 ) == 0 ) 
  {
    /* Noop, for now */
    act( "$n ti avvolge in una ragnatela appiccicosa!", TRUE, ch, 0, 0, 
         TO_ROOM );
    cast_web( GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0 );
    return TRUE;
  }
  return FALSE;
}

/**************************************************************************
  Okay, the idea is this: If the PC or NPC in this room isn't flying,
  it is walking on the trapper. Doesn't matter if it's sneaking, or
  invisible, or whatever. The trapper will attack both PCs and NPCs,
  so don't have a lot of wandering NPCs around it. 
***************************************************************************/

int trapper( struct char_data *ch, int cmd, char *arg, struct char_data *mob, 
             int type )
{
  struct char_data *tch;

  if( cmd || !AWAKE(ch) ) 
    return (FALSE);

  if( !ch->specials.fighting ) 
  {
    for( tch = real_roomp(ch->in_room)->people; tch; tch = tch->next_in_room )
      if( ch != tch && !IS_IMMORTAL( tch ) && !IS_AFFECTED( tch,AFF_FLYING ) ) 
      {
        set_fighting( ch, tch );
        return(TRUE);
      }
    /* Nobody here */
    return(FALSE);
  }
  else 
  {
    if( GetMaxLevel( ch->specials.fighting ) > MAX_MORT ) 
      return(FALSE);
    
    /* Equipment must save against crush - will fail 25% of the time */
    DamageStuff( ch->specials.fighting, TYPE_CRUSH, number( 0, 7 ) );
    
    /* Make the poor sucker save against paralzyation, or suffocate */
    if( saves_spell( ch->specials.fighting, SAVING_PARA ) )
    {
      act( "Non riesci a respirare, $N ti sta` soffocando!",
           FALSE, ch->specials.fighting, 0, ch, TO_CHAR );
      act( "$N sta soffocando $n!",
           FALSE, ch->specials.fighting, 0, ch, TO_ROOM );
      return( FALSE );
    }
    else 
    {
      act( "Boccheggi in cerca d'aria!",
           FALSE, ch->specials.fighting, 0, ch, TO_CHAR);
      act( "$N ti soffoca fino alla morte!",
           FALSE, ch->specials.fighting, 0, ch, TO_CHAR);
      act( "$n soffoca dentro $N!",
           FALSE, ch->specials.fighting, 0, ch, TO_ROOM);
      act( "$n e` mort$b!", FALSE, ch->specials.fighting, 0, ch, TO_ROOM);
      mudlog( LOG_PLAYERS, "%s has suffocated to death.", 
              GET_NAME(ch->specials.fighting));
      die( ch->specials.fighting, 0 );
      ch->specials.fighting = 0x0;
      return( TRUE );
    }
  }
}

/***************************************************************************
  Le guardie dei trogloditi si chiamano fra loro quando vengono attaccate
  Inoltre, se non stanno combattendo, attaccano il piu` buono nella stanza.
***************************************************************************/

int troguard( struct char_data *ch, int cmd, char *arg, struct char_data *mob, 
              int type)
{
  struct char_data *tch, *good;
  int max_good;
  
  if( cmd || !AWAKE( ch ) )
    return (FALSE);
  
  if( ch->specials.fighting ) 
  {
    if( GET_POS(ch) == POSITION_FIGHTING )
    {
      FighterMove(ch);
    } 
    else 
    {
      StandUp(ch);
    }
    
    if( !check_soundproof(ch) ) 
    {
      act( "$c0009[$c0015$n$c0009] urla 'Il nemico e` fra noi! Aiutatemi "
           "fratelli!'", TRUE, ch, 0, 0, TO_ROOM );
      if( ch->specials.fighting )
        CallForGuard( ch, ch->specials.fighting, 3, TROGCAVES );
      return(TRUE);
    }
    return FALSE;
  }
  
  max_good = -1001;
  good = 0;
  
  for( tch = real_roomp( ch->in_room )->people; tch; tch = tch->next_in_room )
    if( GET_ALIGNMENT( tch ) > max_good && !IS_IMMORTAL( tch ) &&
        GET_RACE(tch) != RACE_TROGMAN && GET_RACE(tch) != RACE_ARACHNID ) 
    {
      max_good = GET_ALIGNMENT( tch );
      good = tch;
    }
  
  if( check_peaceful( ch, "" ) )
    return FALSE;
  
  if( good )
  {
    if( !check_soundproof( ch ) )
      act( "$c0009[$c0015$n$c0009] urla 'Muori invasore! Prendi questo!'", 
           FALSE, ch, 0, 0, TO_ROOM );
    hit( ch, good, TYPE_UNDEFINED );
    return(TRUE);
  }
  
  return(FALSE);
}

/****************************************************************************
  Il cuoco dei trogloditi se trova un animale lo attacca. Se c'e` un corpo
  lo mette nella pentola.
*****************************************************************************/
int trogcook( struct char_data *ch, int cmd, char *arg, struct char_data *mob, 
              int type)
{
  struct char_data *tch;
  struct obj_data *corpse;
  char buf[MAX_INPUT_LENGTH];

  if( cmd || !AWAKE(ch) ) 
    return (FALSE);
  
  if( ch->specials.fighting )
  {
    if( GET_POS( ch ) != POSITION_FIGHTING ) 
      StandUp(ch);
    return FALSE;
  }
  
  for( tch = real_roomp( ch->in_room )->people; tch; tch = tch->next_in_room )
    if( IS_NPC( tch ) && IsAnimal( tch ) && CAN_SEE( ch, tch ) )
    {
      if( !check_soundproof( ch ) )
        act( "$c0015[$c0013$n$c0015] ridacchia 'Qualcos'altro per lo stufato!'",
             FALSE, ch, 0, 0, TO_ROOM );
      hit( ch, tch, TYPE_UNDEFINED );
      return TRUE;
    }
  
  corpse = get_obj_in_list_vis( ch,"corpse", 
                                real_roomp( ch->in_room )->contents );
  if( corpse ) 
  {
    do_get( ch, "corpse", -1 );
    act( "$c0015[$c0013$n$c0015] ridacchia 'Nella zuppa con il resto!'", 
         FALSE, ch, 0, 0, TO_ROOM );
    sprintf(buf, "put corpse pot");
    command_interpreter( ch, buf );
    return(TRUE);
  }
  return FALSE;
}

/**************************************************************************
  Lo shaman chiama Golgar in aiuto se sta` combattendo.
**************************************************************************/
#define DEITY 21124
#define DEITY_NAME "golgar"

int shaman( struct char_data *ch, int cmd, char *arg, struct char_data *mob, 
            int type )
{
  struct char_data *god, *tch;
  
  if( cmd || !AWAKE(ch) )
    return (FALSE);
  
  if( ch->specials.fighting ) 
  {
    if( number( 0, 3 ) == 0 )
    {
      for( tch = real_roomp( ch->in_room )->people; tch; 
           tch = tch->next_in_room )
      {
        if( !IS_NPC( tch ) && GetMaxLevel( tch ) > 20 && CAN_SEE( ch, tch ) ) 
        {
          if( !( god = get_char_room_vis( ch, DEITY_NAME ) ) ) 
          {
            act( "$c0009[$c0015$n$c0009] urla 'Golgar, vieni in aiuto del tuo "
                 "umile servo!'", FALSE, ch, 0, 0, TO_ROOM);
            if( number( 0,8 ) == 0) 
            {
              act( "$c0015C'e` un accecante lampo di luce!",
                   FALSE, ch, 0, 0, TO_ROOM );
              god = read_mobile( DEITY, VIRTUAL );
              char_to_room( god, ch->in_room );
            }
          }
          else if( number( 0, 2 ) == 0 )
            act( "$c0009[$c0015$n$c0009] urla 'E adesso morirai!'", 
                 FALSE, ch, 0, 0, TO_ROOM );
        }
      }
    }
    else 
      return( cleric( ch, cmd, arg, mob, type ) );
  }
  return FALSE;
}

/*************************************************************************
  Golgar aiuta i trogloditi e lo shaman se stanno combattendo.
  Se lo shaman non c'e` e nessu troglodita combatte, scompare e torna nel
  vuoto. Se lo shaman c'e`, ma non combatte, si incazza e lo attacca.
*************************************************************************/

#define SHAMAN_NAME "shaman"

int golgar( struct char_data *ch, int cmd, char *arg, struct char_data *mob, 
            int type )
{
  struct char_data *shaman, *tch;

  if(cmd) 
    return (FALSE);
  
  if( !ch->specials.fighting ) 
  {
    if( !( shaman = get_char_room_vis( ch, SHAMAN_NAME ) ) ) 
    {
      for( tch = real_roomp( ch->in_room)->people; tch; 
           tch = tch->next_in_room )
      {
        if( IS_NPC( tch ) && GET_RACE( tch ) == RACE_TROGMAN )
        {
          if( tch->specials.fighting && !IS_NPC( tch->specials.fighting ) ) 
          {
            act( "$c0015[$c0013$n$c0015 ruggisce 'Morte a chi attacca la mia "
                 "gente!'", FALSE, ch, 0, 0, TO_ROOM );
            hit( ch, tch->specials.fighting, TYPE_UNDEFINED );
            return FALSE;
          }
        }
      }
      if( number(0,5) == 0 ) 
      {
        act( "$n sparisce lentamente nel vuoto etereo.",
             FALSE, ch, 0, 0, TO_ROOM);
        extract_char( ch );
      }
    }
    else 
    {
      if( !shaman->specials.fighting ) 
      {
        act( "$c0015[$c0013$n$c0015] ruggisco 'Per quale motivo mi hai "
             "chiamato!'", FALSE, ch, 0, 0, TO_ROOM );
        hit( ch, shaman, TYPE_UNDEFINED );
        return TRUE;
      }
      else 
      {
        act( "$c0009[$c0015$n$c0009] urla 'Come ti permetti di toccare un "
             "mio seguace! Muori!'", FALSE, ch, 0, 0, TO_ROOM );
        hit( ch, shaman->specials.fighting, TYPE_UNDEFINED );
        return TRUE;
      }
    }
  }
  else 
    return( magic_user( ch, cmd, arg, mob, type ) );
  return FALSE;
}

/****************************************************************************
  Lattimore e` un personaggio chiave per la quest. E` l'unico che ti da` la 
  chiave per uscire dall'outpost. Per avere la chiave, e` necessario dargli
  alcune cose da mangiare (almeno 5) o 20 oggetti non da mangiare, o il
  rod usato per aprire la plate nella cucina.
  Inoltre Lattimore fa altre cosine carine, tipo mettersi a dormire la
  sera, mangiare all'ora di pranzo ecc...
*****************************************************************************/

char *lattimore_descs[] = 
{
  "A small orc cerca di scassinare un armadietto.\n\r",
  "A small orc cammina risoluto per la stanza.\n\r",
  "A small orc mangia dello stufato di ratto.\n\r",
  "A small orc si nasconde sotto un letto.\n\r",
  "A small orc dorme profondamente su un letto.\n\r",
  "C'e` a small orc su un barile.\n\r",
  "A small orc corre velocemente per il corridoio.\n\r"
};


#define Lattimore_Initialize  0
#define Lattimore_Lockers     1
#define Lattimore_FoodRun     2
#define Lattimore_Eating      3
#define Lattimore_GoHome      4
#define Lattimore_Hiding      5
#define Lattimore_Sleeping    6
#define Lattimore_Run         7
#define Lattimore_Item        8

#define Kitchen   21310
#define Barracks  21277
#define Storeroom 21319
#define Conf      21322
#define Trap      21335
#define EarthQ    21334

#define CrowBar   21114
#define PostKey   21150

int lattimore( struct char_data *pChar, int nCmd, char *szArg, 
               struct char_data *pMob, int nType )
{
  struct char_data *pTarget;
  struct obj_data *pObj;
  char szObjName[ 80 ], szPlayerName[ 80 ];
  int nDir;
  
  pMob->lStartRoom = 0;

  if( nType == EVENT_TICK )
  {
    if( pMob->master )
    {
      FreeList( (CharElem **)&pMob->act_ptr );
      return FALSE;
    }
    
    if( !AWAKE( pMob ) ) 
      return FALSE;
    
    if( pMob->specials.fighting )
    {
      if( !IS_MOB( pMob->specials.fighting ) && 
          CAN_SEE( pMob, pMob->specials.fighting ) )
        AddIntData( (CharElem **)&pMob->act_ptr, pMob->specials.fighting, -5 );
          
      if( GetIntData( (CharElem *)pMob->act_ptr, pMob->specials.fighting ) < 0 )
      {
        strcpy( pMob->player.long_descr, lattimore_descs[ 6 ] );
        pMob->generic = Lattimore_Run;
      }

      return FALSE;
    }

    switch( pMob->generic ) 
    {
    /* This case is used at startup, and after player interaction*/
    case Lattimore_Initialize:
  
      if( time_info.hours < 5 || time_info.hours > 21 ) 
      {
        strcpy( pMob->player.long_descr, lattimore_descs[ 3 ] );
        if( pMob->in_room != Barracks ) 
        {
          char_from_room( pMob );
          char_to_room( pMob, Barracks);
        }
        pMob->generic = Lattimore_Hiding;
      }
      else if( time_info.hours < 11 ) 
      {
        strcpy( pMob->player.long_descr, lattimore_descs[ 4 ] );
        if( pMob->in_room != Barracks ) 
        {
          char_from_room( pMob );
          char_to_room( pMob, Barracks );
        }
        pMob->generic = Lattimore_Sleeping;
      }
      else if( time_info.hours < 16 ||
               ( time_info.hours > 17 && time_info.hours < 22 ) ) 
      {
        strcpy( pMob->player.long_descr, lattimore_descs[ 0 ] );
        if( pMob->in_room != Barracks ) 
        {
          char_from_room( pMob );
          char_to_room( pMob, Barracks );
        }
        pMob->generic = Lattimore_Lockers;
      }
      else if( time_info.hours < 19 ) 
      {
        strcpy( pMob->player.long_descr, lattimore_descs[ 1 ] );
        pMob->generic = Lattimore_FoodRun;
      }
      return FALSE;
      break;
        
    case Lattimore_Lockers:
  
      if( time_info.hours == 17 ) 
      {
        strcpy( pMob->player.long_descr, lattimore_descs[ 1 ] );
        pMob->generic = Lattimore_FoodRun;
      }
      else if( time_info.hours > 21 || time_info.hours < 5 ) 
      {
        act( "$n alza la testa come per ascoltare qualcosa.",
             FALSE, pMob, 0, 0, TO_ROOM);
        act( "$n, con lo sguardo spaventato, si infila sotto un letto.",
             FALSE, pMob, 0, 0, TO_ROOM);
        strcpy( pMob->player.long_descr, lattimore_descs[ 3 ] );
        pMob->generic = Lattimore_Hiding;
      }
      return FALSE;
      break;
        
    case Lattimore_FoodRun:
        
      if( pMob->in_room != Kitchen ) 
      {
        nDir = choose_exit_global( pMob->in_room, Kitchen, 100 );
        if( nDir < 0 ) 
        {
          do_say( pMob, "Ragazzi, mi sono perso!'", 0 );
          nDir = choose_exit_global( pMob->in_room, Barracks, 100 );
          if( nDir < 0 ) 
          {
            char_from_room( pMob );
            char_to_room( pMob, Barracks );
          }
        }
        else
          go_direction( pMob, nDir );
      }
      else 
      {
        act( "$n prende una pentola dall'armadio ed inizia a farsi uno "
             "stufato.", FALSE, pMob, 0, 0, TO_ROOM );
        strcpy( pMob->player.long_descr, lattimore_descs[ 2 ] );
        pMob->generic = Lattimore_Eating;
      }
      return FALSE;
      break;
        
    case Lattimore_Eating:
        
      if( time_info.hours > 18 )
      {
        act( "$n si frega lo stomaco e sorride soddisfatt$b.",
             FALSE, pMob, 0, 0, TO_ROOM);
        strcpy( pMob->player.long_descr, lattimore_descs[ 1 ] );
        pMob->generic = Lattimore_GoHome;
      }
      else if( !number( 0, 2 ) ) 
      {
        act( "$n prende del pane dal forno.",
             FALSE, pMob, 0, 0, TO_ROOM );
        act( "$n intinge il pane nello stufato e se lo mangia.",
             FALSE, pMob, 0, 0, TO_ROOM );
      }
      return FALSE;
      break;
        
    case Lattimore_GoHome:

      if( pMob->in_room != Barracks ) 
      {
        nDir = choose_exit_global( pMob->in_room, Barracks, 100 );
        if( nDir < 0 ) 
        {
          do_say( pMob, "Ragazzi, mi sono perso!", 0 );
          nDir = choose_exit_global( pMob->in_room, Kitchen, 100 );
          if( nDir < 0 )
          {
            char_from_room( pMob );
            char_to_room( pMob, Barracks );
          }
        }
        else
          go_direction( pMob, nDir );
      }
      else 
      {
        act( "$n tira fuori un grimaldello e prova ad aprire un'altro "
             "armadietto.", FALSE, pMob, 0, 0, TO_ROOM );
        strcpy( pMob->player.long_descr, lattimore_descs[ 0 ] );
        pMob->generic = Lattimore_Lockers;
      }         
      return FALSE;
      break;
  
    case Lattimore_Hiding:
  
      if( time_info.hours > 5 && time_info.hours < 22 ) 
      {
        strcpy( pMob->player.long_descr, lattimore_descs[ 4 ] );
        pMob->generic = Lattimore_Sleeping;
      }
      return FALSE;
      break;
        
    case Lattimore_Sleeping:
  
      if( time_info.hours > 11 ) 
      {
        act( "$n si sveglia e si stiracchia con uno sbadiglio.",
             FALSE, pMob, 0, 0, TO_ROOM );
        act( "$n tira fuori un grimaldello e prova ad aprire un altro "
             "armadietto.", FALSE, pMob, 0, 0, TO_ROOM );
        strcpy( pMob->player.long_descr, lattimore_descs[ 0 ] );
        pMob->generic = Lattimore_Lockers;
      }
      return FALSE;
      break;
        
    case Lattimore_Run:
  
      if( pMob->in_room != Storeroom && pMob->in_room != Trap )
      {
        if( pMob->in_room == EarthQ ) 
          return FALSE;
        nDir = choose_exit_global( pMob->in_room, Storeroom, 100 );
        if( nDir < 0 ) 
        {
          do_say( pMob, "Ragazzi, mi sono perso!'", 0 );
          nDir = choose_exit_global( pMob->in_room, Kitchen, 100 );
          if( nDir < 0 ) 
          {
            char_from_room( pMob );
            char_to_room( pMob, Barracks );
          }
        }
        else 
          go_direction( pMob, nDir );
      }
      else if( pMob->in_room == Trap ) 
      {
        static int nTrapCounter = 0;
        if( !IS_AFFECTED( pMob, AFF_FLYING ) ) 
        {
          /* Get him up off the floor */
          act( "$n sogghigna e sale velocemente su un barile.",
               FALSE, pMob, 0, 0, TO_ROOM );
          SET_BIT( pMob->specials.affected_by, AFF_FLYING );
          strcpy( pMob->player.long_descr, lattimore_descs[ 5 ] );
          nTrapCounter = 0;
        }
        else 
          nTrapCounter++;
          /* Wait a while, then go home */
        if( nTrapCounter == 50 )
        {
          pMob->generic = Lattimore_GoHome;
          REMOVE_BIT( pMob->specials.affected_by, AFF_FLYING );
          strcpy( pMob->player.long_descr, lattimore_descs[ 1 ] );
          go_direction( pMob, 1 );
        }
      }
      return FALSE;
      break;

    case Lattimore_Item:
  
      if( pMob->in_room != Conf ) 
      {
        nDir = choose_exit_global( pMob->in_room, Conf, 100 );
        if( nDir < 0 ) 
        {
          do_say( pMob, "Ragazzi, mi sono perso!", 0 );
          nDir = choose_exit_global( pMob->in_room, Barracks, 100 );
          if( nDir < 0 ) 
          {
            char_from_room( pMob );
            char_to_room( pMob, Barracks );
          }
        }
        else
          go_direction( pMob, nDir );
      }
      else 
      {
        for( pTarget = real_roomp( pMob->in_room )->people; pTarget; 
             pTarget = pTarget->next_in_room )
        {
          if( !IS_NPC( pTarget ) && CAN_SEE( pMob, pTarget ) )
          {
            if( GetIntData( (CharElem *)pMob->act_ptr, pTarget ) >= 20 ) 
            {
              act( "$n si infila sotto il grande tavolo.",
                   FALSE, pMob, 0, 0, TO_ROOM );
              pObj = read_object( PostKey, VIRTUAL );
              if( ( IS_CARRYING_N( pTarget ) + 1 ) < CAN_CARRY_N( pTarget ) ) 
              {
                act( "$N esce fuori da sotto il tavolo e ti da $p.",
                     FALSE, pTarget, pObj, pMob, TO_CHAR );
                act( "$n esce fuori da sotto il tavolo con $p che da a $N.",
                     FALSE, pMob, pObj, pTarget, TO_NOTVICT );
                obj_to_char( pObj, pTarget );
              }
              else 
              {
                act( "$n esce fuori da sotto il tavolo e getta $p per $N.",
                     FALSE, pMob, pObj, pTarget, TO_ROOM );
                obj_to_room( pObj, pMob->in_room );
              }
              RemoveFromList( (CharElem **)&pMob->act_ptr, pTarget );
              break;
            }
          }
        }
        /* Dude's not here - oh well, go home. */
        pMob->generic = Lattimore_GoHome;
      }
      return FALSE;
      break;
        
    default:
      pMob->generic = Lattimore_Initialize;
      return FALSE;
      break;

    }
  }
  else if( nType == EVENT_COMMAND && nCmd == CMD_GIVE ) 
  {
    szArg = one_argument( szArg, szObjName );
    if( *szObjName && 
        ( pObj = get_obj_in_list_vis( pChar, szObjName, pChar->carrying ) ) )
    {
      only_argument( szArg, szPlayerName );
      if( *szPlayerName && get_char_room_vis( pChar, szPlayerName ) == pMob )
      {
        act( "Dai $p a $N.", TRUE, pChar, pObj, pMob, TO_CHAR );
        act( "$n da` $p a $N.", TRUE, pChar, pObj, pMob, TO_ROOM );

        switch( pObj->obj_flags.type_flag ) 
        {
        case ITEM_FOOD:
          if( pObj->obj_flags.value[ 3 ] ) 
          {
            act( "$n annusa $p e lo getta disgustat$b.", 
                 TRUE, pMob, pObj, 0, TO_ROOM );
            obj_from_char( pObj );
            obj_to_room( pObj, pMob->in_room );
            if( !IS_MOB( pChar ) && CAN_SEE( pMob, pChar) )
              AddIntData( (CharElem **)&pMob->act_ptr, pChar, -5 );
            else 
              return TRUE;
          }
          else 
          {
            act( "$n prende $p e lo ingoia affamat$b.", 
                 TRUE, pMob, pObj, 0, TO_ROOM );
            extract_obj( pObj );
            if( !IS_MOB( pChar ) && CAN_SEE( pMob, pChar ) )
              AddIntData( (CharElem **)&pMob->act_ptr, pChar, 4 );
            else 
              return TRUE;
          }
          break;
        case ITEM_KEY:
          /* What he really wants */
          if( pObj->item_number >= 0 &&
              obj_index[ pObj->item_number ].iVNum == CrowBar ) 
          {
            act( "$n prende $p salta su e giu` contento.", 
                 TRUE, pMob, pObj, 0, TO_ROOM );
            obj_from_char( pObj );
            if( !pMob->equipment[ HOLD ] ) 
              equip_char( pMob, pObj, HOLD );
            else
              obj_to_char( pObj, pMob );
            if( !IS_MOB( pChar ) && CAN_SEE( pMob, pChar ) )
              AddIntData( (CharElem **)&pMob->act_ptr, pChar, 20 );
            else 
              return TRUE;
            break;
          }
          /* Il break qui non ci va, perche` se la chiave data a Lattimore
             non e` il rod (CrowBar) deve fare quello che fa per qualunque
             altro oggetto */
        default:
          /* Any other types of items */
          act( "$n guarda a $p con curiosita`.", TRUE, pMob, pObj, 0, TO_ROOM );
          if( !IS_MOB( pChar ) && CAN_SEE( pMob, pChar ) )
            AddIntData( (CharElem **)&pMob->act_ptr, pChar, 1 );
          else
            return TRUE;
          break;
        }
        /* They gave something to him, and the status was affected,
           now we set the pointer according to the status value */
        if( GetIntData( (CharElem *)pMob->act_ptr, pChar ) < 0 ) 
        {
          strcpy( pMob->player.long_descr, lattimore_descs[ 6 ] );
          pMob->generic = Lattimore_Run;
        }
        else if( GetIntData( (CharElem *)pMob->act_ptr, pChar ) >= 20 ) 
        {
          strcpy( pMob->player.long_descr, lattimore_descs[ 6 ] );
          pMob->generic = Lattimore_Item;
        }
        return TRUE;
      }
    }
  }
  else if( nType == EVENT_DEATH )
  {
    FreeList( (CharElem **)&pMob->act_ptr );
  }
  return FALSE;
}

/****************************************************************************
  Hai! Questo fa male. Mentre combatte casta chill touch, cone of cold o
  ice storm. Meglio evitarlo.
****************************************************************************/

int coldcaster( struct char_data *ch, int cmd, char *arg, 
                struct char_data *mob, int type)
{
  struct char_data *vict;
  byte lspell;

  if (cmd || !AWAKE(ch))
    return(FALSE);

  /* Find a dude to to evil things upon ! */
  
  vict = ch->specials.fighting;
  
  if (!vict)
    vict = FindVictim(ch);
  
  if (!vict) 
    return(FALSE);

  lspell = number(0,9);

  switch(lspell) 
  {
  case 0: 
  case 1: 
  case 2: 
  case 3:
    act( "$N ti tocca!", TRUE, vict, 0, ch, TO_CHAR );
    cast_chill_touch( GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0 );
    break;
  case 4: 
  case 5: 
  case 6:
    cast_cone_of_cold( GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0 );
    break;
  case 7: 
  case 8: 
  case 9:
    cast_ice_storm( GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0 );
    break;
  }

  return(TRUE);
  
}


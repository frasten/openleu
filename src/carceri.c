#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>

#include "protos.h"
#include "carceri.h"
#include "charlist.h"
#include "cmdid.h"
#include "fight.h"

extern struct zone_data *zone_table;
extern struct index_data *mob_index;

#define PIAZZA_ALMA 3005

/*****************************************************************************
  Minicius cerca di raggiungere la piazza principale di Alma. Una volta
  arrivato li, si comporta come specificato nei flag act.
*****************************************************************************/
int Minicius( struct char_data *pChar, int nCmd, const char *szArg, 
              struct char_data *pMob, int nType )
{
  if( nType == EVENT_TICK && pMob->generic == 0 )
  {
    if( pMob->in_room == PIAZZA_ALMA )
      pMob->generic = 1;
    else
    {
      int nDir = choose_exit_global( pMob->in_room, PIAZZA_ALMA, MAX_ROOMS );
      if( nDir == -1 ) /* can't go anywhere, wait... */
        return FALSE;
      go_direction( pMob, nDir );
      return TRUE;
    }
  }
  return FALSE;
}

/*****************************************************************************
  Chiunque veda il Verme della Morte, viene addormentato dal suo sguardo
  ipnotico, a meno con non azzecchi un tiro salvezza contro paralisi.
*****************************************************************************/
int VermeDellaMorte( struct char_data *pChar, int nCmd, const char *szArg, 
                     struct char_data *pMob, int nType )
{
  if( nType == EVENT_TICK && AWAKE( pMob ) )
  {
    struct room_data *pRoom;
    
    if( ( pRoom = real_roomp( pMob->in_room ) ) != NULL )
    {
      struct char_data *pNext, *pTar;
      for( pTar = pRoom->people; pTar; pTar = pNext )
      {
        pNext = pTar->next_in_room;
        if( CAN_SEE( pTar, pMob ) && 
            ( ( IS_PC( pTar ) && 
                !IS_SET( pTar->specials.act, PLR_NOHASSLE ) ) ||
              ( IS_NPC( pTar ) && 
                ( ( pTar->specials.zone != pMob->specials.zone &&
                    !strchr( zone_table[ pTar->specials.zone ].races, 
                             GET_RACE( pTar ) ) ) ||
                  IS_SET( pTar->specials.act, ACT_ANNOYING ) ) ) ) && 
            GET_POS( pTar ) > POSITION_SLEEPING &&
            !IsImmune( pTar, IMM_SLEEP ) )
        {
          if( IsSusc( pTar, IMM_SLEEP ) || 
              ( !saves_spell( pTar, SAVING_PARA ) && 
                ( !IsResist( pTar, IMM_SLEEP ) || 
                  !saves_spell( pTar, SAVING_PARA ) ) ) )
          {
            act( "$N ti guarda fisso. La tua vista si sdoppia.", FALSE,
                 pTar, 0, pMob, TO_CHAR );
            act( "$n cade a terra addormentat$b.", TRUE, pTar, 0, 0, 
                 TO_ROOM );
            if( pTar->specials.fighting )
              stop_fighting( pTar );
            GET_POS( pTar ) = POSITION_SLEEPING;
          }
        }
      }
    }
    else
      mudlog( LOG_SYSERR, 
              "pMob in invalid room in VermeDellaMorte( carceri.c )" );
  }
  return FALSE;
}
/*****************************************************************************
  Il figlio di Kyuss terrorizza al solo guardarlo. Percio` se il tiro
  salvezza fallisce, si fugge terrorizzati. Altrimenti si puo` restare nella
  locazione e` si e` immuni alla paura verso di lui per 12 ore.

  In pMob->act_ptr viene tenuta una lista dei giocatori che hanno questa
  temporanea immunita` dalla paura verso la creatura. Questa lista viene
  mantenuta dalle funzioni in 'charlist.c'.
*****************************************************************************/

int KyussSon( struct char_data *pChar, int nCmd, const char *szArg, 
              struct char_data *pMob, int nType )
{
  struct char_data *pTar;
  struct room_data *pRoom;

  if( pMob == NULL )
  {
    mudlog( LOG_SYSERR, "pMob == NULL in KyussSon( carceri.c )" );
    return FALSE;
  }

  if( nType == EVENT_TICK )
  {
    UpdateList( (CharElem **)&pMob->act_ptr );
    
    if( ( pRoom = real_roomp( pMob->in_room ) ) != NULL )
    {
      struct char_data *pNext;
      for( pTar = pRoom->people; pTar; pTar = pNext )
      {
        pNext = pTar->next_in_room;
        if( CAN_SEE( pTar, pMob ) && 
            !IsInList( (CharElem *)pMob->act_ptr, pTar ) && 
            ( ( IS_PC( pTar ) && 
                !IS_SET( pTar->specials.act, PLR_NOHASSLE ) ) ||
              ( IS_NPC( pTar ) && 
                ( ( pTar->specials.zone != pMob->specials.zone &&
                    !strchr( zone_table[ pTar->specials.zone ].races, 
                             GET_RACE( pTar ) ) ) ||
                  IS_SET( pTar->specials.act, ACT_ANNOYING ) ) ) ) )
        {
          if( !saves_spell( pTar, SAVING_PARA ) )
          {
            act( "Quando vedi $N, sei preso da un incontrollabile panico!",
                 TRUE, pTar, 0, pMob, TO_CHAR );
            do_flee( pTar, "", 0 );
          }
          else
          {
            InsertInList( (CharElem **)&pMob->act_ptr, pTar, 
                          ( SECS_PER_MUD_HOUR * 48 ) / PULSE_MOBILE );
          }
        }
      }
    }
    else
      mudlog( LOG_SYSERR, "pMob in invalid room in KyussSon( carceri.c )" );
  }
  else if( nType == EVENT_DEATH )
    FreeList( (CharElem **)&pMob->act_ptr );
  return FALSE;
}

/****************************************************************************
  La Piovra ha fino ad otto tentacoli. Questi tentacoli li tira fuori quando
  nella stanza c'e` qualcuno che odia o che sta combattendo contro di lei.
  
  I tentacoli non sono altro che mob, che la procedura crea e manda a 
  cobattere contro il carattere che la piovra odia. Il tentacolo viene
  creato solo se il carattere odiato ha meno di 2 attaccanti (in modo da
  non mandare piu` di 2 tentacoli su un carattere).

  Se il tentacolo non combatte e non e` attaccato, la piovra lo ritira.
  Alla morte della piovra tutti i tentacoli vengono ritirati.
  Da notare che se un tentacolo muore il contatore dei tentacoli (in generic)
  non viene aggiurnato, e` quindi la piovra avra` sempre meno tentacoli.
*****************************************************************************/
#define TENTACOLI 320
int Piovra( struct char_data *pChar, int nCmd, const char *szArg,
            struct char_data *pMob, int nType )
{
  struct room_data *pRoom;
  if( pMob == NULL )
  {
    mudlog( LOG_SYSERR, "pMob == NULL in Piovra( carceri.h )" );
    return FALSE;
  }
  
  if( ( pRoom = real_roomp( pMob->in_room ) ) != NULL )
  {
    if( nType == EVENT_TICK && pMob->generic < 8 && AWAKE( pMob ) )
    {
      struct char_data *pNext, *pTar;
      
      for( pTar = pRoom->people; pTar; pTar = pNext )
      {
        pNext = pTar->next_in_room;
        
        if( IS_PC( pTar ) && IS_SET( pTar->specials.act, PLR_NOHASSLE ) )
          continue;  /* Ignora chi e` in NOHASSLE */
          
        if( pTar->specials.fighting && 
            mob_index[ pTar->specials.fighting->nr ].iVNum == TENTACOLI &&
            CAN_SEE( pMob, pTar ) )
          AddHated( pMob, pTar );
        if( ( pTar->specials.fighting == pMob || Hates( pMob, pTar ) ) &&
            CAN_SEE( pMob, pTar ) && ( !pTar->attackers || 
            ( pTar->attackers == 1 && pMob->specials.fighting == pTar ) ) )
        {
          struct char_data *pTentacolo = read_mobile( TENTACOLI, VIRTUAL );
          if( pTentacolo )
          {
            char_to_room( pTentacolo, pMob->in_room );
            hit( pTentacolo, pTar, TYPE_CRUSH );
            pMob->generic++;
            if( pMob->mult_att > 1 )
              pMob->mult_att--;
          }
          else
            mudlog( LOG_ERROR, 
                    "Cannot load mob Tentacolo in Piovra(carceri.c)" );
                            
        }
        else if( mob_index[ pTar->nr ].iVNum == TENTACOLI && 
                 !pTar->specials.fighting && pTar->attackers <= 0 )
        {
          extract_char( pTar );
          pMob->generic--;
          if( pMob->mult_att < 6 )
            pMob->mult_att++;
        }
      }
    }
    else if( nType == EVENT_DEATH )
    {
      struct char_data *pNext, *pTar;
      
      for( pTar = pRoom->people; pTar; pTar = pNext )
      {
        pNext = pTar->next_in_room;
        if( mob_index[ pTar->nr ].iVNum == TENTACOLI )
          extract_char( pTar );
      }
    }
    else if( nType == EVENT_COMMAND && nCmd == CMD_BASH ) 
    {
      while( isspace( *szArg ) )
        szArg++;
        
      if( ( !*szArg && pChar->specials.fighting == pMob ) ||
          ( *szArg && get_char_room_vis( pChar, szArg ) == pMob ) )
      {
        act( "Cerchi di colpire $N ma rimbalzi contro il suo corpo morbido.", 
             TRUE, pChar, 0, pMob, TO_CHAR );
        act( "$n cerca di colpire $N ma rimbalza contro il suo corpo morbido.",
             TRUE, pChar, 0, pMob, TO_ROOM );
        WAIT_STATE( pChar, PULSE_VIOLENCE * 2 );
        GET_POS( pChar ) = POSITION_SITTING;
        if( AWAKE( pMob ) )
          AddHated( pMob, pChar );
        return TRUE;
      }
    }
  }
  else
    mudlog( LOG_SYSERR, "pMob in invalid room in Piovra( carceri.c )" );

  return FALSE;
}

/*****************************************************************************
  Il moribondo, normalmente e` incapacitato con 0 HP. Se qualcuno lo
  aiuta, dice una frase (diversa a second del mob).
*****************************************************************************/
#define NANODELLECOLLINE 314

int Moribondo( struct char_data *pChar, int nCmd, const char *szArg,
               struct char_data *pMob, int nType )
{
  if( pMob == NULL || pChar == NULL )
  {
    mudlog( LOG_SYSERR, 
            "pMob == NULL || pChar == NULL in Moribondo( carceri.h )" );
    return FALSE;
  }

  if( nType == EVENT_TICK )
  {
    switch( pMob->generic )
    {
    case 0:
      GET_HIT( pMob ) = -3;
      GET_POS( pMob ) = POSITION_INCAP;
      pMob->generic = 1;
      break;
    case 1:
      if( GET_HIT( pMob ) > 0 && !pMob->specials.fighting )
      {
        do_say( pMob, "Grazie! Grazie! Chiunque sia stato.", 0 );
        if( pMob->player.description )
          free( pMob->player.description );
        pMob->player.description = strdup( "Il piccolo nano e` contento e "
                                           "grato a chi lo ha aiutato.\n\r" );
        pMob->generic = 2;
      }
      else if( !pMob->specials.fighting )
      {
        MakeNoise( pMob->in_room, NULL, "Senti qualcuno lamentarsi.\n\r" );
      }
      break;
    case 2:
      if( !pMob->specials.fighting )
      {
        do_say( pMob, 
                "Guardate vicino al letto. C'e` un passaggio segreto!", 0 );
        pMob->generic = 3;
      }
      break;
    default:
      if( !number( 0, 5 ) && !pMob->specials.fighting )
      {
        do_say( pMob, 
                "Guardate vicino al letto. C'e` un passaggio segreto!", 0 );
      }
      break;
    }
  }
  return FALSE;
}

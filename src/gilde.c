#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

#include "protos.h"
#include "cmdid.h"
#include "gilde.h"
#include "charlist.h"

typedef struct structSocioGilda
{
  char szNomeSocio[ 21 ];
  struct structSocioGilda *pNext;
} TSocioGilda;

typedef struct
{
  int nGuardia;
  long nGuardiaRoom;
  int nDir;
  int nBanchiere;
  int nBanca;
  int nBanchiereXP;
  int nBancaXP;
  char szBaseFileName[ 81 ];
  int nLibroSoci;
  char szNomeCapo[ 21 ];
  TSocioGilda *pListaSoci;
} TDatiGilda;

extern struct index_data *obj_index;
extern struct title_type titles[MAX_CLASS][ABS_MAX_LVL];
extern struct index_data *mob_index;

TDatiGilda *pDatiGilde = NULL;

#define GUILD_DIR "gilde"
#define GUILD_GOLD_TOT ".gold.tot"
#define GUILD_GOLD_LOG ".gold.log"
#define GUILD_EXP_TOT  ".exp.tot"
#define GUILD_EXP_LOG  ".exp.log"
#define GUILD_MEMBER   ".member"
#define GUILD_DAT      "leudata.gui"

#define GUILD_MAXXP_DEP 100000
#define GUILD_MAXXP_PRE 5000000


/*****************************************************************************
 * Le funzioni che seguono servono per implementare la lista dei membri di 
 * una gilda. Mi piacerebbe convertire tutto in C++...
 * **************************************************************************/
                                                    


/*****************************************************************************
 * Inizializza la lista dei nomi dei membri della gilda leggendoli dal file
 * appropriato. Il parametro nIndex e' l'indice dell'array pDatiGilda
 * corrispondente alla gilda desiderata.
 * **************************************************************************/

void InitializeMemberList( int nIndex )
{
  char szFileName[ 256 ];
  char szDummy[ 161 ];
  FILE *pfMemberList;
  
  sprintf( szFileName, "%s/%s%s", GUILD_DIR, 
           pDatiGilde[ nIndex ].szBaseFileName, GUILD_MEMBER );
  
  if( ( pfMemberList = fopen( szFileName, "r+" ) ) != NULL )
  {
    szDummy [ 0 ] = 0;
    fscanf( pfMemberList, " %160s \n", szDummy );
    strncpy( pDatiGilde[ nIndex ].szNomeCapo, szDummy, 
             sizeof( pDatiGilde[ nIndex ].szNomeCapo ) - 1 );
    
    while( !feof( pfMemberList ) )
    {
      szDummy [ 0 ] = 0;
      fscanf( pfMemberList, " %160s \n", szDummy );
      if( strlen( szDummy ) )
      {        
        TSocioGilda *pSocio = (TSocioGilda *)calloc( 1, 
                                                     sizeof( TSocioGilda ) );
        if( pSocio )
        {
          strncpy( pSocio->szNomeSocio, szDummy, 
                   sizeof( pSocio->szNomeSocio ) - 1 );
          pSocio->pNext = pDatiGilde[ nIndex ].pListaSoci;
          pDatiGilde[ nIndex ].pListaSoci = pSocio;
        }
        else
        {
          mudlog( LOG_ERROR, 
                  "Cannot alloc memory in InitializeMemberList (gilde.c)." );
        }
      }            
    }
    
    fclose( pfMemberList );
  }  
}

void UpdateGuildListFile( int nIndex )
{
  char szFileName[ 256 ];
  FILE *pfMemberList;
  
  sprintf( szFileName, "%s/%s%s", GUILD_DIR, 
           pDatiGilde[ nIndex ].szBaseFileName, GUILD_MEMBER );
  
  if( ( pfMemberList = fopen( szFileName, "w" ) ) != NULL )
  {
    TSocioGilda *pSocio;
    
    fprintf( pfMemberList, "%s\n", pDatiGilde[ nIndex ].szNomeCapo );
    for( pSocio = pDatiGilde[ nIndex ].pListaSoci; pSocio;
         pSocio = pSocio->pNext )
    {
      fprintf( pfMemberList, "%s\n", pSocio->szNomeSocio );
    }
    fclose( pfMemberList );
  }
  else
  {
    mudlog( LOG_ERROR, 
             "Cannot create file %s in UpdateGuildListFile (gilde.c).",
            szFileName );
  }
}

  

int IsInGuildList( int nIndex, char *szNomeMembro )
{
  TSocioGilda *pSocio;
  
  for( pSocio = pDatiGilde[ nIndex ].pListaSoci; pSocio; 
       pSocio = pSocio->pNext )
  {
    if( strcasecmp( pSocio->szNomeSocio, szNomeMembro ) == 0 )
    {
      return TRUE;
    }
  }
  return FALSE;
}

void AppendToGuildList( int nIndex, char *szNomeMembro )
{
  TSocioGilda *pSocio = (TSocioGilda *)calloc( 1, sizeof( TSocioGilda ) );
  if( pSocio )
  {
    strncpy( pSocio->szNomeSocio, szNomeMembro, 
             sizeof( pSocio->szNomeSocio ) - 1 );
    pSocio->pNext = pDatiGilde[ nIndex ].pListaSoci;
    pDatiGilde[ nIndex ].pListaSoci = pSocio;
  
    UpdateGuildListFile( nIndex );
  }
  else
  {
    mudlog( LOG_ERROR, 
            "Cannot allocate memory in AppendToGuildList (gilde.c)." );
  }
}

void RemoveFromGuildList( int nIndex, char *szNomeMembro )
{
  TSocioGilda **ppSocio;
  int bRemoved = FALSE;
  
  for( ppSocio = &pDatiGilde[ nIndex ].pListaSoci; *ppSocio; 
       ppSocio = &(*ppSocio)->pNext )
  {
    if( strcasecmp( (*ppSocio)->szNomeSocio, szNomeMembro ) == 0 )
    {
      TSocioGilda *pSocio = *ppSocio;
      *ppSocio = (*ppSocio)->pNext;
      free( pSocio );
      bRemoved = TRUE;
      break;
    }
  }
  
  if( bRemoved )
  {
    UpdateGuildListFile( nIndex );
  }
}

/*************************************/
/* predicates for find_path function */

int IsGuildGuardRoomFP( int room, void *tgt_room )
{
	return room == *(int *)tgt_room;
}

/****************************************************************************
 * PlayersGuildGuard va assegnata ai guardiani delle gilde dei giocatori.
 * Il guardiano blocchera` tutti quelli che tentano di entrare nella gilda,
 * ma non sono iscritti nel libro.
 * E` possibile dare una chiave da tenere (HOLD) alla guardia, in modo che 
 * in caso di perdita, il capo della gilda la puo` avere.
 * Deve dare il comando 'ask guardia chiave' e la guardia dara` la chiave al
 * capo (e solo al capo). In caso di morte della guardia, questa distruggera`
 * la chiave tenuta.
 ****************************************************************************/

int PlayersGuildGuard( struct char_data *pCh, int nCmd, const char *pArg,
                       struct char_data *pMob, int nType )
{
  int nIndex;
  
  if( !pMob || !pCh || pMob->nr < 0 )
  {
    mudlog( LOG_SYSERR, 
            "!pMob || !pCh || pMob->nr < 0 in PlayersGuildGuard (gilde.c)" );
    return FALSE;
  }
  
  nIndex = pMob->generic;
  
  if( !nIndex )
  {
    
    for( nIndex = 0; pDatiGilde[ nIndex ].nGuardia &&
                      pDatiGilde[ nIndex ].nGuardia != 
                     mob_index[ pMob->nr ].iVNum; nIndex++ );
      
    if( !pDatiGilde[ nIndex ].nGuardia )
    {      
      mudlog( LOG_ERROR, 
              "PlayersGuildMaster assigned to wrong mob '%s' (%ld).",
              GET_NAME( pMob ), pMob->nr );
      mob_index[ pMob->nr ].func = NULL;
    }
    pMob->generic = nIndex + 1;
  }
  else
  {
    nIndex--;
  }  
    
  if( nType == EVENT_COMMAND && AWAKE( pMob ) )
  {
    if( nCmd >= CMD_NORTH && nCmd <= CMD_DOWN )
    {
      if( pMob->in_room == pDatiGilde[ nIndex ].nGuardiaRoom && 
          pDatiGilde[ nIndex ].nDir == nCmd - 1 )
      {
        if( !IsInGuildList( nIndex, GET_NAME( pCh ) ) &&
            strcasecmp( GET_NAME( pCh ), 
                        pDatiGilde[ nIndex ].szNomeCapo ) != 0 &&
            ( GetMaxLevel( pCh ) < IMPLEMENTOR ||
              !IS_SET( pCh->specials.act, PLR_NOHASSLE ) ) )
        {
          do_action( pMob, "", CMD_SHAKE );
          act( "$c0015[$c0005$N$c0015] ti dice 'Dove credi di andare tu ? "
               "L'ingresso e` riservato!'", FALSE, pCh, 0, pMob, TO_CHAR );
          act( "$c0015[$c0005$N$c0015] dice a $n 'Dove credi di andare tu ? "
               "L'ingresso e` riservato!'", FALSE, pCh, 0, pMob, TO_ROOM );
          return TRUE;
        }
      } /* O la stanza o la direzione non sono quelle controllate. */
    }
    else if( nCmd == CMD_DOORBASH )
    {
      do_action( pMob, GET_NAME( pCh ), CMD_GLARE );
      act( "$c0015[$c0005$N$c0015] ti dice 'Non ci pensare neppure.'",
           FALSE, pCh, 0, pMob, TO_CHAR );
      act( "$c0015[$c0005$N$c0015] dice a $n 'Non ci pensare neppure.'",
           FALSE, pCh, 0, pMob, TO_ROOM );
      return TRUE;
    }
    else if( nCmd == CMD_ASK )
    {
      char szBuffer[ 180 ];
      one_argument( pArg, szBuffer );
      if( isname( szBuffer, GET_NAME( pMob ) ) )
      {        
        do_ask( pCh, pArg, CMD_ASK );
        pArg = one_argument( pArg, szBuffer );
        if( isname( pDatiGilde[ nIndex ].szNomeCapo, GET_NAME( pCh ) ) )
        {
          char chEorA = ( GET_SEX( pCh ) == SEX_FEMALE ? 'a' : 'e' );
          if( pMob->equipment[ HOLD ] && 
              isname2( pArg, pMob->equipment[ HOLD ]->name ) )
          {
            sprintf( szBuffer, "%s Certo signor%c, subito signor%c", 
                     GET_NAME( pCh ), chEorA, chEorA );
            do_tell( pMob, szBuffer, CMD_TELL );
            act( "$n da` $p a $N", TRUE, pMob, pMob->equipment[ HOLD ], pCh,
                 TO_NOTVICT );
            act( "$n ti da` $p", TRUE, pMob, pMob->equipment[ HOLD ], pCh, 
                TO_VICT );
            obj_to_char( unequip_char( pMob, HOLD ), pCh );
          }
          else
          {
            sprintf( szBuffer, "%s Mi dispiace signor%c, ma non ce l'ho", 
                     GET_NAME( pCh ), chEorA );
            do_tell( pMob, szBuffer, CMD_TELL );
          }
        }
        else
        {
          act( "$N continua la sua guardia, ignorando le tue richieste.",
               TRUE, pCh, NULL, pMob, TO_CHAR );
          act( "$N continua la sua guardia, ignorando le richieste di $n.",
               TRUE, pCh, NULL, pMob, TO_ROOM );
        }
        return TRUE;        
      }
    }
  }
  else if( nType == EVENT_DEATH )
  {
    if( pMob->equipment[ HOLD ] )
    {
      struct obj_data *pObj = unequip_char( pMob, HOLD );
      act( "$n, con un ultimo sforzo disperato, distrugge $p.",
           TRUE, pMob, pObj, 0, TO_ROOM );
      extract_obj( pObj );
    }
  }
  else if( nType == EVENT_TICK )
  {
    if( !AWAKE( pMob ) )
    {
      if( !IS_AFFECTED( pMob, AFF_SLEEP ) )
      {
        do_wake( pMob, "", 0 );
        return TRUE;
      }
    }
    else if( GET_POS( pMob ) > POSITION_SLEEPING &&
             GET_POS( pMob ) < POSITION_FIGHTING )
    {
      do_stand( pMob, "", 0 );
      return TRUE;
    }
    else if( pMob->in_room != pDatiGilde[ nIndex ].nGuardiaRoom )
    {
			// Cerca di tornare alla gilda
      int iDir;
      
      iDir = find_path( pMob->in_room, IsGuildGuardRoomFP,
                        (void *)pDatiGilde[ nIndex ].nGuardiaRoom, -5000, 0 );
      if( iDir >= 0 ) 
      { 
        go_direction( pMob, iDir );
        return TRUE;
      }
    }
    else if( !pMob->equipment[HOLD] && pMob->carrying )
    {
      struct obj_data *pObj;
      for( pObj = pMob->carrying; pObj; pObj = pObj->next_content )
      {
        if( CAN_WEAR( pObj, ITEM_HOLD ) )
        {
          perform_wear( pMob, pObj, 13 );
          obj_from_char( pObj );
          equip_char( pMob, pObj, HOLD );
          return TRUE;
        }
      }
    }
    else
    {
      struct char_data *pCapo;
      pCapo = get_char_room_vis( pMob, pDatiGilde[ nIndex ].szNomeCapo );
    
      if( pCapo != NULL )
      {
        if( !number( 0, 10 ) )
        {        
          do_action( pMob, GET_NAME( pCapo ), CMD_KNEEL );
          if( GET_SEX( pCapo ) == SEX_MALE )
          {
            act( "$c0015[$c0005$N$c0015] ti dice 'Ai suoi ordini signore, "
                 "benvenuto.'", FALSE, pCapo, 0, pMob, TO_CHAR );
            act( "$c0015[$c0005$N$c0015] dice a $n 'Ai suoi ordini signore, "
                 "benvenuto.'", FALSE, pCapo, 0, pMob, TO_ROOM );
          }
          else if( GET_SEX( pCapo ) == SEX_FEMALE )
          {
            act( "$c0015[$c0005$N$c0015] ti dice 'Ai suoi ordini signora, "
                 "benvenuta.'", FALSE, pCapo, 0, pMob, TO_CHAR );
            act( "$c0015[$c0005$N$c0015] dice a $n 'Ai suoi ordini signora, "
                 "benvenuta.'", FALSE, pCapo, 0, pMob, TO_ROOM );
          }
          return TRUE;
        }
      }
    }
  }
  
  return FALSE;
}

/****************************************************************************
 * Procedura di servizio per GuildBanker.
 ****************************************************************************/

void GuildDeposit( struct char_data *pCh, struct char_data *pMob, 
                   const char *pArg, int nIndex )
{
  
  char szBuffer[ 256 ];
    
  long lGold = -1;
  
  sscanf( pArg, "%ld", &lGold );
  if( lGold > 0 )
  {
    if( lGold <= GET_GOLD( pCh ) )
    {
      FILE *pfTotal;
      char szFileName[ 256 ];
      
      sprintf( szFileName, "%s/%s%s", GUILD_DIR,
               pDatiGilde[ nIndex ].szBaseFileName, GUILD_GOLD_TOT );
      if( ( pfTotal = fopen( szFileName, "r+" ) ) != NULL )
      {
        long lTotalGold = 0;                
        rewind( pfTotal );
        fscanf( pfTotal, "%ld\n", &lTotalGold );
        rewind( pfTotal );
        lTotalGold += lGold;
        fprintf( pfTotal, "%ld\n", lTotalGold );
        GET_GOLD( pCh ) -= lGold;
        fclose( pfTotal );
              
        act( "$c0013[$c0015$N$c0013] ti dice 'Grazie di aver "
             "contribuito alla prosperita` della gilda'", FALSE, pCh, 
             NULL, pMob, TO_CHAR );
        sprintf( szBuffer, "$n deposita %ld monete nelle casse della gilda.",
                 lGold );
        act( szBuffer, TRUE, pCh, NULL, NULL, TO_ROOM );
                                              
      }
      else
      {
        act( "$c0013[$c0015$N$c0013] ti dice 'Acc., non trovo il registro.", 
             FALSE, pCh, NULL, pMob, TO_CHAR );
        mudlog( LOG_ERROR, "Cannot open file %s in GuildBank", 
                szFileName );
      }
    }
    else
    {
      act( "$c0013[$c0015$N$c0013] ti dice 'Non mi sembra che "
           "tu abbia tutte quelle monete'", FALSE, pCh, NULL, pMob,
           TO_CHAR );
    }
  }
  else
  {
    act( "$c0013[$c0015$N$c0013] ti dice 'Non ho capito "
         "quante monete indende versare. Puo` ripetere ?'", FALSE, pCh,
         NULL, pMob, TO_CHAR );
  }
}

/****************************************************************************
 * Procedura di servizio per GuildBanker.
 ****************************************************************************/

void GuildBalance( struct char_data *pCh, struct char_data *pMob, 
                   int nIndex )
{  
  if( ( isname( pDatiGilde[ nIndex ].szNomeCapo, GET_NAME( pCh ) ) ||
        GetMaxLevel( pCh ) >= IMPLEMENTOR ) )
  {
    FILE *pfTotal;
    char szFileName[ 256 ];
        
    sprintf( szFileName, "%s/%s%s", GUILD_DIR, 
             pDatiGilde[ nIndex ].szBaseFileName, GUILD_GOLD_TOT );
    if( ( pfTotal = fopen( szFileName, "a+" ) ) != NULL )
    {
      char szBuffer[ 256 ];
      long lTotalGold = 0;
          
      rewind( pfTotal );
      fscanf( pfTotal, "%ld\n", &lTotalGold );
      fclose( pfTotal );
      sprintf( szBuffer, "$c0013[$c0015$N$c0013] ti dice '"
                         "Nelle casse della gilda ci sono %ld monete d'oro'",
               lTotalGold );
      act( szBuffer, FALSE, pCh, NULL, pMob, TO_CHAR );
    }
    else
    {
      act( "$c0013[$c0015$N$c0013] ti dice 'Acc., non trovo il registro.", 
           FALSE, pCh, NULL, pMob, TO_CHAR );
      mudlog( LOG_ERROR, "Cannot open file %s in GuildBank", 
              szFileName );
    }
  }
  else
  {
    act( "$c0013[$c0015$N$c0013] ti dice 'Mi dispiace, ma sono "
         "informazioni riservate'", FALSE, pCh, NULL, pMob, TO_CHAR );
  }
}

/****************************************************************************
 * Procedura di servizio per GuildBanker.
 ****************************************************************************/

void GuildWithdraw( struct char_data *pCh, struct char_data *pMob, 
                    const char *pArg, int nIndex )
{
  struct char_data *pGuildMaster;

  if( ( ( pGuildMaster = get_char_room( pDatiGilde[ nIndex ].szNomeCapo, 
                                        pCh->in_room ) ) != NULL && 
        !IS_LINKDEAD( pGuildMaster ) ) )
  {
    long lGold = -1;
    sscanf( pArg, "%ld", &lGold );
    if( lGold > 0 )
    {
      FILE *pfTotal;
      char szFileName[ 256 ];
      
      sprintf( szFileName, "%s/%s%s", GUILD_DIR, 
               pDatiGilde[ nIndex ].szBaseFileName, GUILD_GOLD_TOT );
      if( ( pfTotal = fopen( szFileName, "r+" ) ) != NULL )
      {
        long lTotalGold = 0;
        rewind( pfTotal );
        fscanf( pfTotal, "%ld\n", &lTotalGold );
        if( lGold <= lTotalGold )
        {
          char szBuffer[ 256 ];
          rewind( pfTotal );
          lTotalGold -= lGold;
          fprintf( pfTotal, "%ld\n", lTotalGold );
          GET_GOLD( pCh ) += lGold;
          act( "$c0013[$c0015$N$c0013] ti dice 'Ecco i soldi richiesti'", 
               FALSE, pCh, NULL, pMob, TO_CHAR );
          sprintf( szBuffer, "$n preleva %ld monete dalle casse della gilda.",
                   lGold );
          act( szBuffer, TRUE, pCh, NULL, NULL, TO_ROOM );
                                              
        }
        else
        {
          act( "$c0013[$c0015$N$c0013] ti dice 'La banca non ha "
               "tutte quelle monete'", FALSE, pCh, NULL, pMob, TO_CHAR );
        }
        fclose( pfTotal );
              
      }
      else
      {
        act( "$c0013[$c0015$N$c0013] ti dice 'Acc., non trovo il registro.",
             FALSE, pCh, NULL, pMob, TO_CHAR );
        mudlog( LOG_ERROR, "Cannot open file %s in GuildBank", 
                szFileName );
      }
    }
    else
    {
      act( "$c0013[$c0015$N$c0013] ti dice 'Non ho capito "
           "quante monete indende versare. Puo` ripetere ?'", FALSE, pCh,
           NULL, pMob, TO_CHAR );
    }
  }
  else
  {
    char pMsg[] = "$c0015[$c0005$N$c0015] dice 'Il prelievo puo` essere "
                  "fatto solo in presenza del capo della gilda'";
    act( pMsg, FALSE, pCh, NULL, pMob, TO_CHAR );
    act( pMsg, FALSE, pCh, NULL, pMob, TO_ROOM );
  }
}

/**************************************************************************
 * GuildBank permette di realizzare le banche delle gilde. A differenza
 * delle banche normali, i soldi versati, rimangono nella banca.
 *
 * Possono depositare nelle banche solo gli appartenenti alla gilda iscritti
 * nel libro della gilda.
 *
 * Il bilancio della banca lo puo` leggere solo il capogilda, mentre 
 * possono prelevare dalla banca solo gli appartenenti alla gilda che
 * tengono (HOLD) il simbolo della gilda e solo in presenza del capogilda.
 *
 *************************************************************************/

int GuildBanker( struct char_data *pCh, int nCmd, const char *pArg, 
                 struct char_data *pMob, int nType )
{

  if( !pMob || !pCh )
  {
    mudlog( LOG_SYSERR, "pCh or pMob == NULL in GuildBank" );
    return FALSE;
  }

  if( nType == EVENT_COMMAND && 
      ( nCmd == CMD_DEPOSIT || nCmd == CMD_WITHDRAW || nCmd == CMD_BALANCE ) )
  {
    int nIndex;
    for( nIndex = 0; pDatiGilde[ nIndex ].nBanchiereXP &&
                     pDatiGilde[ nIndex ].nBanchiereXP != 
                     mob_index[ pMob->nr ].iVNum; nIndex++ );
      
    if( pDatiGilde[ nIndex ].nBanchiere )
    {

      if( pDatiGilde[ nIndex ].nBanca && 
          pMob->in_room != pDatiGilde[ nIndex ].nBanca )
        return FALSE;
    
      if( IS_PC( pCh ) )
      {          
        
        if( IsInGuildList( nIndex, GET_NAME( pCh ) ) ||
            strcasecmp( GET_NAME( pCh ), 
                        pDatiGilde[ nIndex ].szNomeCapo ) == 0 ||
            ( GetMaxLevel( pCh ) >= IMPLEMENTOR && 
              IS_SET( pCh->specials.act, PLR_NOHASSLE ) ) )
        {
          switch( nCmd )
          {
           case CMD_DEPOSIT:
            GuildDeposit( pCh, pMob, pArg, nIndex );
            break;
            
           case CMD_BALANCE:
            GuildBalance( pCh, pMob, nIndex );
            break;
            
           case CMD_WITHDRAW:
            GuildWithdraw( pCh, pMob, pArg, nIndex );
            break;
          }
        }
        else
        {
          char pMsg[] = "$c0015[$c0005$N$c0015] dice 'Mi dispiace, ma solo i "
                        "membri della gilda possono usare la banca'";
          act( pMsg, FALSE, pCh, NULL, pMob, TO_CHAR );
          act( pMsg, FALSE, pCh, NULL, pMob, TO_ROOM );
        }
      }
      else
      {
        act( "Solo i giocatori possono usare la banca.", FALSE, pCh,
             NULL, NULL, TO_CHAR );
      }        
          
      return TRUE;
    }
    else
    {        
      mudlog( LOG_ERROR, 
              "GuildXPBanker assigned to wrong mob '%s' (%ld).",
              GET_NAME( pMob ), pMob->nr );
    }
  }
  
  return FALSE;
}

/****************************************************************************
 * Procedura di servizio per GuildXPBanker.
 ****************************************************************************/

int GuildBalanceXP( struct char_data *pChar, struct char_data *pMob, 
                    int nIndex )
{
  if( ( isname( pDatiGilde[ nIndex ].szNomeCapo, GET_NAME( pChar ) ) ||
        GetMaxLevel( pChar ) >= IMPLEMENTOR ) )
  {
    CharElem *pCurr;
    char szBuffer[ 128 ];
    struct string_block sb;
    FILE *pfTotal;
    char szFileName[ 256 ];
      
      
    init_string_block( &sb );
    sprintf( szBuffer, 
             "$c0013[$c0015%.20s$c0013] ti dice 'Ecco l'estratto conto:'"
             "$c0007\n\r\n\r", GET_NAME_DESC( pMob ) );
    append_to_string_block( &sb, szBuffer );
    append_to_string_block( &sb, "Versamenti nelle ultime 24 ore:\n\r\n\r" );

    for( pCurr = (CharElem *)pMob->act_ptr; pCurr; 
         pCurr = pCurr->pNext )
    {
      if( pCurr->nIntData > 0 )
      {          
        sprintf( szBuffer, "%-20.20s %7d\n\r", 
                 GET_NAME( (struct char_data*)pCurr->pWho ), 
                 pCurr->nIntData );
        append_to_string_block( &sb, szBuffer );
      }        
    }
    append_to_string_block( &sb, "                     -------\n\r" );
    sprintf( szBuffer, "Totale               %7d\n\r", 
             SumIntDataPos( (CharElem *)pMob->act_ptr ) );
    append_to_string_block( &sb, szBuffer );
      
    append_to_string_block( &sb, 
                           "\n\rPrelevamenti negli ultimi 3 giorni:\n\r\n\r" );

    for( pCurr = (CharElem *)pMob->act_ptr; pCurr; 
         pCurr = pCurr->pNext )
    {
      if( pCurr->nIntData < 0 )
      {          
        sprintf( szBuffer, "%-20.20s %7d\n\r", 
                 GET_NAME( (struct char_data*)pCurr->pWho ), 
                 -pCurr->nIntData );
        append_to_string_block( &sb, szBuffer );
      }        
    }
    append_to_string_block( &sb, "                     -------\n\r" );
    sprintf( szBuffer, "Totale               %7d\n\r", 
             SumIntDataNeg( (CharElem *)pMob->act_ptr ) );
    append_to_string_block( &sb, szBuffer );
      
    sprintf( szFileName, "%s/%s%s", GUILD_DIR, 
             pDatiGilde[ nIndex ].szBaseFileName, GUILD_EXP_TOT );
    if( ( pfTotal = fopen( szFileName, "a+" ) ) != NULL )
    {
      long lTotalXP = 0;
      
      rewind( pfTotal );
      fscanf( pfTotal, "%ld\n", &lTotalXP );
      fclose( pfTotal );
      sprintf( szBuffer, 
               "\n\rLa gilda ha %ld punti esperienza a disposizione.\n\r",
               lTotalXP );
      append_to_string_block( &sb, szBuffer );
        
    }
    else
    {
      mudlog( LOG_ERROR, "Cannot open file %s in GuildListXP (gilde.c)",
              szFileName );
      act( "$c0013[$c0015$N$c0013] ti dice 'Acc., non trovo il registro.",
           FALSE, pChar, NULL, pMob, TO_CHAR );
    }
      
    page_string_block( &sb, pChar );
    destroy_string_block( &sb );
  }
  else
  {
    act( "$c0013[$c0015$N$c0013] ti dice 'Mi dispiace, "
         "ma sono informazioni riservate'", FALSE, pChar, NULL, 
         pMob, TO_CHAR );
  }
  return TRUE;
}

/****************************************************************************
 * Procedura di servizio per GuildXPBanker.
 ****************************************************************************/

int GuildDepositXP( struct char_data *pChar,  const char *szArg, 
                    struct char_data *pMob, int nIndex )
{
  if( !IsInList( (CharElem *)pMob->act_ptr, pChar ) )
  {
    long lXP = -1;
    sscanf( szArg, "%ld", &lXP );
    if( lXP > 0 )
    {
      int i, nMaxXPLevNeed = 0;
      for( i = MAGE_LEVEL_IND; i < MAX_CLASS; i++ )
        nMaxXPLevNeed = MAX( nMaxXPLevNeed, 
                             titles[ i ][ (int)GET_LEVEL( pChar, i ) ].exp );
      if( GET_EXP( pChar ) > ( nMaxXPLevNeed + 
                               ( lXP / HowManyClasses(pChar ) ) ) &&
          ( lXP + SumIntDataPos( (CharElem *)pMob->act_ptr ) ) <= 
          GUILD_MAXXP_DEP )
      {
        FILE *pfTotal;
        char szFileName[ 256 ];
  
        sprintf( szFileName, "%s/%s%s", GUILD_DIR, 
                 pDatiGilde[ nIndex ].szBaseFileName, GUILD_EXP_TOT );
        if( ( pfTotal = fopen( szFileName, "r+" ) ) != NULL )
        {
          char szBuffer[ 160 ];
          long lTotalXP = 0;
          
          rewind( pfTotal );
          fscanf( pfTotal, "%ld\n", &lTotalXP );
          rewind( pfTotal );
          lTotalXP += lXP;
          fprintf( pfTotal, "%ld\n", lTotalXP );
          GET_EXP( pChar ) -= lXP / HowManyClasses( pChar );
          fclose( pfTotal );
          InsertInListInt( (CharElem **)&pMob->act_ptr, pChar, 
                           ( SECS_PER_MUD_HOUR * 4 * 24 ) / PULSE_MOBILE,
                           (int)lXP );
              
          act( "$c0013[$c0015$N$c0013] ti dice 'Grazie di aver "
               "contribuito alla prosperita` della gilda'", FALSE, 
               pChar, NULL, pMob, TO_CHAR );
          sprintf( szBuffer, "$n deposita %ld XP nelle casse della gilda.",
                   lXP );
          act( szBuffer, TRUE, pChar, NULL, NULL, TO_ROOM );
        }
        else
        {
          mudlog( LOG_ERROR, "Cannot open file %s in GuildDepositXP (gilde.c)",
                  szFileName );
          act( "$c0013[$c0015$N$c0013] ti dice 'Acc., non trovo il registro.",
               FALSE, pChar, NULL, pMob, TO_CHAR );
          return FALSE;
        }
      }
      else
      {
        if( SumIntDataPos( (CharElem *)pMob->act_ptr ) >= GUILD_MAXXP_DEP )
        {
          act( "$c0013[$c0015$N$c0013] ti dice 'Mi dispiace, "
               "ma non si possono versare altri XP per oggi.'", FALSE, pChar, 
               NULL, pMob, TO_CHAR );
        }
        else if( ( GET_EXP( pChar ) - nMaxXPLevNeed ) > 0 )
        {
          char szBuffer[ 160 ];
          sprintf( szBuffer, 
                   "$c0013[$c0015$N$c0013] ti dice 'Mi dispiace, "
                   "ma puoi versare al massimo %d XP.'",
                   MIN( ( GET_EXP( pChar ) - nMaxXPLevNeed ) * 
                        HowManyClasses( pChar ),
                        GUILD_MAXXP_DEP - 
                        SumIntDataPos( (CharElem *)pMob->act_ptr ) ) );
          act( szBuffer, FALSE, pChar, NULL, pMob, TO_CHAR );
        }
        else
          act( "$c0013[$c0015$N$c0013] ti dice 'Mi dispiace, "
               "ma non hai XP da versare.'", FALSE, pChar, NULL, 
               pMob, TO_CHAR );
      }
    }
    else
    {
      act( "$c0013[$c0015$N$c0013] ti dice 'Non ho capito "
           "quanti XP indendi versare. Puoi ripetere ?'", FALSE, pChar,
           NULL, pMob, TO_CHAR );
    }
  }
  else
    act( "$c0013[$c0015$N$c0013] ti dice 'Mi dispiace, "
         "ma puoi effettuare un solo versamento al giorno.'", FALSE, 
         pChar, NULL, pMob, TO_CHAR );
  return TRUE;
}

/****************************************************************************
 * Procedura di servizio per GuildXPBanker.
 ****************************************************************************/

int GuildWithdrawXP( struct char_data *pChar,  const char *szArg,
                     struct char_data *pMob, int nIndex )
{
  long lXP = -1;
  sscanf( szArg, "%ld", &lXP );
  if( lXP > 0 )
  {
    if( get_char_room( pDatiGilde[ nIndex ].szNomeCapo, pChar->in_room ) !=
        NULL )
    {      
      FILE *pfTotal;
      char szFileName[ 256 ];
        
      sprintf( szFileName, "%s/%s%s", GUILD_DIR, 
               pDatiGilde[ nIndex ].szBaseFileName, GUILD_EXP_TOT );
      if( ( pfTotal = fopen( szFileName, "r+" ) ) != NULL )
      {
        long lTotalXP = 0;
          
        rewind( pfTotal );
        fscanf( pfTotal, "%ld\n", &lTotalXP );
        
        if( ( lXP + SumIntDataNeg( (CharElem *)pMob->act_ptr ) ) <= 
            GUILD_MAXXP_PRE && lXP <= lTotalXP )
        {        
          long lOldXP = GET_EXP( pChar );
      
          gain_exp( pChar, lXP );
          lXP = ( GET_EXP( pChar ) - lOldXP ) * HowManyClasses( pChar );
          if( lXP > 0 )
          {          
            char szBuffer[ 256 ];
            
            rewind( pfTotal );
            lTotalXP -= lXP;
            fprintf( pfTotal, "%ld\n", lTotalXP );
            sprintf( szBuffer, 
                     "$c0013[$c0015$N$c0013] ti dice 'Hai prelevato %ld "
                     "punti esperienza.'", lXP ); 
            act( szBuffer, FALSE, pChar, NULL, pMob, TO_CHAR );
            sprintf( szBuffer, 
                     "$n preleva %ld punti esperienza dalle casse della "
                     "gilda.", lXP );
            act( szBuffer, TRUE, pChar, NULL, NULL, TO_ROOM );
            InsertInListInt( (CharElem **)&pMob->act_ptr, pChar, 
                             ( SECS_PER_MUD_HOUR * 4 * 24 * 3 ) / PULSE_MOBILE,
                             (int)( -lXP ) );
          }
          else
          {
            act( "$c0013[$c0015$N$c0013] ti dice 'Mi dispiace, ma non posso "
                 "darti punti esperienza.'", FALSE, pChar, NULL, pMob, 
                 TO_CHAR );
          }        
          fclose( pfTotal );
        }
        else
        {
          long lMaxToPrel = MIN( GUILD_MAXXP_PRE - 
                                 SumIntDataNeg( (CharElem *)pMob->act_ptr ),
                                 lTotalXP );
          if( lMaxToPrel <= 0 )
          {
            act( "$c0013[$c0015$N$c0013] ti dice 'Mi dispiace, ma non si "
                 "possono prelevare altri XP per il momento.'", FALSE, pChar, 
                 NULL, pMob, TO_CHAR );
          }
          else
          {
            char szBuffer[ 256 ];
            sprintf( szBuffer, 
                     "$c0013[$c0015$N$c0013] ti dice 'Mi dispiace, ma puoi "
                     "prelevare al massimo %ld punti per il momento.'",
                     lMaxToPrel );
            act( szBuffer, FALSE, pChar, NULL, pMob, TO_CHAR );
          }
        }
      }      
      else
      {
        mudlog( LOG_ERROR, 
                "Cannot open file %s in GuildWithdrawXP (gilde.c).",
                szFileName );
        act( "$c0013[$c0015$N$c0013] ti dice 'Acc., non trovo il registro.",
             FALSE, pChar, NULL, pMob, TO_CHAR );
      }
    }
    else
    {
      act( "$c0013[$c0015$N$c0013] ti dice 'E` possibile prelevare solo in "
           "presenza del capogilda.'", FALSE, pChar, NULL, pMob, TO_CHAR );
    }    
  }
  else
  {
    act( "$c0013[$c0015$N$c0013] ti dice 'Non ho capito "
         "quanti XP indendi prelevare. Puoi ripetere ?'", FALSE, pChar,
         NULL, pMob, TO_CHAR );
  }
  return TRUE;      
}


/****************************************************************************
 * GuildXPBanker va assegnata al banchiere della banca di XP delle gilde dei 
 * giocatori.
 *
 * I comandi ammessi sono:
 * 
 * deposit puo` essere dato da chiunque sia iscritto nel libro della gilda.
 *         E` possibile depositare al massimo 100.000 XP al giorno e solo la
 *         quantita` di XP che supera quella necessaria per il livello a cui
 *         e` il giocatore.
 * withdraw puo` essere dato da chiunque sia iscritto nel libro della gilda
 *          e solo inpresenza del capogilda.
 *          E` possibile prelevare un massimo di 5.000.000 ogni 3 giorni ed
 *          al massimo il quantitativo di XP necessario a due livelli superiori
 *          all'attuale.
 * balance puo` essere dato solo dal capo della gilda. Permette di vedere la
 *         quantita` di XP presenti nella banca ed una lista
 *         dei giocatori che hanno versato XP nelle ultime ventiquattrore o
 *         prelevato negli ultimi 3 giorni.
 ****************************************************************************/

int GuildXPBanker( struct char_data *pChar, int nCmd, const char *szArg,
                   struct char_data *pMob, int nType )
{
  if( pMob == NULL )
  {
    mudlog( LOG_SYSERR, "pMob == NULL in GuildMemberBook (gilde.c)" );
    return TRUE;
  }
  else if( pMob->nr >= 0 )
  {    
    if( nType == EVENT_COMMAND && 
        ( nCmd == CMD_DEPOSIT || nCmd == CMD_WITHDRAW || 
          nCmd == CMD_BALANCE ) )
    {
      int i;
      for( i = 0; pDatiGilde[ i ].nBanchiereXP &&
                  pDatiGilde[ i ].nBanchiereXP != 
                  mob_index[ pMob->nr ].iVNum; i++ );
      
      if( pDatiGilde[ i ].nBanchiereXP )
      {

        if( pDatiGilde[ i ].nBancaXP && 
            pMob->in_room != pDatiGilde[ i ].nBancaXP )
          return FALSE;
        
        if( IS_PC( pChar ) )
        {          
          
          if( IsInGuildList( i, GET_NAME( pChar ) ) || 
              strcasecmp( GET_NAME( pChar ), 
                          pDatiGilde[ i ].szNomeCapo ) == 0 ||
              ( GetMaxLevel( pChar ) >= IMPLEMENTOR && 
                IS_SET( pChar->specials.act, PLR_NOHASSLE ) ) )
          {
            switch( nCmd )
            {
             case CMD_BALANCE:
              GuildBalanceXP( pChar, pMob, i );
              break;
             case CMD_DEPOSIT:
              GuildDepositXP( pChar,  szArg, pMob, i );
              break;
             case CMD_WITHDRAW:
              GuildWithdrawXP( pChar,  szArg, pMob, i );
              break;
            }
          }
          else
          {
            
            act( "$c0013[$c0015$N$c0013] dice 'Mi dispiace, ma solo i "
                 "membri della gilda possono usare la banca'", FALSE, pChar, 
                 NULL, pMob, TO_CHAR );
          }
        }
        else
        {
          act( "Solo i giocatori possono usare la banca.", FALSE, pChar,
               NULL, NULL, TO_CHAR );
        }        
          
        return TRUE;
      }
      else
      {        
        mudlog( LOG_ERROR, 
                "GuildXPBanker assigned to wrong mob '%s' (%ld).",
                GET_NAME( pMob ), pMob->nr );
      }
      
    }
    else if( nType == EVENT_TICK )
    {
      UpdateList( (CharElem **)&pMob->act_ptr );
    }
    else if( nType == EVENT_DEATH )
    {
      FreeList( (CharElem **)&pMob->act_ptr );
    }
  }  
  
  return FALSE;
}

/****************************************************************************
 * Questa funzione viene attaccata al libro dei membri della gilda. Il capo
 * gilda puo` inserire nuovi membri (che devono essere presenti) con 
 * write <nomemembro> <nomelibro>, cancellarli con 
 * remove <nomemembro> <nomelibro> e visualizzarne la lista con 
 * read <nomelibro> (quest'ultimo comando non e` limitato al solo capogilda).
 * *************************************************************************/

int GuildMemberBook( struct char_data *pChar, int nCmd, const char *szArg,
                     struct obj_data *pObj, int nType )
{
  if( pObj == NULL )
  {
    mudlog( LOG_SYSERR, "pObj == NULL in GuildMemberBook (gilde.c)" );
    return TRUE;
  }
  else if( pObj->item_number >= 0 )
  {    
    if( nType == EVENT_COMMAND && ( nCmd == CMD_READ || nCmd == CMD_WRITE ||
                                    nCmd == CMD_REMOVE ) )
    {
      int i;
      for( i = 0; pDatiGilde[ i ].nLibroSoci && 
                  pDatiGilde[ i ].nLibroSoci != 
                  obj_index[ pObj->item_number ].iVNum; i++ );
      
      if( pDatiGilde[ i ].nLibroSoci )
      {            
        if( nCmd == CMD_READ )
        {
          char szArgument[ 256 ];
          one_argument( szArg, szArgument );
          if( get_obj_vis_accessible( pChar, szArgument ) == pObj )
          {
            char szBuffer[ 256 ];
            TSocioGilda *pSocio;
            act( "Questo e` $p", FALSE, pChar, pObj, NULL, TO_CHAR );
            sprintf( szBuffer, "Il capo della gilda e` %c%s.", 
                     UPPER( pDatiGilde[ i ].szNomeCapo[ 0 ] ),
                     pDatiGilde[ i ].szNomeCapo + 1 );
            act( szBuffer, FALSE, pChar, NULL, NULL, TO_CHAR );
            act( "I membri della gilda sono:", FALSE, pChar, NULL,
                 NULL, TO_CHAR );
            act( "$n esamina $p", TRUE, pChar, pObj, NULL, TO_ROOM );
            
            for( pSocio = pDatiGilde[ i ].pListaSoci; pSocio; 
                 pSocio = pSocio->pNext )
            {
              act( pSocio->szNomeSocio, FALSE, pChar, NULL, NULL,  TO_CHAR );
            }
            return TRUE;
          }          
        }
        else if( nCmd == CMD_WRITE )
        {
          char szNomeMembro[ 256 ];
          char szNomeLibro[ 256 ];
          struct char_data *pMembro;
            
          argument_interpreter( szArg, szNomeMembro, szNomeLibro );
          
          if( get_obj_vis( pChar, szNomeLibro ) == pObj )
          {
            if( strcasecmp( GET_NAME( pChar ), 
                            pDatiGilde[ i ].szNomeCapo ) == 0 )
            {                        
              if( ( pMembro = get_char_room_vis( pChar, szNomeMembro ) ) != 
                  NULL )
              {
                if( pMembro != pChar )
                {
                  if( IS_PC( pMembro ) )
                  {
                    if( !IsInGuildList( i, GET_NAME( pMembro ) ) )
                    {                    
                      AppendToGuildList( i, GET_NAME( pMembro ) );
                      act( "$n scrive il nome di $N su $p.", TRUE, pChar, 
                           pObj, pMembro, TO_NOTVICT );
                      act( "$n scrive il tuo nome su $p.", TRUE, pChar, pObj, 
                           pMembro, TO_VICT );
                      act( "Scrivi il nome di $N su $p.", TRUE, pChar, pObj,
                           pMembro, TO_CHAR );
                    }
                    else
                    {
                      act( "$N e` gia` membro della gilda.", TRUE, pChar, NULL,
                           pMembro, TO_CHAR );
                    }                  
                  }
                  else
                  {
                    act( "Puoi iscrivere solo giocatori nella tua gilda.",
                         TRUE, pChar, NULL, NULL, TO_CHAR );
                  }
                }
                else
                {
                  act( "Ma tu sei gia` il capo.", TRUE, pChar, NULL, NULL, 
                       TO_CHAR );
                }              
              }
              else
              {
                act( "Il nuovo membro deve essere presente durante "
                     "l'iscrizione.", FALSE, pChar, NULL, NULL, TO_CHAR );
              }
            }
            else
            {
              act( "Mi dispiace, ma solo il capo della gilda puo` aggiungere "
                   "nuovi membri.", FALSE, pChar, NULL, NULL, TO_CHAR );
            }
            return TRUE;
          }                      
        }
        else if( nCmd == CMD_REMOVE )
        {
          char szNomeMembro[ 256 ];
          char szNomeLibro[ 256 ];
            
          argument_interpreter( szArg, szNomeMembro, szNomeLibro );
          
          if( get_obj_vis( pChar, szNomeLibro ) == pObj )
          {
            if( strcasecmp( GET_NAME( pChar ),
                            pDatiGilde[ i ].szNomeCapo ) == 0 )
            {                        
              if( IsInGuildList( i, szNomeMembro ) )            
              {
                if( strcasecmp( szNomeMembro, GET_NAME( pChar ) ) != 0 )
                {                
                  RemoveFromGuildList( i, szNomeMembro );
                  act( "$n cancella un nome da $p", TRUE, pChar, pObj, NULL,
                       TO_ROOM );
                  act( "Fatto.", FALSE, pChar, NULL, NULL, TO_CHAR );
                }
                else
                {
                  act( "Non puoi cancellarti dalla lista. Tu sei il capo!", 
                       FALSE, pChar, NULL, NULL, TO_CHAR );
                }              
              }
              else
              {
                act( "Quel nome non e` presente nella lista.", FALSE, pChar,
                     NULL, NULL, TO_CHAR );
              }            
            }
            else
            {
              act( "Mi dispiace, ma solo il capo della gilda puo` cancellare "
                   "i membri.", FALSE, pChar, NULL, NULL, TO_CHAR );
            }            
            return TRUE;            
          }    
        }        
      }
      else
      {
        mudlog( LOG_SYSERR, "GuildMemberBook assigned to obj not in "
                            "pDatiGilde (gilde.c)" );
      }      
    }
  }
  
  return FALSE;
}


/****************************************************************************
 * Queste tre funzioni (assignMob, AssignRoom ed AssignObj) servono per
 * assegnare delle procedure speciali a, rispettivamente, un mobile, una
 * stanza ed un oggetto.
 * *************************************************************************/

void AssignMob( int vnum, int (*proc)( struct char_data *, int, const char *, 
                                       struct char_data *, int ) )
{
  
  if( vnum > 0 )
  {  
    int rnum;
    if( ( rnum = real_mobile( vnum ) ) < 0 )
    {
      mudlog( LOG_ERROR, 
              "Mobile %d not found in database. AssignMob (gilde.c)",
              vnum );
    }
    else
    {            
      mob_index[ rnum ].func = CASTVF proc;
    }
  }
}
        
void AssignRoom( int vnum, int (*proc)( struct char_data *, int, const char *,
                                        struct room_data *, int ) )
{
  
  if( vnum > 0 )
  {  
    struct room_data *pRoom;
    if( ( pRoom = real_roomp( vnum ) ) == NULL )
    {
      mudlog( LOG_ERROR, 
              "Room %d not found in database. AssignRoom (gilde.c)",
              vnum );
    }
    else
    {            
      pRoom->funct = proc;
    }
  }
}

void AssignObj( int vnum, int (*proc)( struct char_data *, int, const char *,
                                       struct obj_data *, int ) )
{
  
  if( vnum > 0 )
  {  
    int rnum;
    if( ( rnum = real_object( vnum ) ) < 0 )
    {
      mudlog( LOG_ERROR, 
              "Object %d not found in database. AssignObj (gilde.c)",
              vnum );
    }
    else
    {            
      obj_index[ rnum ].func = CASTVF proc;
    }
  }
}

/***************************************************************************
 * BootGilde si occupa di leggere il file GUILD_DAT ed assegnare le 
 * procedure speciali ai mob ed agli oggetti delle gilde.
 * I mob sono la guardia della gilda, il banchiere ed il banchiere degli XP.
 * Poi c'e' il libro dei soci nel quale e possibile inserire i nomi dei
 * soci della gilda.
 **************************************************************************/

void BootGuilds()
{
  FILE *pfDatiGilde;
  char szDummy[ 160 ];
  
  if( ( pfDatiGilde = fopen( GUILD_DAT, "r+" ) ) != NULL )
  {
    int nMax = 0;
    while( !feof( pfDatiGilde ) )
    {
      fgets( szDummy, sizeof( szDummy ), pfDatiGilde );
      nMax++;
    }
    
    rewind( pfDatiGilde );
    
    if( ( pDatiGilde = (TDatiGilda *)calloc( nMax + 1, 
                                             sizeof( TDatiGilda ) ) ) != NULL )
    {
      int i;
      
      for( i = 0; i < nMax && !feof( pfDatiGilde ); i++ )
      {
        fscanf( pfDatiGilde, "%80s %d %ld %d %d %d %d %d %d\n",
                 pDatiGilde[ i ].szBaseFileName, &pDatiGilde[ i ].nGuardia,
                &pDatiGilde[ i ].nGuardiaRoom,   &pDatiGilde[ i ].nDir,
                &pDatiGilde[ i ].nBanchiere,     &pDatiGilde[ i ].nBanca,
                &pDatiGilde[ i ].nBanchiereXP,   &pDatiGilde[ i ].nBancaXP,
                &pDatiGilde[ i ].nLibroSoci );
        
        AssignMob( pDatiGilde[ i ].nGuardia, PlayersGuildGuard );
        AssignMob( pDatiGilde[ i ].nBanchiere, GuildBanker );
        AssignMob( pDatiGilde[ i ].nBanchiereXP, GuildXPBanker );
        AssignObj( pDatiGilde[ i ].nLibroSoci, GuildMemberBook );
        InitializeMemberList( i );
      }
    }
    else
    {
      mudlog( LOG_ERROR, 
              "Cannot alloc memory for pDatiGilde in BootGilde (gilde.c)" );
    }            
        
    fclose( pfDatiGilde );
  }
  else
  {
    mudlog( LOG_ERROR, "File delle gilde %s non trovato. (gilde.c)", GUILD_DAT );
  }
}

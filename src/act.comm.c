/*
 * BenemMUD v1.0   Released 12/1994
 * See license.doc for distribution terms.   BanemMUD is based on DIKUMUD
 * Tradotto in Italiano da Emanuele Benedetti
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "protos.h"


void RecurseRoom( long lInRoom, int iLevel, int iMaxLevel, 
             unsigned char *achVisitedRooms );

/* extern variables */

extern struct room_data *world;
extern struct descriptor_data *descriptor_list;


void do_say( struct char_data *ch, const char *argument, int cmd )
{
  int i;
  char buf[MAX_INPUT_LENGTH+40];
  
  if( apply_soundproof( ch ) )
    return;
  
  for( i = 0; *(argument + i) == ' '; i++ );
  
  if (!*(argument + i))
    send_to_char("Si, ma COSA vuoi dire ?\n\r", ch);
  else
  {
    sprintf(buf,"$c0015[$c0005$n$c0015] dice '%s'", argument + i);
    act(buf,FALSE,ch,0,0,TO_ROOM);
    if (IS_NPC(ch)||(IS_SET(ch->specials.act, PLR_ECHO)))
    {
      sprintf(buf,"Tu dici '%s'\n\r", argument + i);
      send_to_char(buf, ch);
    }
  }
}


void do_report(struct char_data *ch, const char *argument, int cmd)
{
  char buf[100];


  if (apply_soundproof(ch))
    return;

  if (IS_NPC(ch))
    return;  
   
  if( GET_HIT(ch) > GET_MAX_HIT(ch) ||                /* bug fix */
      GET_MANA(ch) > GET_MAX_MANA(ch) ||
      GET_MOVE(ch) > GET_MAX_MOVE(ch)) 
  {
    send_to_char("Mi spiace, ma non puoi farlo ora.\n\r",ch);
    return;
  }

  sprintf( buf,"$c0014[$c0015$n$c0014] dichiara "
           "'HP:%2.0f%% MANA:%2.0f%% MV:%2.0f%%'",
           ((float)GET_HIT(ch) / (int)GET_MAX_HIT(ch)) * 100.0 + 0.5,
           ((float)GET_MANA(ch) / (int)GET_MAX_MANA(ch)) * 100.0 + 0.5,
           ((float)GET_MOVE(ch) / (int)GET_MAX_MOVE(ch)) * 100.0 + 0.5);
  act(buf,FALSE,ch,0,0,TO_ROOM);
  sprintf( buf,"$c0014Dichiari 'HP:%2.0f%% MANA:%2.0f%% MV:%2.0f%%'", 
           ((float)GET_HIT(ch)  / (int)GET_MAX_HIT(ch))  * 100.0 + 0.5,
           ((float)GET_MANA(ch) / (int)GET_MAX_MANA(ch)) * 100.0 + 0.5,
           ((float)GET_MOVE(ch) / (int)GET_MAX_MOVE(ch)) * 100.0 + 0.5);

  act(buf,FALSE, ch,0,0,TO_CHAR);
 
}



void do_shout(struct char_data *ch, const char *argument, int cmd)
{
  char buf1[MAX_INPUT_LENGTH+40];
  struct descriptor_data *i;
  extern int Silence;
  
  if (!IS_NPC(ch) && IS_SET(ch->specials.act, PLR_NOSHOUT)) 
  {
    send_to_char("Non puoi urlare!!\n\r", ch);
    return;
  }

  if( IS_NPC(ch) && 
      (Silence == 1) &&
      (IS_SET(ch->specials.act, ACT_POLYSELF))) 
  {
    send_to_char("Polymorphed shouting has been banned.\n\r", ch);
    send_to_char("It may return after a bit.\n\r", ch);
    return;
  }

  if (apply_soundproof(ch))
    return;
  
  for (; *argument == ' '; argument++);
  
  if( ch->master && IS_AFFECTED(ch, AFF_CHARM))
  {
    if( !IS_IMMORTAL(ch->master))
    {
      send_to_char("Non credo proprio :-)", ch->master);
      return;
    }
  }
  
  if( ( GET_MOVE(ch)<10 || GET_MANA(ch)<10 ) && GetMaxLevel(ch) < LOW_IMMORTAL )
  {
    send_to_char("Non hai abbastanza forza per gridare !\n\r",ch);
    return;
  }

  if (!(*argument))
    send_to_char("Vuoi urlare ? Ottimo ! Ma COSA ??\n\r", ch);
  else
  {
    if (IS_NPC(ch) || IS_SET(ch->specials.act, PLR_ECHO))
    {
      sprintf(buf1,"$c0009Tu gridi '%s'", argument);
      act(buf1,FALSE, ch,0,0,TO_CHAR);
    }
    sprintf(buf1, "$c0009[$c0015$n$c0009] grida '%s'", argument);

    act( "$c0009[$c0015$n$c0009] alza la testa e grida forte", FALSE, ch, 0, 0, 
         TO_ROOM);
    
    if (GetMaxLevel(ch)<LOW_IMMORTAL)
    {
      GET_MOVE(ch) -= 10;
      GET_MANA(ch) -= 10;
    }
    
    for (i = descriptor_list; i; i = i->next)
    {
      if( i->character != ch && !i->connected &&
          (IS_NPC(i->character) ||
          (!IS_SET(i->character->specials.act, PLR_NOSHOUT) &&
          !IS_SET(i->character->specials.act, PLR_DEAF))) &&
          !check_soundproof(i->character))
      {
        act(buf1, 0, ch, 0, i->character, TO_VICT);
      }
    }
  }
}


void do_gossip(struct char_data *ch, const char *argument, int cmd)
{
  extern int Silence;
#if 1
  char buf1[MAX_INPUT_LENGTH+40];
  struct descriptor_data *i;

  int IsRoomDistanceInRange( int nFirstRoom, int nSecondRoom, int nRange );
  
  if( !IS_NPC( ch ) && IS_SET( ch->specials.act, PLR_NOSHOUT ) )
  {
    send_to_char("Non puoi gridare, parlare od annunciare.\n\r", ch);
    return;
  }

  if( IS_NPC(ch) && 
      (Silence == 1) &&
      (IS_SET(ch->specials.act, ACT_POLYSELF))) 
  {
    send_to_char("Polymorphed gossiping has been banned.\n\r", ch);
    send_to_char("It may return after a bit.\n\r", ch);
    return;
  }

  if( apply_soundproof( ch ) )
    return;
  
  for( ; *argument == ' '; argument++ );
  
  if( ch->master && IS_AFFECTED( ch, AFF_CHARM ) )
  {
    if( !IS_IMMORTAL( ch->master ) )
    {
      send_to_char( "Non credo proprio :-)", ch->master );
      return;
    }
  }
  
  if( !( *argument ) )
    send_to_char( "Parlare ? Ma di COSA !\n\r", ch );
  else
  {
    if( IS_NPC( ch ) || IS_SET( ch->specials.act, PLR_ECHO ) )
    {
      sprintf( buf1,"$c0011Tu dici '%s'", argument );
      act( buf1, FALSE, ch, 0, 0, TO_CHAR );
    }
    sprintf(buf1, "$c0011[$c0015$n$c0011] vi dice '%s'", argument);
    for( i = descriptor_list; i; i = i->next )
    {
      if( i->character != ch && !i->connected &&
          ( IS_NPC( i->character ) ||
            !IS_SET( i->character->specials.act, PLR_NOGOSSIP ) ) &&
          !check_soundproof( i->character ) )
      {

        if( i->character->in_room != NOWHERE )
        {
#if 1
          if( real_roomp( ch->in_room )->zone == 
              real_roomp( i->character->in_room )->zone ||
              GetMaxLevel( i->character ) >= LOW_IMMORTAL ||
              GetMaxLevel( ch ) >= LOW_IMMORTAL )
#else
          if( GetMaxLevel( i->character ) >= LOW_IMMORTAL ||
              GetMaxLevel( ch ) >= LOW_IMMORTAL ||
              IsRoomDistanceInRange( ch->in_room, i->character->in_room, 10 ) )
#endif
          {
            act(buf1, 0, ch, 0, i->character, TO_VICT);
          }
        }
      }
    } /* end for */
  }
#else
  if( !IS_NPC( ch ) && IS_SET( ch->specials.act, PLR_NOSHOUT ) )
  {
    send_to_char("Non puoi gridare, parlare od annunciare.\n\r", ch);
    return;
  }
  
  if( IS_NPC(ch) && 
      Silence == 1 &&
      IS_SET( ch->specials.act, ACT_POLYSELF ) )
  {
    send_to_char("Polymorphed gossiping has been banned.\n\r", ch);
    send_to_char("It may return after a bit.\n\r", ch);
    return;
  }

  if( apply_soundproof( ch ) )
    return;
  
  for( ; *argument == ' '; argument++ );
  
  if( ch->master && IS_AFFECTED( ch, AFF_CHARM ) )
  {
    if( !IS_IMMORTAL( ch->master ) )
    {
      send_to_char( "Non credo proprio :-)", ch->master );
      return;
    }
  }
  
  if( !( *argument ) )
    send_to_char( "Parlare ? Ma di COSA !\n\r", ch );
  else
  {
    char szBuffer[ MAX_INPUT_LENGTH + 40 ];
    if( IS_NPC( ch ) || IS_SET( ch->specials.act, PLR_ECHO ) )
    {
      sprintf( szBuffer,"$c0011Tu dici '%s'", argument );
      act( szBuffer, FALSE, ch, 0, 0, TO_CHAR );
    }
    sprintf( szBuffer, "$c0011[$c0015$n$c0011] vi dice '%s'", argument );
    
    unsigned char *achVisitedRooms = new unsigned char[ WORLD_SIZE ];
    if( achVisitedRooms )
    {
      memset( achVisitedRooms, 0, WORLD_SIZE * sizeof( unsigned char ) );
      RecurseRoom( ch->in_room, 0, 10, achVisitedRooms );
      
      for( struct descriptor_data *pDesc = descriptor_list; pDesc; 
           pDesc = pDesc->next )
      {
        if( pDesc->character != ch && pDesc->connected == CON_PLYNG &&
          ( IS_NPC( pDesc->character ) ||
            !IS_SET( pDesc->character->specials.act, PLR_NOGOSSIP ) ) &&
            !check_soundproof( pDesc->character ) )
        {
          if( pDesc->character->in_room > NOWHERE &&
              ( achVisitedRooms[ pDesc->character->in_room ] ||
                GetMaxLevel( pDesc->character ) >= LOW_IMMORTAL ||
                GetMaxLevel( ch ) >= LOW_IMMORTAL ) )
          {
            act( szBuffer, 0, ch, 0, pDesc->character, TO_VICT);
          }
        }
      }
      delete achVisitedRooms;
    }
  }
#endif
}


void do_auction(struct char_data *ch, const char *argument, int cmd)
{
  char buf1[MAX_INPUT_LENGTH+40];
  struct descriptor_data *i;
  extern int Silence;
  
  if (!IS_NPC(ch) && IS_SET(ch->specials.act, PLR_NOSHOUT))
  {
    send_to_char("Non puoi gridare, parlare od annunciare.\n\r", ch);
    return;
  }
  if( IS_NPC(ch) && 
      (Silence == 1) &&
      (IS_SET(ch->specials.act, ACT_POLYSELF))) 
  {
    send_to_char("Polymorphed auctioning has been banned.\n\r", ch);
    send_to_char("It may return after a bit.\n\r", ch);
    return;
  }
  

  if (apply_soundproof(ch))
    return;
  
  for (; *argument == ' '; argument++);
  
  if (ch->master && IS_AFFECTED(ch, AFF_CHARM))
  {
    if (!IS_IMMORTAL(ch->master))
    {
      send_to_char("Non credo proprio :-)", ch->master);
      return;
    }
  }
  
  if (!(*argument))
    send_to_char("Annunciare ? D'accordo, ma che cosa ?\n\r", ch);
  else
  {
    if (IS_NPC(ch) || IS_SET(ch->specials.act, PLR_ECHO))
    {
      sprintf(buf1,"$c0010Tu annunci '%s'", argument);
      act(buf1,FALSE, ch,0,0,TO_CHAR);
    }
    sprintf(buf1, "$c0010[$c0015$n$c0010] annuncia '%s'", argument);

    for (i = descriptor_list; i; i = i->next)
    {
      if( i->character != ch && !i->connected &&
          (IS_NPC(i->character) ||
          (!IS_SET(i->character->specials.act, PLR_NOSHOUT) &&
          !IS_SET(i->character->specials.act, PLR_NOAUCTION))) &&
          !check_soundproof(i->character))
      {
      #if ZONE_COMM_ONLY
                        /* gossip in zone only */
        if (i->character->in_room != NOWHERE)
        {
          if( real_roomp(ch->in_room)->zone == 
              real_roomp(i->character->in_room)->zone
              ||GetMaxLevel(i->character) >=LOW_IMMORTAL
              ||GetMaxLevel(ch) >= LOW_IMMORTAL )
          {
            act(buf1, 0, ch, 0, i->character, TO_VICT);
          }
        }
      #else
        act(buf1, 0, ch, 0, i->character, TO_VICT);
      #endif          
      }
    }
  }
}






void do_commune(struct char_data *ch, const char *argument, int cmd)
{
  static char buf1[MAX_INPUT_LENGTH];
  struct descriptor_data *i;
  
  
  for (; *argument == ' '; argument++);
  
  if (!(*argument))
    send_to_char("Comunicare fra gli Dei e` ottimo, ma COSA ?\n\r",ch);
  else
  {
    if (IS_NPC(ch) || IS_SET(ch->specials.act, PLR_ECHO))
    {
      sprintf(buf1,"$c0012Tu pensi '%s'", argument);
      act(buf1,FALSE, ch,0,0,TO_CHAR);
    }
    sprintf(buf1, "$c0012::$c0015$n$c0012:: '%s'", argument);
    
    for (i = descriptor_list; i; i = i->next)
      if( i->character != ch && !i->connected && !IS_NPC(i->character) &&
          !IS_SET(i->character->specials.act, PLR_NOSHOUT) &&
          (GetMaxLevel(i->character) >= LOW_IMMORTAL))
        act(buf1, 0, ch, 0, i->character, TO_VICT);
  }
}


void do_tell(struct char_data *ch, const char *argument, int cmd)
{
  struct char_data *vict;
  char name[100], message[MAX_INPUT_LENGTH+20],
  buf[MAX_INPUT_LENGTH+60];


  if (apply_soundproof(ch))
    return;
  
  half_chop(argument,name,message);

  if(!*name || !*message)
  {
    send_to_char("A chi e` che vuoi parlare ?\n\r", ch);
    return;
  }
  else if (!(vict = get_char_vis(ch, name)))
  {
    send_to_char("Non c'e` nessuno con quel nome qui...\n\r", ch);
    return;
  }
  else if (ch == vict)
  {
    send_to_char("Parlare a se stessi puo` essere utile, a volte...\n\r", ch);
    return;
  }
  else if( GET_POS(vict) == POSITION_SLEEPING && !IS_IMMORTAL(ch) )
  {
    act("$E sta dormendo, shhh.",FALSE,ch,0,vict,TO_CHAR);
    return;
  }
  else if( !(GetMaxLevel(ch) >= LOW_IMMORTAL) &&
           !IS_NPC( vict ) && IS_SET( vict->specials.act,PLR_NOTELL ) )
  {
     act("$N non sta ascoltando adesso.",FALSE,ch,0,vict,TO_CHAR);
     return;
  }
  else if( ( GetMaxLevel( vict ) >= LOW_IMMORTAL ) && 
           ( GetMaxLevel( ch ) >= LOW_IMMORTAL ) &&
             ( GetMaxLevel( ch ) < GetMaxLevel( vict ) ) && 
           !IS_NPC( vict ) && IS_SET( vict->specials.act, PLR_NOTELL ) )
  {
     act("$N non sta ascoltando adesso.",FALSE,ch,0,vict,TO_CHAR);
     return;
  }
  else if( IS_LINKDEAD( vict ) )
  {
    send_to_char( "Non puo` sentirti. Ha perso il senso della realta`.\n\r", 
                  ch );
    return;
  } 

  if( check_soundproof( vict ) )
  {
    send_to_char( "Non riesce a sentire nemmeno la sua voce, li` dentro.\n\r",
                  ch);
    return;
  }

#if ZONE_COMM_ONLY
  if( real_roomp(ch->in_room)->zone != 
      real_roomp(vict->in_room)->zone
      && GetMaxLevel(ch) < LOW_IMMORTAL )
  {
    send_to_char( "Quella persona non e` abbastanza vicina per sentirti.\n\r",
                  ch);
    return;
  }
#endif
        
  sprintf( buf,"$c0013[$c0015%s$c0013] ti dice '%s'",
           (IS_NPC(ch) ? ch->player.short_descr : GET_NAME(ch)), message);
  act(buf,FALSE, vict,0,0,TO_CHAR);

  if( IS_NPC( ch ) || IS_SET( ch->specials.act, PLR_ECHO ) )
  { 
     sprintf( buf,"$c0013Tu dici a %s %s'%s'",
             (IS_NPC(vict) ? vict->player.short_descr : GET_NAME(vict)), \
             (IS_AFFECTED2(vict, AFF2_AFK) ? "(che e` AFK) " : ""), message);
     act(buf,FALSE, ch,0,0,TO_CHAR);
  }
  act("$c0013$n dice qualcosa a $N.", FALSE, ch, 0, vict, TO_NOTVICT);
}



void do_whisper(struct char_data *ch, const char *argument, int cmd)
{
  struct char_data *vict;
  char name[100], message[MAX_INPUT_LENGTH],
  buf[MAX_INPUT_LENGTH];

  if (apply_soundproof(ch))
    return;
  
  half_chop(argument,name,message);
  
  if(!*name || !*message)
    send_to_char("A chi vuoi sussurrare ? e cosa ?\n\r", ch);
  else if (!(vict = get_char_room_vis(ch, name)))
    send_to_char("Non c'e` nessuno con quel nome qui...\n\r", ch);
  else if (vict == ch)
  {
    act( "$n sussurra silenziosamente a se stesso.", FALSE, ch, 0, 0, TO_ROOM );
    send_to_char( "Sembra che le tue orecchie non siano abbastanza vicine "
                  "alla bocca...\n\r", ch);
  }
  else if( IS_LINKDEAD( vict ) )
  {
    send_to_char( "Non puo` sentirti. Ha perso il senso della realta`.\n\r", 
                  ch );
    return;
  } 
  else
  {
    if (check_soundproof(vict))
      return;

    sprintf(buf,"$c0005[$c0015$n$c0005] ti sussurra '%s'",message);
    act(buf, FALSE, ch, 0, vict, TO_VICT);
    if (IS_NPC(ch) || (IS_SET(ch->specials.act, PLR_ECHO)))
    {
      sprintf( buf,"$c0005Tu sussurri a %s%s, '%s'",
               (IS_NPC(vict) ? vict->player.name : GET_NAME(vict)), \
               (IS_AFFECTED2(vict, AFF2_AFK) ? " (che e` AFK)" : ""), message);
      act(buf,FALSE, ch,0,0,TO_CHAR);
    }
    act("$c0005$n sussurra qualcosa a $N.", FALSE, ch, 0, vict, TO_NOTVICT);
  }
}


void do_ask(struct char_data *ch, const char *argument, int cmd)
{
  struct char_data *vict;
  char name[100], message[MAX_INPUT_LENGTH],
  buf[MAX_INPUT_LENGTH];

  if (apply_soundproof(ch))
    return;
  
  half_chop(argument,name,message);
  
  if(!*name || !*message)
    send_to_char( "A chi vuoi chiedere... e cosa ?\n\r", ch);
  else if (!(vict = get_char_room_vis(ch, name)))
    send_to_char("Non vedi nessuno con quel nome qui...\n\r", ch);
  else if (vict == ch)
  {
    act( "$c0006[$c0015$n$c0006] si chiede qualcosa... trovera` la soluzione ?", 
         FALSE,ch,0,0,TO_ROOM);
    act( "$c0006Oltre alla domanda, conosci anche la risposta ?", FALSE, ch, 0,
         0, TO_CHAR);
  }
  else if( IS_LINKDEAD( vict ) )
  {
    send_to_char( "Non puo` sentirti. Ha perso il senso della realta`.\n\r", 
                  ch );
    return;
  } 
  else
  {
    if (check_soundproof(vict))
      return;

    sprintf(buf,"$c0006[$c0015$n$c0006] ti chiede '%s'",message);
    act(buf, FALSE, ch, 0, vict, TO_VICT);
    
    if (IS_NPC(ch) || (IS_SET(ch->specials.act, PLR_ECHO)))
    {
      sprintf( buf, "$c0006Tu chiedi a %s%s, '%s'",
               (IS_NPC(vict) ? vict->player.name : GET_NAME(vict)), \
               (IS_AFFECTED2(vict, AFF2_AFK) ? " (che e` AFK)" : ""), message);
      act(buf,FALSE, ch,0,0,TO_CHAR);
    }
    act("$c0006$n fa una domanda a $N.",FALSE,ch,0,vict,TO_NOTVICT);
  }
}



#define MAX_NOTE_LENGTH 1000      /* arbitrary */

void do_write(struct char_data *ch, const char *argument, int cmd)
{
  struct obj_data *paper = 0, *pen = 0;
  char papername[MAX_INPUT_LENGTH], penname[MAX_INPUT_LENGTH],
       buf[MAX_STRING_LENGTH];
  
  argument_interpreter(argument, papername, penname);
  
  if (!ch->desc)
    return;
  
  if (!*papername)  /* nothing was delivered */
  {
    send_to_char("write (on) papername (with) penname.\n\r", ch);
    return;
  }

  if (!*penname)
  {
    send_to_char("write (on) papername (with) penname.\n\r", ch);
    return;
  }
  if (!(paper = get_obj_in_list_vis(ch, papername, ch->carrying)))
  {
    sprintf(buf, "Tu non hai nessun %s.\n\r", papername);
    send_to_char(buf, ch);
    return;
  }
  if (!(pen = get_obj_in_list_vis(ch, penname, ch->carrying)))
  {
    sprintf(buf, "Tu non hai nessun %s.\n\r", papername);
    send_to_char(buf, ch);
    return;
  }

  /* ok.. now let's see what kind of stuff we've found */
  if (pen->obj_flags.type_flag != ITEM_PEN)
  {
    act( "Non puoi scrivere con $p.", FALSE, ch, pen, 0, TO_CHAR );
  }
  else if (paper->obj_flags.type_flag != ITEM_NOTE)
  {
      act("Non puoi scrivere su $p.", FALSE, ch, paper, 0, TO_CHAR);
  }
  else if (paper->action_description && *paper->action_description)
  {
    send_to_char("C'e` gia` scritto sopra qualcosa.\n\r", ch);
    return;
  }
  else
  {
    /* we can write - hooray! */
    send_to_char
        ("Ok... puoi scrivere... finisci la nota con un @.\n\r", ch);
    act("$n comincia a scrivere qualcosa.", TRUE, ch, 0,0,TO_ROOM);
#if 0
    if( paper->action_description )
      free( paper->action_description );
    paper->action_description = NULL;
#endif
    ch->desc->str = &paper->action_description;
    ch->desc->max_str = MAX_NOTE_LENGTH;
  }
}

const char *RandomWord()
{
  const char *string[50] = {
    "argle",
    "bargle",
    "glop",
    "glyph",
    "hussamah",  /* 5 */
    "rodina",
    "mustafah",
    "angina",
    "il",
    "fribble",  /* 10 */
    "fnort",
    "frobozz",
    "zarp",
    "ripple",
    "yrk",    /* 15 */
    "yid",
    "yerf",
    "oork",
    "grapple",
    "rosso",   /* 20 */
    "blu",
    "tu",
    "me",
    "ftagn",
    "hastur",   /* 25 */
    "brob",
    "gnort",
    "lram",
    "truck",
    "uccidi",    /* 30 */
    "cthulhu",
    "huzzah",
    "acetacitacilicio",
    "idroxipropilene",
    "summah",     /* 35 */
    "hummah",
    "biscotti",
    "ema",
    "voglia",
    "wadapatang",   /* 40 */
    "pterodactilo",
    "frob",
    "yuma",
    "gomma",
    "lo-pan",   /* 45 */
    "sushi",
    "yaya",
    "yoyodine",
    "yaazr",
    "bipsnop"   /* 50 */
  };

  return( string[ number( 0, 49 ) ] );

}

void do_sign(struct char_data *ch, const char *argument, int cmd)
{
  int i;
  char buf[MAX_INPUT_LENGTH+40];
  char buf2[MAX_INPUT_LENGTH];
  char *p;
  int diff;
  struct char_data *t;
  struct room_data *rp;
    
  for (i = 0; *(argument + i) == ' '; i++);
  
  if (!*(argument + i))
    send_to_char( "Daccordo, ma cosa vuoi dire ?\n\r", ch);
  else
  {

    rp = real_roomp(ch->in_room);
    if (!rp)
      return;

    if (!HasHands(ch))
    {
      send_to_char("Si giusto... ma con QUALI MANI ?????????\n\r", ch);
      return;
    }

    strcpy(buf, argument+i);
    buf2[0] = '\0';
    /*
      work through the argument, word by word.  if you fail your
      skill roll, the word comes out garbled.
      */
    p = strtok(buf, " ");  /* first word */

    diff = strlen(buf);

    while (p)
    {
      if (ch->skills && number(1,75+strlen(p))<ch->skills[SKILL_SIGN].learned)
      {
        strcat(buf2, p);
      }
      else
      {
        strcat(buf2, RandomWord());
      }
      strcat(buf2, " ");
      diff -= 1;
      p = strtok(0, " ");  /* next word */
    }
    /*
      if a recipient fails a roll, a word comes out garbled.
      */

    /*
      buf2 is now the "corrected" string.
      */

    sprintf(buf,"$c0015[$c0005$n$c0015], con i segni,  dice '%s'", buf2);

    for (t = rp->people;t;t=t->next_in_room)
    {
      if (t != ch)
      {
        if (t->skills && number(1,diff) < t->skills[SKILL_SIGN].learned)
        {
          act(buf, FALSE, ch, 0, t, TO_VICT);
        }
        else
        {
          act("$n muove le mani in modo molto buffo.", 
              FALSE, ch, 0, t, TO_VICT);          
        }
      }
    }

    if (IS_NPC(ch)||(IS_SET(ch->specials.act, PLR_ECHO)))
    {
      sprintf(buf,"Tu hai detto '%s'\n\r", argument + i);
      send_to_char(buf, ch);
    }
  }
}

        /* speak elvish, speak dwarvish, etc...                    */
void do_speak(struct char_data *ch, const char *argument, int cmd)
{
  char buf[255];
  int i;

  #define MAX_LANGS 8 

  const char *lang_list[MAX_LANGS] =
  {
    "common",
    "elvish",
    "halfling",
    "dwarvish",
    "orcish",
    "giantish",
    "ogre",
    "gnomish"
  };

  only_argument(argument,buf);

  if (buf[0] == '\0')
  {
    send_to_char("In quale lingua vuoi parlare ?\n\r",ch);
    return;
  }

  if( strstr( buf,"common" ) ) 
    i = SPEAK_COMMON;
  else if( strstr( buf, "elvish" ) ) 
    i = SPEAK_ELVISH;
  else if( strstr( buf, "halfling" ) ) 
    i = SPEAK_HALFLING;
  else if( strstr( buf, "dwarvish" ) ) 
    i = SPEAK_DWARVISH;
  else if( strstr( buf, "orcish" ) ) 
    i = SPEAK_ORCISH;
  else if( strstr( buf, "giantish" ) ) 
    i = SPEAK_GIANTISH;
  else if( strstr( buf, "ogre" ) ) 
    i = SPEAK_OGRE;
  else if( strstr( buf, "gnomish" ) ) 
    i = SPEAK_GNOMISH; 
  else
    i = -1;

  if (i == -1)
  {
    send_to_char("Non e` un linguaggio molto conosciuto.\n\r",ch);
    return;
  } 

                /* set language that we're gonna speak */         
  ch->player.speaks = i; 
  sprintf(buf,"Sei concentrato nel parlare %s.\n\r",lang_list[i-1]);
  send_to_char(buf,ch);
}

        /* this is where we do the language says */
void do_new_say(struct char_data *ch, const char *argument, int cmd)
{
  int i, learned, skill_num;
  char buf[MAX_INPUT_LENGTH+40];
  char buf2[MAX_INPUT_LENGTH];
  char buf3[MAX_INPUT_LENGTH+40];  
  char *p;
  int diff;
  struct char_data *t;
  struct room_data *rp;

  if (!argument) 
    return;

  for( i = 0; *(argument + i) == ' '; i++);
  
  if (!argument[i])
    send_to_char("Ok, ma cosa hai da dire ?\n\r", ch);
  else
  {

    if (apply_soundproof(ch))
      return;

    rp = real_roomp(ch->in_room);
    if (!rp)
      return;


    if (!ch->skills)
    {
      learned = 0;
      skill_num = LANG_COMMON;
    }
    else
    {
                /* find the language we are speaking */

      switch(ch->player.speaks)
      {
        case SPEAK_COMMON:
          learned = ch->skills[LANG_COMMON].learned;
          skill_num=LANG_COMMON;
          break;
        case SPEAK_ELVISH:
          learned = ch->skills[LANG_ELVISH].learned;
          skill_num=LANG_ELVISH;
          break;
        case SPEAK_HALFLING:
          learned = ch->skills[LANG_HALFLING].learned;
          skill_num=LANG_HALFLING;
          break;
        case SPEAK_DWARVISH:
          learned = ch->skills[LANG_DWARVISH].learned;
          skill_num=LANG_DWARVISH;
          break;
        case SPEAK_ORCISH:
          learned = ch->skills[LANG_ORCISH].learned;
          skill_num=LANG_ORCISH;
          break;
        case SPEAK_GIANTISH:
          learned = ch->skills[LANG_GIANTISH].learned;
          skill_num=LANG_GIANTISH;
          break;
        case SPEAK_OGRE:
          learned = ch->skills[LANG_OGRE].learned;
          skill_num=LANG_OGRE;
          break;
        case SPEAK_GNOMISH:
          learned = ch->skills[LANG_GNOMISH].learned;
          skill_num=LANG_GNOMISH;
          break;

        default:
          learned = ch->skills[LANG_COMMON].learned;
          skill_num = LANG_COMMON;
        break;
      } /* end switch */
    }
/* end finding language */


    strcpy(buf, argument+i);
    buf2[0] = '\0'; 

   /* we use this for ESP and immortals and comprehend lang */
    sprintf(buf3,"$c0015[$c0005$n$c0015] dice '%s'",buf);
    
    /*
      work through the argument, word by word.  if you fail your
      skill roll, the word comes out garbled.
      */
    p = strtok(buf, " ");  /* first word */

    diff = strlen(buf);


    while (p)
    {
      if (number(1,75+strlen(p))<learned || GetMaxLevel(ch) >= LOW_IMMORTAL)
      {
        strcat(buf2, p);
      }
      else
      {
        /* add case statement here to use random words from clips of elvish */
        /* dwarvish etc so the words look like they came from that language */
        strcat(buf2, RandomWord());
      }
      strcat(buf2, " ");
      diff -= 1;
      p = strtok(0, " ");  /* next word */
    }
    /*
      if a recipient fails a roll, a word comes out garbled.
      */

    /*
      buf2 is now the "corrected" string.
      */
    if (!*buf2 || !*buf2)
    {
      send_to_char("OK, ma cosa hai da dire ?\n\r", ch);
      return;
    }

    sprintf(buf,"$c0015[$c0005$n$c0015] dice '%s'", buf2);
    
    for (t = rp->people;t;t=t->next_in_room)
    {
      if (t != ch)
      {
        if( ( t->skills && number(1,diff) < t->skills[skill_num].learned ) || 
            GetMaxLevel( t ) >= LOW_IMMORTAL || IS_NPC( t ) ||
            affected_by_spell( t, SKILL_ESP ) || 
            affected_by_spell( t, SPELL_COMP_LANGUAGES ) ||
            GetMaxLevel( ch ) >= LOW_IMMORTAL )
        {

                        /* these guys always understand */
            if( GetMaxLevel(t) >= LOW_IMMORTAL || 
                affected_by_spell(t,SKILL_ESP) ||
                affected_by_spell(t,SPELL_COMP_LANGUAGES) || IS_NPC(t) )
                  act(buf3, FALSE,ch,0,t,TO_VICT); 
            else
                              /* otherwise */
                              
              act(buf, FALSE, ch, 0, t, TO_VICT);
        }
        else
        {
          act( "$c0010$n parla una lingua che non riesci a capire.", FALSE, 
               ch, 0, t, TO_VICT);          
        }
      }
    }

    if (IS_NPC(ch)||(IS_SET(ch->specials.act, PLR_ECHO)))
    {
      sprintf( buf,"$c0015Tu dici '%s'", argument + i );
      act(buf,FALSE, ch,0,0,TO_CHAR);
    }
  }
}



void do_gtell(struct char_data *ch, const char *argument, int cmd)
{
  int i;
  struct char_data *k;
  struct follow_type *f;
  char buf[MAX_STRING_LENGTH];

 if (apply_soundproof(ch))
   return;
  
  for (i = 0; *(argument + i) == ' '; i++);

  if(!*(argument+i))
  {
    send_to_char("Cosa vuoi dire al gruppo ?\n\r", ch);
    return;
  } 

  if (!IS_AFFECTED(ch, AFF_GROUP))
  {
    send_to_char("Forse dovresti unirti ad un gruppo, prima.\n\r", ch);
    return;
  }
  else
  {
    if (ch->master)
      k = ch->master;
    else
      k = ch;
      
    for(f=k->followers; f; f=f->next)
    {
      if (IS_AFFECTED(f->follower, AFF_GROUP))
      {
        if (!f->follower->desc)
        {
          /* link dead */
        }
        else if (ch == f->follower)
        {
          /* can't tell yourself! */
        }
        else if (!check_soundproof(f->follower))
        {
          sprintf(buf,"$c0012[$c0015%s$c0012] dice al gruppo '%s'",
                       (IS_NPC(ch) ? ch->player.short_descr : 
                                    GET_NAME(ch)), argument+i);
          act(buf, FALSE,f->follower,0,0,TO_CHAR);
        } /* !soundproof */
      }
    } /* end for loop */

                /* send to master now */
    if (ch->master)
    {
      if (IS_AFFECTED(ch->master, AFF_GROUP))
      {
        if (!ch->master->desc)
        {
          /* link dead */
        }
        else if (ch == ch->master)
        {
           /* can't tell yourself! */
        }
        else if (!check_soundproof(ch->master))
        {
          sprintf( buf,"$c0012[$c0015%s$c0012] dice al gruppo '%s'",
                    (IS_NPC(ch) ? ch->player.short_descr : 
                                 GET_NAME(ch)), argument+i);
          act(buf, FALSE,ch->master,0,0,TO_CHAR);
        } /* !soundproof */
      }
    }         /* end master send */                
 
    if (IS_NPC(ch) || IS_SET(ch->specials.act, PLR_ECHO))
    { 
      sprintf(buf,"$c0012Tu dici al gruppo '%s'",argument+i);
      act(buf, FALSE,ch,0,0,TO_CHAR);
    } /* if echo */
    
  } /* they where grouped... */   
} /* end of gtel */

/*
 * 'Split' originally by Gnort, God of Chaos. I stole it from Merc
 * and changed it to work with mine :) Heh msw
 */
 
void do_split(struct char_data *ch, const char *argument, int cmd)
{

  bool is_same_group( struct char_data *ach, struct char_data *bch );

  char buf[MAX_STRING_LENGTH];
  char arg[MAX_INPUT_LENGTH];
  struct char_data *gch;
  int members, amount, share, extra;

  one_argument( argument, arg );

  if ( arg[0] == '\0' )
  {
    send_to_char( "Quanto vuoi dividere ?\n\r", ch );
    return;
  }
    
  amount = atoi( arg );

  if ( amount < 0 )
  {
    send_to_char( "Non cercare di fare il furbo con il tuo gruppo.\n\r", ch );
    return;
  }

  if ( amount == 0 )
  {
    send_to_char( "Hai diviso zero monete, ma nessuno lo ha notato.\n\r", ch );
    return;
  }

  if ( ch->points.gold < amount )
  {
    send_to_char( "Non hai tutto quell'oro.\n\r", ch );
    return;
  }
  
  members = 0;
  for( gch = real_roomp(ch->in_room)->people;
       gch != NULL; gch = gch->next_in_room )
  {
    if( is_same_group( gch, ch ) )
      members++;
  }

  if( members < 2 )
  {
    send_to_char( "Ma cosa vuoi dividere che sei solo.\n\r", ch );
    return;
  }
            
  share = amount / members;
  extra = amount % members;

  if( share == 0 )
  {
    send_to_char( "C'e` poco da dividere, siete in troppi.\n\r", ch );
    return;
  }

  ch->points.gold -= amount;
  ch->points.gold += share + extra;

  sprintf( buf,
           "Hai diviso %d monete d'oro. La tua parte e` di %d monete.\n\r",
           amount, share + extra );
  send_to_char( buf, ch );

  sprintf( buf, "$n divide %d monete d'oro. La tua parte e` di %d monete.",
           amount, share );

  for( gch = real_roomp(ch->in_room)->people; 
       gch != NULL; gch = gch->next_in_room )
  {
    if( gch != ch && is_same_group( gch, ch ) )
    {
      act( buf,FALSE,ch, NULL, gch, TO_VICT );
      gch->points.gold += share;
    }
  }

  return;
}


void do_pray( struct char_data *ch, const char *argument, int cmd )
{
  struct affected_type af;
  static char buf1[MAX_INPUT_LENGTH];
  struct descriptor_data *i;
  int ii=0; 

  if( IS_NPC( ch ) )
    return;  

  if( affected_by_spell( ch, SPELL_PRAYER ) )
  {
    send_to_char( "Hai gia` pregato oggi.\n\r", ch );
    return;
  }
   
  for( ; *argument == ' '; argument++ );
  
  if( !( *argument ) )
    send_to_char( "Vuoi pregare. Ottimo, ma chi ? "
                  "(pray <NomeDio> <preghiera>)\n\r", ch );
  else 
  {
    ii = (int)( GetMaxLevel( ch ) * 1.5 );

    if( HasClass( ch, CLASS_CLERIC | CLASS_DRUID ) )
      ii +=10;  /* clerics get a 10% bonus :) */

    if( ii > number( 1, 101 ) )
    {
      if( IS_NPC( ch ) || IS_SET( ch->specials.act, PLR_ECHO ) )
      {
        sprintf(buf1,"Tu preghi '%s'\n\r", argument );
        send_to_char( buf1, ch );
      }
      sprintf( buf1, "$c0014[$c0015$n$c0014] prega '%s'", argument);
    
      for( i = descriptor_list; i; i = i->next)
      {
        if( i->character != ch && ! i->connected && ! IS_NPC( i->character ) &&
            !IS_SET( i->character->specials.act, PLR_NOSHOUT ) &&
            ( GetMaxLevel( i->character ) >= LOW_IMMORTAL ) )
          act( buf1, 0, ch, 0, i->character, TO_VICT );
      } /* end for */
   
    } /* failed prayer */
    else
      send_to_char( "Le tue preghiere sono state ignorate.\n\r", ch );

    af.type = SPELL_PRAYER;
    af.duration = 24;
    af.modifier = 0;
    af.location = APPLY_NONE;
    af.bitvector = 0;
    affect_to_char( ch, &af );    
    return;
  }
} 

/* modified by Aarcerak */
bool is_same_group( struct char_data *ach, struct char_data *bch )
{
  if( !IS_AFFECTED( ach, AFF_GROUP ) || !IS_AFFECTED( bch, AFF_GROUP ) )
    return 0;
  if( ach->master != NULL ) 
    ach = ach->master;
  if( bch->master != NULL ) 
    bch = bch->master;
  return( ach == bch );
}


void do_telepathy( struct char_data *ch, const char *argument, int cmd )
{
  struct char_data *vict;
  char name[100], message[MAX_INPUT_LENGTH+20],
  buf[MAX_INPUT_LENGTH+60];

  half_chop( argument, name, message );

  if( !HasClass( ch, CLASS_PSI ) && !IS_AFFECTED( ch, AFF_TELEPATHY ) )
  {
    send_to_char( "Cosa pensi di essere ? Un telepate ?\n\r", ch );
    return;
  }

  if( GET_MANA( ch ) < 5 && !IS_AFFECTED( ch, AFF_TELEPATHY ) )
  {
    send_to_char( "Non hai la potenza mentale sufficiente.\n\r",ch);
    return;
  }

  if( !*name || !*message)
  {
    send_to_char("A chi vuoi mandare il tuo pensiero ?\n\r", ch);
    return;
  }
  else if( !( vict = get_char_vis( ch, name ) ) )
  {
    send_to_char("Non c'e` nessuno con quel nome qui...\n\r", ch );
    return;
  }
  else if( ch == vict )
  {
    send_to_char( "Nella tua mente risuona il tuo pensiero...\n\r", ch );
    return;
  } 
  else if( GET_POS( vict ) == POSITION_SLEEPING && !IS_IMMORTAL( ch ) ) 
  {
    act("$E sta dormendo, shhh.",FALSE,ch,0,vict,TO_CHAR);
    return;
  }
  else if( IS_NPC( vict ) && !( vict->desc ) )
  {
    send_to_char("Non c'e` nessuno con quel nome qui...\n\r", ch );
    return;
  }
  else if( !( GetMaxLevel( ch ) >= LOW_IMMORTAL ) &&
           IS_SET( vict->specials.act, PLR_NOTELL ) )
  {
    act( "$N non sta ascoltando adesso.", FALSE, ch, 0, vict, TO_CHAR );
    return;
  }
  else if( ( GetMaxLevel( vict ) >= LOW_IMMORTAL ) &&
           ( GetMaxLevel( ch ) >= LOW_IMMORTAL ) &&
             ( GetMaxLevel( ch ) < GetMaxLevel( vict ) ) &&
           IS_SET( vict->specials.act, PLR_NOTELL ) )
  {
    act( "La mente di $N e` chiusa in questo momento !", FALSE, ch, 0, vict,
         TO_CHAR);
    return;
  }
  else if( !vict->desc )
  {
    send_to_char( "Non puo` sentirti. Gli e` caduta la linea (link dead).\n\r", 
                  ch);
    return;
  }

/*
  if (check_soundproof(vict)) {
        send_to_char("In a silenced room, try again later.\n\r",ch);
        return;
  }
*/

  if( !IS_AFFECTED( ch, AFF_TELEPATHY ) )
    GET_MANA( ch ) -=5;  
 
  sprintf( buf, "$c0013[$c0015%s$c0013] ti manda il pensiero '%s'",
           ( IS_NPC( ch ) ? ch->player.short_descr : 
                            GET_NAME( ch ) ), message );
  act( buf, FALSE, vict, 0, 0, TO_CHAR );

  if( IS_NPC( ch ) || IS_SET( ch->specials.act, PLR_ECHO ) )
  { 
    sprintf( buf, "$c0013Tu mandi a %s il pensiero '%s'",
             ( IS_NPC( vict ) ? vict->player.short_descr : 
                                GET_NAME( vict ) ), message );
    act(buf,FALSE, ch,0,0,TO_CHAR);
  }
}


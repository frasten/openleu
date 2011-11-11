#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

#include "protos.h"
#include "cmdid.h"
#include "status.h"
#include "fight.h"

/*   external vars  */

extern struct room_data *world;
extern struct char_data *character_list;
extern struct descriptor_data *descriptor_list;
extern struct index_data *obj_index;
extern struct time_info_data time_info;
extern struct index_data *mob_index;
extern struct weather_data weather_info;
extern int top_of_world;
extern struct int_app_type int_app[26];

extern struct title_type titles[4][ABS_MAX_LVL];
extern const char *dirs[];

extern int gSeason;  /* what season is it ? */


/* Bjs Shit Begin */

#define Bandits_Path   2180
#define BASIL_GATEKEEPER_MAX_LEVEL 10
#define Fountain_Level 20

#define CMD_SAY 17
#define CMD_ASAY 169

int ghost(struct char_data *ch, int cmd, const char *arg, struct char_data *mob, int type)
{
  void cast_energy_drain( byte level, struct char_data *ch, const char *arg,
                         int type,struct char_data *tar_ch,
                         struct obj_data *tar_obj );

  if (cmd || !AWAKE(ch))
    return(FALSE);


  if (ch->specials.fighting &&
      (ch->specials.fighting->in_room == ch->in_room))
  {
    act("$n touches $N!", 1, ch, 0, ch->specials.fighting, TO_NOTVICT);
    act("$n touches you!", 1, ch, 0, ch->specials.fighting, TO_VICT);
    cast_energy_drain( GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL,
                      ch->specials.fighting, 0);

    return TRUE;
  }
  return FALSE;
}

int druid_protector(struct char_data *ch, int cmd, const char *arg, struct char_data *mob, int type)
{
  static int b=1;  /* use this as a switch, to avoid double challenges */

  if (cmd) {
    if (cmd<=6 && cmd>=1 && IS_PC(ch)) {
      if (b) {
        b = 0;
        send_to_char("Basil Great Druid looks at you\n\r", ch);
        if ((ch->in_room == Bandits_Path ) && (cmd == 1)) {
          if ((BASIL_GATEKEEPER_MAX_LEVEL < GetMaxLevel(ch)) &&
              (GetMaxLevel(ch) < LOW_IMMORTAL))     {
            if (!check_soundproof(ch)) {
              act("Basil the Great Druid tells you 'Begone Unbelievers!'",
                  TRUE, ch, 0, 0, TO_CHAR);
            }
            act("Basil Great Druid grins evilly.", TRUE, ch, 0, 0, TO_CHAR);
            return(TRUE);
          }
        }
      } else {
        b = 1;
      }
      return(FALSE);
    } /* cmd 1 - 6 */
    return(FALSE);
  } else {
    if (ch->specials.fighting) {
      if ((GET_POS(ch) < POSITION_FIGHTING) &&
          (GET_POS(ch) > POSITION_STUNNED)){
        StandUp(ch);
      } else {
        FighterMove(ch);
      }
      return(FALSE);
    }
  }
  return(FALSE);
} /* end druid_protector */


int Magic_Fountain(struct char_data *ch, int cmd, const char *arg, struct room_data *rp, int type)
{

  char buf[MAX_INPUT_LENGTH];

  if (cmd==11) { /* drink */

    only_argument(arg,buf);

    if (str_cmp(buf, "fountain") && str_cmp(buf, "water")) {
      return(FALSE);
    }

    send_to_char("You drink from the fountain\n\r", ch);
    act("$n drinks from the fountain", FALSE, ch, 0, 0, TO_ROOM);


    if(GET_COND(ch,THIRST)>20) {
           act("You do not feel thirsty.",FALSE,ch,0,0,TO_CHAR);
           return(TRUE);
        }
    if(GET_COND(ch,FULL)>20) {
           act("You do are full.",FALSE,ch,0,0,TO_CHAR);
           return(TRUE);
        }

    GET_COND(ch,THIRST) = 24;
    GET_COND(ch,FULL)+=1;

    switch (number(0, 40)) {

    /* Lets try and make 1-10 Good, 11-26 Bad, 27-40 Nothing */
        case 1:
          cast_refresh(Fountain_Level, ch, "", SPELL_TYPE_SPELL, ch, 0);
        break;
        case 2:
          cast_stone_skin(Fountain_Level, ch, "", SPELL_TYPE_SPELL, ch, 0);
        break;
        case 3:
          cast_cure_serious(Fountain_Level, ch, "", SPELL_TYPE_SPELL, ch, 0);
        break;
        case 4:
          cast_cure_light(Fountain_Level, ch, "", SPELL_TYPE_SPELL, ch, 0);
        break;
        case 5:
          cast_armor(Fountain_Level, ch, "", SPELL_TYPE_SPELL, ch, 0);
        break;
        case 6:
          cast_bless(Fountain_Level, ch, "", SPELL_TYPE_SPELL, ch, 0);
        break;
        case 7:
          cast_invisibility(Fountain_Level, ch, "", SPELL_TYPE_SPELL, ch, 0);
        break;
        case 8:
          cast_strength(Fountain_Level, ch, "", SPELL_TYPE_SPELL, ch, 0);
        break;
        case 9:
          cast_remove_poison(Fountain_Level, ch, "", SPELL_TYPE_SPELL, ch, 0);
        break;
        case 10:
          cast_true_seeing(Fountain_Level, ch, "", SPELL_TYPE_SPELL, ch, 0);
        break;

/* Time for the nasty Spells */

        case 11:
          cast_dispel_magic(Fountain_Level, ch, "", SPELL_TYPE_SPELL, ch, 0);
        break;
        case 12:
          cast_teleport(Fountain_Level, ch, "", SPELL_TYPE_SPELL, ch, 0);
        break;
        case 13:
          cast_web(Fountain_Level, ch, "", SPELL_TYPE_SPELL, ch, 0);
        break;
        case 14:
          cast_curse(Fountain_Level, ch, "", SPELL_TYPE_SPELL, ch, 0);
        break;
        case 15:
          cast_blindness(Fountain_Level, ch, "", SPELL_TYPE_SPELL, ch, 0);
        break;
        case 16:
          cast_weakness(Fountain_Level, ch, "", SPELL_TYPE_SPELL, ch, 0);
        break;
        case 17:
          cast_poison(Fountain_Level, ch, "", SPELL_TYPE_SPELL, ch, 0);
        break;
        case 18:
          cast_cause_light(Fountain_Level, ch, "", SPELL_TYPE_SPELL, ch, 0);
        break;
        case 19:
          cast_cause_critic(Fountain_Level, ch, "", SPELL_TYPE_SPELL, ch, 0);
        break;
        case 20:
          cast_dispel_magic(Fountain_Level, ch, "", SPELL_TYPE_SPELL, ch, 0);
        break;
        case 21:
          cast_magic_missile(Fountain_Level, ch, "", SPELL_TYPE_SPELL, ch, 0);
        break;
        case 22:
          cast_faerie_fire(Fountain_Level, ch, "", SPELL_TYPE_SPELL, ch, 0);
        break;
        case 23:
          cast_flamestrike(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, ch, 0);
        break;
        case 24:
          cast_burning_hands(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, ch, 0);
        break;
        case 25:
          cast_acid_blast(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, ch, 0);
        break;
        case 26:
          cast_energy_drain(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, ch, 0);
        break;

         /* And of course nothing */

        default:
           send_to_char("The fluid tastes like dry sand in your mouth.\n\r", ch);
        break;
       }
  return(TRUE);
  }

  /* All commands except fill and drink */
  return(FALSE);
}


/* Bjs Shit End */

int DruidAttackSpells(struct char_data *ch, struct char_data *vict, int level)
{
  switch(level) {
  case 1:
  case 2:
  case 3:
  case 4:
  case 5:
  case 6:
  case 7:
    act("$n pronuncia le parole 'yow!'", 1, ch, 0, 0, TO_ROOM);
    cast_cause_light(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL,
                 vict, 0);
    return(FALSE);
    break;
  case 8:
  case 9:
  case 10:
  case 11:
  case 12:
  case 13:
    if (!IS_SET(vict->M_immune, AFF_POISON) &&
        !IS_AFFECTED(vict, AFF_POISON)){
      act("$n pronuncia le parole 'yuk'", 1, ch, 0, 0, TO_ROOM);
      cast_poison(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL,
                  vict, 0);
      return(FALSE);
    } else {
      act("$n pronuncia le parole 'ouch'", 1, ch, 0, 0, TO_ROOM);
      cast_cause_serious(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL,
                         vict, 0);
      return(FALSE);
    }
    break;
  case 14:
  case 15:
  case 16:
  case 17:
  case 18:
  case 19:
  case 20:
  case 21:
    act("$n pronuncia le parole 'OUCH!'", 1, ch, 0, 0, TO_ROOM);
    cast_cause_critic(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL,
                       vict, 0);
    return(FALSE);
  case 22:
  case 23:
  case 24:
  case 25:
  case 26:
  case 27:
  case 28:
  case 29:
  case 30:
  case 31:
  case 32:
  case 33:
  case 34:
  case 35:
  case 36:
  case 37:
  case 38:
  case 39:
  case 40:
  case 41:
  case 42:
  case 43:
  case 44:
    if (!IS_SET(vict->M_immune, IMM_FIRE)) {
      act("$n pronuncia le parole 'fwoosh'", 1, ch, 0, 0, TO_ROOM);
      cast_firestorm(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
      return(FALSE);
    } else {
      act("$n pronuncia le parole 'OUCH!'", 1, ch, 0, 0, TO_ROOM);
      cast_cause_critic(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL,
                         vict, 0);
      return(FALSE);
    }
    break;
  default:
    act("$n pronuncia le parole 'kazappapapapa'", 1, ch, 0, 0, TO_ROOM);
    cast_chain_lightn(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL,
                      vict, 0);
    return(FALSE);
    break;
  }
}


#if 0
int Summoner(struct char_data *ch, int cmd, const char *arg, struct char_data *mob, int type)
{
/*   extern struct descriptor_data *descriptor_list; */
  struct descriptor_data *d;
  struct char_data *targ=0;
  struct char_list *i;
  char buf[128];

  extern char EasySummon;

  if (cmd || !AWAKE(ch))
    return(FALSE);

  if (check_soundproof(ch)) return(FALSE);

  if (ch->specials.fighting)  return(FALSE);

  if (check_nomagic(ch, 0, 0))
    return(TRUE);

  /*
  **  wait till at 75% of hitpoints.
  */

  if (GET_HIT(ch) > ((GET_MAX_HIT(ch)*3)/4)) {
    /*
    **  check for hatreds
    */
    if (IS_SET(ch->hatefield, HATE_CHAR)) {
      if (ch->hates.clist) {
        for (i = ch->hates.clist; i; i = i->next) {
          if (i->op_ch) {  /* if there is a char_ptr */
            targ = i->op_ch;
            if (IS_PC(targ)) {
              sprintf(buf, "You hate %s\n\r", targ);
              send_to_char(buf, ch);
              break;
            }
          } else {  /* look up the char_ptr */
            for (d=descriptor_list; d; d = d->next) {
              if (d->character && i->name && (strcmp(GET_NAME(d->character), i->name)==0)) {
                targ = d->character;
                break;
              }
            }
          }
        }
      }
    }
    if (targ)
    {
      act("$n pronuncia le parole 'Your ass is mine!'.",
           1, ch, 0, 0, TO_ROOM);
      if (EasySummon == 1)
      {
        spell_summon(GetMaxLevel(ch), ch, targ, 0);
      } else
      {
        if (GetMaxLevel(ch) < 32)         {
          if (number(0,10))           {
            do_say(ch, "Curses!  Foiled again!\n\r", 0);
            return(0);
          }
        }

        /* Easy Summon was turned off and they where < 32nd level */
        /* so we portal to them! */
        spell_portal(GetMaxLevel(ch), ch, targ, 0);
        command_interpreter(ch,"enter portal");
      }
      if (targ->in_room == ch->in_room) {
         if (NumCharmedFollowersInRoom(ch) > 0) {
           sprintf(buf, "followers kill %s", GET_NAME(targ));
           do_order(ch, buf, 0);
         }
         hit(ch, targ, 0);
      }
      return(FALSE);
    } else {
      return(FALSE);
    }

  } else {
    return(FALSE);
  }
}
#endif
int Summoner(struct char_data *ch, int cmd, const char *arg, struct char_data *mob, int type)
{
  /*   extern struct descriptor_data *descriptor_list; */
  struct descriptor_data *d;
  struct char_data *targ=0;
  struct char_list *i;
  char buf[255];

  extern char EasySummon;

  if (cmd || !AWAKE(ch))
    return(FALSE);

  if (check_soundproof(ch)) 
    return(FALSE);

  if (ch->specials.fighting)  
    return(FALSE);

  if (check_nomagic(ch, 0, 0))
    return(TRUE);

  /*
  **  wait till at 75% of hitpoints.
  */

  if (GET_HIT(ch) > ((GET_MAX_HIT(ch)*3)/4)) 
  {
    /*
    **  check for hatreds
    */
    if (IS_SET(ch->hatefield, HATE_CHAR)) 
    {
      if (ch->hates.clist) 
      {
        for (i = ch->hates.clist; i; i = i->next) 
        {
          if (i->op_ch) 
          {  
            /* if there is a char_ptr */
            targ = i->op_ch;
            if (IS_PC(targ)) 
            {
              sprintf( buf, "You hate %s\n\r", GET_NAME( targ ) );
              send_to_char(buf, ch);
              break;
            }
          }
          else if( i->name )
          {
            /* look up the char_ptr */
            mudlog( LOG_CHECK, "%s cerca di summonare qualcuno.", 
                    GET_NAME_DESC( ch ) );
            for( d = descriptor_list; d; d = d->next )
            {
#if 1
              if( d->connected == CON_PLYNG && d->character &&
                  GET_NAME(d->character) &&
                  strcmp( GET_NAME( d->character ), i->name ) == 0 )
              {
                targ = d->character;
                break;
              }                
#endif
            }
            mudlog( LOG_CHECK, "  Ricerca finita." );
          }
        }
      }
    }
    if (targ)
    {
      act("$n pronuncia le parole 'Your ass is mine!'.",
           1, ch, 0, 0, TO_ROOM);
      if (EasySummon == 1)
      {
        if (!IS_SET(ch->player.iClass, CLASS_PSI))
          spell_summon(GetMaxLevel(ch), ch, targ, 0);
        else if (ch->skills[SKILL_SUMMON].learned &&
                 (GET_MAX_HIT(targ) <= GET_HIT(ch)))
          do_mindsummon(ch,targ->player.name,0);
        else 
          do_psi_portal(ch,targ->player.name,0);
      }
      else
      {
        if (GetMaxLevel(ch) < 32)
          if (number(0,10))
          {
            do_say(ch, "Curses!  Foiled again!\n\r", 0);
            return(0);
          }
        /* Easy Summon was turned off and they were < 32nd level */
        /* so we portal to them! */
        if (!IS_SET(ch->player.iClass,CLASS_PSI))
        {
          spell_portal(GetMaxLevel(ch), ch, targ, 0);
          command_interpreter(ch,"enter portal");
        }
        else  /*its a psi summoner, do his stuff*/
        {
          /*with easy_summon turned off must portal, so ..*/
          if (!ch->skills[SKILL_PORTAL].learned)
            ch->skills[SKILL_PORTAL].learned = ch->skills[SKILL_SUMMON].learned;
          do_psi_portal(ch,targ->player.name,0);
        }
      }
      if (targ->in_room == ch->in_room)
      {
        if (NumCharmedFollowersInRoom(ch) > 0)
        {
          sprintf(buf, "followers kill %s", GET_NAME(targ));
          do_order(ch, buf, 0);
        }
        act("$n says, 'And now my young $N... You will DIE!",0,ch,0,targ,TO_ROOM);
        if (!IS_SET(ch->player.iClass,CLASS_PSI))
          spell_dispel_magic(GetMaxLevel(ch),ch, targ, 0);
        else 
          do_blast(ch,targ->player.name,1);
      }
      return(FALSE);
    } 
    else 
    {
      return(FALSE);
    }

  }
  else
  {
    return(FALSE);
  }
}
/*---------------end of summoner---------------*/


int monk(struct char_data *ch, int cmd, const char *arg, struct char_data *mob, int type)
{

  if (cmd || !AWAKE(ch))
    return(FALSE);

  if (ch->specials.fighting) {
      MonkMove(ch);
  }
  return(FALSE);
}


typedef struct T1000_data {

   int state;
   struct char_data *vict;

} T1000_data;

#if 0
#define T1000_SEARCHING   0
#define T1000_HUNTING     1

int T1000( struct char_data *ch, char *line, int cmd, struct char_data *mob, int type)
{
   int count;
   struct descriptor_data *i;

/*    extern struct descriptor_data *descriptor_list; */

   if (ch->specials.hunting == 0)
     ch->generic = T1000_SEARCHING;

    switch(ch->generic) {
       case T1000_SEARCHING: {
          count = number(0,200);
          for (i = descriptor_list; count>0; i= i->next) {
             if (!i) {
               i = descriptor_list;
             }
          }
          if (i) {
             ch->specials.hunting = i->character;
             ch->generic = T1000_HUNTING;
          }
       }
       case T1000_HUNTING: {
          if (ch->in_room == ch->specials.hunting->in_room) {
          } else {
          }
       }
    }
}
#endif

void invert(const char *arg1, char *arg2)
{
  register int i = 0;
  register int len = MIN( strlen(arg1) - 1, 253 );

  for( i = 0; i <= len; i++ )
  {
    *(arg2 + i) = *(arg1 + (len - i));
  }
  *(arg2 + i) = '\0';
}

int jive_box( struct char_data *ch, int cmd, const char *arg, struct obj_data *obj,
              int type)
{
  char buf[255], buf2[255], buf3[255], tmp[255];

  if( type == EVENT_COMMAND )
  {
    switch(cmd)
    {
     case CMD_SAY:
     case CMD_SAY_APICE:
      invert( arg, buf );
      do_say( ch, buf, cmd );
      return TRUE;
      break;
      
     case CMD_TELL:
      half_chop( arg, tmp, buf );
      invert( buf, buf2 );
      sprintf( buf3, "%s %s", tmp, buf2 );
      do_tell( ch, buf3, cmd );
      return TRUE;
      break;
     
     case CMD_WHISPER:
      half_chop( arg, tmp, buf );
      invert( buf, buf2 );
      sprintf( buf3, "%s %s", tmp, buf2 );
      do_whisper( ch, buf3, cmd );
      return TRUE;
      break;
     
     case CMD_ASK:
      half_chop( arg, tmp, buf );
      invert( buf, buf2 );
      sprintf( buf3, "%s %s", tmp, buf2 );
      do_ask( ch, buf3, cmd );
      return TRUE;
      break;
     
     case CMD_GOSSIP:
      invert( arg, buf );
      do_gossip( ch, buf, cmd );
      return TRUE;
      break;
      
     case CMD_AUCTION:
      invert( arg, buf );
      do_auction( ch, buf, cmd );
      return TRUE;
      break;
      
     case CMD_GTELL:
      invert( arg, buf );
      do_gtell( ch, buf, cmd );
      return TRUE;
      break;
      
     case CMD_SHOUT:
      invert( arg, buf );
      do_shout( ch, buf, cmd );
      return TRUE;
      break;
    }
  }
  return FALSE;
}


int magic_user(struct char_data *ch, int cmd, const char *arg, struct char_data *mob, int type)
{
  struct char_data *vict;

  byte lspell;

  if( cmd || !AWAKE(ch) || IS_AFFECTED( ch, AFF_PARALYSIS ) )
    return(FALSE);

  if( !ch->specials.fighting && !IS_PC(ch) )
  {
    if ((GET_POS(ch) > POSITION_STUNNED) &&
        (GET_POS(ch) < POSITION_FIGHTING))
    {
      StandUp(ch);
      return(TRUE);
    }
    SET_BIT(ch->player.iClass, CLASS_MAGIC_USER);

    if (GetMaxLevel(ch) >= 25)
    {
      if (!ch->desc)
      {
        if( Summoner( ch, cmd, arg, mob, type ) )
          return(TRUE);
        else
        {
          if( NumCharmedFollowersInRoom( ch ) < 5 &&
              ( IS_SET( ch->hatefield, HATE_CHAR ) || 
                IS_SET( ch->hatefield, FEAR_CHAR ) ) )
          {
            act( "$n pronuncia le parole 'Here boy!'.", 1, ch, 0, 0, TO_ROOM );
            cast_mon_sum7( GetMaxLevel( ch ), ch, "", SPELL_TYPE_SPELL, ch, 0 );
            do_order( ch, "followers guard on", 0 );
            return(TRUE);
          }
        }
      }
    }

    if( !ch->desc )
    { /* make sure it is a mob not a pc */
      /* Now, lets cast a few spells on ourself */
      /* low level prep spells here */

      if (!affected_by_spell(ch,SPELL_SHIELD))
      {
        act("$n pronuncia le parole 'dragon'.", 1, ch, 0, 0, TO_ROOM);
        cast_shield(GetMaxLevel(ch),ch,GET_NAME(ch),SPELL_TYPE_SPELL,ch,0);
        return(TRUE);
      }

      if (!affected_by_spell(ch,SPELL_STRENGTH))
      {
        act("$n pronuncia le parole 'giant'.", 1, ch, 0, 0, TO_ROOM);
        cast_strength(GetMaxLevel(ch),ch,GET_NAME(ch),SPELL_TYPE_SPELL,ch,0);
        return(TRUE);
      }

      if (!IS_EVIL(ch) && !affected_by_spell(ch,SPELL_PROTECT_FROM_EVIL))
      {
        act("$n pronuncia le parole 'anti-evil'.", 1, ch, 0, 0, TO_ROOM);
        cast_protection_from_evil(GetMaxLevel(ch),ch,"",SPELL_TYPE_SPELL,ch,0);
        return(TRUE);
      }

      if( (IS_AFFECTED(ch,AFF_FIRESHIELD) || IS_AFFECTED(ch,AFF_SANCTUARY)) &&
          (!affected_by_spell(ch,SPELL_GLOBE_DARKNESS)) )
      {
        act("$n pronuncia le parole 'darkness'.", 1, ch, 0, 0, TO_ROOM);
        cast_globe_darkness( GetMaxLevel(ch),ch,GET_NAME(ch),
                             SPELL_TYPE_SPELL,ch,0);
        return(TRUE);
      }

        /* high level prep spells here */
      if (GetMaxLevel(ch) >= 25)
      {
        if (!affected_by_spell(ch,SPELL_ARMOR))
        {
          act("$n pronuncia le parole 'dragon'.", 1, ch, 0, 0, TO_ROOM);
          cast_armor(GetMaxLevel(ch),ch,"",SPELL_TYPE_SPELL,ch,0);
          return(TRUE);
        }

        if (!affected_by_spell(ch,SPELL_STONE_SKIN))
        {
          act("$n pronuncia le parole 'stone'.", 1, ch, 0, 0, TO_ROOM);
          cast_stone_skin(GetMaxLevel(ch),ch,"",SPELL_TYPE_SPELL,ch,0);
          return(TRUE);
        }

        if (!affected_by_spell(ch,SPELL_GLOBE_MINOR_INV))
        {
          act("$n pronuncia le parole 'haven'.", 1, ch, 0, 0, TO_ROOM);
          cast_globe_minor_inv(GetMaxLevel(ch),ch,"",SPELL_TYPE_SPELL,ch,0);
          return(TRUE);
        }

        if (!affected_by_spell(ch,SPELL_GLOBE_MAJOR_INV))
        {
          act("$n pronuncia le parole 'super haven'.", 1, ch, 0, 0, TO_ROOM);
          cast_globe_major_inv(GetMaxLevel(ch),ch,"",SPELL_TYPE_SPELL,ch,0);
          return(TRUE);
        }
      } /* end high level prep spells */
      return(FALSE);
    } /* end prep spells */
  } /* !ch->specials.fighting && !IS_PC(ch) */

  if( !ch->specials.fighting )
    return(FALSE);

  if( !IS_PC(ch) )
  {
    if ((GET_POS(ch) > POSITION_STUNNED) &&
        (GET_POS(ch) < POSITION_FIGHTING))
    {
      if (GET_HIT(ch) > GET_HIT(ch->specials.fighting)/2)
        StandUp(ch);
      else
      {
        StandUp(ch);
        do_flee(ch, "\0", 0);
      }

      return(TRUE);
    }
  }

  if (check_soundproof(ch))
    return(FALSE);

  if (check_nomagic(ch, 0, 0))
    return(FALSE);

  /* Find a dude to to evil things upon ! */

  vict = ch->specials.fighting;

  if (!vict)
    vict = FindVictim(ch);

  if (!vict)
    return(FALSE);

  lspell = number(0,GetMaxLevel(ch)); /* gen number from 0 to level */
  if (!IS_PC(ch))
  {
    lspell+= GetMaxLevel(ch)/5;   /* weight it towards the upper levels of
                                     the mage's range */
  }
  lspell = MIN(GetMaxLevel(ch), lspell);

  /*
  **  check your own problems:
  */

  if (lspell < 1)
    lspell = 1;

  if (IS_AFFECTED(ch, AFF_BLIND) && (lspell > 15))
  {
    act("$n pronuncia le parole 'Let me see the light!'.",
        TRUE, ch, 0, 0, TO_ROOM);
    cast_cure_blind(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, ch, 0);
    return TRUE;
  }

  if (IS_AFFECTED(ch, AFF_BLIND))
    return(FALSE);

  if ((IS_AFFECTED(vict, AFF_SANCTUARY)) && (lspell > 10) &&
      (GetMaxLevel(ch) > (GetMaxLevel(vict))))
  {
    act("$n pronuncia le parole 'Use MagicAway Instant Magic Remover'.",
        1, ch, 0, 0, TO_ROOM);
    cast_dispel_magic(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
    return(FALSE);

  }

  if ((IS_AFFECTED(vict, AFF_FIRESHIELD)) && (lspell > 10) &&
      (GetMaxLevel(ch) > (GetMaxLevel(vict))))
  {
    act("$n pronuncia le parole 'Use MagicAway Instant Magic Remover'.",
        1, ch, 0, 0, TO_ROOM);
    cast_dispel_magic(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
    return(FALSE);

  }

  if (!IS_PC(ch))
  {
    if ((GET_HIT(ch) < (GET_MAX_HIT(ch) / 4)) && (lspell > 28) &&
        !IS_SET(ch->specials.act, ACT_AGGRESSIVE))
    {
      act("$n checks $s watch.", TRUE, ch, 0, 0, TO_ROOM);
      act("$n pronuncia le parole 'Oh my, would you just LOOK at the time!'",
          1, ch, 0, 0, TO_ROOM);

      vict = FindMobDiffZoneSameRace(ch);
      if (vict)
      {
        spell_teleport_wo_error(GetMaxLevel(ch), ch, vict, 0);
        return(TRUE);
      }
      cast_teleport(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, ch, 0);
      return(FALSE);
    }
  }

  if (!IS_PC(ch))
  {
    if ((GET_HIT(ch) < (GET_MAX_HIT(ch) / 4)) && (lspell > 15) &&
        (!IS_SET(ch->specials.act, ACT_AGGRESSIVE)))
    {
      act("$n pronuncia le parole 'Woah! I'm outta here!'",
          1, ch, 0, 0, TO_ROOM);
      cast_teleport(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, ch, 0);
      return(FALSE);
    }
  }


  if  (GET_HIT(ch) > (GET_MAX_HIT(ch) / 2) &&
       !IS_SET(ch->specials.act, ACT_AGGRESSIVE) &&
       GetMaxLevel(vict) < GetMaxLevel(ch) && (number(0,1)))
  {

    /*
     **  Non-damaging case:
     */

    if (((lspell>8) && (lspell<50)) && (number(0,6)==0))
    {
      act("$n pronuncia le parole 'Icky Sticky!'.", 1, ch, 0, 0, TO_ROOM);
      cast_web(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
      return TRUE;
    }

    if (((lspell>5) && (lspell<10)) && (number(0,6)==0))
    {
      act("$n pronuncia le parole 'You wimp'.", 1, ch, 0, 0, TO_ROOM);
      cast_weakness(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
      return TRUE;
    }

    if (((lspell>5) && (lspell<10)) && (number(0,7)==0))
    {
      act("$n pronuncia le parole 'Bippety boppity Boom'.",1,ch,0,0,TO_ROOM);
      cast_armor(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, ch, 0);
      return TRUE;
    }

    if (((lspell>12) && (lspell<20)) && (number(0,7)==0))
    {
      act("$n pronuncia le parole '&#%^^@%*#'.", 1, ch, 0, 0, TO_ROOM);
      cast_curse(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
      return TRUE;
    }

    if (((lspell>10) && (lspell < 20)) && (number(0,5)==0))
    {
      act("$n pronuncia le parole 'yabba dabba do'.", 1, ch, 0, 0, TO_ROOM);
      cast_blindness(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
      return TRUE;
    }

    if (((lspell>8) && (lspell < 40)) && (number(0,5)==0) &&
        (vict->specials.fighting != ch))
    {
      act("$n pronuncia le parole 'You are getting sleepy'.",
          1, ch, 0, 0, TO_ROOM);
      cast_charm_monster(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
      if (IS_AFFECTED(vict, AFF_CHARM))
      {
        char buf[200];

        if (!vict->specials.fighting)
        {
          sprintf(buf, "%s kill %s",
                  GET_NAME(vict), GET_NAME(ch->specials.fighting));
          do_order(ch, buf, 0);
        }
        else
        {
          sprintf(buf, "%s remove all", GET_NAME(vict));
          do_order(ch, buf, 0);
        }
      }
    }

    /*
    **  The really nifty case:
    */
    switch(lspell)
    {
      case 1:
      case 2:
      case 3:
      case 4:
      case 5:
      case 6:
      case 7:
      case 8:
      case 9:
      case 10:
        act("$n pronuncia le parole 'Here boy!'.", 1, ch, 0, 0, TO_ROOM);
        cast_mon_sum1(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
        do_order(ch, "followers guard on", 0);
        return(TRUE);
        break;
      case 11:
      case 12:
      case 13:
        act("$n pronuncia le parole 'Here boy!'.", 1, ch, 0, 0, TO_ROOM);
        cast_mon_sum2(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
        do_order(ch, "followers guard on", 0);
        return(TRUE);
        break;
      case 14:
      case 15:
        act("$n pronuncia le parole 'Here boy!'.", 1, ch, 0, 0, TO_ROOM);
        cast_mon_sum3(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
        do_order(ch, "followers guard on", 0);
        return(TRUE);
        break;
      case 16:
      case 17:
      case 18:
        act("$n pronuncia le parole 'Here boy!'.", 1, ch, 0, 0, TO_ROOM);
        cast_mon_sum4(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
        do_order(ch, "followers guard on", 0);
        return(TRUE);
        break;
      case 19:
      case 20:
      case 21:
      case 22:
        act("$n pronuncia le parole 'Here boy!'.", 1, ch, 0, 0, TO_ROOM);
        cast_mon_sum5(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
        do_order(ch, "followers guard on", 0);
        return(TRUE);
        break;
      case 23:
      case 24:
      case 25:
        act("$n pronuncia le parole 'Here boy!'.", 1, ch, 0, 0, TO_ROOM);
        cast_mon_sum6(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
        do_order(ch, "followers guard on", 0);
        return(TRUE);
        break;
      case 26:
      default:
        act("$n pronuncia le parole 'Here boy!'.", 1, ch, 0, 0, TO_ROOM);
        cast_mon_sum7(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
        do_order(ch, "followers guard on", 0);
        return(TRUE);
        break;
    }
  }
  else
  {
    switch (lspell)
    {
      case 1:
      case 2:
        act("$n pronuncia le parole 'bang! bang! pow!'.", 1, ch, 0, 0, TO_ROOM);
        cast_magic_missile(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
        break;
      case 3:
      case 4:
      case 5:
        act("$n pronuncia le parole 'ZZZZzzzzzzTTTT'.", 1, ch, 0, 0, TO_ROOM);
        cast_shocking_grasp(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
        break;
      case 6:
      case 7:
      case 8:
        if (ch->attackers <= 2)
        {
          act("$n pronuncia le parole 'Icky Sticky!'.", 1, ch, 0, 0, TO_ROOM);
          cast_web(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
          break;
        }
        else
        {
          act("$n pronuncia le parole 'Fwoosh!'.", 1, ch, 0, 0, TO_ROOM);
          cast_burning_hands(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
          break;
        }
      case 9:
      case 10:
        act("$n pronuncia le parole 'SPOOGE!'.", 1, ch, 0, 0, TO_ROOM);
        cast_acid_blast(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
        break;
      case 11:
      case 12:
      case 13:
        if (ch->attackers <= 2)
        {
          act("$n pronuncia le parole 'KAZAP!'.", 1, ch, 0, 0, TO_ROOM);
          cast_lightning_bolt( GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict,
                               0);
          break;
        }
        else
        {
          act("$n pronuncia le parole 'Ice Ice Baby!'.", 1, ch, 0, 0, TO_ROOM);
          cast_ice_storm(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
          break;
        }
      case 14:
      case 15:
        act("$n pronuncia le parole 'Ciao!'.", 1, ch, 0, 0, TO_ROOM);
        cast_teleport(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
        break;
      case 16:
      case 17:
      case 18:
      case 19:
        act("$n pronuncia le parole 'maple syrup'.", 1, ch, 0, 0, TO_ROOM);
        cast_slow(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
        break;
      case 20:
      case 21:
      case 22:
        if (IS_EVIL(ch))
        {
          act("$n pronuncia le parole 'slllrrrrrrpppp'.", 1, ch, 0, 0, TO_ROOM);
          cast_energy_drain(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
          break;
        }
      case 23:
      case 24:
      case 25:
      case 26:
      case 27:
      case 28:
      case 29:
        if (ch->attackers <= 2)
        {
          act("$n pronuncia le parole 'Look! A rainbow!'.", 1, ch, 0, 0, TO_ROOM);
          cast_colour_spray(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
          break;
        }
        else
        {
          act("$n pronuncia le parole 'Get the sensation!'.", 1, ch, 0, 0, TO_ROOM);
          cast_cone_of_cold(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
          break;
        }
      case 30:
      case 31:
      case 32:
      case 33:
      case 34:
      case 35:
        act("$n pronuncia le parole 'Hasta la vista, Baby'.", 1, ch,0,0,TO_ROOM);
        cast_fireball(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
        break;
      case 36:
      case 37:
        act("$n pronuncia le parole 'KAZAP KAZAP KAZAP!'.", 1, ch, 0, 0, TO_ROOM);
        cast_chain_lightn(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
        break;
      case 38:
        act("$n pronuncia le parole 'duhhh'.", 1, ch, 0, 0, TO_ROOM);
        cast_feeblemind(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
        break;
      case 39:
        act("$n pronuncia le parole 'STOP'.", 1, ch, 0, 0, TO_ROOM);
        cast_paralyze(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
        break;
      case 40:
      case 41:
        if (ch->attackers <= 2)
        {
          act("$n pronuncia le parole 'frag'.", 1, ch,0,0,TO_ROOM);
          cast_meteor_swarm(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
          break;
        }
        else
        {
          act("$n pronuncia le parole 'Whew, whata smell!'.", 1, ch,0,0,TO_ROOM);
          cast_incendiary_cloud(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict,
                                0);
          break;
        }
      /* ...  case 50: */
      default:
        if (ch->attackers <= 2)
        {
          act("$n pronuncia le parole 'ZZAAPP!'.", 1, ch,0,0,TO_ROOM);
          cast_disintegrate(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
          break;
        }
        else
        {
          act("$n pronuncia le parole 'Whew, whata smell!'.", 1, ch,0,0,TO_ROOM);
          cast_incendiary_cloud(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
          break;
        }
    }
  }
  return TRUE;
}


int cleric(struct char_data *ch, int cmd, const char *arg, struct char_data *mob, 
           int type)
{
  struct char_data *vict;
  byte lspell, healperc=0;
  
  SetStatus( "Cleric spec procs started", GET_NAME_DESC( ch ) );


  if( type != EVENT_TICK || !AWAKE(ch) )
    return(FALSE);

  if( StandUp(ch) )
    return(TRUE);

  if( check_soundproof(ch) )
    return(FALSE);

  if( check_nomagic( ch, 0, 0 ) )
    return(FALSE);


  if( !ch->specials.fighting )
  {
    if (GET_HIT(ch) < GET_MAX_HIT(ch)-10)
    {
      lspell = GetMaxLevel( ch );
      if ( lspell >= 20)
      {
        act( "$n pronuncia le parole 'Woah! Adesso si` che sto bene!'.", 
             1, ch,0,0,TO_ROOM);
        cast_heal(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, ch, 0);
      }
      else if (lspell > 12)
      {
        act( "$n pronuncia le parole 'Hey! Mi sento decisamente meglio!'.", 1, 
             ch, 0, 0, TO_ROOM);
        cast_cure_critic(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, ch, 0);
      }
      else if (lspell > 8)
      {
        act("$n pronuncia le parole 'Mi sento molto meglio, ora!'.", 1, ch,0,0,
            TO_ROOM);
        cast_cure_serious(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, ch, 0);
      }
      else
      {
        act("$n pronuncia le parole 'Mi sento meglio!'.", 1, ch,0,0,TO_ROOM);
        cast_cure_light(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, ch, 0);
      }
      return TRUE;
    }


    if (!ch->desc)
    {
      /* make sure it is a mob */
      /* low level prep */
      if (!affected_by_spell(ch,SPELL_ARMOR))
      {
        act("$n pronuncia le parole 'protect'.",FALSE,ch,0,0,TO_ROOM);
        cast_armor(GetMaxLevel(ch),ch,"",SPELL_TYPE_SPELL,ch,0);
        return(TRUE);
      }

      if (!affected_by_spell(ch,SPELL_BLESS))
      {
        act("$n pronuncia le parole 'bless'.",FALSE,ch,0,0,TO_ROOM);
        cast_bless(GetMaxLevel(ch),ch,"",SPELL_TYPE_SPELL,ch,0);
        return(TRUE);
      }

      if (!affected_by_spell(ch,SPELL_AID))
      {
        act("$n pronuncia le parole 'aid'.",FALSE,ch,0,0,TO_ROOM);
        cast_aid(GetMaxLevel(ch),ch,"",SPELL_TYPE_SPELL,ch,0);
        return(TRUE);
      }

      if (!affected_by_spell(ch,SPELL_DETECT_MAGIC))
      {
        act("$n pronuncia le parole 'detect magic'.",FALSE,ch,0,0,TO_ROOM);
        cast_detect_magic(GetMaxLevel(ch),ch,"",SPELL_TYPE_SPELL,ch,0);
        return(TRUE);
      }

      if (!affected_by_spell(ch,SPELL_PROTECT_FROM_EVIL) && !IS_EVIL(ch))
      {
        act("$n pronuncia le parole 'anti evil'.",FALSE,ch,0,0,TO_ROOM);
        cast_protection_from_evil(GetMaxLevel(ch),ch,"",SPELL_TYPE_SPELL,ch,0);
        return(TRUE);
      }
      /* end low level prep */

      if (GetMaxLevel(ch)>24)
      {
        /* high level prep */
        if (!affected_by_spell(ch,SPELL_PROT_FIRE))
        {
          act("$n pronuncia le parole 'resist fire'.",FALSE,ch,0,0,TO_ROOM);
          cast_prot_fire(GetMaxLevel(ch),ch,"",SPELL_TYPE_SPELL,ch,0);
          return(TRUE);
        }
        if (!affected_by_spell(ch,SPELL_PROT_COLD))
        {
          act("$n pronuncia le parole 'resist cold'.",FALSE,ch,0,0,TO_ROOM);
          cast_prot_cold(GetMaxLevel(ch),ch,"",SPELL_TYPE_SPELL,ch,0);
          return(TRUE);
        }
        if (!affected_by_spell(ch,SPELL_PROT_ENERGY))
        {
          act("$n pronuncia le parole 'resist energy'.",FALSE,ch,0,0,TO_ROOM);
          cast_prot_energy(GetMaxLevel(ch),ch,"",SPELL_TYPE_SPELL,ch,0);
          return(TRUE);
        }
        if (!affected_by_spell(ch,SPELL_PROT_ELEC))
        {
          act("$n pronuncia le parole 'resist electricity'.",FALSE,ch,0,0,
              TO_ROOM);
          cast_prot_elec(GetMaxLevel(ch),ch,"",SPELL_TYPE_SPELL,ch,0);
          return(TRUE);
        }
      }        /* end high level prep */

      /* low level removes */
      if (affected_by_spell(ch,SPELL_POISON))
      {
        act("$n pronuncia le parole 'remove poison'.",FALSE,ch,0,0,TO_ROOM);
        cast_remove_poison(GetMaxLevel(ch),ch,"",SPELL_TYPE_SPELL,ch,0);
        return(TRUE);
      }
      if (affected_by_spell(ch,SPELL_BLINDNESS))
      {
        act("$n pronuncia le parole 'cure blind'.",FALSE,ch,0,0,TO_ROOM);
        cast_cure_blind(GetMaxLevel(ch),ch,"",SPELL_TYPE_SPELL,ch,0);
        return(TRUE);
      }

      /* end low level removes */

      /* hi level removes */
      if (GetMaxLevel(ch) >24) 
      {
        if (affected_by_spell(ch,SPELL_CURSE))
        {
          act("$n pronuncia le parole 'remove curse'.",FALSE,ch,0,0,TO_ROOM);
          cast_remove_curse(GetMaxLevel(ch),ch,"",SPELL_TYPE_SPELL,ch,0);
          return(TRUE);
        }

        if (affected_by_spell(ch,SPELL_PARALYSIS))
        {
          act("$n pronuncia le parole 'remove paralysis'.",FALSE,ch,0,0,TO_ROOM);
          cast_remove_paralysis(GetMaxLevel(ch),ch,"",SPELL_TYPE_SPELL,ch,0);
          return(TRUE);
        }

      }        /* end hi level removes */


      if (GET_MOVE(ch) < GET_MAX_MOVE(ch)/2)
      {
        act("$n pronuncia le parole 'lemon aid'.",FALSE,ch,0,0,TO_ROOM);
        cast_refresh(GetMaxLevel(ch),ch,"",SPELL_TYPE_SPELL,ch,0);
        return(TRUE);
      }
    } /* ^ was a npc/not a pc! */
  }


  /* Find a dude to to evil things upon ! */
  
  vict = NULL;

  if( ( vict = ch->specials.fighting ) == NULL )
  {
    if( ( vict = FindAHatee( ch ) ) == NULL )
    {
      if( IS_SET( ch->specials.act, ACT_AGGRESSIVE ) )
      {
        vict = FindVictim(ch);
      }
    }
  }
  

  if( !vict )
    return(FALSE);

  /*
   * gen number from 0 to level
   */

  lspell = number( 0, GetMaxLevel( ch ) );
  lspell+= GetMaxLevel(ch) / 5;
  lspell = MIN( GetMaxLevel( ch ), lspell );

  if( lspell < 1 )
    lspell = 1;


  if( GET_HIT( ch ) < GET_MAX_HIT(ch) / 4 && lspell > 31 &&
      !IS_SET( ch->specials.act, ACT_AGGRESSIVE ) )
  {
    act( "$n pronuncia le parole 'Woah! Via di qui!'",
         1, ch, 0, 0, TO_ROOM );
    vict = FindMobDiffZoneSameRace(ch);
    if (vict)
    {
      cast_astral_walk(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
    }
    else
    {
      cast_teleport(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, ch, 0);
    }
    
    return TRUE ;
  }


  /*
   * first -- hit a foe, or help yourself?
   */

  if( ch->points.hit < ch->points.max_hit / 1.3 )
    healperc = 2;
  else if( ch->points.hit < (ch->points.max_hit / 2) )
    healperc = 4;
  else if( ch->points.hit < (ch->points.max_hit / 4) )
    healperc = 6;
  else if( ch->points.hit < (ch->points.max_hit / 8) )
    healperc = 8;

  if( number( 1, healperc + 2 ) <= 3 )
  {
    /* do harm */

    /* call lightning */
    if( OUTSIDE(ch) && weather_info.sky >= SKY_RAINING && lspell >= 15 &&
        number(0,5) == 0 )
    {
      act("$n fischia.",1,ch,0,0,TO_ROOM);
      act("$n pronuncia le parole 'Tuoni e fulmini!'.",1,ch,0,0,TO_ROOM);
      cast_call_lightning(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
      return(TRUE);
    }

    switch(lspell)
    {
     case 1:
     case 2:
     case 3:
      act("$n pronuncia le parole 'Moo ha ha!'.",1,ch,0,0,TO_ROOM);
      cast_cause_light(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
      break;
     case 4:
     case 5:
     case 6:
      act("$n pronuncia le parole 'Hocus Pocus!'.",1,ch,0,0,TO_ROOM);
      cast_blindness(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
      break;
     case 7:
      act("$n pronuncia le parole 'Va-Voom!'.",1,ch,0,0,TO_ROOM);
      cast_dispel_magic(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
      break;
     case 8:
      act("$n pronuncia le parole 'Urgle Blurg'.",1,ch,0,0,TO_ROOM);
      cast_poison(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
      break;
     case 9:
     case 10:
      act("$n pronuncia le parole 'Prendi questo!'.",1,ch,0,0,TO_ROOM);
      cast_cause_critic(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
      break;
     case 11:
      act("$n pronuncia le parole 'Brucia Baby Brucia'.",1,ch,0,0,TO_ROOM);
      cast_flamestrike(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
      break;
     case 13:
     case 14:
     case 15:
     case 16:
      if (!IS_SET(vict->M_immune, IMM_FIRE))
      {
        act("$n pronuncia le parole 'Brucia Baby Brucia'.",1,ch,0,0,TO_ROOM);
        cast_flamestrike(GetMaxLevel(ch),ch,"",SPELL_TYPE_SPELL,vict,0);
      }
      else if (IS_AFFECTED(vict, AFF_SANCTUARY) &&
               ( GetMaxLevel(ch) > GetMaxLevel(vict)))
      {
        act("$n pronuncia le parole 'Va-Voom!'.",1,ch,0,0,TO_ROOM);
        cast_dispel_magic(GetMaxLevel(ch),ch,"",SPELL_TYPE_SPELL,vict,0);
      }
      else
      {
        act("$n pronuncia le parole 'Prendi questo'.",1,ch,0,0,TO_ROOM);
        cast_cause_critic(GetMaxLevel(ch),ch,"",SPELL_TYPE_SPELL, vict, 0);
      }
      break;
      
     case 17:
     case 18:
     case 19:
     default:
      act("$n pronuncia le parole 'Fa male, vero?'.",1,ch,0,0,TO_ROOM);
      cast_harm(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
      break;
    }

    return(TRUE);

  }
  else
  {
    /* do heal */

    if (IS_AFFECTED(ch, AFF_BLIND) && (lspell >= 4) & (number(0,3)==0))
    {
      act("$n pronuncia le parole 'Benem, dammi la luce!'.", 1, ch,0,0,
          TO_ROOM);
      cast_cure_blind( GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, ch, 0);
      return(TRUE);
    }

    if( IS_AFFECTED( ch, AFF_CURSE ) && lspell >= 6 && number( 0, 6 ) == 0 )
    {
      act( "$n pronuncia le parole 'Via questa roba!", 1, ch, 0, 0, TO_ROOM );
      cast_remove_curse(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, ch, 0);
      return(TRUE);
    }

    if (IS_AFFECTED(ch, AFF_POISON) && (lspell >= 5) && (number(0,6)==0))
    {
      act( "$n pronuncia le parole 'Grazie Joy, non sono piu` avvelenato, "
           "ora!'.", 1, ch,0,0,TO_ROOM);
      cast_remove_poison(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, ch, 0);
      return(TRUE);
    }


    switch(lspell)
    {
     case 1:
     case 2:
      if (!affected_by_spell(ch,SPELL_ARMOR))
      {
        act("$n pronuncia le parole 'Abrazak'.",1,ch,0,0,TO_ROOM);
        cast_armor(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, ch, 0);
        break;
      }
     case 3:
     case 4:
     case 5:
      act( "$n pronuncia le parole 'Mi sento meglio, ora!'.", 1, ch,0,0,
           TO_ROOM);
      cast_cure_light(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, ch, 0);
      break;
     case 6:
     case 7:
     case 8:
     case 9:
     case 10:
      act( "$n pronuncia le parole 'Mi sento molto meglio, ora!'.", 1, ch,
           0, 0, TO_ROOM);
      cast_cure_serious(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, ch, 0);
      break;
     case 11:
     case 12:
     case 13:
     case 14:
     case 15:
     case 16:
      act( "$n pronuncia le parole 'Hey! Mi sento veramente meglio!'.", 1,
           ch, 0, 0, TO_ROOM);
      cast_cure_critic(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, ch, 0);
      break;
     case 17:
     case 18: /* heal */
      act( "$n pronuncia le parole 'Woah! Adesso si` che sto bene!'.", 1,
           ch, 0, 0, TO_ROOM);
      cast_heal(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, ch, 0);
      break;
     default:
      if (!affected_by_spell(ch,SPELL_SANCTUARY))
      {
        act("$n pronuncia le parole 'Oooh, bello!'.", 1, ch,0,0,TO_ROOM);
        cast_sanctuary(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, ch, 0);
      }
      else
      {
        act( "$n pronuncia le parole 'Woah! Adesso si` che sto bene!'.", 1,
             ch, 0,0,TO_ROOM);
        cast_heal(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, ch, 0);
      }
      break;

    }

    return(TRUE);
  }
}

int ninja_master(struct char_data *ch, int cmd, const char *arg, struct char_data *mob, int type)
{
  char buf[256];
  const char *n_skills[] = {
    "disarm",   /* No. 245 */
    "doorbash", /* No. 259 */
    "spy",
    "retreat",
    "switch opponents",
    "riding",
    "disguise",
    "climb",
    "\n",
  };
  int percent=0, number=0;
  int charge, sk_num;

  if (!AWAKE(ch))
    return(FALSE);

  if (!cmd) {
    if (ch->specials.fighting) {
      return(fighter(ch, cmd, arg, ch, 0));
    }
    return(FALSE);
  }

  if (!ch->skills) return(FALSE);

  if (check_soundproof(ch)) return(FALSE);

  for(; *arg==' '; arg++); /* ditch spaces */

  if ((cmd==164)||(cmd==170)) {
    if (!arg || (strlen(arg) == 0)) {
      sprintf(buf," disarm  :  %s\n\r",how_good(ch->skills[SKILL_DISARM].learned));
      send_to_char(buf,ch);
      sprintf(buf," doorbash:  %s\n\r",how_good(ch->skills[SKILL_DOORBASH].learned));
      send_to_char(buf,ch);
      sprintf(buf," spy:       %s\n\r",how_good(ch->skills[SKILL_SPY].learned));
      send_to_char(buf,ch);
      sprintf(buf," retreat:   %s\n\r",how_good(ch->skills[SKILL_RETREAT].learned));
      send_to_char(buf,ch);
      sprintf(buf," switch:    %s\n\r",how_good(ch->skills[SKILL_SWITCH_OPP].learned));
      send_to_char(buf,ch);
      sprintf(buf," riding:    %s\n\r",how_good(ch->skills[SKILL_RIDE].learned));
      send_to_char(buf,ch);
      sprintf(buf," disguise:  %s\n\r",how_good(ch->skills[SKILL_DISGUISE].learned));
      send_to_char(buf,ch);
      sprintf(buf, " climb:     %s\n\r",how_good(ch->skills[SKILL_CLIMB].learned));
      send_to_char(buf,ch);
      return(TRUE);
    } else {
      number = old_search_block(arg,0,strlen(arg),n_skills,FALSE);
      send_to_char ("The ninja master says ",ch);
      if (number == -1) {
        send_to_char("'I do not know of this skill.'\n\r", ch);
        return(TRUE);
      }
      charge = GetMaxLevel(ch) * 500;
      switch(number) {
      case 0:
      case 1:
        sk_num = SKILL_DISARM;
        if (!HasClass(ch, CLASS_WARRIOR)) {
          send_to_char
            ("'You do not possess the necessary fighting skills'\n\r",ch);
          return(TRUE);
        }
        break;
      case 2:
        sk_num = SKILL_DOORBASH;
        if( !HasClass( ch, CLASS_WARRIOR ) && !HasClass( ch, CLASS_PALADIN ) )
        {
          send_to_char
            ("'You do not possess the necessary fighting skills'\n\r",ch);
          return(TRUE);
        }
        break;
      case 3:
        sk_num = SKILL_SPY;
        if (!HasClass(ch, CLASS_THIEF)) {
          send_to_char
            ("'You do not possess the necessary thieving skills'\n\r",ch);
          return(TRUE);
        }
        break;
      case 4:
        sk_num = SKILL_RETREAT;
        if (!HasClass(ch, CLASS_THIEF)) {
          send_to_char
            ("'I only teach this skill to thieves'\n\r",ch);
          return(TRUE);
        }
        break;
      case 5:
        sk_num = SKILL_SWITCH_OPP;
        if (!HasClass(ch, CLASS_WARRIOR)) {
          send_to_char
            ("'I only teach this to fighters'\n\r",ch);
          return(TRUE);
        }
        break;
      case 6:
        sk_num = SKILL_RIDE;
        break;
      case 7:
        sk_num = SKILL_DISGUISE;
        if (!HasClass(ch, CLASS_THIEF)) {
          send_to_char
            ("'I only teach this skill to thieves'\n\r",ch);
          return(TRUE);
        }
        break;
      case 8:
        sk_num = SKILL_CLIMB;
        if(!HasClass(ch, CLASS_THIEF)) {
          send_to_char("'I only teach this skill to thieves'\n\r", ch);
          return(TRUE);
        }
        break;
      default:
        mudlog( LOG_SYSERR, "Strangeness in ninjamaster (%d)", number);
        send_to_char("'Ack!  I feel faint!'\n\r", ch);
        return(TRUE);
      }
    }

    if (sk_num == SKILL_HUNT || sk_num == SKILL_DODGE) {
      if (ch->skills[sk_num].learned >= 95) {
        send_to_char
          ("'You are a master of this art, I can teach you no more.'\n\r",ch);
        return(TRUE);
      }
    } else {
      if (ch->skills[sk_num].learned > 45) {
        send_to_char("'You must learn from practice and experience now.'\n\r", ch);
        return TRUE;
      }
    }

    if (GET_GOLD(ch) < charge){
      send_to_char
        ("'Ah, but you do not have enough money to pay.'\n\r",ch);
      return(TRUE);
    }

    if (ch->specials.spells_to_learn <= 0) {
      send_to_char
        ("'You must first use the knowledge you already have.'\n\r",ch);
      return(FALSE);
    }

    GET_GOLD(ch) -= charge;
    send_to_char("'We will now begin.'\n\r",ch);
    ch->specials.spells_to_learn--;

    if (!IS_SET(ch->skills[sk_num].flags, SKILL_KNOWN))
     SET_BIT(ch->skills[sk_num].flags, SKILL_KNOWN);

    percent = ch->skills[sk_num].learned + int_app[ (int)GET_INT(ch) ].learn;
    ch->skills[sk_num].learned = MIN(95, percent);

    if (ch->skills[sk_num].learned >= 95) {
      send_to_char("'You are now a master of this art.'\n\r", ch);
      return(TRUE);
    }
  }
  return(FALSE);
}

int RepairGuy( struct char_data *ch, int cmd, const char *arg, struct char_data *mob, int type)
{
  char obj_name[80], vict_name[80], buf[MAX_INPUT_LENGTH];
  int cost, ave;
  struct char_data *vict;
  struct obj_data *obj;
  int (*rep_guy)( struct char_data *, int, const char *, struct char_data *, int );
  /* special procedure for this mob/obj       */

  if (!AWAKE(ch))
    return(FALSE);

  rep_guy = RepairGuy;

  if (IS_NPC(ch)) {
    if(cmd == 72) {
      arg=one_argument(arg,obj_name);
      if (!*obj_name)
        return(FALSE);
      if (!(obj = get_obj_in_list_vis(ch, obj_name, ch->carrying)))
        return(FALSE);
      arg=one_argument(arg, vict_name);
      if(!*vict_name)
        return(FALSE);
      if (!(vict = get_char_room_vis(ch, vict_name)))
        return(FALSE);
      if (!IS_NPC(vict))
        return(FALSE);
      if (mob_index[vict->nr].func == CASTVF rep_guy) {
        send_to_char("Nah, you really wouldn't want to do that.",ch);
      return(TRUE);
      }
    } else
    return(FALSE);
  }


  if (cmd == 72) { /* give */
    /* determine the correct obj */
    arg=one_argument(arg,obj_name);
    if (!*obj_name) {
      send_to_char("Give what?\n\r",ch);
      return(FALSE);
    }
    if (!(obj = get_obj_in_list_vis(ch, obj_name, ch->carrying))) {
      send_to_char("Give what?\n\r",ch);
      return(TRUE);
    }
    arg=one_argument(arg, vict_name);
    if(!*vict_name)        {
      send_to_char("To who?\n\r",ch);
      return(FALSE);
    }
    if (!(vict = get_char_room_vis(ch, vict_name)))        {
      send_to_char("To who?\n\r",ch);
      return(FALSE);
    }

    if (vict->specials.fighting) {
         send_to_char("Not while they are fighting!\n\r",ch);
        return(TRUE);
      }

    /* the target is the repairman, or an NPC */
    if (!IS_NPC(vict))
      return(FALSE);

    if (mob_index[vict->nr].func == CASTVF rep_guy) {
      /* we have the repair guy, and we can give him the stuff */
      act("You give $p to $N.",TRUE,ch,obj,vict,TO_CHAR);
      act("$n gives $p to $N.",TRUE,ch,obj,vict,TO_ROOM);
    } else {
      return(FALSE);
    }

    act("$N looks at $p.", TRUE, ch, obj, vict, TO_CHAR);
    act("$N looks at $p.", TRUE, ch, obj, vict, TO_ROOM);

    /* make all the correct tests to make sure that everything is kosher */

    if (ITEM_TYPE(obj) == ITEM_ARMOR && obj->obj_flags.value[1] > 0) {
      if (obj->obj_flags.value[1] > obj->obj_flags.value[0]) {
        /* get the value of the object */
        cost = obj->obj_flags.cost;
        /* divide by value[1]   */
        cost /= obj->obj_flags.value[1];
        /* then cost = difference between value[0] and [1] */
        cost *= (obj->obj_flags.value[1] - obj->obj_flags.value[0]);
        if (GetMaxLevel(vict) > 25) /* super repair guy */
          cost *= 2;
        if (cost > GET_GOLD(ch)) {
          if (check_soundproof(ch)) {
            act("$N shakes $S head.\n\r",
              TRUE, ch, 0, vict, TO_ROOM);
            act("$N shakes $S head.\n\r",
              TRUE, ch, 0, vict, TO_CHAR);
          } else {
                act("$N says 'I'm sorry, you don't have enough money.'",
                 TRUE, ch, 0, vict, TO_ROOM);
             act("$N says 'I'm sorry, you don't have enough money.'",
                 TRUE, ch, 0, vict, TO_CHAR);
           }
        } else {
          GET_GOLD(ch) -= cost;

          sprintf(buf, "You give $N %d coins.",cost);
          act(buf,TRUE,ch,0,vict,TO_CHAR);
          act("$n gives some money to $N.",TRUE,ch,obj,vict,TO_ROOM);

          /* fix the armor */
          act("$N fiddles with $p.",TRUE,ch,obj,vict,TO_ROOM);
          act("$N fiddles with $p.",TRUE,ch,obj,vict,TO_CHAR);
          if (GetMaxLevel(vict) > 25) {
            obj->obj_flags.value[0] = obj->obj_flags.value[1];
          } else {
            ave = MAX(obj->obj_flags.value[0],
                      (obj->obj_flags.value[0] +
                       obj->obj_flags.value[1] ) /2);
            obj->obj_flags.value[0] = ave;
            obj->obj_flags.value[1] = ave;
          }
          if (check_soundproof(ch)) {
            act("$N smiles broadly.",TRUE,ch,0,vict,TO_ROOM);
            act("$N smiles broadly.",TRUE,ch,0,vict,TO_CHAR);
          } else {
               act("$N says 'All fixed.'",TRUE,ch,0,vict,TO_ROOM);
            act("$N says 'All fixed.'",TRUE,ch,0,vict,TO_CHAR);
          }
        }
      } else {
        if (check_soundproof(ch)) {
          act("$N shrugs.",
               TRUE,ch,0,vict,TO_ROOM);
          act("$N shrugs.",
               TRUE,ch,0,vict,TO_CHAR);
        }else{
           act("$N says 'Your armor looks fine to me.'",
               TRUE,ch,0,vict,TO_ROOM);
           act("$N says 'Your armor looks fine to me.'",
               TRUE,ch,0,vict,TO_CHAR);
        }
      }
    } else {
      if (GetMaxLevel(vict) < 25 || (ITEM_TYPE(obj)!=ITEM_WEAPON)) {
        if (check_soundproof(ch)) {
          act("$N shakes $S head.\n\r",
              TRUE, ch, 0, vict, TO_ROOM);
          act("$N shakes $S head.\n\r",
              TRUE, ch, 0, vict, TO_CHAR);
        } else {
          if (ITEM_TYPE(obj) != ITEM_ARMOR) {
            act("$N says 'That isn't armor.'",TRUE,ch,0,vict,TO_ROOM);
            act("$N says 'That isn't armor.'",TRUE,ch,0,vict,TO_CHAR);
          } else {
            act("$N says 'I can't fix that...'", TRUE, ch, 0, vict, TO_CHAR);
            act("$N says 'I can't fix that...'", TRUE, ch, 0, vict, TO_ROOM);
          }
        }
      } else {

        struct obj_data *pNew;

/* weapon repair.  expensive!   */
        cost = obj->obj_flags.cost;
        pNew = read_object(obj->item_number, REAL);
        if (obj->obj_flags.value[2])
          cost /= obj->obj_flags.value[2];

        cost *= (pNew->obj_flags.value[2] - obj->obj_flags.value[2]);

        if (cost > GET_GOLD(ch)) {
          if (check_soundproof(ch)) {
            act("$N shakes $S head.\n\r",
              TRUE, ch, 0, vict, TO_ROOM);
            act("$N shakes $S head.\n\r",
              TRUE, ch, 0, vict, TO_CHAR);
          } else {
                act("$N says 'I'm sorry, you don't have enough money.'",
                 TRUE, ch, 0, vict, TO_ROOM);
             act("$N says 'I'm sorry, you don't have enough money.'",
                 TRUE, ch, 0, vict, TO_CHAR);
             extract_obj(pNew);
           }
        } else {
          GET_GOLD(ch) -= cost;

          sprintf(buf, "You give $N %d coins.",cost);
          act(buf,TRUE,ch,0,vict,TO_CHAR);
          act("$n gives some money to $N.",TRUE,ch,obj,vict,TO_ROOM);

          /* fix the weapon */
          act("$N fiddles with $p.",TRUE,ch,obj,vict,TO_ROOM);
          act("$N fiddles with $p.",TRUE,ch,obj,vict,TO_CHAR);

          obj->obj_flags.value[2] = pNew->obj_flags.value[2];
          extract_obj(pNew);

          if (check_soundproof(ch)) {
            act("$N smiles broadly.",TRUE,ch,0,vict,TO_ROOM);
            act("$N smiles broadly.",TRUE,ch,0,vict,TO_CHAR);
          } else {
               act("$N says 'All fixed.'",TRUE,ch,0,vict,TO_ROOM);
            act("$N says 'All fixed.'",TRUE,ch,0,vict,TO_CHAR);
          }
        }



      }
    }

    act("$N gives you $p.",TRUE,ch,obj,vict,TO_CHAR);
    act("$N gives $p to $n.",TRUE,ch,obj,vict,TO_ROOM);
    return(TRUE);
  } else {
    if (cmd) return FALSE;
    return(fighter(ch, cmd, arg,mob,type));
  }
}


int Samah( struct char_data *ch, int cmd, const char *arg, struct char_data *mob, int type)
{
  char *p, buf[256];
  struct char_data *Sammy;  /* Samah's own referent pointer */
  struct char_data *t, *t2, *t3;
  int purge_nr;
  struct room_data *rp;

  rp = real_roomp(ch->in_room);
  if (!rp) return(FALSE);

  if (cmd) {

    if (GET_RACE(ch) == RACE_SARTAN || GET_RACE(ch) == RACE_PATRYN ||
        GetMaxLevel(ch) == BIG_GUY)
      return(FALSE);

    Sammy = (struct char_data *)FindMobInRoomWithFunction(ch->in_room, Samah);

    for (;*arg==' ';arg++); /* skip whitespace */
    strcpy(buf, arg);

    if (cmd == 207) { /* recite */
      act("$n glares at you", FALSE, Sammy, 0, ch, TO_VICT);
      act("$n glares at $N", FALSE, Sammy, 0, ch, TO_NOTVICT);
      p = (char *)strtok(buf, " ");
      if (strncmp("recall", p, strlen(p))==0) {
        act("$n says 'And just where do you think you're going, Mensch?", FALSE, Sammy, 0, 0, TO_ROOM);
        return(TRUE);
      }
    } else if (cmd == 84) { /* cast */
      act("$n glares at you", FALSE, Sammy, 0, ch, TO_VICT);
      act("$n glares at $N", FALSE, Sammy, 0, ch, TO_NOTVICT);
      /* we use strlen(p)-1 because if we use the full length, there is
         the obligatory ' at the end.  We must ignore this ', and get
         on with our lives */
      p = (char *)strtok(buf, " ");
      if (strncmp("'word of recall'", p, strlen(p)-1)==0) {
        act("$n says 'And just where do you think you're going, Mensch?", FALSE, Sammy, 0, 0, TO_ROOM);
        return(TRUE);
      } else if (strncmp("'astral walk'", p, strlen(p)-1)==0) {
        act("$n says 'Do you think you can astral walk in and out of here like the ", FALSE, Sammy, 0, 0, TO_ROOM);
        act("wind,...Mensch?'", FALSE, Sammy, 0, 0, TO_ROOM);
        return(TRUE);
      } else if (strncmp("'teleport'", p, strlen(p)-1)==0) {
        act("$n says 'And just where do you think you're going, Mensch?", FALSE, Sammy, 0, 0, TO_ROOM);
        return(TRUE);
      } else if (strncmp("'polymorph'", p, strlen(p)-1)==0) {
        act("$n says 'I like you the way you are...Mensch.", FALSE, Sammy, 0, 0, TO_ROOM);
        return(TRUE);
      }
    } else if (cmd == 17 || cmd == 169) { /* say */
      act("$n glares at you", FALSE, Sammy, 0, ch, TO_VICT);
      act("$n glares at $N", FALSE, Sammy, 0, ch, TO_NOTVICT);
      act("$n says 'Mensch should be seen, and not heard'", FALSE, Sammy, 0, 0, TO_ROOM);
      return(TRUE);
    } else if (cmd == 40 || cmd == 214 || cmd == 177) { /* emote */
      act("$n glares at you", FALSE, Sammy, 0, ch, TO_VICT);
      act("$n glares at $N", FALSE, Sammy, 0, ch, TO_NOTVICT);
      act("$n says 'Cease your childish pantonimes, Mensch.'", FALSE, Sammy, 0, 0, TO_ROOM);
      return(TRUE);
    } else if (cmd == 19 || cmd == 18 || cmd == 83) { /* say, shout whisp */
      act("$n glares at you", FALSE, Sammy, 0, ch, TO_VICT);
      act("$n glares at $N", FALSE, Sammy, 0, ch, TO_NOTVICT);
      act("$n says 'Speak only when spoken to, Mensch.'", FALSE, Sammy, 0, 0, TO_ROOM);
      return(TRUE);
    } else if (cmd == 86) {  /* ask */
      act("$n glares at you", FALSE, Sammy, 0, ch, TO_VICT);
      act("$n glares at $N", FALSE, Sammy, 0, ch, TO_NOTVICT);
      act("$n says 'Your ignorance is too immense to be rectified at this time. Mensch.'", FALSE, Sammy, 0, 0, TO_ROOM);
      return(TRUE);
    } else if (cmd == 87 || cmd == 46) {  /* order, force */
      act("$n glares at you", FALSE, Sammy, 0, ch, TO_VICT);
      act("$n glares at $N", FALSE, Sammy, 0, ch, TO_NOTVICT);
      act("$n says 'I'll be the only one giving orders here, Mensch'", FALSE, Sammy, 0, 0, TO_ROOM);
      return(TRUE);
    } else if (cmd == 151) {
      act("$n glares at you", FALSE, Sammy, 0, ch, TO_VICT);
      act("$n glares at $N", FALSE, Sammy, 0, ch, TO_NOTVICT);
      act("$n says 'Cease this cowardly behavior, Mensch'", FALSE, Sammy, 0, ch, TO_ROOM);
      return(TRUE);
    } else if (cmd == 63) {
      act("$n glares at you", FALSE, Sammy, 0, ch, TO_VICT);
      act("$n glares at $N", FALSE, Sammy, 0, ch, TO_NOTVICT);
      act("$n says 'Pay attention when I am speaking, Mensch'", FALSE, Sammy, 0, ch, TO_ROOM);
      return(TRUE);
    }
  } else {
    if (ch->specials.fighting) {
    } else {
      /*
        check for followers in the room
        */
      for (t = rp->people; t; t= t->next_in_room) {
        if (IS_NPC(t) && !IS_PC(t) && t->master && t != ch &&
            t->master != ch) {
          break;
        }
      }
      if (t) {
        act("$n says 'What is $N doing here?'", FALSE, ch, 0, t, TO_ROOM);
        act("$n makes a magical gesture", FALSE, ch, 0, 0, TO_ROOM);
        purge_nr = t->nr;
        for (t2 = rp->people; t2; t2 = t3) {
          t3 = t2->next_in_room;
          if (t2->nr == purge_nr && !IS_PC(t2)) {
            act("$N, looking very surprised, quickly fades out of existence.", FALSE, ch, 0, t2, TO_ROOM);
            extract_char(t2);
          }
        }
      } else {
        /*
          check for polymorphs in the room
          */
        for (t = rp->people; t; t= t->next_in_room) {
          if (IS_NPC(t) && IS_PC(t)) {  /* ah.. polymorphed :-) */
            /*
              I would like to digress at this point, and state that
              I feel that George Bush is an incompetent fool.
              Thank you.
              */
            act("$n glares at $N", FALSE, ch, 0, t, TO_NOTVICT);
            act("$n glares at you", FALSE, ch, 0, t, TO_VICT);
            act("$n says 'Seek not to disguise your true form from me...Mensch.", FALSE, ch, 0, t, TO_ROOM);
            act("$n traces a small rune in the air", FALSE, ch, 0, 0, TO_ROOM);
            act("$n has forced you to return to your original form!", FALSE, ch, 0, t, TO_VICT);
            do_return(t, "", 1);
            return(TRUE);
          }
        }
      }
    }
  }
  return(FALSE);

}



#define GIVE 72
#define GAIN 243

int MakeQuest( struct char_data *ch, struct char_data *gm, int iClass,
               const char *arg, int cmd)
{
  char obj_name[50], vict_name[50];
  struct char_data *vict;
  struct obj_data *obj;

  extern struct QuestItem QuestList[4][IMMORTAL];

#if EASY_LEVELS
  if (GET_LEVEL(ch, iClass) > 0) {  /* for now.. so as not to give it away */
    if (cmd == GAIN) {
      GainLevel(ch, iClass);
      return(TRUE);
    }
    return(FALSE);
  }
#endif

  if( cmd == GIVE )
  {
    arg = one_argument(arg,obj_name);
    arg = one_argument(arg, vict_name);
    if( !( obj = get_obj_in_list_vis( ch, obj_name, ch->carrying ) ) )
    {
      send_to_char( "You do not seem to have anything like that.\n\r", ch );
      return TRUE;
    }
    if( !( vict = get_char_room_vis( ch, vict_name ) ) )
    {
      send_to_char( "No one by that name around here.\n\r", ch );
      return TRUE;
    }
    if( vict == gm )
    {
      if( obj->item_number >= 0 && obj_index[obj->item_number].iVNum ==
          QuestList[ iClass ][ (int)GET_LEVEL(ch, iClass) ].item )
      {
        act( "$n graciously takes your gift of $p", FALSE, gm, obj, ch,
             TO_VICT );
        obj_from_char(obj);
        extract_obj(obj);
        GainLevel(ch, iClass);
        return(TRUE);
      }
      else
      {
        act( "$n shakes $s head", FALSE, gm, 0, 0, TO_ROOM );
        act( "$n says 'That is not the item which i desire'", FALSE, gm, 0, 0,
             TO_ROOM );
        return FALSE;
      }
    }
    else
    {
      return(FALSE);
    }
  }
  else if (cmd == GAIN)
  {
    if( GET_EXP(ch) < titles[iClass][ (int)GET_LEVEL( ch, iClass ) + 1 ].exp )
    {
      send_to_char("You are not yet ready to gain\n\r", ch);
      return(FALSE);
    }

    if( GET_LEVEL( ch, iClass ) < 10 )
    {
      GainLevel(ch, iClass);
      return(TRUE);
    }

    if( QuestList[ iClass ][ (int)GET_LEVEL(ch, iClass) ].item )
    {
      act( "$n shakes $s head", FALSE, gm, 0, 0, TO_ROOM );
      act( "$n tells you 'First you must prove your mastery of knowledge'",
           FALSE, gm, 0, ch, TO_VICT);
      act( "$n tells you 'Give to me the item that answers this riddle'",
           FALSE, gm, 0, ch, TO_VICT);
      act( "$n tells you 'And you shall have your level'\n\r",
           FALSE, gm, 0, ch, TO_VICT);
      send_to_char( QuestList[ iClass ][ (int)GET_LEVEL(ch, iClass) ].where, ch );
      send_to_char( "\n\rGood luck", ch );
/*
  fix to handle limited items:
  Dunno how it will turn out.. but hopefully it should be ok.
*/
      if( obj_index[ real_object( QuestList[ iClass ]
          [ (int)GET_LEVEL( ch, iClass ) ].item ) ].number > 5 &&
          GET_LEVEL( ch, iClass ) < 40 )
        obj_index[ real_object( QuestList[ iClass ]
                 [ (int)GET_LEVEL( ch, iClass ) ].item ) ].number = 0;

      return(FALSE);
    }
    else
    {
      GainLevel(ch, iClass);
      return(TRUE);
    }
  }
  return(FALSE);
}


int AbyssGateKeeper( struct char_data *ch, int cmd, const char *arg, struct char_data *mob, int type)
{

  if (cmd || !AWAKE(ch))
    return(FALSE);

  if (!cmd) {
    if (ch->specials.fighting) {
      fighter(ch, cmd, arg,mob,type);
    }
  }
  else if (cmd == 5)
  {
    send_to_char( "The gatekeeper shakes his head, and blocks your way.\n\r", 
                  ch );
    act( "The guard shakes his head, and blocks $n's way.",
         TRUE, ch, 0, 0, TO_ROOM );
    return(TRUE);
  }
  return(FALSE);
}

bool IsCharInRange( char_data *pChar, char_data *pTarget, int iRange );

int creeping_death( struct char_data *ch, int cmd, const char *arg,
                    struct char_data *mob, int type )
{
  struct char_data *t, *next;
  struct room_data *rp;
  struct obj_data *co, *o;


  if( type != EVENT_TICK )
    return(FALSE);

  if (check_peaceful(ch,0))
  {
    act( "$n si disperde. Tiri un sospiro di sollievo.", FALSE, ch,
         0, 0, TO_ROOM);
    extract_char(ch);

    return(TRUE);
  }
  else if( ch->specials.fighting )
  {  /* kill */

    t = ch->specials.fighting;
    if( t->in_room == ch->in_room && !IS_IMMORTAL( t ) && !IsVeggie( t ) &&
        !IsUndead( t ) && !WeaponImmune( t ))
    {
      act("$N viene avvolt$B da $n!", FALSE, ch, 0, t, TO_NOTVICT);
      act( "$N viene mangiat$B viv$B dalle centinaia di piccole fauci di $n.",
           FALSE, ch, 0, t, TO_NOTVICT);
      act( "Vieni avvolt$B da $n e mangiat$B viv$B da centinaia di piccole "
           "fauci.", FALSE, ch, 0, t, TO_VICT);
      GET_HIT(ch) -= GET_HIT(t);
      if( IS_PC( t ) )
      {
        char szBuffer[ 256 ];
        
        sprintf( szBuffer, "%s e` stat%s uccis%s da %s a %s\n\r",
                 GET_NAME( t ), SSLF( t ), SSLF( t ),
                 ch->player.short_descr, t->in_room > -1 ?
                 (real_roomp( t->in_room ) )->name : "Nowhere" );
        send_to_all( szBuffer );
      }
      die( t, 0 );
      /* find the corpse and destroy it */
      rp = real_roomp(ch->in_room);
      if (!rp) 
        return(FALSE);
      for (co = rp->contents; co; co = co->next_content)
      {
        if (IS_CORPSE(co))
        {  /* assume 1st corpse is victim's */
          while (co->contains)
          {
            o = co->contains;
            obj_from_obj(o);
            obj_to_room(o, ch->in_room);
          }
          extract_obj(co);  /* remove the corpse */
        }
      }
    }
    if (GET_HIT(ch) < 0)
    {
      act("$n si disperde. Tiri un sospiro di sollievo.", FALSE, ch,
          0, 0, TO_ROOM);
      extract_char(ch);

      return(TRUE);
    }
    return(TRUE);
  }
  else if( mob->act_ptr && 
           IsCharInRange( mob, (char_data *)mob->act_ptr, 3 ) )
  {
    /* make everyone with any brains flee */
    for( t = real_roomp(ch->in_room)->people; t; t = next )
    {
      next = t->next_in_room;
      if (t != ch)
      {
        if( ( !IS_PC( t ) || !IS_SET( t->specials.act, PLR_NOHASSLE ) ) &&
            CAN_SEE( t, mob ) && !saves_spell( t, SAVING_PETRI ) )
        {
          do_flee(t, "\0", 0);
        }
      }
    }

    bool bFound = FALSE;
    /* find someone in the room to flay */
    for( t = real_roomp(ch->in_room)->people; t; t = next )
    {
      next = t->next_in_room;
      if( !IS_IMMORTAL(t) && t != ch && CAN_SEE( mob, t ) && !IsVeggie( t ) &&
          !IsUndead( t ) && !WeaponImmune( t ) )
      {
        bFound = TRUE;
        act("$N viene avvolt$B da $n!", FALSE, ch, 0, t, TO_NOTVICT);
        act( "$N viene mangiat$B viv$B dalle centinaia di piccole fauci "
             "di $n.", FALSE, ch, 0, t, TO_NOTVICT);
        act( "Vieni avvolt$B da $n e mangiat$B viv$B da centinaia di piccole "
             "fauci.", FALSE, ch, 0, t, TO_VICT);
        GET_HIT(ch) -= GET_HIT(t);
        if( IS_PC( t ) )
        {
          char szBuffer[ 256 ];
        
          sprintf( szBuffer, "%s e` stat%s uccis%s da %s a %s\n\r",
                   GET_NAME( t ), SSLF( t ), SSLF( t ),
                   ch->player.short_descr, t->in_room > -1 ?
                   (real_roomp( t->in_room ) )->name : "Nowhere" );
          send_to_all( szBuffer );
        }
        die( t, 0 );
        /* find the corpse and destroy it */
        rp = real_roomp(ch->in_room);
        if (!rp)
          return(FALSE);
        for (co = rp->contents; co; co = co->next_content)
        {
          if (IS_CORPSE(co))
          {  /* assume 1st corpse is victim's */
            while (co->contains)
            {
              o = co->contains;
              obj_from_obj(o);
              obj_to_room(o, ch->in_room);
            }
            extract_obj(co);  /* remove the corpse */
          }
        }
        
        if( GET_HIT(ch) < 0 )
        {
          act( "$n si disperde. Tiri un sospiro di sollievo.", FALSE, ch,
               0, 0, TO_ROOM);
          extract_char(ch);
          return(TRUE);
        }
        return TRUE;
        // break;  /* end the loop */
      }
    }
    /* the generic is the direction of travel */

    if( !bFound )
    {
      if( ch->generic < 1 || ch->generic > 6 ||
          !ValidMove( ch, ch->generic - 1 ) )
      {
        act( "$n si disperde. Tiri un sospiro di sollievo.", FALSE, ch,
             0, 0, TO_ROOM);
        
        extract_char(ch);
        
        return TRUE;
      }
      else
      {
        do_move(ch, "\0", ch->generic);
        return TRUE;
      }
    }
  }
  else
  {
    act( "$n si disperde. Tiri un sospiro di sollievo.", FALSE, ch,
         0, 0, TO_ROOM);
        
    extract_char(ch);
    
    return TRUE;
  }
  
  return FALSE;
}

#if 0
/*
   shanty town kids
*/
int shanty_town_kids( struct char_data *ch, int cmd, const char *arg, struct char_data *mob, int type)
{

  if (!AWAKE(ch)) return(FALSE);


  /*
    harrass low levellers.
  */
  if (cmd >= 1 && cmd <= 6) {
    if (GetMaxLevel(ch) <= 5 && number(0,2)==0) {
      act("A street kid sticks out a foot and trips you as you try to leave",
          FALSE, ch, 0, 0, TO_CHAR);
      act("A street kid sticks out a foot and trips $n",
          FALSE, ch, 0, 0, TO_ROOM);
      GET_POS(ch) = POSITION_SITTING;
      act("The street kid laughs at you", FALSE,ch, 0, 0, TO_CHAR);
      act("The street kid laughs at $n", FALSE,ch, 0, 0, TO_ROOM);
    }
  }

  /*
    steal from mid-levellers
  */


  /*
    backstab high levellers
    */


  if (cmd) {


  }

  if (ch->specials.fighting) {
    act("$N runs between $n's legs", FALSE, ch->specials.fighting, 0, ch, TO_ROOM);
    act("$N runs between your legs", FALSE, ch->specials.fighting, 0, ch, TO_CHAR);
    vict = ch->specials.fighting;
    stop_fighting(ch);
    stop_fighting(vict);
  }

}

#endif

void  Submit(struct char_data *ch, struct char_data *t)
{
  char buf[200];

  switch(number(1,5)) {
  case 1:
    sprintf(buf, "bow %s", GET_NAME(t));
    command_interpreter(ch, buf);
    break;
  case 2:
    sprintf(buf, "smile %s", GET_NAME(t));
    command_interpreter(ch, buf);
    break;
  case 3:
    sprintf(buf, "wink %s", GET_NAME(t));
    command_interpreter(ch, buf);
    break;
  case 4:
    sprintf(buf, "wave %s", GET_NAME(t));
    command_interpreter(ch, buf);
    break;
  default:
    act("$n inchina la testa verso te", 0, ch, 0, t, TO_VICT);
    act("$n inchina la testa verso $N", 0, ch, 0, t, TO_NOTVICT);
    break;
  }
}

void  SayHello(struct char_data *ch, struct char_data *t)
{
  char buf[200];

  switch(number(1,10)) {
  case 1:
    do_say(ch, "Buona fortuna avventuriero",0);
    break;
  case 2:
    if (t->player.sex == SEX_FEMALE)
      do_say(ch, "Buon giorno, mia Signora",0);
    else
      do_say(ch, "Buon giorno, mio Signore", 0);
    break;
  case 3:
    if (t->player.sex == SEX_FEMALE)
      do_say(ch, "Buona giornata a te, gentile Signora",0);
    else
      do_say(ch, "Buona giornata a te, gentile Signore", 0);
    break;
  case 4:
    if (t->player.sex == SEX_FEMALE)
      sprintf(buf, "Fate spazio! Fate spazio alla signora %s!", GET_NAME(t));
    else
      sprintf(buf, "Fate spazio! Fate spazio al signor %s!", GET_NAME(t));
    do_say(ch, buf, 0);
    break;
  case 5:
    do_say(ch, "Possano le profezie sorriderti",0);
    break;
  case 6:
    do_say(ch, "E` un piacere rivederla ancora.",0);
    break;
  case 7:
    if (t->player.sex == SEX_FEMALE)
      do_say( ch, "Lei e` sempre la benvenuta.", 0 );
    else
      do_say( ch, "Lei e` sempre il benvenuto.",0);
    break;
  case 8:
    if (t->player.sex == SEX_FEMALE)
      do_say(ch, "Mia signora, accetti i miei omaggi",0);
    else
      do_say(ch, "Mio signore, accetti i miei omaggi",0);
    break;
  case 9:
    if (time_info.hours > 6 && time_info.hours < 12)
      sprintf(buf, "Buon giorno, %s", GET_NAME(t));
    else if (time_info.hours >=12 && time_info.hours < 20)
      sprintf(buf, "Buon pomeriggio, %s", GET_NAME(t));
    else if (time_info.hours >= 20 && time_info.hours <= 24)
      sprintf(buf, "Buona sera, %s", GET_NAME(t));
    else
      sprintf(buf, "Sveglio per una caccia notturna, %s?\n", GET_NAME(t));
    do_say(ch, buf, 0);
    break;
  case 10:{
    char buf2[80];
    if (time_info.hours < 6)
      strcpy(buf2,"a sera");
    else if (time_info.hours < 12)
      strcpy(buf2, "a mattina");
    else if (time_info.hours < 20)
      strcpy(buf2, "o pomeriggio");
    else strcpy(buf2, "a notte");

    switch(weather_info.sky) {
    case SKY_CLOUDLESS:
      sprintf(buf, "Bel tempo abbiamo quest%s, non e` vero, %s ?",
              buf2, GET_NAME(t));
    case SKY_CLOUDY:
      sprintf(buf, "Quest%s si fanno ottime passeggiate, %s.", buf2, GET_NAME(t));
      break;
    case SKY_RAINING:
      sprintf(buf, "Spero che la pioggia di quest%s porti il sereno.", buf2 );
      break;
    case SKY_LIGHTNING:
      sprintf(buf, "Che brutt%s, %s!",
              buf2, GET_NAME(t));
      break;
    default:
      sprintf(buf, "Piacevole quest%s, non credi ?", buf2);
      break;
    }
    do_say(ch, buf, 0);
  }
    break;
  }
}


void GreetPeople(struct char_data *ch)
{
  struct char_data *tch;

  if (!IS_SET(ch->specials.act, ACT_GREET)) {
    for (tch=real_roomp(ch->in_room)->people; tch; tch = tch->next_in_room) {
      if (!IS_NPC(tch) && !number(0,8)) {
        if (tch && GetMaxLevel(tch) > GetMaxLevel(ch)) {
        if (CAN_SEE(ch,tch))  {
           Submit(ch, tch);
           SayHello(ch, tch);
           SET_BIT(ch->specials.act, ACT_GREET);
          }
          break;
        }
      }
    }
  } else if (!number(0, 100)) {
    REMOVE_BIT(ch->specials.act, ACT_GREET);
  }
}

#define PRISON_ROOM 2639
#define PRISON_LET_OUT_ROOM 2640
int PrisonGuard( struct char_data *ch, int cmd, const char *arg, struct char_data *mob, int type)
{
  char *p, buf[256];
  struct char_data *PGuard;  /* guards own referent pointer */
  struct char_data *t, *t2, *t3;
  int purge_nr,i;
  struct room_data *rp;
  static int timehere;

  rp = real_roomp(ch->in_room);
  if (!rp) return(FALSE);

  if (ch->in_room != PRISON_ROOM)
     return(FALSE);

    PGuard = (struct char_data *)FindMobInRoomWithFunction(ch->in_room,
                            PrisonGuard);

for (t=character_list,i=0;t;t= t2) {
     t2=t->next;
     if (PGuard->in_room == t->in_room && PGuard!=t && !IS_IMMORTAL(t)) {

     i++;
     if (GET_POS(t) < POSITION_STANDING) {
        do_stand(t,"",0);
               sprintf(buf,"This is not a vacation %s, get up!", GET_NAME(t));
         do_say(PGuard,buf,0);
       }

      if (IS_MURDER(t)) {       /* do murder kick out */
       if (timehere >= 80) {
act("$n glares at you and says 'You are free to roam the world again, murderer.'",
           FALSE, PGuard, 0, t, TO_VICT);
act("$n glares at $N and says 'You are free to roam the world again, murderer.'",
           FALSE, PGuard, 0, t, TO_NOTVICT);
          REMOVE_BIT(t->player.user_flags,MURDER_1);
          REMOVE_BIT(t->player.user_flags,STOLE_1);
          mudlog( LOG_PLAYERS, "Removing MURDER and STOLE bit from %s.",
                  GET_NAME( t ) );
          char_from_room(t);
          char_to_room(t,PRISON_LET_OUT_ROOM);
          do_look(t,"",15);
          sprintf(buf,"The prisoner convicted of murder, %s, is now free!",
                 GET_NAME(t));
          do_shout(PGuard,buf,0);
           timehere=0;
           return(TRUE);
         } else
          if (number(0,1))
             timehere++;
       }

      if (IS_STEALER(t))
      {        /* do stole kick out */
        if (timehere >= 30)
        {
          act( "$n glares at you and says 'You are free to roam the world again, thief.'",
               FALSE, PGuard, 0, t, TO_VICT);
          act( "$n glares at $N and says 'You are free to roam the world again, thief.'",
               FALSE, PGuard, 0, t, TO_NOTVICT);
          REMOVE_BIT(t->player.user_flags,STOLE_1);
          REMOVE_BIT(t->player.user_flags,MURDER_1);
          mudlog( LOG_PLAYERS, "Removing STOLE and MURDER bit from %s.",
                  GET_NAME(t) );

          char_from_room(t);
          char_to_room(t,PRISON_LET_OUT_ROOM);
          do_look(t,"",15);

          sprintf(buf,"The prisoner convicted of robbery, %s, is now free!",
                 GET_NAME(t));
          do_shout(PGuard,buf,0);
          timehere=0;
          return(TRUE);
        }
        else if (number(0,1))
          timehere++;
      }

    } /* end for */

 if (i == 0)
    timehere = 0;

 if (cmd) {
 if (IS_IMMORTAL(ch)) return(FALSE);

    PGuard = (struct char_data *)FindMobInRoomWithFunction(ch->in_room,
                            PrisonGuard);

    for (;*arg==' ';arg++); /* skip whitespace */
    strcpy(buf, arg);

if (cmd ==302) { /* gos */
      act("$n glares at you", FALSE, PGuard, 0, ch, TO_VICT);
      act("$n glares at $N", FALSE, PGuard, 0, ch, TO_NOTVICT);
      act("$n says 'Shut up!", FALSE, PGuard, 0, 0, TO_ROOM);
     return(TRUE);
    }  else
if (cmd ==304) { /* auc */
      act("$n glares at you", FALSE, PGuard, 0, ch, TO_VICT);
      act("$n glares at $N", FALSE, PGuard, 0, ch, TO_NOTVICT);
      act("$n says 'Shut up!", FALSE, PGuard, 0, 0, TO_ROOM);
     return(TRUE);
    }  else
    if (cmd == 207) { /* recite */
      act("$n glares at you", FALSE, PGuard, 0, ch, TO_VICT);
      act("$n glares at $N", FALSE, PGuard, 0, ch, TO_NOTVICT);
      p = (char *)strtok(buf, " ");
      if (strncmp("recall", p, strlen(p))==0) {
        act("$n says 'And just where do you think you're going?", FALSE, PGuard, 0, 0, TO_ROOM);
        return(TRUE);
      }
    } else if (cmd == 84) { /* cast */
      act("$n glares at you", FALSE, PGuard, 0, ch, TO_VICT);
      act("$n glares at $N", FALSE, PGuard, 0, ch, TO_NOTVICT);
      /* we use strlen(p)-1 because if we use the full length, there is
         the obligatory ' at the end.  We must ignore this ', and get
         on with our lives */
      p = (char *)strtok(buf, " ");
      if (strncmp("'word of recall'", p, strlen(p)-1)==0) {
        act("$n says 'And just where do you think you're going?", FALSE, PGuard, 0, 0, TO_ROOM);
        return(TRUE);
      } else if (strncmp("'astral walk'", p, strlen(p)-1)==0) {

        act("$n says 'Do you think you can astral walk in and out of here like the ", FALSE, PGuard, 0, 0, TO_ROOM);
        act("wind?'", FALSE, PGuard, 0, 0, TO_ROOM);
        return(TRUE);
      } else if (strncmp("'teleport'", p, strlen(p)-1)==0) {
        act("$n says 'And just where do you think you're going?", FALSE, PGuard, 0, 0, TO_ROOM);
        return(TRUE);
      } else if (strncmp("'polymorph'", p, strlen(p)-1)==0) {
        act("$n says 'I like you the way you are.", FALSE, PGuard, 0, 0, TO_ROOM);
        return(TRUE);
      }
    } else if (cmd == 17 || cmd == 169) { /* say */
      act("$n glares at you", FALSE, PGuard, 0, ch, TO_VICT);
      act("$n glares at $N", FALSE, PGuard, 0, ch, TO_NOTVICT);
      act("$n says 'Prisoners should be seen, and not heard'", FALSE, PGuard, 0, 0, TO_ROOM);
      return(TRUE);
    } else if (cmd == 40 || cmd == 214 || cmd == 177) { /* emote */
      act("$n glares at you", FALSE, PGuard, 0, ch, TO_VICT);
      act("$n glares at $N", FALSE, PGuard, 0, ch, TO_NOTVICT);
      act("$n says 'Cease your childish pantonimes.'", FALSE, PGuard, 0, 0, TO_ROOM);
      return(TRUE);
    } else if (cmd == 19 || cmd == 18 || cmd == 83) { /* say, shout whisp */
      act("$n glares at you", FALSE, PGuard, 0, ch, TO_VICT);
      act("$n glares at $N", FALSE, PGuard, 0, ch, TO_NOTVICT);
      act("$n says 'Speak only when spoken to.'", FALSE, PGuard, 0, 0, TO_ROOM);
      return(TRUE);
    } else if (cmd == 86) {  /* ask */
      act("$n glares at you", FALSE, PGuard, 0, ch, TO_VICT);
      act("$n glares at $N", FALSE, PGuard, 0, ch, TO_NOTVICT);
      act("$n says 'Your ignorance is too immense to be rectified at this time.'", FALSE, PGuard, 0, 0, TO_ROOM);
      return(TRUE);
    } else if (cmd == 87 || cmd == 46) {  /* order, force */
      act("$n glares at you", FALSE, PGuard, 0, ch, TO_VICT);
      act("$n glares at $N", FALSE, PGuard, 0, ch, TO_NOTVICT);
      act("$n says 'I'll be the only one giving orders here.'", FALSE, PGuard, 0, 0, TO_ROOM);
      return(TRUE);
    } else if (cmd == 151) {
      act("$n glares at you", FALSE, PGuard, 0, ch, TO_VICT);
      act("$n glares at $N", FALSE, PGuard, 0, ch, TO_NOTVICT);
      act("$n says 'Cease this cowardly behavior.'", FALSE, PGuard, 0, ch, TO_ROOM);
      return(TRUE);
    } else if (cmd == 63) {
      act("$n glares at you", FALSE, PGuard, 0, ch, TO_VICT);
      act("$n glares at $N", FALSE, PGuard, 0, ch, TO_NOTVICT);
      act("$n says 'Pay attention when I am speaking.'", FALSE, PGuard, 0, ch, TO_ROOM);
      return(TRUE);
    }
  } else {
    if (ch->specials.fighting) {
    } else {
      /*
        check for followers in the room
        */
      for (t = rp->people; t; t= t->next_in_room) {
        if (IS_NPC(t) && !IS_PC(t) && t->master && t != ch &&
            t->master != ch) {
          break;
        }
      }
      if (t) {
        act("$n says 'What is $N doing here?'", FALSE, ch, 0, t, TO_ROOM);
        act("$n makes a magical gesture", FALSE, ch, 0, 0, TO_ROOM);
        purge_nr = t->nr;
        for (t2 = rp->people; t2; t2 = t3) {
          t3 = t2->next_in_room;
          if (t2->nr == purge_nr && !IS_PC(t2)) {
            act("$N, looking very surprised, quickly fades out of existence.", FALSE, ch, 0, t2, TO_ROOM);
            extract_char(t2);
          }
        }
      } else {
        /*
          check for polymorphs in the room
          */
        for (t = rp->people; t; t= t->next_in_room) {
          if (IS_NPC(t) && IS_PC(t)) {  /* ah.. polymorphed :-) */
            /*
              I would like to digress at this point, and state that
              I feel that George Bush is an incompetent fool.
              Thank you.
              */
            act("$n glares at $N", FALSE, ch, 0, t, TO_NOTVICT);
            act("$n glares at you", FALSE, ch, 0, t, TO_VICT);
            act("$n says 'Seek not to disguise your true form from me.", FALSE, ch, 0, t, TO_ROOM);
            act("$n traces a small rune in the air", FALSE, ch, 0, 0, TO_ROOM);
            act("$n has forced you to return to your original form!", FALSE, ch, 0, t, TO_VICT);
            do_return(t, "", 1);
            return(TRUE);
          }
        }
      }
    }
  }
 }
  return(FALSE);

}

int GenericCityguardHateUndead(struct char_data *ch, int cmd, const char *arg, struct char_data *mob,int type)
{
  struct char_data *tch, *evil;
  int max_evil;

  if (cmd || !AWAKE(ch))
    return (FALSE);

  if (ch->specials.fighting) {
    fighter(ch, cmd, arg, mob, type);

    if (!check_soundproof(ch)) {

       if (number(0,100) == 0) {
         do_shout(ch, "To me, my fellows! I am in need of thy aid!", 0);
       } else {
         act("$n shouts 'To me, my fellows! I need thy aid!'",
             TRUE, ch, 0, 0, TO_ROOM);
       }

       if (ch->specials.fighting)
         CallForGuard(ch, ch->specials.fighting, 3, type);

       return(TRUE);
     }
  }

  max_evil = 0;
  evil = 0;

  if (check_peaceful(ch, ""))
    return FALSE;

#if 0

        /* disabled, to many bugs in murder/stole jail and settings */
        /* might try and find them some other time                */

  for (tch=real_roomp(ch->in_room)->people; tch; tch = tch->next_in_room) {

      if (IS_MURDER(tch) && CAN_SEE(ch, tch)) {
      if (GetMaxLevel(tch)>=20 ) {
      if (!check_soundproof(ch))
         act("$n screams 'MURDERER!!!  EVIL!!!  KILLER!!!  BANZAI!!'",
          FALSE, ch, 0, 0, TO_ROOM);
      hit(ch, tch, TYPE_UNDEFINED);
      return(TRUE);
      } else {
    act("$n thawacks $N muttering 'Murderer' and has $M dragged off to prison!",
         TRUE, ch, 0, tch, TO_NOTVICT);
    act("$n thawacks you muttering 'Murderer' and has you dragged off to prison!",
         TRUE, ch, 0, tch, TO_VICT);
        char_from_room(tch);
        char_to_room(tch,PRISON_ROOM);
        do_look(tch,"",15);
        act("The prison door slams shut behind you!",
         TRUE, ch, 0, tch, TO_VICT);
        return(TRUE);
       }
     } else
       if (IS_STEALER(tch) && CAN_SEE(ch, tch)) {
       if (!number(0,30)) {
      if (!check_soundproof(ch))
         act("$n screams 'ROBBER!!!  EVIL!!!  THIEF!!!  BANZAI!!'",
          FALSE, ch, 0, 0, TO_ROOM);
      hit(ch, tch, TYPE_UNDEFINED);
      return(TRUE);
      } else {
    act("$n thawacks $N muttering 'Thief' and has $M dragged off to prison!",
         TRUE, ch, 0, tch, TO_NOTVICT);
    act("$n thawacks you muttering 'Thief' and has you dragged off to prison!",
         TRUE, ch, 0, tch, TO_VICT);
        char_from_room(tch);
        char_to_room(tch,PRISON_ROOM);
        do_look(tch,"",15);
        act("The prison door slams shut behind you!",
         TRUE, ch, 0, tch, TO_VICT);
        return(TRUE);
        }
      }
    }/* end for! */

#endif                /* end of stole/murder */

  for (tch=real_roomp(ch->in_room)->people; tch; tch = tch->next_in_room) {
    if  (   (IS_NPC(tch)  &&  (IsUndead(tch)) && CAN_SEE(ch, tch))
         || (IsGoodSide(ch) && IsBadSide(tch) && CAN_SEE(ch, tch))
         || (IsGoodSide(tch) && IsBadSide(ch) && CAN_SEE(ch, tch))
         ) {

      max_evil = -1000;
      evil = tch;
      if (!check_soundproof(ch))
         act("$n screams 'EVIL!!! BANZAI!!'",FALSE, ch, 0, 0, TO_ROOM);
         hit(ch, evil, TYPE_UNDEFINED);
         return(TRUE);
    }

    if (!IS_PC(tch)) {
      if (tch->specials.fighting) {
        if ((GET_ALIGNMENT(tch) < max_evil) &&
            (!IS_PC(tch) || !IS_PC(tch->specials.fighting))) {
             max_evil = GET_ALIGNMENT(tch);
             evil = tch;
        }
      }
    }
  }

  if (evil && (GET_ALIGNMENT(evil->specials.fighting) >= 0)) {
    if (IS_MURDER(evil) ||
        GET_HIT(evil->specials.fighting) > GET_HIT(evil) ||
        (evil->specials.fighting->attackers > 3)) {
      if (!check_soundproof(ch))
        act("$n screams 'PROTECT THE INNOCENT! BANZAI!!! CHARGE!!!'",FALSE, ch, 0, 0, TO_ROOM);
      hit(ch, evil, TYPE_UNDEFINED);
      return(TRUE);
    } else
    if (!IS_MURDER(evil->specials.fighting)) {
      if (!check_soundproof(ch))
        act("$n yells 'There's no need to fear! $n is here!'",
            FALSE, ch, 0, 0, TO_ROOM);

      if (!ch->skills)
        SpaceForSkills(ch);

      if (!ch->skills[SKILL_RESCUE].learned)
         ch->skills[SKILL_RESCUE].learned = GetMaxLevel(ch)*3+30;
      do_rescue(ch, GET_NAME(evil->specials.fighting), 0);
    }
  }

  GreetPeople(ch);

  return(FALSE);
}

int GenericCityguard(struct char_data *ch, int cmd, const char *arg, struct char_data *mob, int type)
{
  struct char_data *tch, *evil;
  int max_evil;

  if (cmd || !AWAKE(ch))
    return (FALSE);

  if (ch->specials.fighting) {
    fighter(ch, cmd, arg,mob,type);

    if (!check_soundproof(ch)) {
       if (number(0,120) == 0) {
         do_shout(ch, "To me, my fellows! I am in need of thy aid!", 0);
       } else {
         act("$n shouts 'To me, my fellows! I need thy aid!'",
             TRUE, ch, 0, 0, TO_ROOM);
       }

       if (ch->specials.fighting)
         CallForGuard(ch, ch->specials.fighting, 3, type);

       return(TRUE);
     }
  }

  max_evil = 1000;
  evil = 0;

  if (check_peaceful(ch, ""))
    return FALSE;

  for (tch=real_roomp(ch->in_room)->people; tch; tch = tch->next_in_room)
  {
    if (tch->specials.fighting)
    {
      if ((GET_ALIGNMENT(tch) < max_evil) &&
          (IS_NPC(tch) || IS_NPC(tch->specials.fighting)))
          {
        max_evil = GET_ALIGNMENT(tch);
        evil = tch;
      }
    }
  }

  if (evil && (GET_ALIGNMENT(evil->specials.fighting) >= 0)) {
    if (!check_soundproof(ch)) {
       act("$n screams 'PROTECT THE INNOCENT! BANZAI!!! CHARGE!!! SPOON!'",
        FALSE, ch, 0, 0, TO_ROOM);
    }
    hit(ch, evil, TYPE_UNDEFINED);
    return(TRUE);
  }

  GreetPeople(ch);

  return(FALSE);
}

#define HOLDING_MAX  10   /* max mobs that can be in tank :) */
#define HOLDING_TANK 60  /* room number to drop the mobs in */
int DogCatcher( struct char_data *ch, int cmd, const char *arg,
                struct char_data *mob, int type)
{
  char buf[128];
  struct char_data *tch;
  
  mob->lStartRoom = 0;

  if( ch == NULL || mob == NULL )
  {
    mudlog( LOG_SYSERR, "ch or mob == NULL in DogCatcher ('spec_procs2.c')" );
    return FALSE;
  }

  if( type == EVENT_TICK && ch->specials.fighting )
  {
    if( magic_user(ch, cmd, arg, mob, type ) )
      return(TRUE);

    if( !check_soundproof( ch ) )
    {
      if (number(0,120) == 0)
      {
        do_shout(ch, "Guardie a me! Ho bisogno del vostro aiuto!", 0);
        if( ch->specials.fighting )
          CallForGuard( ch, ch->specials.fighting, 3, type );
         return(TRUE);
      }
    }
  }

  if( ch && check_peaceful( ch, "" ) )
    return FALSE;

  if( type == EVENT_COMMAND && ch && cmd && AWAKE(ch) )
  {

    if( ch->desc )   /* keep newbies from trying to kill Seneca */
      if( GetMaxLevel(ch) < 15)
        switch(cmd)
        {
          case 370:
          case 283:
          case 84:
#if 0
            send_to_char( "Seneca dice 'Vallo a fare da qualche altra "
                          "parte.'\n\r", ch );
#else
            sprintf( buf, "%s Vallo a fare da qualche altra parte.",
                     GET_NAME( ch ) );
            do_tell( mob, buf, 0 );
#endif
            return(TRUE);
            break;
          case 25:
          case 154:
          case 157:
          case 159:
          case 156:
          case 354:
          case 346:
            send_to_char( "Hai deciso di non sprecare la tua vita.\n\r",ch);
            return(TRUE);
            break;
          default:
            return(FALSE);
            break;
        } /* end switch */

    return(FALSE);
  }        /* end was cmd */


  for( tch=real_roomp(ch->in_room)->people; tch; tch = tch->next_in_room )
  {
    if( !IsHumanoid(tch) && !IS_PC(tch) && IS_NPC(tch) && CAN_SEE(ch,tch))
    {

#if 0                  /* always true, find out why! */
      rp = real_roomp(HOLDING_TANK);
      if (MobCountInRoom(rp->people) >= HOLDING_MAX)
      {
        mudlog( LOG_CHECK, "Catcher room was full" );
        return(FALSE);
      }
#endif
      /* check level of mob, greater than catcher? */
      if (GetMaxLevel(tch)>GetMaxLevel(ch))
        if (number(1,100)>50)
          return(FALSE); /* he was higher level and we just decided to */
                                 /* to ignore him for now */

      /* check if mob is follower */

      if( tch->master && (tch->master->in_room == ch->in_room) &&
          CAN_SEE(ch, tch->master) )
      {
        if( tch->master != mob )
        {
          switch (number(0,3))
          {
            case 0:
              if (!check_soundproof(ch))
                act( "$n tira della polvere a $N dice 'La fortuna sia con te.'",
                     FALSE, ch, 0, tch, TO_ROOM);
              break;
            case 1:
              if (!check_soundproof(ch))
                act( "$n punta la bacchetta a $N "
                     "'E` meglio che ve ne andiate tu ed il tuo padrone.'",
                     FALSE, ch, 0, tch, TO_ROOM);
              break;
            case 2:
              sprintf( buf,"%s lancia un'occhiata irritata a %s sbuffando.",
                       ch->player.short_descr,GET_NAME(tch->master));
              if( !check_soundproof(ch) )
                act(buf, FALSE, ch, 0, tch, TO_ROOM);
              break;
            case 3:
            default:
              if( !check_soundproof(ch) )
                act( "$n ruggisce verso $N 'Se il tuo padrone non fosse qui...'",
                     FALSE, ch, 0, tch, TO_ROOM);
              break;
          } /* end switch */
        }
        else
        {
          act( "$n tira fuori una bacchetta e la punta a $N.",
               FALSE, ch, 0, tch, TO_ROOM);
          act( "$c0015$N scompare in un accecante lampo di luce!",
               FALSE, ch, 0 , tch, TO_ROOM);
          char_from_room(tch);
          char_to_room( tch, HOLDING_TANK );
          act( "$c0015$n appare in un accecante lampo di luce!",
               FALSE, tch, 0 , 0, TO_ROOM);
          act( "$n soffia sulla sua bacchetta e sorride.",
               FALSE, ch, 0, 0, TO_ROOM);
        }
        return(TRUE);
      }

      /* mob was just a mob and was not a follower type */

      act( "$n tira fuori una bacchetta e la punta a $N.",
           FALSE, ch, 0, tch, TO_ROOM);
      if( !check_soundproof(ch))
        do_say( ch, "Tornatene da dove sei venuto!", 0 );

      act( "$c0015$N scompare in un accecante lampo di luce!",
           FALSE, ch, 0 , tch, TO_ROOM);
      char_from_room(tch);
      char_to_room( tch, HOLDING_TANK );
      act( "$c0015$n appare in un accecante lampo di luce!",
           FALSE, tch, 0 , 0, TO_ROOM);
      act( "$n soffia sulla sua bacchetta e sorride.",
           FALSE, ch, 0, 0, TO_ROOM);
      if (!check_soundproof(ch))
        do_say( ch, "Gli insegno io ad invadere la mia citta`.", 0 );

      return(TRUE); /* done with one mob move */
    } /* else was humanoid or PC */
  } /* end for */

  if( !number( 0, 10 ) )
  {
    switch(number(1,5))
    {
      case 1:
        do_say(ch, "Attenzione! Non provate a combattere in citta`.", 0);
        break;
      case 2:
        do_say(ch, "Chiudero` tutte le bestie nel recinto, cosi` non mi "
                   "annoieranno piu`.", 0 );
        break;
      case 3:
        do_say(ch, "Il Sindaco di Alma ha vietato i combattimenti nella sua "
                   "citta`!",0);
        break;
      case 4:
        do_say(ch, "Dove e` scappato quel fottuto scrivano!?",0);
        break;
      default:
        do_say(ch, "Avete visto degli invasori?",0);
        break;
    } /* end say switch */
    return(TRUE);
  }
  else if (!number(0,70))
  {
    switch( number( 1, 4 ) )
    {
      case 1:
        do_gossip( ch, "Ricordatevi che chi combattera` in citta` verra` "
                       "punito!", 0 );
        break;
      case 2:
        do_gossip(ch,"Qualcuno puo` controllare il recinto degli animali?",0);
        break;
      case 3:
        do_gossip( ch,"Qualcuno ha visto lo scrivano?", 0 );
        break;
      case 4:
        do_gossip( ch,"Oh eroi, siate pronti a difendere Alma dagli invasori!",
                   0 );
        break;
      default:
        break;
    } /* end gossip switch */
    return(TRUE);
  } /* end gossip loop */

  GreetPeople(ch);
  return(FALSE);
}


int PrydainGuard(struct char_data *ch, int cmd, const char *arg, struct char_data *mob, int type)
{
  return(GenericCityguardHateUndead(ch,cmd,arg,mob,PRYDAIN));
}


struct breath_victim 
{
  struct char_data        *ch;
  int        yesno; /* 1 0 */
  struct breath_victim        *next;
};

struct breath_victim *choose_victims(struct char_data *ch,
                                     struct char_data *first_victim)
{
  /* this is goofy, dopey extraordinaire */
  struct char_data *cons;
  struct breath_victim *head = NULL, *temp=NULL;

  for( cons = real_roomp(ch->in_room)->people; cons; cons = cons->next_in_room )
  {
    temp = (struct breath_victim *)malloc(sizeof(*temp));
    temp->ch = cons;
    temp->next = head;
    head = temp;
    if (first_victim == cons) 
    {
      temp->yesno = 1;
    }
    else if (ch == cons) 
    {
      temp->yesno = 0;
    }
    else if( ( in_group(first_victim, cons) ||
               cons == first_victim->master ||
               cons->master == first_victim) &&
               (temp->yesno = (3 != dice(1,5))) ) 
    {
      /* group members will get hit 4/5 times */
    } 
    else if (cons->specials.fighting == ch) 
    {
      /* people fighting the dragon get hit 4/5 times */
      temp->yesno = (3 != dice(1,5));
    } 
    else /* bystanders get his 2/5 times */
      temp->yesno = (dice(1,5)<3);
  }
  return head;
}

void free_victims(struct breath_victim *head)
{
  struct  breath_victim *temp;

  while (head) 
  {
    temp = head->next;
    free(head);
    head = temp;
  }
}

void breath_weapon( struct char_data *ch, struct char_data *target,
                    int mana_cost, void (*func)( byte, struct char_data *, 
                                                 const char *, int, 
                                                 struct char_data *, 
                                                 struct obj_data * ) )
{
  struct breath_victim *hitlist, *scan;
  struct char_data *tmp;
  int        victim;

  act("$n rears back and inhales",1,ch,0,ch->specials.fighting,TO_ROOM);
  victim=0;

  for (tmp = real_roomp(ch->in_room)->people; tmp; tmp = tmp->next_in_room)
  {
    if (tmp == ch || IS_IMMORTAL(tmp)) 
    {
      continue;
    } 
    else 
    {
      victim=1;
      cast_fear( GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, tmp, 0);
    }
  }

  hitlist = choose_victims(ch, target);

  if (func!=NULL && victim && hitlist) 
  {
    act("$n Breathes...", 1, ch, 0, ch->specials.fighting, TO_ROOM);

    for (scan = hitlist; scan; scan = scan->next) 
    {
      if (!scan->yesno ||
          IS_IMMORTAL(scan->ch) ||
          scan->ch->in_room != ch->in_room /* this could happen if
                                              someone fled, I guess */
          )
        continue;
      func( GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, scan->ch, 0);
    }
    GET_MANA(ch) -= mana_cost;
  } 
  else 
  {
    act( "$n Breathes...coughs and sputters...",
         1, ch, 0, ch->specials.fighting, TO_ROOM);
    do_flee(ch, "", 0);
  }

  free_victims(hitlist);
}

void use_breath_weapon( struct char_data *ch, struct char_data *target,
                        int cost, void (*func)( byte, struct char_data *, 
                                                const char *, int, 
                                                struct char_data *, 
                                                struct obj_data * ) )
{
  if (GET_MANA(ch)>=0) 
  {
    breath_weapon(ch, target, cost, func);
  } 
  else if ((GET_HIT(ch) < GET_MAX_HIT(ch)/2) && (GET_MANA(ch) >= -cost)) 
  {
    breath_weapon(ch, target, cost, func);
  } 
  else if ((GET_HIT(ch) < GET_MAX_HIT(ch)/4) && (GET_MANA(ch) >= -2*cost)) 
  {
    breath_weapon(ch, target, cost, func);
  } 
  else if (GET_MANA(ch)<=-3*cost) 
  {
    breath_weapon(ch, target, 0, NULL); /* sputter */
  }
}


static funcp breaths[] = 
{
  cast_acid_breath, 
  0, 
  cast_frost_breath, 
  0, 
  cast_lightning_breath, 
  0,
  cast_fire_breath, 
  0,
  cast_acid_breath, 
  cast_fire_breath, 
  cast_lightning_breath, 
  0
};

struct breather breath_monsters[] = 
{
  { 230,   55, breaths+0 },
  { 233,   55, breaths+4 },
  { 243,   55, breaths+2 },
  { 3670,  30, breaths+2 },
  { 3674,  45, breaths+6 },
  { 3675,  45, breaths+8 },
  { 3676,  30, breaths+6 },
  { 3952,  20, breaths+8 },
  { 5005,  45, breaths+4 },
  { 6112,  55, breaths+4 },
  { 6635,  55, breaths+0 },
  { 6609,  30, breaths+0 },
  { 6642,  45, breaths+2 },
  { 6801,  55, breaths+2 },
  { 6802,  55, breaths+2 },
  { 6824,  55, breaths+0 },
  { 7040,  55, breaths+6 },
  { 9217,  45, breaths+4 },
  { 9418,  45, breaths+2 },
  { 9419,  45, breaths+2 },

  { 12010, 45, breaths+6 },  /* werts stuff... */
  { 12011, 45, breaths+6 },

  { 15858, 45, breaths+0 },
  { 15879, 30, breaths+0 },
  { 16620, 45, breaths+0 },
  { 16700, 45, breaths+4 },
  { 16738, 75, breaths+6 },
  { 18003, 20, breaths+8 },
  { 20002, 55, breaths+6 },
  { 20017, 55, breaths+6 },
  { 20016, 55, breaths+6 },
  { 20016, 55, breaths+6 },
  { 25009, 30, breaths+6 },
  { 25504, 30, breaths+4 },
  { 27016, 30, breaths+6 },
  { -1 },
};

int BreathWeapon( struct char_data *ch, int cmd, const char *arg, 
                  struct char_data *mob, int type)
{
  struct breather *scan;
  int        count;

  if (cmd)
    return FALSE;


  if (ch->specials.fighting &&
      (ch->specials.fighting->in_room == ch->in_room)) 
  {

    for (scan = breath_monsters;
         scan->vnum >= 0 && scan->vnum != mob_index[ch->nr].iVNum;
         scan++)
      ;

    if (scan->vnum < 0)
    {
      mudlog( LOG_ERROR, "monster %s tries to breath, but isn't listed.",
              ch->player.short_descr);
      return FALSE;
    }

    for (count=0; scan->breaths[count]; count++)
      ;

    if (count<1)
    {
      mudlog( LOG_ERROR, "monster %s has no breath weapons",
              ch->player.short_descr);
      return FALSE;
    }

    use_breath_weapon(ch, ch->specials.fighting, scan->cost,
                      scan->breaths[dice(1,count)-1]);
  }

  return (FALSE);
}




int sailor(struct char_data *ch, int cmd, const char *arg, struct char_data *mob, int type)
{
  char buf[256];
  const char *n_skills[] = {
    "swim",
    "\n",
  };
  int percent=0, number=0;
  int charge, sk_num;

  if (!AWAKE(ch))
    return(FALSE);

  if (!cmd) {
    if (ch->specials.fighting) {
      return(fighter(ch, cmd, arg, mob, type));
    }
    return(FALSE);
  }

  if (!ch->skills) return(FALSE);

  if (check_soundproof(ch)) return(FALSE);

  for(; *arg==' '; arg++); /* ditch spaces */

  if ((cmd==164)||(cmd==170)) {
    if (!arg || (strlen(arg) == 0)) {
      sprintf(buf," swim   :  %s\n\r",how_good(ch->skills[SKILL_SWIM].learned));
      send_to_char(buf,ch);
      return(TRUE);
    } else {
      number = old_search_block(arg,0,strlen(arg),n_skills,FALSE);
      send_to_char ("The sailor says ",ch);
      if (number == -1) {
        send_to_char("'I do not know of this skill.'\n\r", ch);
        return(TRUE);
      }
      charge = GetMaxLevel(ch) * 500;
      switch(number) {
      case 0:
      case 1:
        sk_num = SKILL_SWIM;
        break;
      default:
        mudlog( LOG_SYSERR, "Strangeness in sailor (%d)", number);
        send_to_char("'Ack!  I feel faint!'\n\r", ch);
        return TRUE;
      }
    }

    if (GET_GOLD(ch) < charge){
      send_to_char
        ("'Ah, but you do not have enough money to pay.'\n\r",ch);
      return(TRUE);
    }

    if (ch->skills[sk_num].learned > 60) {
      send_to_char("You must learn from practice and experience now.\n\r", ch);
      return(TRUE);
    }

    if (ch->specials.spells_to_learn <= 0) {
      send_to_char
        ("'You must first earn more practices you already have.\n\r",ch);
      return(TRUE);
    }

    GET_GOLD(ch) -= charge;
    send_to_char("'We will now begin.'\n\r",ch);
    ch->specials.spells_to_learn--;

    percent = ch->skills[sk_num].learned + int_app[ (int)GET_INT(ch) ].learn;
    ch->skills[sk_num].learned = MIN(95, percent);

    if (ch->skills[sk_num].learned >= 95) {
      send_to_char("'You are now a master of this art.'\n\r", ch);
    }
    return(TRUE);
  }
  return(FALSE);
}

int loremaster(struct char_data *ch, int cmd, const char *arg, struct char_data *mob, int type)
{
  char buf[256];
  const char *n_skills[] = {
    "necromancy",
    "vegetable lore",
    "animal lore",
    "reptile lore",
    "people lore",
    "giant lore",
    "other lore",
    "read magic",
    "demonology",
    "sign language",
    "common",
    "elvish",
    "halfling",
    "dwarvish",
    "orcish",
    "giantish",
    "ogre",
    "gnomish",
    "\n",
  };
  int percent=0, number=0;
  int charge, sk_num;

  if (!AWAKE(ch))
    return(FALSE);

  if (!ch->skills) return(FALSE);

  if (!cmd) {
    if (ch->specials.fighting) {
      return(fighter(ch, cmd, arg,mob,type));
    }
    return(FALSE);
  }

  if (check_soundproof(ch)) return(FALSE);

  for(; *arg==' '; arg++); /* ditch spaces */

  if ((cmd==164)||(cmd==170)) {
    if (!arg || (strlen(arg) == 0)) {
      sprintf(buf," necromancy      :  %s\n\r",how_good(ch->skills[SKILL_CONS_UNDEAD].learned));
      send_to_char(buf,ch);
      sprintf(buf," vegetable lore  :  %s\n\r",how_good(ch->skills[SKILL_CONS_VEGGIE].learned));
      send_to_char(buf,ch);
      sprintf(buf," demonology      :  %s\n\r",how_good(ch->skills[SKILL_CONS_DEMON].learned));
      send_to_char(buf,ch);
      sprintf(buf," animal lore     :  %s\n\r",how_good(ch->skills[SKILL_CONS_ANIMAL].learned));
      send_to_char(buf,ch);
      sprintf(buf," reptile lore  :  %s\n\r",how_good(ch->skills[SKILL_CONS_REPTILE].learned));
      send_to_char(buf,ch);
      sprintf(buf," people lore     :  %s\n\r",how_good(ch->skills[SKILL_CONS_PEOPLE].learned));
      send_to_char(buf,ch);
      sprintf(buf," giant lore      :  %s\n\r",how_good(ch->skills[SKILL_CONS_GIANT].learned));
      send_to_char(buf,ch);
      sprintf(buf," other lore      :  %s\n\r",how_good(ch->skills[SKILL_CONS_OTHER].learned));
      send_to_char(buf,ch);
      sprintf(buf," read magic      :  %s\n\r",how_good(ch->skills[SKILL_READ_MAGIC].learned));
      send_to_char(buf,ch);
      sprintf(buf," sign language   :  %s\n\r",how_good(ch->skills[SKILL_SIGN].learned));
      send_to_char(buf,ch);

      sprintf(buf," common   :  %s\n\r",how_good(ch->skills[LANG_COMMON].learned));
      send_to_char(buf,ch);
      sprintf(buf," elvish   :  %s\n\r",how_good(ch->skills[LANG_ELVISH].learned));
      send_to_char(buf,ch);
      sprintf(buf," halfling :  %s\n\r",how_good(ch->skills[LANG_HALFLING].learned));
      send_to_char(buf,ch);
      sprintf(buf," dwarvish :  %s\n\r",how_good(ch->skills[LANG_DWARVISH].learned));
      send_to_char(buf,ch);
      sprintf(buf," orcish   :  %s\n\r",how_good(ch->skills[LANG_ORCISH].learned));
      send_to_char(buf,ch);
      sprintf(buf," giantish :  %s\n\r",how_good(ch->skills[LANG_GIANTISH].learned));
      send_to_char(buf,ch);
      sprintf(buf," ogre     :  %s\n\r",how_good(ch->skills[LANG_OGRE].learned));
      send_to_char(buf,ch);
      sprintf(buf," gnomish  :  %s\n\r",how_good(ch->skills[LANG_GNOMISH].learned));
      send_to_char(buf,ch);

      return(TRUE);
    } else {
      number = old_search_block(arg,0,strlen(arg),n_skills,FALSE);
      send_to_char ("The loremaster says ",ch);
      if (number == -1) {
        send_to_char("'I do not know of this skill.'\n\r", ch);
        return(TRUE);
      }
      charge = GetMaxLevel(ch) * 500;
      switch(number) {
      case 0:
      case 1:
        sk_num = SKILL_CONS_UNDEAD;
        break;
      case 2:
        sk_num = SKILL_CONS_VEGGIE;
        break;
      case 3:
        sk_num = SKILL_CONS_ANIMAL;
        break;
      case 4:
        sk_num = SKILL_CONS_REPTILE;
        break;
      case 5:
        sk_num = SKILL_CONS_PEOPLE;
        break;
      case 6:
        sk_num = SKILL_CONS_GIANT;
        break;
      case 7:
        sk_num = SKILL_CONS_OTHER;
        break;
      case 8:
        if (!HasClass(ch, CLASS_CLERIC) && !HasClass(ch, CLASS_MAGIC_USER)) {
          sk_num = SKILL_READ_MAGIC;
        } else {
          send_to_char("'You already have this skill!'\n\r",ch);
          if (ch->skills)
            if (!ch->skills[SKILL_READ_MAGIC].learned)
              ch->skills[SKILL_READ_MAGIC].learned = 95;
          return TRUE;
        }
        break;

      case 9:
        sk_num = SKILL_CONS_DEMON;
        break;
      case 10:
        sk_num = SKILL_SIGN;
        break;

        case 11:
        sk_num = LANG_COMMON;
        break;
        case 12:
        sk_num = LANG_ELVISH;
        break;
        case 13:
        sk_num = LANG_HALFLING;
        break;
        case 14:
        sk_num = LANG_DWARVISH;
        break;
        case 15:
        sk_num = LANG_ORCISH;
        break;
        case 16:
        sk_num = LANG_GIANTISH;
        break;
        case 17:
        sk_num = LANG_OGRE;
        break;
        case 18:
        sk_num = LANG_GNOMISH;
        break;

      default:
        mudlog( LOG_SYSERR, "Strangeness in loremaster (%d)", number);
        send_to_char("'Ack!  I feel faint!'\n\r", ch);
        return(TRUE);
      }
    }

    if (GET_GOLD(ch) < charge){
      send_to_char
        ("'Ah, but you do not have enough money to pay.'\n\r",ch);
      return(TRUE);
    }


    if (ch->specials.spells_to_learn <= 0) {
      send_to_char
        ("'You must first earn more practices you already have.\n\r",ch);
      return(TRUE);
    }

    GET_GOLD(ch) -= charge;
    send_to_char("'We will now begin.'\n\r",ch);

    if (!IS_SET(ch->skills[sk_num].flags, SKILL_KNOWN))
      SET_BIT(ch->skills[sk_num].flags, SKILL_KNOWN);

    ch->specials.spells_to_learn--;

    percent = ch->skills[sk_num].learned + int_app[ (int)GET_INT(ch) ].learn;
    ch->skills[sk_num].learned = MIN(95, percent);

    if (ch->skills[sk_num].learned >= 95) {
      send_to_char("'You are now a master of this art.'\n\r", ch);
    }
    return TRUE;
  }
  return(FALSE);
}

int hunter(struct char_data *ch, int cmd, const char *arg, struct char_data *mob, int type)
{
  char buf[256];
  const char *n_skills[] =
  {
    "hunt",
    "find traps",
    "disarm traps",
    "value item",
    "find food",  /* 5 */
    "find water", /* 6 */
    "\n",
  };
  int percent=0, number=0;
  int charge, sk_num;

  if (!AWAKE(ch))
    return(FALSE);

  if (!cmd)
  {
    if (ch->specials.fighting)
    {
      return(fighter(ch, cmd, arg,mob,type));
    }
    return(FALSE);
  }

  if (!ch->skills)
    return(FALSE);

  if (check_soundproof(ch))
    return(FALSE);

  for(; *arg==' '; arg++); /* ditch spaces */

  if ((cmd==164)||(cmd==170))
  {
    if (!arg || (strlen(arg) == 0))
    {
      sprintf(buf," hunt           :  %s\n\r",how_good(ch->skills[SKILL_HUNT].learned));
      send_to_char(buf,ch);
      sprintf(buf," find traps     :  %s\n\r",how_good(ch->skills[SKILL_FIND_TRAP].learned));
      send_to_char(buf,ch);
      sprintf(buf," disarm traps   :  %s\n\r",how_good(ch->skills[SKILL_REMOVE_TRAP].learned));
      send_to_char(buf,ch);
      sprintf(buf," value item     :  %s\n\r",how_good(ch->skills[SKILL_EVALUATE].learned));
      send_to_char(buf,ch);
      sprintf(buf," find food      :  %s\n\r",how_good(ch->skills[SKILL_FIND_FOOD].learned));
      send_to_char(buf,ch);
      sprintf(buf," find water     :  %s\n\r",how_good(ch->skills[SKILL_FIND_WATER].learned));
      send_to_char(buf,ch);

      return(TRUE);
    }
    else
    {

      number = old_search_block(arg,0,strlen(arg),n_skills,FALSE);
      if( number == -1 )
      {
        act( "$N ti dice: 'Non conosco questo skill.'", TRUE, ch, 0, mob,
             TO_CHAR );
        return(TRUE);
      }

      charge = GetMaxLevel(ch) * 500;
      switch(number)
      {
      case 0:
      case 1:
        sk_num = SKILL_HUNT;
        break;
      case 2:
        sk_num = SKILL_FIND_TRAP;
        break;
      case 3:
        sk_num = SKILL_REMOVE_TRAP;
        break;
      case 4:
        sk_num = SKILL_EVALUATE;
        break;
      case 5:
        sk_num = SKILL_FIND_FOOD;
        break;
      case 6:
        sk_num = SKILL_FIND_WATER;
        break;
      default:
        mudlog( LOG_SYSERR, "Strangeness in hunter (%d)", number);
        send_to_char("'Ack!  I feel faint!'\n\r", ch);
        return(FALSE);
      }
    }


    if( !HasClass( ch, CLASS_THIEF ) &&
        ( sk_num == SKILL_FIND_TRAP ||
          sk_num == SKILL_REMOVE_TRAP ||
          sk_num == SKILL_EVALUATE ) )
    {
      act( "$N ti dice: 'Non il tipo di persona che possa imparare certe "
           "cose'.", TRUE, ch, 0, mob, TO_CHAR );
      return(TRUE);
    }

    if( sk_num == SKILL_HUNT && !HasClass( ch, CLASS_THIEF ) &&
        !HasClass( ch, CLASS_BARBARIAN ) && !HasClass( ch, CLASS_RANGER ) )
    {
      act( "$N ti dice: 'Non il tipo di persona che possa imparare certe "
           "cose'.", TRUE, ch, 0, mob, TO_CHAR );
      return(TRUE);
    }

    if( GET_GOLD( ch ) < charge )
    {
      act( "$N ti dice: 'Ah, ma non hai abbastanza soldi'.",
           TRUE, ch, 0, mob, TO_CHAR );
      return(TRUE);
    }

    if( ch->skills[ sk_num ].learned >= 95 )
    {
      act( "$N ti dice: 'Sei un maestro in quest'arte. Non posso insegnarti "
             "piu` nulla'.", TRUE, ch, 0, mob, TO_CHAR );
      return(TRUE);
    }

    if( ch->specials.spells_to_learn <= 0 )
    {
      act( "$N ti dice: 'Devi prima guadagnarti qualche sessione di pratica'.",
           TRUE, ch, 0, mob, TO_CHAR );
      return(TRUE);
    }

    GET_GOLD(ch) -= charge;
    act( "$N tiene la sua lezione.", TRUE, ch, 0, mob, TO_CHAR );
    ch->specials.spells_to_learn--;

    percent = ch->skills[sk_num].learned + int_app[ (int)GET_INT(ch) ].learn;

    if( !IS_SET(ch->skills[sk_num].flags, SKILL_KNOWN) )
      SET_BIT(ch->skills[sk_num].flags, SKILL_KNOWN);

    ch->skills[sk_num].learned = MIN(95, percent);

    if( ch->skills[sk_num].learned >= 95 )
    {
      act( "$N ti dice: 'Ora sei un maestro in quest'arte'.",
           TRUE, ch, 0, mob, TO_CHAR );
    }
    return TRUE;
  }

  return FALSE;
}

int archer_instructor( struct char_data *ch, int cmd, const char *arg,
                       struct char_data *mob, int type )
{
  char buf[256];
  const char *n_skills[] =
  {
    "spot",
    "\n",
  };
  int percent=0, number=0;
  int charge, sk_num;

  if (!AWAKE(ch))
    return(FALSE);

  if (ch->specials.fighting)
    return(archer(ch, cmd, arg,mob,type));

  if (!ch->skills)
    return(FALSE);

  if (check_soundproof(ch))
    return(FALSE);

  for(; *arg==' '; arg++); /* ditch spaces */

  if( cmd == CMD_PRACTICE || cmd == CMD_PRACTISE )
  {
    if( !arg || strlen(arg) == 0 )
    {
      send_to_char("You can learn :\n\r",ch);
      sprintf(buf," spot           :  %s\n\r",how_good(ch->skills[SKILL_SPOT].learned));
      send_to_char(buf,ch);
      return TRUE;
    }
    else
    {
      number = old_search_block(arg,0,strlen(arg),n_skills,FALSE);
      send_to_char ("L'arciere dice: ",ch);
      if (number == -1)
      {
        send_to_char( "'Non conosco questa abilita`.''\n\r", ch );
        return TRUE;
      }

      charge = GetMaxLevel(ch) * 500;
      switch(number)
      {
      case 0:
      case 1:
        sk_num = SKILL_SPOT;
        break;
      default:
        mudlog( LOG_SYSERR, "Strangeness in archer (%d)", number);
        send_to_char("'Ack!  I feel faint!'\n\r", ch);
        return FALSE;
      }
    }


    if( !HasClass( ch, CLASS_WARRIOR | CLASS_BARBARIAN | CLASS_RANGER |
                       CLASS_PALADIN ) &&  sk_num == SKILL_SPOT )
    {
      act( "$n ti dice 'Non sei il tipo che puo` imparare cose come questa.'",
           TRUE, mob, 0, ch, TO_VICT );
      return TRUE;
    }

    if( GET_GOLD( ch ) < charge )
    {
      send_to_char ("'Ah, ma non hai abbastanza soldi.'\n\r", ch );
      return TRUE;
    }

    if( ch->skills[sk_num].learned >= 95)
    {
      send_to_char ( "'Sei gia` un maestro in questa abilita`. "
                     "Non posso insegnarti altro.'\n\r", ch );
      return TRUE;
    }

    if( ch->specials.spells_to_learn <= 0 )
    {
      send_to_char( "'Devi prima guadagnarti piu` pratiche.\n\r", ch );
      return TRUE;
    }

    GET_GOLD(ch) -= charge;
    send_to_char( "'Iniziamo le lezioni...'\n\r", ch );
    ch->specials.spells_to_learn--;

    percent = ch->skills[sk_num].learned + int_app[ (int)GET_INT(ch) ].learn;

    if( !IS_SET( ch->skills[sk_num].flags, SKILL_KNOWN ) )
      SET_BIT( ch->skills[sk_num].flags, SKILL_KNOWN );

    ch->skills[sk_num].learned = MIN( 95, percent );

    if( ch->skills[sk_num].learned >= 95 )
    {
      send_to_char( "L'arciere ti dice: 'Sei diventato un maestro in questa "
                    "abilita.'\n\r", ch );
    }
    return TRUE;
  }

  return(archer(ch,cmd,arg,mob,type));
}


int monk_master(struct char_data *ch, int cmd, const char *arg, struct char_data *mob, int type)
{
  char buf[256];
  const char *n_skills[] = {
    "quivering palm", /* No. 245 */
    "feign death", /* No. 259 */
    "retreat",
    "kick",
    "hide",
    "sneak",
    "pick locks",
    "safe fall",
    "disarm",
    "dodge",
    "switch opponents",
    "springleap",
    "\n",
  };
  int percent=0, number=0;
  int charge, sk_num;

  if (!AWAKE(ch))
    return(FALSE);

  if (!cmd) {
    if (ch->specials.fighting) {
      return(fighter(ch, cmd, arg,mob,type));
    }
    return(FALSE);
  }

  if (!ch->skills) return(FALSE);

  if (check_soundproof(ch)) return(FALSE);

  for(; *arg==' '; arg++); /* ditch spaces */

  if ((cmd==164)||(cmd==170))
  {
    if (IS_NPC(ch))
    {
      send_to_char("The Monk Master tells you 'What do i look like, an animal trainer?'\n\r",ch);
      return(FALSE);
    }

    if (!arg || (strlen(arg) == 0))
    {
      sprintf(buf, "You have %d practices remaining\n\r", ch->specials.spells_to_learn);
      send_to_char(buf, ch);
      sprintf(buf," disarm  :        %s\n\r",how_good(ch->skills[SKILL_DISARM].learned));
      send_to_char(buf,ch);
      sprintf(buf," quivering palm:  %s [Must be 30th level]\n\r",how_good(ch->skills[SKILL_QUIV_PALM].learned));
      send_to_char(buf,ch);
      sprintf(buf," safe fall:       %s\n\r",how_good(ch->skills[SKILL_SAFE_FALL].learned));
      send_to_char(buf,ch);
      sprintf(buf," springleap:      %s\n\r",how_good(ch->skills[SKILL_SPRING_LEAP].learned));
      send_to_char(buf,ch);
      sprintf(buf," retreat:         %s\n\r",how_good(ch->skills[SKILL_RETREAT].learned));
      send_to_char(buf,ch);
      sprintf(buf," pick locks:      %s\n\r",how_good(ch->skills[SKILL_PICK_LOCK].learned));
      send_to_char(buf,ch);
      sprintf(buf," hide:            %s\n\r",how_good(ch->skills[SKILL_HIDE].learned));
      send_to_char(buf,ch);
      sprintf(buf," sneak:           %s\n\r",how_good(ch->skills[SKILL_SNEAK].learned));
      send_to_char(buf,ch);
      sprintf(buf," feign death:     %s\n\r",how_good(ch->skills[SKILL_FEIGN_DEATH].learned));
      send_to_char(buf,ch);
      sprintf(buf," dodge:           %s\n\r",how_good(ch->skills[SKILL_DODGE].learned));
      send_to_char(buf,ch);
      sprintf(buf," switch:          %s\n\r",how_good(ch->skills[SKILL_SWITCH_OPP].learned));
      send_to_char(buf,ch);
      sprintf(buf," kick:            %s\n\r",how_good(ch->skills[SKILL_KICK].learned));
      send_to_char(buf,ch);
      return(TRUE);
    }
    else
    {
      number = old_search_block(arg,0,strlen(arg),n_skills,FALSE);
      send_to_char ("The ancient master says ",ch);
      if (number == -1)
      {
        send_to_char("'I do not know of this skill.'\n\r", ch);
        return(TRUE);
      }
      charge = GetMaxLevel(ch) * 500;
      switch(number)
      {
       case 0:
       case 1:
        sk_num = SKILL_QUIV_PALM;
        if (!HasClass(ch, CLASS_MONK))
        {
          send_to_char("'You do not possess the proper skills'\n\r", ch);
          return(TRUE);
        }
        else if (GET_LEVEL(ch, MONK_LEVEL_IND) < 30)
        {
          send_to_char("'You are not high enough level'\n\r", ch);
          return(TRUE);
        }
        break;
       case 2:
        sk_num = SKILL_FEIGN_DEATH;
        if (!HasClass(ch, CLASS_MONK))
        {
          send_to_char("'You do not possess the proper skills'\n\r", ch);
          return(TRUE);
        }
        break;
       case 3:
        sk_num = SKILL_RETREAT;
        if (!HasClass(ch, CLASS_WARRIOR) && !(HasClass(ch, CLASS_MONK)))
        {
          send_to_char
            ("'You do not possess the necessary fighting skills'\n\r",ch);
          return(TRUE);
        }
        break;
       case 4:
        sk_num = SKILL_KICK;
        if (!HasClass(ch, CLASS_WARRIOR) && !(HasClass(ch, CLASS_MONK)))
        {
          send_to_char
            ("'You do not possess the necessary fighting skills'\n\r",ch);
          return(TRUE);
        }
        break;
       case 5:
        sk_num = SKILL_HIDE;
        if (!HasClass(ch, CLASS_THIEF) && !(HasClass(ch, CLASS_MONK)))
        {
          send_to_char
            ("'You do not possess the necessary skills'\n\r",ch);
          return(TRUE);
        }
        break;
       case 6:
        sk_num = SKILL_SNEAK;
        if (!HasClass(ch, CLASS_THIEF) && !(HasClass(ch, CLASS_MONK)))
        {
          send_to_char
            ("'You do not possess the necessary skills'\n\r",ch);
          return(TRUE);
        }
        break;
       case 7:
        sk_num = SKILL_PICK_LOCK;
        if (!HasClass(ch, CLASS_THIEF) && !(HasClass(ch, CLASS_MONK)))
        {
          send_to_char
            ("'You do not possess the necessary skills'\n\r",ch);
          return(TRUE);
        }
        break;
       case 8:
        sk_num = SKILL_SAFE_FALL;
        if (!HasClass(ch, CLASS_MONK))
        {
          send_to_char("'You do not possess the proper skills'\n\r", ch);
          return(TRUE);
        }
        break;
       case 9:
        sk_num = SKILL_DISARM;
        if (!HasClass(ch, CLASS_WARRIOR) && !(HasClass(ch, CLASS_MONK)))
        {
          send_to_char
            ("'You do not possess the necessary fighting skills'\n\r",ch);
          return(TRUE);
        }
        break;
       case 10:
        sk_num = SKILL_DODGE;
        if( !HasClass( ch, CLASS_WARRIOR ) && !HasClass( ch, CLASS_MONK ) &&
            !HasClass( ch, CLASS_PALADIN ) )
        {
          send_to_char
            ("'You do not possess the necessary fighting skills'\n\r",ch);
          return(TRUE);
        }
        break;
       case 11:
        sk_num = SKILL_SWITCH_OPP;
        if (!HasClass(ch, CLASS_WARRIOR) && !(HasClass(ch, CLASS_MONK)))
        {
          send_to_char
            ("'You do not possess the necessary fighting skills'\n\r",ch);
          return(TRUE);
        }
        break;
       case 12:
        sk_num = SKILL_SPRING_LEAP;
        if (!HasClass(ch, CLASS_MONK))
        {
          send_to_char("'You do not possess the proper skills'\n\r", ch);
          return(TRUE);
        }
        break;

       default:
        mudlog( LOG_SYSERR, "Strangeness in monk master (%d)", number);
        send_to_char("'Ack!  I feel faint!'\n\r", ch);
        return(TRUE);
      }
    }

    if( !HasClass(ch, CLASS_MONK) && GET_GOLD(ch) < charge)
    {
      send_to_char
        ("'Ah, but you do not have enough money to pay.'\n\r",ch);
      return(TRUE);
    }

    if (sk_num == SKILL_SAFE_FALL || sk_num == SKILL_DODGE)
    {
      if (ch->skills[sk_num].learned >= 95)
      {
        send_to_char
          ("'You are a master of this art, I can teach you no more.'\n\r",ch);
        SET_BIT(ch->skills[sk_num].flags, SKILL_KNOWN);
        return(TRUE);
      }
    } 
    else
    {
      if (ch->skills[sk_num].learned > 45)
      {
        SET_BIT(ch->skills[sk_num].flags, SKILL_KNOWN);
        send_to_char("'You must learn from practice and experience now.'\n\r", ch);
        return TRUE;
      }
    }

    if (ch->specials.spells_to_learn <= 0)
    {
      send_to_char
        ("'You must first use the knowledge you already have.\n\r",ch);
      return(TRUE);
    }

    if (!HasClass(ch, CLASS_MONK))
    {
      GET_GOLD(ch) -= charge;
    }
    send_to_char("'We will now begin.'\n\r",ch);
    ch->specials.spells_to_learn--;

    percent = ch->skills[sk_num].learned + int_app[ (int)GET_INT(ch) ].learn;
    ch->skills[sk_num].learned = MIN(95, percent);


    if (!IS_SET(ch->skills[sk_num].flags, SKILL_KNOWN))
    {
      SET_BIT(ch->skills[sk_num].flags, SKILL_KNOWN);
      SET_BIT(ch->skills[sk_num].flags, SKILL_KNOWN_MONK);
    }

    if (ch->skills[sk_num].learned >= 95)
      send_to_char("'You are now a master of this art.'\n\r", ch);
    return(TRUE);
  }
  else if (cmd == 243)
  {
    if (HasClass(ch, CLASS_MONK))
    {
      if (GET_LEVEL(ch,MONK_LEVEL_IND) <= 9)
      {
        GainLevel(ch, MONK_LEVEL_IND);
      } else {
        send_to_char("You must fight another monk for this title\n\r",ch);
      }
      return(TRUE);
    }

  }
  return(FALSE);
}

int DruidGuildMaster(struct char_data *ch, int cmd, const char *arg, struct char_data *mob, int type)
{
  int number, i, percent;
  char buf[MAX_INPUT_LENGTH];
  struct char_data *guildmaster;
  extern const char *spells[];
  extern struct spell_info_type spell_info[MAX_SPL_LIST];

  if ((cmd != 164) && (cmd != 170) && (cmd != 243)) return(FALSE);

  if (!ch->skills) return(FALSE);

  if (IS_IMMORTAL(ch))
    return(FALSE);

  if (check_soundproof(ch)) return(FALSE);

  guildmaster = FindMobInRoomWithFunction(ch->in_room, DruidGuildMaster);

  if (!guildmaster) return(FALSE);

  if (IS_NPC(ch)) {
    act("$N tells you 'What do i look like, an animal trainer?'", FALSE,
        ch, 0, guildmaster, TO_CHAR);
        return(FALSE);
  }

  if (HasClass(ch, CLASS_DRUID)) {
     if (cmd == 243) {  /* gain */
       if (GET_LEVEL(ch,DRUID_LEVEL_IND) <= 9) {
          GainLevel(ch, DRUID_LEVEL_IND);
        } else {
          send_to_char("You must fight another druid for this title\n\r",ch);
        }
        return(TRUE);
     }

     for (;*arg == ' '; arg++);

     if (!*arg) {
      sprintf(buf,"You have got %d practice sessions left.\n\r",
              ch->specials.spells_to_learn);
      send_to_char(buf, ch);
      send_to_char("You can practise any of these spells:\n\r", ch);
      for(i=0; *spells[i] != '\n'; i++)
        if (spell_info[i+1].spell_pointer &&
            (spell_info[i+1].min_level_druid<=
             GET_LEVEL(ch,DRUID_LEVEL_IND)) &&
            (spell_info[i+1].min_level_druid <=
             GetMaxLevel(guildmaster)-10)) {

            sprintf(buf,"[%d] %s %s \n\r",
                    spell_info[i+1].min_level_druid,
                    spells[i],how_good(ch->skills[i+1].learned));
          send_to_char(buf, ch);
        }

      return(TRUE);

    }
    for (;isspace(*arg);arg++);
    number = old_search_block(arg,0,strlen(arg),spells,FALSE);
    if(number == -1) {
      send_to_char("You do not know of this spell...\n\r", ch);
      return(TRUE);
    }
    if (GET_LEVEL(ch,DRUID_LEVEL_IND) < spell_info[number].min_level_druid) {
      send_to_char("You do not know of this spell...\n\r", ch);
      return(TRUE);
    }
    if (GetMaxLevel(guildmaster)-10 < spell_info[number].min_level_druid) {
      do_say(guildmaster, "I don't know of this spell.", 0);
      return(TRUE);
    }

    if (ch->specials.spells_to_learn <= 0) {
      send_to_char("You do not seem to be able to practice now.\n\r", ch);
      return(TRUE);
    }

    if (ch->skills[number].learned >= 45) {
      send_to_char("You must use this skill to get any better.  I cannot train you further.\n\r", ch);
      SET_BIT(ch->skills[number].flags, SKILL_KNOWN);
      return(TRUE);
    }

    send_to_char("You Practice for a while...\n\r", ch);
    ch->specials.spells_to_learn--;

if (!IS_SET(ch->skills[number].flags, SKILL_KNOWN)) {
     SET_BIT(ch->skills[number].flags, SKILL_KNOWN);
     SET_BIT(ch->skills[number].flags, SKILL_KNOWN_DRUID);
    }

    percent = ch->skills[number].learned+int_app[ (int)GET_RINT(ch) ].learn;
    ch->skills[number].learned = MIN(95, percent);

    if (ch->skills[number].learned >= 95) {
      send_to_char("You are now learned in this area.\n\r", ch);
      return(TRUE);

    }
  }
  else
    send_to_char("Oh.. i bet you think you're a druid?\n\r", ch);

  return(TRUE);
}



int Devil(struct char_data *ch, int cmd, const char *arg, struct char_data *mob, int type)
{
  return(magic_user(ch, cmd, arg, mob, type));
}

int Demon(struct char_data *ch, int cmd, const char *arg, struct char_data *mob, int type)
{
  return(magic_user(ch, cmd, arg, mob, type));

}


void DruidHeal(struct char_data *ch, int level)
{
  if (level > 13) {
    act("$n pronuncia le parole 'Woah! I feel GOOD! Heh.'.",
        1, ch,0,0,TO_ROOM);
    cast_cure_critic(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, ch, 0);
  } else if (level > 8) {
    act("$n pronuncia le parole 'I feel much better now!'.",
        1, ch,0,0,TO_ROOM);
    cast_cure_serious(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, ch, 0);
  } else {
    act("$n pronuncia le parole 'I feel good!'.", 1, ch,0,0,TO_ROOM);
    cast_cure_light(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, ch, 0);
  }
}

void DruidTree(struct char_data *ch)
{

  act("$n pronuncia le parole 'harumph!'", FALSE, ch, 0, 0, TO_ROOM);
  act("$n takes on the form and shape of a huge tree!", FALSE, ch, 0, 0, TO_ROOM);
  GET_RACE(ch)=RACE_TREE;
  ch->points.max_hit = GetMaxLevel(ch)*10;
  ch->points.hit += GetMaxLevel(ch)*5;
  free(ch->player.long_descr);
  free(ch->player.short_descr);
  ch->player.short_descr = (char *)strdup("The druid-tree");
  ch->player.long_descr = (char *)strdup("A huge tree stands here");
  GET_HIT(ch) = MIN(GET_MAX_HIT(ch), GET_HIT(ch));
  ch->mult_att = 6;
  ch->specials.damsizedice = 6;
  REMOVE_BIT(ch->specials.act, ACT_SPEC);

}

void DruidMob(struct char_data *ch)
{

  act("$n pronuncia le parole 'lagomorph'", FALSE, ch, 0, 0, TO_ROOM);
  act("$n takes on the form and shape of a huge lion", FALSE, ch, 0, 0, TO_ROOM);
  GET_RACE(ch)=RACE_PREDATOR;
  ch->points.max_hit *= 2;
  ch->points.hit += GET_HIT(ch)/2;
  free(ch->player.long_descr);
  free(ch->player.short_descr);
  ch->player.short_descr = (char *)strdup("The druid-lion");
  ch->player.long_descr = (char *)strdup("A huge lion stands here, his tail twitches menacingly");
  GET_HIT(ch) = MIN(GET_MAX_HIT(ch), GET_HIT(ch));
  ch->mult_att = 3;
  ch->specials.damnodice = 3;
  ch->specials.damsizedice = 4;
  REMOVE_BIT(ch->specials.act, ACT_SPEC);

}

int DruidChallenger(struct char_data *ch, int cmd, const char *arg, struct char_data *mob, int type)
{
  struct room_data *rp;
  int level;
  struct char_data *vict;
  int i;

  if (cmd) return(FALSE);
  if (!AWAKE(ch)) return(FALSE);

  rp = real_roomp(ch->in_room);
  if (!rp) return(FALSE);

  if (check_soundproof(ch)) return(FALSE);

  GreetPeople(ch);

  if (check_nomagic(ch, 0, 0))
    return(FALSE);

  if (GET_POS(ch) < POSITION_FIGHTING &&
      GET_POS(ch) > POSITION_SLEEPING) {
    StandUp(ch);
    return(TRUE);
  }

  if (number(0,101) > GetMaxLevel(ch)+40)   /* they 'failed' */
    return(TRUE);

  if (!ch->specials.fighting) {
    level = number(1, GetMaxLevel(ch));
    if (GET_HIT(ch) < GET_MAX_HIT(ch)) {
      DruidHeal(ch, level);
      return(TRUE);
    }
    if (!ch->equipment[WIELD]) {
      if (GetMaxLevel(ch) > 4) {
        act("$n pronuncia le parole 'gimme a light'", 1, ch, 0, 0, TO_ROOM);
        cast_flame_blade(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, ch, 0);
      }
      return(TRUE);
    }
    if (!affected_by_spell(ch, SPELL_BARKSKIN) && number(0,1)) {
      act("$n pronuncia le parole 'woof woof'", 1, ch, 0, 0, TO_ROOM);
      cast_barkskin(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, ch, 0);
      return(TRUE);
    }

    if (ch->in_room == 501) {
      if ((vict = FindAHatee(ch))==NULL) {
        vict = FindVictim(ch);
        if (vict)
          do_hit(ch, GET_NAME(vict), 0);
        return(FALSE);
      }
    }

  } else {

    if ((vict = FindAHatee(ch))==NULL)
      vict = FindVictim(ch);

    if (!vict)
      vict = ch->specials.fighting;

    if (!vict) return(FALSE);

    level = number(1, GetMaxLevel(ch));

    if ((GET_HIT(ch) < GET_MAX_HIT(ch)/2) && number(0,1)) {
      DruidHeal(ch, level);
      return(TRUE);
    }

    if (GetMaxLevel(ch) >= 20 && (!IS_AFFECTED(vict, AFF_SILENCE)) &&
        !number(0,4)) {
      act("$n pronuncia le parole 'ssshhhh'", 1, ch, 0, 0, TO_ROOM);
      cast_silence(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
      return(FALSE);
    }

    if (IS_AFFECTED(vict, AFF_FIRESHIELD) || IS_AFFECTED(vict, AFF_SANCTUARY)){
      if (GetMaxLevel(ch) >= GetMaxLevel(vict)) {
        act("$n pronuncia le parole 'use instaway instant magic remover'",
            1, ch, 0, 0, TO_ROOM);
        cast_dispel_magic(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
        return(TRUE);
      }
    }

    if (IsUndead(vict) && !number(0,2)) {
      act("$n pronuncia le parole 'see the light!'", 1, ch, 0, 0, TO_ROOM);
      cast_sunray(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
      return(FALSE);
    }

    if (vict->equipment[WIELD] && level > 19 ) {
      act("$n pronuncia le parole 'frogemoth'", 1, ch, 0, 0, TO_ROOM);
      cast_warp_weapon(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
      return(FALSE);
    }

    if (level > 23) {
      for (i=0;i<MAX_WEAR;i++) {
        if (vict->equipment[i]) {
          act("$n pronuncia le parole 'barbecue?'", 1, ch, 0, 0, TO_ROOM);
          cast_warp_weapon(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL,
                           vict, 0);
          return(FALSE);
        }
      }
    }

    if (IS_SET(rp->room_flags, NO_SUM) || IS_SET(rp->room_flags, TUNNEL) ||
        IS_SET(rp->room_flags, PRIVATE)) {
      DruidAttackSpells(ch, vict, level);
    } else { /*summon followers, call lightning */
      if (rp->sector_type == SECT_FOREST) {
        if (level > 16 && !number(0,5)) {
          act("$n pronuncia le parole 'briar'", 1, ch, 0, 0, TO_ROOM);
          cast_entangle(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL,
                        vict, 0);
          return(FALSE);
        }
        if (level >= 8 && !number(0,3)) {
          act("$n pronuncia le parole 'snap!'", 1, ch, 0, 0, TO_ROOM);
          cast_snare(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL,
                     vict, 0);
          return(FALSE);
        }
        if (level > 30 && ch->mult_att<6 && !number(0,8)) {
          DruidTree(ch);
          return(FALSE);
        }
        if (ch->mult_att < 2 && level > 10 && !number(0,8)) {
          DruidMob(ch);
          return(FALSE);
        }
        /* summoning */
        if (level > 30 && !number(0,4)) {
          act("$n pronuncia le parole 'Where is my SERVANT!'", FALSE, ch, 0,
              0, TO_ROOM);
          cast_fire_servant(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL,
                     0, 0);
          return(FALSE);
          do_order(ch, "followers guard on", 0);
        } else {
          if (level > 10 && !number(0,5)) {
          act("$n whistles", FALSE, ch, 0,
              0, TO_ROOM);
            cast_animal_summon_1(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL,
                              0, 0);
            return(FALSE);
          }
          if (level > 16 && !number(0,5)) {
          act("$n whistles loudly", FALSE, ch, 0,
              0, TO_ROOM);
            cast_animal_summon_2(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL,
                              0, 0);
            return(FALSE);
          }
          if (level > 24 && !number(0,5)) {
          act("$n whistles extremely loudly", FALSE, ch, 0,
              0, TO_ROOM);
            cast_animal_summon_3(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL,
                              0, 0);
            return(FALSE);
          }
        }
      } else {
        if (!IS_SET(rp->room_flags, INDOORS)) {
          if (level > 8 && !number(0,3)) {
          act("$n pronuncia le parole 'let it rain'", FALSE, ch, 0,
              0, TO_ROOM);
            cast_control_weather(GetMaxLevel(ch), ch, "worse",
                                 SPELL_TYPE_SPELL, 0, 0);
            return(FALSE);
          }
          if (level > 15 && !number(0,2)) {
          act("$n pronuncia le parole 'here lightning'", FALSE, ch, 0,
              0, TO_ROOM);
            cast_call_lightning(GetMaxLevel(ch), ch, "",
                                SPELL_TYPE_SPELL, vict, 0);
            return(FALSE);
          }
        }
      }
      DruidAttackSpells(ch, vict, level);
    }
  }
  return FALSE;
}

int druid( struct char_data *ch, int cmd, const char *arg, struct char_data *mob, 
           int type )
{
  struct room_data *rp;
  int level;
  struct char_data *vict;
  int i;

  if( type == EVENT_COMMAND ) 
    return(FALSE);
  if( !AWAKE(ch) )
    return(FALSE);

  rp = real_roomp(ch->in_room);
  if (!rp) 
    return(FALSE);

  if (check_soundproof(ch)) 
    return(FALSE);

  if( !ch->specials.fighting )
    GreetPeople(ch);

  if( check_nomagic( ch, 0, 0 ) )
    return(FALSE);

  if( GET_POS(ch) < POSITION_FIGHTING && GET_POS(ch) > POSITION_SLEEPING )
  {
    StandUp(ch);
    return(TRUE);
  }

  if( number( 0, 101 ) > GetMaxLevel( ch ) + 40 )   /* they 'failed' */
    return TRUE;

  if( !ch->specials.fighting )
  {
    level = number( 1, GetMaxLevel(ch) );
    if( GET_HIT(ch) < GET_MAX_HIT(ch) )
    {
      DruidHeal( ch, level );
      return(TRUE);
    }
    if( !ch->equipment[ WIELD ] )
    {
      if( GetMaxLevel(ch) > 4 && 
          ch->specials.damnodice * ch->specials.damsizedice < 16 )
      {
        act("$n pronuncia le parole 'gimme a light'", 1, ch, 0, 0, TO_ROOM);
        cast_flame_blade(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, ch, 0);
        return(TRUE);
      }
    }
    if( !affected_by_spell(ch, SPELL_BARKSKIN) && number(0,1) )
    {
      act("$n pronuncia le parole 'woof woof'", 1, ch, 0, 0, TO_ROOM);
      cast_barkskin(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, ch, 0);
      return(TRUE);
    }

  }
  else
  {
    if( ( vict = ch->specials.fighting ) == NULL )
    {
      if( ( vict = FindAHatee(ch) ) == NULL )
      {
        if( ( vict = FindVictim( ch ) ) == NULL )
        {
          return FALSE;
        }
      }
    }

    level = number( 1, GetMaxLevel(ch) );

    if( (GET_HIT(ch) < GET_MAX_HIT(ch)/2) && number(0,1))
    {
      DruidHeal(ch, level);
      return(TRUE);
    }

    if( GetMaxLevel(ch) >= 20 && (!IS_AFFECTED(vict, AFF_SILENCE)) &&
        !number(0,4))
    {
      act("$n pronuncia le parole 'ssshhhh'", 1, ch, 0, 0, TO_ROOM);
      cast_silence(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
      return(FALSE);
    }

    if( IS_AFFECTED(vict, AFF_FIRESHIELD) || IS_AFFECTED(vict, AFF_SANCTUARY) )
    {
      if( GetMaxLevel(ch) >= GetMaxLevel(vict) ) 
      {
        act("$n pronuncia le parole 'use instaway instant magic remover'",
            1, ch, 0, 0, TO_ROOM);
        cast_dispel_magic(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
        return(TRUE);
      }
    }

    if( IsUndead(vict) && !number(0,2) )
    {
      act("$n pronuncia le parole 'see the light!'", 1, ch, 0, 0, TO_ROOM);
      cast_sunray(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
      return(FALSE);
    }

    if( vict->equipment[WIELD] && level > 19 )
    {
      act("$n pronuncia le parole 'frogemoth'", 1, ch, 0, 0, TO_ROOM);
      cast_warp_weapon(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
      return(FALSE);
    }

    if (level > 23) 
    {
      for( i = 0;i < MAX_WEAR; i++ ) 
      {
        if (vict->equipment[i]) 
        {
          act("$n pronuncia le parole 'barbecue?'", 1, ch, 0, 0, TO_ROOM);
          cast_warp_weapon(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL,
                           vict, 0);
          return(FALSE);
        }
      }
    }

    if( IS_SET(rp->room_flags, NO_SUM) || IS_SET(rp->room_flags, TUNNEL) ||
        IS_SET(rp->room_flags, PRIVATE)) 
    {
      DruidAttackSpells(ch, vict, level);
    } 
    else 
    { /*summon followers, call lightning */
      if (rp->sector_type == SECT_FOREST) 
      {
        if (level > 16 && !number(0,5)) 
        {
          act("$n pronuncia le parole 'briar'", 1, ch, 0, 0, TO_ROOM);
          cast_entangle(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL,
                        vict, 0);
          return(FALSE);
        }
        if (level >= 8 && !number(0,3)) 
        {
          act("$n pronuncia le parole 'snap!'", 1, ch, 0, 0, TO_ROOM);
          cast_snare(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL,
                     vict, 0);
          return(FALSE);
        }
        if (level > 30 && ch->mult_att<6 && !number(0,8)) 
        {
          DruidTree(ch);
          return(FALSE);
        }
        if (ch->mult_att < 2 && level > 10 && !number(0,8)) 
        {
          DruidMob(ch);
          return(FALSE);
        }
        /* summoning */
        if (level > 30 && !number(0,4)) 
        {
          act("$n pronuncia le parole 'Where is my SERVANT!'", FALSE, ch, 0,
              0, TO_ROOM);
          cast_fire_servant(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL,
                     0, 0);
          return(FALSE);
          do_order(ch, "followers guard on", 0);
        } 
        else 
        {
          if (level > 10 && !number(0,5)) 
          {
            act( "$n fischia.", FALSE, ch, 0,
                 0, TO_ROOM);
            cast_animal_summon_1(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL,
                              0, 0);
            return(FALSE);
          }
          if (level > 16 && !number(0,5)) 
          {
            act( "$n fischia forte.", FALSE, ch, 0,
                 0, TO_ROOM);
            cast_animal_summon_2(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL,
                              0, 0);
            return(FALSE);
          }
          if (level > 24 && !number(0,5)) 
          {
            act( "$n fischia cosi forte da ferirti i timpani.", FALSE, ch, 0,
                 0, TO_ROOM);
            cast_animal_summon_3(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL,
                              0, 0);
            return(FALSE);
          }
        }
      } 
      else 
      {
        if (!IS_SET(rp->room_flags, INDOORS)) 
        {
          if (level > 8 && !number(0,3)) 
          {
            act( "$n pronuncia le parole 'let it rain'", FALSE, ch, 0,
                 0, TO_ROOM);
            cast_control_weather(GetMaxLevel(ch), ch, "worse",
                                 SPELL_TYPE_SPELL, 0, 0);
            return(FALSE);
          }
          if (level > 15 && !number(0,2)) 
          {
            act( "$n pronuncia le parole 'here lightning'", FALSE, ch, 0,
                 0, TO_ROOM);
            cast_call_lightning(GetMaxLevel(ch), ch, "",
                                SPELL_TYPE_SPELL, vict, 0);
            return(FALSE);
          }
        }
      }
      DruidAttackSpells(ch, vict, level);
    }
  }
  return FALSE;
}

int MonkChallenger(struct char_data *ch, int cmd, const char *arg, struct char_data *mob, int type)
{
  struct char_data *vict;

  if (cmd) return(FALSE);
  if (!AWAKE(ch)) return(FALSE);

  GreetPeople(ch);

  if (!ch->specials.fighting) {
    if (ch->in_room == 551) {
      if ((vict = FindAHatee(ch))==NULL) {
        vict = FindVictim(ch);
        if (vict)
          do_hit(ch, GET_NAME(vict), 0);
        return(FALSE);
      }
    }
  }
  MonkMove(ch);
  return(1);

}

#if 1

#define NOD  35
#define DRUID_MOB 600
#define MONK_MOB  650
#define FLEE 151

int druid_challenge_prep_room(struct char_data *ch, int cmd, const char *arg, struct room_data *rp, int type)
{
  struct room_data *me, *chal;
  int i, newr;
  struct obj_data *o;
  struct char_data *mob;

  me = real_roomp(ch->in_room);
  if (!me) return(FALSE);

  chal = real_roomp(ch->in_room+1);
  if (!chal) {
    send_to_char("The challenge room is gone.. please contact a god\n\r", ch);
    return(TRUE);
  }

  if (cmd == NOD) {

    if (!HasClass(ch, CLASS_DRUID)) {
      send_to_char("You're no druid\n\r", ch);
      return(FALSE);
    }

    if (GET_LEVEL(ch, DRUID_LEVEL_IND) < 10) {
      send_to_char("You have no business here, kid.\n\r", ch);
      return(FALSE);
    }

    if (GET_EXP(ch) <= titles[DRUID_LEVEL_IND]
                         [GET_LEVEL(ch, DRUID_LEVEL_IND)+1].exp-100) {
      send_to_char("You cannot advance now\n\r", ch);
      return(TRUE);
    }

    if (chal->river_speed != 0) {
      send_to_char("The challenge room is busy.. please wait\n\r", ch);
      return(TRUE);
    }
    for (i=0;i<MAX_WEAR;i++) {
      if (ch->equipment[i]) {
        o = unequip_char(ch, i);
        obj_to_char(o, ch);
      }
    }
    while (ch->carrying)
      extract_obj(ch->carrying);

    send_to_char("You are taken into the combat room.\n\r", ch);
    act("$n is ushered into the combat room", FALSE, ch, 0, 0, TO_ROOM);
    newr = ch->in_room+1;
    char_from_room(ch);
    char_to_room(ch, newr);
    /* load the mob at the same lev as char */
    mob = read_mobile(DRUID_MOB+GET_LEVEL(ch, DRUID_LEVEL_IND)-10, VIRTUAL);
    if (!mob) {
      send_to_char("The fight is called off.  go home\n\r", ch);
      return(TRUE);
    }
    char_to_room(mob, ch->in_room);
    chal->river_speed = 1;
    do_look(ch, "", 15);
    REMOVE_BIT(ch->specials.act, PLR_WIMPY);
    return(TRUE);
  }

  return(FALSE);

}


int druid_challenge_room(struct char_data *ch, int cmd, const char *arg, struct room_data *rp, int type)
{
  struct char_data *i;
  struct room_data *me;
  int rm;

   me = real_roomp(ch->in_room);
   if (!me) return(FALSE);

   rm = ch->in_room;

   if (!me->river_speed) return(FALSE);

   if (IS_PC(ch)) {
    REMOVE_BIT(ch->specials.act, PLR_WIMPY);
   }

   if (cmd == FLEE) {
     /* this person just lost */
     send_to_char("You lose\n\r",ch);
     if (IS_PC(ch)) {
       if (IS_NPC(ch)) {
         do_return(ch,"",0);
       }
       GET_EXP(ch) = MIN(titles[DRUID_LEVEL_IND]
                         [ (int)GET_LEVEL(ch, DRUID_LEVEL_IND) ].exp,
                         GET_EXP(ch));
       send_to_char("Go home\n\r", ch);
       char_from_room(ch);
       char_to_room(ch, rm-1);
       me->river_speed = 0;
       while (me->people)
         extract_char(me->people);

       return(TRUE);
     } else {
       if (mob_index[ch->nr].iVNum >= DRUID_MOB &&
           mob_index[ch->nr].iVNum <= DRUID_MOB+40) {
         extract_char(ch);
         /*
           find pc in room;
           */
         for (i=me->people;i;i=i->next_in_room)
           if (IS_PC(i)) {
             if (IS_NPC(i)) {
               do_return(i,"",0);
             }
             GET_EXP(i) = MAX(titles[DRUID_LEVEL_IND]
                              [GET_LEVEL(i, DRUID_LEVEL_IND)+1].exp+1,
                              GET_EXP(i));
             GainLevel(i, DRUID_LEVEL_IND);
             char_from_room(i);
             char_to_room(i, rm-1);
             if (affected_by_spell(i, SPELL_POISON)) {
               affect_from_char(ch, SPELL_POISON);
             }
             if (affected_by_spell(i, SPELL_HEAT_STUFF)) {
               affect_from_char(ch, SPELL_HEAT_STUFF);
             }

             while (me->people)
               extract_char(me->people);

             while (me->contents)
               extract_obj(me->contents);

             me->river_speed = 0;

             return(TRUE);
           }
         return(TRUE);
       } else {
         return(FALSE);
       }
     }
   }
  return(FALSE);

}


int monk_challenge_room(struct char_data *ch, int cmd, const char *arg, struct room_data *rp, int type)
{
  struct char_data *i;
  struct room_data *me;
  int rm;

   rm = ch->in_room;

   me = real_roomp(ch->in_room);
   if (!me) return(FALSE);

   if (!me->river_speed) return(FALSE);

   if (IS_PC(ch)) {
    REMOVE_BIT(ch->specials.act, PLR_WIMPY);
   }

   if (cmd == FLEE) {
     /* this person just lost */
     send_to_char("You lose\n\r",ch);
     if (IS_PC(ch)) {
       if (IS_NPC(ch)) {
         do_return(ch,"",0);
       }
       GET_EXP(ch) = MIN(titles[MONK_LEVEL_IND]
                         [ (int)GET_LEVEL(ch, MONK_LEVEL_IND) ].exp,
                         GET_EXP(ch));
       send_to_char("Go home\n\r", ch);
       char_from_room(ch);
       char_to_room(ch, rm-1);
       me->river_speed = 0;
       return(TRUE);
     } else {
       if (mob_index[ch->nr].iVNum >= MONK_MOB &&
           mob_index[ch->nr].iVNum <= MONK_MOB+40) {
         extract_char(ch);
         /*
           find pc in room;
           */
         for (i=me->people;i;i=i->next_in_room)
           if (IS_PC(i)) {
             if (IS_NPC(i)) {
               do_return(i,"",0);
             }
             GET_EXP(i) = MAX(titles[MONK_LEVEL_IND]
                              [GET_LEVEL(i, MONK_LEVEL_IND)+1].exp+1,
                              GET_EXP(i));
             GainLevel(i, MONK_LEVEL_IND);
             char_from_room(i);
             char_to_room(i, rm-1);

             while (me->people)
               extract_char(me->people);

             while (me->contents)
               extract_obj(me->contents);

             me->river_speed = 0;
             return(TRUE);
           }
         return(TRUE);
       } else {
         return(FALSE);
       }
     }
   }
  return(FALSE);

}

int monk_challenge_prep_room(struct char_data *ch, int cmd, const char *arg, struct room_data *rp, int type)
{
  struct room_data *me, *chal;
  int i, newr;
  struct obj_data *o;
  struct char_data *mob;

   me = real_roomp(ch->in_room);
   if (!me) return(FALSE);

  chal = real_roomp(ch->in_room+1);
  if (!chal) {
    send_to_char("The challenge room is gone.. please contact a god\n\r", ch);
    return(TRUE);
  }

  if (cmd == NOD) {

    if (!HasClass(ch, CLASS_MONK)) {
      send_to_char("You're no monk\n\r", ch);
      return(FALSE);
    }

    if (GET_LEVEL(ch, MONK_LEVEL_IND) < 10) {
      send_to_char("You have no business here, kid.\n\r", ch);
      return(FALSE);
    }

    if (GET_EXP(ch) <= titles[MONK_LEVEL_IND]
                         [GET_LEVEL(ch, MONK_LEVEL_IND)+1].exp-100) {
      send_to_char("You cannot advance now\n\r", ch);
      return(TRUE);
    }

    if (chal->river_speed != 0) {
      send_to_char("The challenge room is busy.. please wait\n\r", ch);
      return(TRUE);
    }
    for (i=0;i<MAX_WEAR;i++) {
      if (ch->equipment[i]) {
        o = unequip_char(ch, i);
        obj_to_char(o, ch);
      }
    }
    while (ch->carrying)
      extract_obj(ch->carrying);

    send_to_char("You are taken into the combat room.\n\r", ch);
    act("$n is ushered into the combat room", FALSE, ch, 0, 0, TO_ROOM);
    newr = ch->in_room+1;
    char_from_room(ch);
    char_to_room(ch, newr);
    /* load the mob at the same lev as char */
    mob = read_mobile(MONK_MOB+GET_LEVEL(ch, MONK_LEVEL_IND)-10, VIRTUAL);
    if (!mob) {
      send_to_char("The fight is called off.  go home\n\r", ch);
      return(TRUE);
    }
    char_to_room(mob, ch->in_room);
    chal->river_speed = 1;
    do_look(ch, "", 15);
    REMOVE_BIT(ch->specials.act, PLR_WIMPY);
    return(TRUE);
  }

  return(FALSE);
}


#endif




/************************************************************************/
#if 0
/*
  glass teleport ring
*/
int glass_teleport_ring(struct char_data *ch, int cmd, const char *arg, struct obj_data *obj, int type)
{



}

#endif

int portal(struct char_data *ch, int cmd, const char *arg, struct obj_data *obj, int type)
{
  struct obj_data *port;
  char obj_name[50];

  if( type == EVENT_COMMAND )
  {
    if( cmd != CMD_ENTER )
      return(FALSE);

    arg = one_argument(arg,obj_name);
    if( !( port = get_obj_in_list_vis( ch, obj_name,
                                       real_roomp(ch->in_room)->contents)))
      return(FALSE);

    if (port != obj)
      return(FALSE);

    if( port->obj_flags.value[1] <= 0 || port->obj_flags.value[1] > 32000 )
      send_to_char("The portal leads nowhere\n\r", ch);
    else
    {
      act("$n enters $p, and vanishes!", FALSE, ch, port, 0, TO_ROOM);
      act("You enter $p, and you are transported elsewhere", FALSE, ch, port, 0, TO_CHAR);
      char_from_room(ch);
      char_to_room(ch, port->obj_flags.value[1]);
      do_look(ch, "", 15);
      act("$n appears from thin air!", FALSE, ch, 0, 0, TO_ROOM);
    }
    return TRUE;
  }
  else
  {
    obj->obj_flags.value[0]--;
    if (obj->obj_flags.value[0] == 0)
    {
      if ((obj->in_room != NOWHERE) &&(real_roomp(obj->in_room)->people))
      {
        act("$p vanishes in a cloud of smoke!",
            FALSE, real_roomp(obj->in_room)->people, obj, 0, TO_ROOM);
        act("$p vanishes in a cloud of smoke!",
            FALSE, real_roomp(obj->in_room)->people, obj, 0, TO_CHAR);
      }
      extract_obj(obj);
    }
  }
  return FALSE;
}

int scraps(struct char_data *ch, int cmd, const char *arg, struct obj_data *obj, int type)
{
  if( type != EVENT_COMMAND)
  {
    if (obj->obj_flags.value[0])
      obj->obj_flags.value[0]--;

    if (obj->obj_flags.value[0] == 0 && obj->in_room) {
      if ((obj->in_room != NOWHERE) &&(real_roomp(obj->in_room)->people)) {
        act("$p disintegrates into atomic particles!",
            FALSE, real_roomp(obj->in_room)->people, obj, 0, TO_ROOM);
        act("$p disintegrates into atomic particles!",
            FALSE, real_roomp(obj->in_room)->people, obj, 0, TO_CHAR);
      }
      extract_obj(obj);
    }
  }
  return FALSE;
}

#define ATTACK_ROOM 3004

int attack_rats(struct char_data *ch, int cmd, const char *arg, struct char_data *mob, int type)
{
  int dir;

  if(type == EVENT_COMMAND)
    return(FALSE);

  if(type == EVENT_WINTER)
  {
    ch->generic = ATTACK_ROOM+number(0,26);
    AddHatred(ch,OP_RACE,RACE_HUMAN);  /* so they'll attack beggars, etc */
  }

  if(type == EVENT_SPRING)
    ch->generic = 0;

  if (ch->generic == 0)
    return(FALSE);

  dir = choose_exit_global(ch->in_room, ch->generic, MAX_ROOMS);
  if(dir == -1)
  {
    ch->generic = 0; /* assume we found it.. start wandering */
    return(FALSE); /* We Can't Go Anywhere. */
  }

  go_direction(ch, dir);
  return FALSE;
}

#define WHO_TO_HUNT  6112 /* green dragon */
#define WHERE_TO_SIT 3007 /* tavern */
#define WHO_TO_CALL  3063 /* mercenary */

int DragonHunterLeader( struct char_data *ch, int cmd, const char *arg,
                        struct char_data *mob, int type )
{
  register struct char_data *i, *j;
  int found = FALSE, dir, count;
  char buf[255];
  
  mob->lStartRoom = 0;

#if 0
  if( type == EVENT_COMMAND)
  {
    if(cmd == 19)
    {
      if( !strncasecmp( arg, "biff", 4 ) )
      {
        do_follow(ch, arg, cmd);
        do_group(mob, GET_NAME(ch), 0);
        return(TRUE);
      }
    }
    return FALSE;
  }
#endif
  
  if( type == EVENT_TICK )
  {
    if(ch->specials.position == POSITION_SITTING)
    {
      ch->generic = 0;
      switch( number( 1, 10 ) )
      {
      case 1:
        do_emote( ch, "farfuglia qualcosa sui bei tempi andati.", 0);
        break;
      case 2:
        do_say( ch, "Mi mancano tanto i vecchi tempi in cui cacciavo draghi "
                    "tutto il giorno.", 0);
        do_say( ch, "Dovrei farlo piu` spesso per mantenermi in forma.", 0);
        break;
      default:
        break;
      }

      return TRUE;
    }
    if(ch->specials.position == POSITION_STANDING)
    {
      if( ch->generic <= 20 )
      {
        ch->generic++;
        return FALSE;
      }
      else if( ch->generic == 21)
      {
        for( i = character_list; i; i = i->next )
        {
          if( IS_MOB(i) && mob_index[i->nr].iVNum == WHO_TO_HUNT )
          {
            found = TRUE;
            break;
          }
        }
        if( !found )
        {
          ch->generic = 25;
          mudlog( LOG_CHECK, "Biff non trova il drago mentra sta per partire" );
          do_say( ch, "Maledizione! Il drago e` morto. Meglio tornare alla "
                      "taverna!", 0 );
        }
        else
        {
          do_say( ch, "Bene, seguitemi, che andiamo ad uccidere un drago!", 0 );
          if( !IS_AFFECTED( ch, AFF_GROUP ) )
            SET_BIT(ch->specials.affected_by, AFF_GROUP);
          /*do_group( ch, "biff", 0 );*/
          ch->generic = 23;
          count = 1;
          for( i = real_roomp(ch->in_room)->people; i; i = i->next_in_room)
          {
            if( IS_MOB(i) && mob_index[i->nr].iVNum == WHO_TO_CALL )
            {
              (*mob_index[i->nr].func)( i, 0, "", ch, EVENT_FOLLOW );
              sprintf( buf, "%d.%s", count, GET_NAME(i) );
              do_group( ch, buf, 0 );
              count++;
            }
            else if( i->master && i->master == ch && GetMaxLevel( i ) > 5 )
            {
              sprintf(buf, "%s", GET_NAME(i));
              do_group(ch, buf, 0);
            }
            else if( i->master && i->master == ch )
            {
              sprintf( buf, "%s Tu sei troppo giovane, ancora!", GET_NAME(i) );
              do_tell(ch, buf, 0);
            }
          }
          spell_fly_group( 40, ch, 0, 0 );
        }
        return FALSE;
      }
      else if( ch->generic == 23 )
      {
        for( i = character_list; i; i = i->next )
        {
          if( IS_MOB( i ) && mob_index[i->nr].iVNum == WHO_TO_HUNT )
          {
            found = TRUE;
            break;
          }
        }
        if( !found )
        {
          ch->generic = 25;
          do_say( ch, "Maledizione! Il drago e` morto! Meglio tornare alla "
                      "taverna!", 0 );
          mudlog( LOG_CHECK, "Il drago di Biff e` morto." );
        }
        else
        {
          dir = choose_exit_global( ch->in_room, i->in_room, MAX_ROOMS );
          if( dir == -1 ) /* can't go anywhere, wait... */
          {
            ch->generic = 25;
            mudlog( LOG_CHECK, "Biff non trova la strada per il Drago." );
            do_say( ch, "Maledizione! Non ricordo la strada.", 0 );
            return(FALSE);
          }
          go_direction( ch, dir );

          if( ch->in_room == i->in_room )
          { /* we're here! */
            do_shout(ch, "Il drago deve morire!", 0);

            for( j = real_roomp(ch->in_room)->people; j; j = j->next_in_room )
              if(IS_MOB(j) && mob_index[j->nr].iVNum == WHO_TO_CALL )
                (*mob_index[j->nr].func)( i, 0, "", j, EVENT_ATTACK );

            ch->generic = 24;
            hit( ch, i, TYPE_UNDEFINED );
          }
          return(FALSE);
        }
      }
      else if( ch->generic == 24 )
      {
        do_say( ch, "Torniama alla taverna! Ho bisogno di bere!", 0 );
        ch->generic = 25;
      }
      else if(ch->generic == 25)
      {
        dir = choose_exit_global( ch->in_room, WHERE_TO_SIT, MAX_ROOMS );
        if( dir != -1 )
          go_direction(ch, dir);
        else if( ch->in_room != WHERE_TO_SIT )
        {
          mudlog( LOG_CHECK, "Biff non trova la strada per tornare al bar" );
          ch->generic = 0;
        }

        if( ch->in_room == WHERE_TO_SIT )
        {
          do_say( ch, "Ahhh, e` tempo per un bel bicchiere di vino!", 0 );
          for( i = real_roomp(ch->in_room)->people; i; i = i->next_in_room )
            if(IS_MOB(i) && (mob_index[i->nr].iVNum == WHO_TO_CALL))
              (*mob_index[i->nr].func)(i, 0, "", i, EVENT_FOLLOW);
          do_sit(ch, "", 0);
          do_say(ch, "Allora taverniere, che ne dici di portarmi un litrozzo "
                     "di rosso ?", 0);
          ch->generic = 0;
        }
      }
    }
  }

  if( type == EVENT_WEEK )
  { /* months are TOO long */
    if( ch->specials.position != POSITION_SITTING )
      return(FALSE); /* We're doing something else, ignore */

    for(i = character_list; i; i = i->next)
    {
      if( IS_MOB(i) && mob_index[i->nr].iVNum == WHO_TO_HUNT )
      {
        found = TRUE;
        break;
      }
    }
    if(!found)
    {
      mudlog( LOG_CHECK, "Biff non trova il Drago." );
      return(FALSE); /* No Dragon in the game, ignore */
    }
    for( i = character_list; i; i = i->next )
    {
      if( IS_MOB(i) && mob_index[i->nr].iVNum == WHO_TO_CALL )
      {
        (*mob_index[i->nr].func)(i, 0, "", ch, EVENT_GATHER);
      }
    }
    do_shout( ch, "Tutti quelli che vogliono cacciare un drago vengano da me!",
              0 );
    do_stand( ch, "", 0 );
  }

  return(FALSE);
}

int HuntingMercenary( struct char_data *ch, int cmd, const char *arg,
                      struct char_data *mob, int type)
{
  int dir;

  if(type == EVENT_COMMAND)
    return(FALSE);

  if(type == EVENT_TICK)
  {
    if(ch->generic == 1)
    { /* Going to room */
      if(!IS_SET(ch->specials.act, ACT_SENTINEL) )
        SET_BIT(ch->specials.act, ACT_SENTINEL);
      dir = choose_exit_global(ch->in_room, WHERE_TO_SIT, MAX_ROOMS);
      if(dir == -1)
        return(FALSE);
      go_direction(ch, dir);

      if(ch->in_room == WHERE_TO_SIT)
        ch->generic = 0;
    }
    return(FALSE);
  }

  if(type == EVENT_GATHER)
  {
    ch->generic = 1;
    return(FALSE);
  }

  if(type == EVENT_ATTACK)
  {
    hit( mob, ch, TYPE_UNDEFINED);
    return(FALSE);
  }

  if(type == EVENT_FOLLOW)
  {
    if(ch == mob)
    {
      if(IS_SET(ch->specials.act, ACT_SENTINEL))
        REMOVE_BIT(ch->specials.act, ACT_SENTINEL);
      ch->generic = 0;
      stop_follower(ch);
    }
    else
     add_follower(ch, mob);
  }

  return(FALSE);
}

int SlotMachine( struct char_data *ch, int cmd, const char *arg,
                 struct obj_data *obj, int type )
{
  int c, i[3], ind = 0;
  char buf[255];
  static long jackpot = 25;


  if( cmd != CMD_PULL )
    return FALSE;

  while( *arg && isspace( *arg ) )
    arg++;
  if( !strlen( arg ) || strncasecmp( arg, "leva", strlen( arg ) ) )
    return FALSE;

  if( GET_GOLD( ch ) < 25 )
  {
    send_to_char( "Non hai abbastanza soldi!\n\r", ch );
    return TRUE;
  }

  act( "$n tira la leva della slot machine.", FALSE, ch, 0, 0, TO_ROOM );
  act( "Gli slot cominciano a girare come tiri la leva della slot machine",
       FALSE, ch, 0, 0, TO_CHAR );

  if( jackpot == 0 )
    jackpot = 25; /* always at LEAST have 25 in there */

  GET_GOLD(ch) -= 25;
  jackpot += 25;

  buf[ 0 ] = '\0';

  for( c = 0; c <= 2; c++ )
  {
    i[ c ] = number( 1, 28 );
    switch( i[ c ] )
    {
    case 1:
    case 2:
    case 3:
    case 4:
    case 5:
    case 6:
    case 7:
      i[c] = 0;
      strcat( buf, "Limone " );
      break;
    case 8:
    case 9:
    case 10:
    case 11:
    case 12:
    case 13:
    case 14:
      i[ c ] = 1;
      strcat( buf, "Arancio " );
      break;
    case 15:
    case 16:
    case 17:
    case 18:
    case 19:
    case 20:
      i[ c ] = 2;
      strcat( buf, "Banana " );
      break;
    case 21:
    case 22:
    case 23:
    case 24:
    case 25:
      i[ c ] = 3;
      strcat( buf, "Pesca " );
      break;
    case 26:
    case 27:
      i[ c ] = 4;
      strcat( buf, "$c0001Bar$c0007 " );
      break;
    case 28:
      i[ c ] = 5;
      strcat( buf, "$c0011Oro$c0007 " );
      break;
    }
  }
  strcat( buf, "\n\r" );
  send_to_char( buf, ch );

  if( i[ 0 ] == i[ 1 ] && i[ 1 ] == i[ 2 ] )
  {
    send_to_char( "$c0015Hai vinto!\n\r", ch );
    /* Ok, they've won, now how much? */
    act( "Una cascata di monete esce dalla slot machine.", FALSE, ch, 0, 0,
         TO_ROOM );

    switch(i[0])
    {
     case 0:
      ind = 25; /* Give them back what they put in */
      break;
     case 1:
      ind = 100;
      break;
     case 2:
      ind = 200;
      break;
     case 3:
      ind = 500;
      break;
     case 4:
      ind = 1000;
      break;
     case 5:
      ind = jackpot; /* Wow! We've won big! */
      act( "$c0014Un sirena inizia a suonare e le luci sopra la slot machine "
           "si accendono!", FALSE, ch, 0, 0, TO_ROOM);
      break;
    }

    if( ind > jackpot )
      ind = jackpot; /* Can only win as much as there is */

    sprintf( buf, "$c0015Hai vinto %d monete!\n\r", ind );
    send_to_char( buf, ch );

    GET_GOLD( ch ) += ind;
    jackpot -= ind;
    return TRUE;
  }

  send_to_char( "Mi dispiace, ma hai perso.\n\r", ch );
  return TRUE;
}

#define AST_MOB_NUM 2715

int astral_portal( struct char_data *ch, int cmd, const char *arg,
                   struct char_data *mob, int type)
{
  int destination[20];
  char buf[50];
  int j;
  struct char_data *portal;

  destination[0]=104;                /* mob 2715 */
  destination[1]=21114;                /* mob 2716 */
  destination[2]=1474;                /* ... */
  destination[3]=1633;
  destination[4]=4109;
  destination[5]=5000;
  destination[6]=5126;
  destination[7]=5221;
  destination[8]=6513;
  destination[9]=7069;
  destination[10]=6601;
  destination[11]=9359;
  destination[12]=13809;
  destination[13]=16925;
  destination[14]=20031;
  destination[15]=27431;
  destination[16]=21210;
  destination[17]=25041;        /* mob 2732 */

  /* To add another color pool, create another mobile (2733, etc) and add */
  /* another destination.                                                 */

  if( cmd != 7 )
    return(FALSE); /* enter */
  one_argument(arg,buf);
  if(*buf)
  {
    if( !str_cmp("pool",buf) || !str_cmp("color",buf) ||
        !str_cmp("color pool",buf) )
    {
      if( ( portal = get_char_room( "color pool", ch->in_room ) ) )
      {
        j = destination[ mob_index[ portal->nr ].iVNum - AST_MOB_NUM ];
        if( j > 0 && j < 32000 )
        {
          send_to_char("\n\r",ch);
          send_to_char("You attempt to enter the pool, and it gives.\n\r",ch);
          send_to_char("You press on further and the pool surrounds you, like some soft membrane.\n\r",ch);
          send_to_char("There is a slight wrenching sensation, and then the color disappears.\n\r",ch);
          send_to_char("\n\r",ch);

          act( "$n entra nella piscina. Le acque colorate l$b avvolgono "
               "inghiottendol$b.", TRUE, ch, 0, 0, TO_ROOM );
          char_from_room(ch);
          char_to_room(ch,j);
          act( "$n si materializza con uno spruzzo di acqua colorata.", TRUE,
               ch, 0, 0, TO_ROOM );
          do_look(ch, "", 15);
          return(TRUE);
        }
      }
    }
    else
      return(FALSE);
  }
  return(FALSE);
}





extern int gevent;
extern float shop_multiplier;

int DwarvenMiners(struct char_data *ch, int cmd, const char *arg, struct char_data *mob, int type)
{
 if(type == EVENT_COMMAND)
   return(FALSE);

 if(type == EVENT_END_STRIKE) {
    if(ch->specials.position == POSITION_SITTING) {
      do_emote(ch, "is off strike.", 0);
      do_stand(ch, "", 0);
      ch->specials.default_pos = POSITION_STANDING;
      ch->player.long_descr = (char *) realloc(ch->player.long_descr,
         sizeof(char) * 50);
      strcpy(ch->player.long_descr, "A dwarven mine-worker is here, working the mines.\n\r");
      if(gevent != 0)
         gevent = 0;
      shop_multiplier = 0;
    }
  }

 if(type == EVENT_DWARVES_STRIKE) {
   if(ch->specials.position == POSITION_STANDING) {
      do_emote(ch, "is on strike.", 0);
      do_sit(ch, "", 0);
      ch->specials.default_pos = POSITION_SITTING;
      ch->player.long_descr = (char *) realloc(ch->player.long_descr,
            sizeof(char) * 55);
      strcpy(ch->player.long_descr, "A dwarven mine-worker is sitting here on-strike\n\r");
    }
    ch->generic = 30;
 }


 if(type == EVENT_TICK) {
    if(gevent != DWARVES_STRIKE) {
      ch->generic = 0;
      return(FALSE);
    }

    ch->generic++;
    if(ch->generic == 30) { /* strike over, back to work */
       PulseMobiles(EVENT_END_STRIKE);
       if(ch->specials.position == POSITION_SITTING) {
         do_emote(ch, "is off strike.", 0);
         do_stand(ch, "", 0);
         ch->specials.default_pos = POSITION_STANDING;
         ch->player.long_descr = (char *) realloc(ch->player.long_descr,
            sizeof(char) * 65);
         strcpy(ch->player.long_descr, "A dwarven mine-worker is here, working the mines.\n\r");
       }
       ch->generic = 0;
       gevent = 0;
       shop_multiplier = 0;
     }
  }

 if((type == EVENT_BIRTH) && (gevent != DWARVES_STRIKE))
    return(FALSE);

 if(type == EVENT_BIRTH) {
    if(ch->specials.position == POSITION_STANDING) {
      do_emote(ch, "is on strike.", 0);
      do_sit(ch, "", 0);
      ch->specials.default_pos = POSITION_SITTING;
      ch->player.long_descr = (char *) realloc(ch->player.long_descr,
            sizeof(char) * 55);
      strcpy(ch->player.long_descr, "A dwarven mine-worker is sitting here on-strike\n\r");
    }
    ch->generic = 30;
  }

 if(type == EVENT_WEEK) {
    if(gevent != 0)
      return(FALSE); /* something else happening? FORGET IT! */

    if(number(1,6) != 5)
      return(FALSE); /* 1 in 6 chance of striking this week */

    PulseMobiles(EVENT_DWARVES_STRIKE);
    gevent = DWARVES_STRIKE;
    switch(number(1,5)) { /*severity*/
    case 1: shop_multiplier = 0.25;
            break;
    case 2: shop_multiplier = 0.5;
            break;
    case 3: shop_multiplier = 0.75;
            break;
    case 4: shop_multiplier = 1.0; /* youch! */
            break;
    case 5: shop_multiplier = 1.5; /* heh ;-) */
             break;
    }
  }
  return(FALSE);
}


/* From the appendages of Gecko... (now you know who to blame =) */

int real_rabbit(struct char_data *ch, int cmd, const char *arg, struct char_data *mob, int type)
{
  struct char_data *i;

  if (cmd || !AWAKE(ch) || ch->specials.fighting)
    return FALSE;

  for (i = real_roomp(ch->in_room)->people; i; i = i->next_in_room)
    if (IS_NPC(i) && (mob_index[i->nr].iVNum == 6005) && !number(0,3)) {
      do_emote(ch, "sees the damn fox and runs like hell.", 0);
      do_flee(ch, "\0", 0);
      return TRUE;
    }

  if (!number(0,5))
  {
    switch(number(1,2))
   {
    case 1:do_emote(ch, "nibbles on some grass.", 0);
           break;
    case 2:do_emote(ch, "bounces lighty to another patch of grass.", 0);
               break;
   }
    return TRUE;
  }
  return FALSE;
}

int real_fox(struct char_data *ch, int cmd, const char *arg, struct char_data *mob, int type)
{
  struct char_data *i;
  struct obj_data  *j, *k, *next;

  if (cmd || !AWAKE(ch) || ch->specials.fighting)
    return FALSE;

  if (ch->generic) {
    ch->generic--;
    return TRUE;
  }

  for (j = real_roomp(ch->in_room)->contents; j; j = j->next_content) {
    if (GET_ITEM_TYPE(j) == ITEM_CONTAINER &&
        j->obj_flags.value[3] &&
        !strcmp(j->name, "corpse rabbit")) {
      do_emote(ch, "gorges on the corpse of a rabbit.", 0);
      for(k = j->contains; k; k = next) {
        next = k->next_content;
        obj_from_obj(k);
        obj_to_room(k, ch->in_room);
      }
      extract_obj(j);
      ch->generic = 10;
      return(TRUE);
    }
  }

  for (i = real_roomp(ch->in_room)->people; i; i = i->next_in_room)
    if (IS_NPC(i) && (mob_index[i->nr].iVNum == 6001) && !number(0,3)) {
      do_emote(ch, "yips and starts to make dinner.", 0);
      hit(ch, i, TYPE_UNDEFINED);
      return TRUE;
    }

  return FALSE;
}


/*
  holy hand-grenade of antioch code
*/

int antioch_grenade(struct char_data *ch, int cmd, const char *arg, struct obj_data *obj, int type)
{

  if (type == EVENT_TICK) {
    if (obj->obj_flags.value[0])
      obj->obj_flags.value[0] -= 1;
  }

  if (type != EVENT_COMMAND)
    return(0);

  if (cmd == CMD_SAY || cmd == CMD_ASAY) {
    while (*arg == ' ')
      arg++;

    if (!strcmp(arg, "one")) {
      obj->obj_flags.value[0] = 4;
    } else if (!strcmp(arg, "two")) {
      if (obj->obj_flags.value[0] >= 3 && (obj->obj_flags.value[0] <= 4))
        obj->obj_flags.value[0] = 15;
    } else if (!strcmp(arg, "five")) {
      if (obj->obj_flags.value[0] >= 14 && obj->obj_flags.value[0] <= 15)
        obj->obj_flags.value[0] = 35;
      else
        obj->obj_flags.value[0] = 0;
    } else if (!strcmp(arg, "three")) {
      if (obj->obj_flags.value[0] >= 14)
        obj->obj_flags.value[0] += 10;
    } else {
      return(0);
    }
  }
  return(0);
}


int barbarian_guildmaster(struct char_data *ch, int cmd, const char *arg, struct char_data *mob, int type)
{
  char buf[256];
  const char *n_skills[] =
  {
    "disarm",           /*  1 */
    "doorbash",         /*  2 */
    "spy",              /*  3 */
    "retreat",          /*  4 */
    "switch opponents", /*  5 */
    "disguise",                 /*  6 */
    "climb",                 /*  7 */
    "hunt",                 /*  8 */
    "dodge",                /*  9 */
    "bash",                /* 10 */
    "first aid",        /* 11 */
    "berserk",          /* 12 */
    "avoid backattack", /* 13 */
    "find food",        /* 14 */
    "find water",       /* 15 */
    "skin",
    "camouflage",
    "bellow",
    "\n",
  };
  int percent=0, number=0;
  int sk_num;

  if (!AWAKE(ch))
    return(FALSE);

  if (!cmd)
  {
    if (ch->specials.fighting)
    {
      return(fighter(ch, cmd, arg, ch, 0));
    }
    return(FALSE);
  }

  if (!ch->skills)
    return(FALSE);

  if (check_soundproof(ch))
    return(FALSE);


  for(; *arg==' '; arg++); /* ditch spaces */

  if( cmd == 164 || cmd == 170 || cmd == 243 )
  {
    if( !HasClass( ch, CLASS_BARBARIAN ) )
    {
      act( "$N ti dice 'Io non insegno ai pagani!'.", TRUE, ch, 0, mob,
           TO_CHAR );
      return(TRUE);
    }

    if( IS_NPC( ch ) )
    {
      act( "$N ti dice 'Ti sembro forse un addestratore di animali ?'.",
           TRUE, ch, 0, mob, TO_CHAR );
      return(FALSE);
    }

    if (cmd==243)
    {
      if( GetMaxLevel( ch ) > MAX_MORT )
      {
        act( "$N ti dice 'Devi imparare da qualcun altro adessso'.",
             TRUE, ch, 0, mob, TO_CHAR );
        return(FALSE);
      } /* to high a level, can't immort them! */

      if( GET_EXP( ch ) <
          titles[ BARBARIAN_LEVEL_IND ]
                [ GET_LEVEL( ch, BARBARIAN_LEVEL_IND ) + 1 ].exp )
      {
        act( "$N ti dice 'Non sei ancora pronto per il nuovo livello'.",
             TRUE, ch, 0, mob, TO_CHAR );
        return(FALSE);
      }
      else
      {
        GainLevel( ch, BARBARIAN_LEVEL_IND );
        return(TRUE);
      }
    } /* end gain */

    if( !arg || (strlen(arg) == 0 ) )
    {
      sprintf(buf,"You have got %d practice sessions left.\n\r",
              ch->specials.spells_to_learn);
      send_to_char(buf,ch);
      sprintf(buf," disarm           :  %s\n\r",how_good(ch->skills[SKILL_DISARM].learned));
      send_to_char(buf,ch);
      sprintf(buf," doorbash         :  %s\n\r",how_good(ch->skills[SKILL_DOORBASH].learned));
      send_to_char(buf,ch);
      sprintf(buf," spy              :  %s\n\r",how_good(ch->skills[SKILL_SPY].learned));
      send_to_char(buf,ch);
      sprintf(buf," retreat          :  %s\n\r",how_good(ch->skills[SKILL_RETREAT].learned));
      send_to_char(buf,ch);
      sprintf(buf," switch           :  %s\n\r",how_good(ch->skills[SKILL_SWITCH_OPP].learned));
      send_to_char(buf,ch);
      sprintf(buf," disguise         :  %s\n\r",how_good(ch->skills[SKILL_DISGUISE].learned));
      send_to_char(buf,ch);
      sprintf(buf, " climb            :  %s\n\r",how_good(ch->skills[SKILL_CLIMB].learned));
      send_to_char(buf,ch);
      sprintf(buf," hunt             :  %s\n\r",how_good(ch->skills[SKILL_HUNT].learned));
      send_to_char(buf,ch);
      sprintf(buf," dodge            :  %s\n\r",how_good(ch->skills[SKILL_DODGE].learned));
      send_to_char(buf,ch);
      sprintf(buf," bash             :  %s\n\r",how_good(ch->skills[SKILL_BASH].learned));
      send_to_char(buf,ch);
      sprintf(buf," first aid        :  %s\n\r",how_good(ch->skills[SKILL_FIRST_AID].learned));
      send_to_char(buf,ch);
      sprintf(buf," berserk          :  %s\n\r",how_good(ch->skills[SKILL_BERSERK].learned));
      send_to_char(buf,ch);
      sprintf(buf," avoid backattack :  %s\n\r",how_good(ch->skills[SKILL_AVOID_BACK_ATTACK].learned));
      send_to_char(buf,ch);
      sprintf(buf," find food        :  %s\n\r",how_good(ch->skills[SKILL_FIND_FOOD].learned));
      send_to_char(buf,ch);
      sprintf(buf," find water       :  %s\n\r",how_good(ch->skills[SKILL_FIND_WATER].learned));
      send_to_char(buf,ch);
      sprintf(buf," skin             :  %s\n\r",how_good(ch->skills[SKILL_TAN].learned));
      send_to_char(buf,ch);
      sprintf(buf," camouflage       :  %s\n\r",how_good(ch->skills[SKILL_HIDE].learned));
      send_to_char(buf,ch);
      sprintf(buf," bellow           :  %s\n\r",how_good(ch->skills[SKILL_BELLOW].learned));
      send_to_char(buf,ch);

      return(TRUE);
    }
    else
    {
      number = old_search_block(arg,0,strlen(arg),n_skills,FALSE);
      send_to_char ("The Barbarian master says ",ch);

      if (number == -1) {
        send_to_char("'I do not know of this skill.'\n\r", ch);
        return(TRUE);
      }

      switch(number)
      {
      case 0:
      case 1:
        sk_num = SKILL_DISARM;
        break;
      case 2:
        sk_num = SKILL_DOORBASH;
        break;
      case 3:
        sk_num = SKILL_SPY;
        break;
      case 4:
        sk_num = SKILL_RETREAT;
        break;
      case 5:
        sk_num = SKILL_SWITCH_OPP;
        break;
      case 6:
        sk_num = SKILL_DISGUISE;
        break;
      case 7:
        sk_num = SKILL_CLIMB;
        break;
      case 8:
        sk_num = SKILL_HUNT;
        break;
      case 9:
        sk_num = SKILL_DODGE;
        break;
     case 10:
        sk_num = SKILL_BASH;
        break;
     case 11:
       sk_num = SKILL_FIRST_AID;
       break;
    case 12:
        sk_num = SKILL_BERSERK;
        break;
    case 13:
        sk_num = SKILL_AVOID_BACK_ATTACK;
        break;
    case 14:
        sk_num = SKILL_FIND_FOOD;
        break;
    case 15:
        sk_num = SKILL_FIND_WATER;
        break;
    case 16:
        sk_num = SKILL_TAN;
        break;
    case 17:
        sk_num = SKILL_HIDE;
        break;
    case 18:
        sk_num = SKILL_BELLOW;
        break;
      default:
        mudlog( LOG_SYSERR, "Strangeness in Barbarian (%d)", number);
        send_to_char("'Ack!  I feel sick!'\n\r", ch);
        return(TRUE);
      }
    }

    if( sk_num == SKILL_HUNT || sk_num == SKILL_DODGE )
    {
      if( ch->skills[sk_num].learned >= 95 )
      {
        send_to_char
          ("'You are a master of this art, I can teach you no more.'\n\r",ch);
        return(TRUE);
      }
    }
    else
    {
      if( ch->skills[sk_num].learned > 45 )
      {
        send_to_char("'You must learn from practice and experience now.'\n\r", ch);
        SET_BIT(ch->skills[sk_num].flags, SKILL_KNOWN);
        return(TRUE);
      }
    }

    if( ch->specials.spells_to_learn <= 0 )
    {
      send_to_char
        ("'You must first use the knowledge you already have.'\n\r",ch);
      return(FALSE);
    }

    send_to_char("'I will now show you the ways of our people...'\n\r",ch);
    ch->specials.spells_to_learn--;

    if (!IS_SET(ch->skills[sk_num].flags, SKILL_KNOWN))
    {
      SET_BIT(ch->skills[sk_num].flags, SKILL_KNOWN);
      SET_BIT(ch->skills[sk_num].flags, SKILL_KNOWN_BARBARIAN);
    }

    percent = ch->skills[sk_num].learned + int_app[ (int)GET_INT(ch) ].learn;
    ch->skills[sk_num].learned = MIN(95, percent);

    if (ch->skills[sk_num].learned >= 95)
      send_to_char("'You are now a master of this art.'\n\r", ch);
    return(TRUE);
  }

  return(FALSE);
}

int RangerGuildmaster(struct char_data *ch, int cmd, const char *arg, struct char_data *mob, int type)
{

  int number, i, percent;
  char buf[MAX_INPUT_LENGTH];
  struct char_data *guildmaster;
  extern const char *spells[];
  extern struct spell_info_type spell_info[MAX_SPL_LIST];

  if (!ch->skills)
    return(FALSE);

  if (check_soundproof(ch))
    return(FALSE);

  if (IS_IMMORTAL(ch))
    return(FALSE);

  guildmaster = FindMobInRoomWithFunction(ch->in_room, RangerGuildmaster);

  if (!guildmaster) return(FALSE);

  if ((cmd != 164) && (cmd != 170) && (cmd != 243) && (cmd != 72))
      return(FALSE);

  if (IS_NPC(ch)) {
    act("$N tells you 'What do i look like, an animal trainer?'", FALSE,
        ch, 0, guildmaster, TO_CHAR);
    return(FALSE);
  }

  for (;*arg == ' '; arg++);

  if (HasClass(ch, CLASS_RANGER))
  {
    if (cmd == 243) /*gain */
    {
      if (GET_LEVEL(ch,RANGER_LEVEL_IND) < GetMaxLevel(guildmaster)-10)
      {
        if( GET_EXP(ch) <
             titles[RANGER_LEVEL_IND][GET_LEVEL(ch, RANGER_LEVEL_IND)+1].exp )
        {
          send_to_char("You are not yet ready to gain.\n\r", ch);
          return(FALSE);
        }
        else
        {
          GainLevel(ch,RANGER_LEVEL_IND);
            return(TRUE);
        }
      }
      else
      {
        send_to_char("I cannot train you.. You must find another.\n\r",ch);
      }
      return(TRUE);
    }


    if (!*arg)
    {
      sprintf(buf,"You have got %d practice sessions left.\n\r",
              ch->specials.spells_to_learn);
      send_to_char(buf, ch);
      send_to_char("You can practise any of these skills:\n\r", ch);
      for(i=0; *spells[i] != '\n'; i++)
        if (spell_info[i+1].min_level_ranger &&
           (spell_info[i+1].min_level_ranger <=
            GET_LEVEL(ch,RANGER_LEVEL_IND)) &&
            (spell_info[i+1].min_level_ranger <=
             GetMaxLevel(guildmaster)-10))
        {
          sprintf(buf,"[%d] %s %s \n\r",
                  spell_info[i+1].min_level_ranger,spells[i],
                  how_good(ch->skills[i+1].learned));
          send_to_char(buf, ch);
        }
      return(TRUE);
    }
    for (;isspace(*arg);arg++);
    number = old_search_block(arg,0,strlen(arg),spells,FALSE);
    if(number == -1)
    {
      send_to_char("You do not know of that skill...\n\r", ch);
      return(TRUE);
    }
    if (GET_LEVEL(ch,RANGER_LEVEL_IND) < spell_info[number].min_level_ranger)
    {
      send_to_char("You do not know of this skill...\n\r", ch);
      return(TRUE);
    }
    if (GetMaxLevel(guildmaster)-10 < spell_info[number].min_level_ranger)
    {
      do_say(guildmaster, "I don't know of this skill.", 0);
      return(TRUE);
    }
    if (ch->specials.spells_to_learn <= 0)
    {
      send_to_char("You do not seem to be able to practice now.\n\r", ch);
      return(TRUE);
    }

    if( number == SKILL_HUNT || number == SKILL_FIND_TRAP ||
        number == SKILL_DODGE )
    {
      if (ch->skills[number].learned >= 95)
      {
        send_to_char("You are a master at this art.\n\r", ch);
        return(TRUE);
      }
    }

    if( number != SKILL_HUNT && number != SKILL_FIND_TRAP &&
        number != SKILL_DODGE && ch->skills[number].learned >= 45)
    {
      send_to_char("You must use this skill to get any better.  I cannot train you further.\n\r", ch);
      return(TRUE);
    }

    send_to_char("You Practice for a while...\n\r", ch);
    ch->specials.spells_to_learn--;


    if (!IS_SET(ch->skills[number].flags, SKILL_KNOWN))
    {
      SET_BIT(ch->skills[number].flags, SKILL_KNOWN);
      SET_BIT(ch->skills[number].flags, SKILL_KNOWN_RANGER);
    }

    percent = ch->skills[number].learned+int_app[ (int)GET_RINT(ch) ].learn;
    ch->skills[number].learned = MIN(95, percent);

    if (ch->skills[number].learned >= 95)
      send_to_char("You are now learned in this area.\n\r", ch);
    return(TRUE);
  }
  else
    send_to_char("What do you think you are, a ranger!??\n\r", ch);

  return(FALSE);
}



int PsiGuildmaster(struct char_data *ch, int cmd, const char *arg, struct char_data *mob, int type)
{

  int number, i, percent;
  char buf[MAX_INPUT_LENGTH];
  struct char_data *guildmaster;
  extern const char *spells[];
  extern struct spell_info_type spell_info[MAX_SPL_LIST];

  if (!ch->skills) return(FALSE);

  if (check_soundproof(ch)) return(FALSE);

  if (IS_IMMORTAL(ch))
    return(FALSE);

  guildmaster = FindMobInRoomWithFunction(ch->in_room, PsiGuildmaster);

  if (!guildmaster) return(FALSE);

  if ((cmd != 164) && (cmd != 170) && (cmd != 243) && (cmd != 72))
      return(FALSE);

  if (IS_NPC(ch)) {
    act("$N tells you 'What do i look like, an animal trainer?'", FALSE,
        ch, 0, guildmaster, TO_CHAR);
    return(FALSE);
  }

  for (;*arg == ' '; arg++);

  if (HasClass(ch, CLASS_PSI)) {
        if (cmd == 243) /*gain */     {
       if (GET_LEVEL(ch,PSI_LEVEL_IND) < GetMaxLevel(guildmaster)-10)
       {
     if (GET_EXP(ch)<
         titles[PSI_LEVEL_IND][GET_LEVEL(ch, PSI_LEVEL_IND)+1].exp)
         {
          send_to_char("You are not yet ready to gain.\n\r", ch);
          return(FALSE);
         } else
         {
          GainLevel(ch,PSI_LEVEL_IND);
          return(TRUE);
         }
        } else {
          send_to_char("I cannot train you.. You must find another.\n\r",ch);
        }
        return(TRUE);
     }


    if (!*arg) {
      sprintf(buf,"You have got %d practice sessions left.\n\r",
              ch->specials.spells_to_learn);
      send_to_char(buf, ch);
      send_to_char("You can practise any of these skills:\n\r", ch);
      for(i=0; *spells[i] != '\n'; i++)
        if (spell_info[i+1].min_level_psi &&
           (spell_info[i+1].min_level_psi <=
            GET_LEVEL(ch,PSI_LEVEL_IND)) &&
            (spell_info[i+1].min_level_psi <=
             GetMaxLevel(guildmaster)-10)) {
          sprintf(buf,"[%d] %s %s \n\r",
                  spell_info[i+1].min_level_psi,spells[i],
                  how_good(ch->skills[i+1].learned));
          send_to_char(buf, ch);
        }
      return(TRUE);
    }
    for (;isspace(*arg);arg++);
    number = old_search_block(arg,0,strlen(arg),spells,FALSE);
    if(number == -1) {
      send_to_char("You do not know of that skill...\n\r", ch);
      return(TRUE);
    }
    if (GET_LEVEL(ch,PSI_LEVEL_IND) < spell_info[number].min_level_psi) {
      send_to_char("You do not know of this skill...\n\r", ch);
      return(TRUE);
    }
    if (GetMaxLevel(guildmaster)-10 < spell_info[number].min_level_psi) {
      do_say(guildmaster, "I don't know of this skill.", 0);
      return(TRUE);
    }
    if (ch->specials.spells_to_learn <= 0) {
      send_to_char("You do not seem to be able to practice now.\n\r", ch);
      return(TRUE);
    }
    if (ch->skills[number].learned >= 45) {
      send_to_char("You must use this spell to get any better.  I cannot train you further.\n\r", ch);
      return(TRUE);
    }
    send_to_char("You Practice for a while...\n\r", ch);
    ch->specials.spells_to_learn--;


    if (!IS_SET(ch->skills[number].flags, SKILL_KNOWN))
    {
      SET_BIT(ch->skills[number].flags, SKILL_KNOWN);
      SET_BIT(ch->skills[number].flags, SKILL_KNOWN_PSI);
    }

    percent = ch->skills[number].learned+int_app[ (int)GET_RINT(ch) ].learn;
    ch->skills[number].learned = MIN(95, percent);

    if (ch->skills[number].learned >= 95)
      send_to_char("You are now learned in this area.\n\r", ch);
    return(TRUE);
  }
  else
    send_to_char("What do you think you are, a psionist!??\n\r", ch);

  return FALSE;
}

int PaladinGuildmaster( struct char_data *ch, int cmd, const char *arg,
                        struct char_data *mob, int type)
{
  int number, i, percent;
  char buf[MAX_INPUT_LENGTH];
  struct char_data *guildmaster;
  extern const char *spells[];
  extern struct spell_info_type spell_info[MAX_SPL_LIST];

  if (!ch->skills)
    return(FALSE);

  if (check_soundproof(ch))
    return(FALSE);

  if (IS_IMMORTAL(ch))
    return(FALSE);

  guildmaster = FindMobInRoomWithFunction( ch->in_room, PaladinGuildmaster );

  if( !guildmaster )
    return(FALSE);

  if( cmd != 164 && cmd != 170 && cmd != 243 && cmd != 72 )
    return(FALSE);

  if (IS_NPC(ch))
  {
    act( "$N ti dice 'Ti sembro un'addestratore di animali?'", FALSE,
         ch, 0, guildmaster, TO_CHAR );
    return(FALSE);
  }

  for( ;*arg == ' '; arg++ );

  if( HasClass( ch, CLASS_PALADIN ) )
  {
    if( cmd == 243 ) /*gain */
    {
      if( GET_LEVEL( ch, PALADIN_LEVEL_IND ) < GetMaxLevel( guildmaster ) - 10 )
      {
        if( GET_EXP( ch ) < titles[ PALADIN_LEVEL_IND ]
            [ GET_LEVEL( ch, PALADIN_LEVEL_IND ) + 1 ].exp )
        {
          send_to_char( "Non sei ancora pronto.\n\r", ch);
          return(FALSE);
        }
        else
        {
          GainLevel(ch,PALADIN_LEVEL_IND);
          return(TRUE);
        }
      }
      else
      {
        send_to_char( "Non posso allenarti, prova da qualcun'altro.\n\r",ch);
      }
      return(TRUE);
    }


    if (!*arg)
    {
      sprintf( buf,"Hai a disposizione %d sezioni di pratica.\n\r",
               ch->specials.spells_to_learn);
      send_to_char(buf, ch);
      send_to_char("Puoi imparare questi skills:\n\r", ch);
      for( i = 0; *spells[ i ] != '\n'; i++ )
      {
        if( spell_info[ i + 1 ].min_level_paladin &&
            (spell_info[i+1].min_level_paladin <=
            GET_LEVEL(ch,PALADIN_LEVEL_IND)) &&
            (spell_info[i+1].min_level_paladin <=
            GetMaxLevel(guildmaster)-10))
        {
          sprintf(buf,"[%d] %s %s \n\r",
                  spell_info[i+1].min_level_paladin,spells[i],
                  how_good(ch->skills[i+1].learned));
          send_to_char(buf, ch);
        }
      }
      return(TRUE);
    }
    for (;isspace(*arg);arg++);
    number = old_search_block(arg,0,strlen(arg),spells,FALSE);
    if(number == -1)
    {
      send_to_char("You do not know of that skill...\n\r", ch);
      return(TRUE);
    }
    if( GET_LEVEL(ch,PALADIN_LEVEL_IND) < spell_info[number].min_level_paladin)
    {
      send_to_char("You do not know of this skill...\n\r", ch);
      return(TRUE);
    }
    if (GetMaxLevel(guildmaster)-10 < spell_info[number].min_level_paladin)
    {
      do_say(guildmaster, "I don't know of this skill.", 0);
      return(TRUE);
    }
    if (ch->specials.spells_to_learn <= 0)
    {
      send_to_char("You do not seem to be able to practice now.\n\r", ch);
      return(TRUE);
    }
    if( ch->skills[number].learned >= 45 )
    {
      send_to_char( "You must use this skill to get any better. "
                    "I cannot train you further.\n\r", ch);
      return(TRUE);
    }
    send_to_char("You Practice for a while...\n\r", ch);
    ch->specials.spells_to_learn--;


    if (!IS_SET(ch->skills[number].flags, SKILL_KNOWN))
    {
      SET_BIT(ch->skills[number].flags, SKILL_KNOWN);
      SET_BIT(ch->skills[number].flags, SKILL_KNOWN_PALADIN);
    }
    percent = ch->skills[number].learned+int_app[ (int)GET_RINT(ch) ].learn;
    ch->skills[number].learned = MIN(95, percent);

    if (ch->skills[number].learned >= 95)
      send_to_char("You are now learned in this area.\n\r", ch);
    return(TRUE);
  }
  else
    send_to_char("Realmente credi di essere un paladino ?\n\r", ch);

  return FALSE;
}



#define SPELL_SPECIAL_COST 100000   /* 100k to specialize per spell */
int mage_specialist_guildmaster(struct char_data *ch, int cmd, const char *arg, struct char_data *mob, int type)
{
  char buf[256];
  int i, number = 0;
  struct char_data *guildmaster;
  extern const char *spells[];
  extern struct spell_info_type spell_info[MAX_SPL_LIST];


  if (!AWAKE(ch))
    return(FALSE);

  if (!cmd) {
    if (ch->specials.fighting) {
      return(magic_user(ch, cmd, arg, ch, 0));
    }
    return(FALSE);
  }

  if (!ch->skills) return(FALSE);

  if (check_soundproof(ch)) return(FALSE);

  guildmaster =
    FindMobInRoomWithFunction(ch->in_room, mage_specialist_guildmaster);

  if (!guildmaster) return(FALSE);

  for(; *arg==' '; arg++); /* ditch spaces */

  if ((cmd==164)||(cmd==170)||cmd==243)
{
  if (!HasClass(ch, CLASS_MAGIC_USER)) {
    do_say(guildmaster,"I do not teach heathens!",0);
    return(TRUE);
    }

  if (IS_NPC(ch)) {
        do_say(guildmaster,"What do I look like? An animal trainer?",0);
   return(FALSE);
  }

     if (!*arg) {
      sprintf(buf,"You have got %d practice sessions left.\n\r",
              ch->specials.spells_to_learn);
      send_to_char(buf, ch);
      send_to_char("You can practise any of these spells:\n\r", ch);
      for(i=0; *spells[i] != '\n'; i++)
        if (spell_info[i+1].spell_pointer &&
            (spell_info[i+1].min_level_magic<=
             GET_LEVEL(ch,MAGE_LEVEL_IND)) &&
            (spell_info[i+1].min_level_magic <=
             GetMaxLevel(guildmaster)-10)) {

            sprintf(buf,"[%d] %s %s \n\r",
                    spell_info[i+1].min_level_magic,
                    spells[i],how_good(ch->skills[i+1].learned));
          send_to_char(buf, ch);
        }

      return(TRUE);

    }
    for (;isspace(*arg);arg++);
    number = old_search_block(arg,0,strlen(arg),spells,FALSE);
    if(number == -1) {
    do_say(guildmaster,"You do not know of this spell.",0);
      return(TRUE);
    }
    if (GET_LEVEL(ch,MAGE_LEVEL_IND) < spell_info[number].min_level_magic) {
        do_say(guildmaster,"You do not know of this spell.",0);
      return(TRUE);
    }
    if (GetMaxLevel(guildmaster)-10 < spell_info[number].min_level_magic) {
      do_say(guildmaster, "I don't know of this spell.", 0);
      return(TRUE);
    }

    if (GET_GOLD(ch) < SPELL_SPECIAL_COST){
      do_say(guildmaster,
        "Ah, but you do not have enough money to pay.",0);
      return(TRUE);
    }

    if (ch->specials.spells_to_learn < 4) {
        do_say(guildmaster,
        "You must first use the knowledge you already have.",0);
      return(TRUE);
    }
        if (IsSpecialized(ch->skills[number].special)) {
        do_say(guildmaster,
          "You are already proficient in this spell!",0);
          return(TRUE);
        }

   if (HowManySpecials(ch) > MAX_SPECIALS(ch)) {
      do_say(guildmaster,
       "You are already specialized in several skills.",0);
      return(TRUE);
     }

  if (ch->skills[number].learned < 95) {
   do_say(guildmaster,
    "You must fully learn this spell first.",0);
      return(TRUE);
    }


    do_say(guildmaster,"Here is how you do that...",0);
    ch->specials.spells_to_learn-=4;
    GET_GOLD(ch) -=SPELL_SPECIAL_COST;
    SET_BIT(ch->skills[number].special, SKILL_SPECIALIZED);
    return(TRUE);
  }
   return(magic_user(ch, cmd, arg, ch, 0));
}


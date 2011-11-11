

/*
***        BenemMUD
***        PSI Skills
*/


#include <stdio.h>
#include <assert.h>

#include "protos.h"
#include "fight.h"

extern struct char_data *character_list;
extern long SystemFlags;

void mind_burn(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
  int dam;
  struct char_data *tmp_victim, *temp;

 if (!ch)
    return;

  dam = dice(1,4) + level/2 + 1;

  send_to_char("Gouts of flame shoot forth from your mind!\n\r", ch);
  act("$n sends a gout of flame shooting from $s mind!\n\r",
          FALSE, ch, 0, 0, TO_ROOM);

  for ( tmp_victim = real_roomp(ch->in_room)->people; tmp_victim; 
       tmp_victim = temp ) {
    temp = tmp_victim->next_in_room;
    if ( (ch->in_room == tmp_victim->in_room) && (ch != tmp_victim)) {
      if ((GetMaxLevel(tmp_victim)>LOW_IMMORTAL) && (!IS_NPC(tmp_victim)))
        return;
      if (!in_group(ch, tmp_victim)) {
        act("You are seared by the burning flame!\n\r",
            FALSE, ch, 0, tmp_victim, TO_VICT);
        heat_blind(tmp_victim);
        if ( saves_spell(tmp_victim, SAVING_SPELL) )
          dam = 0;
        MissileDamage(ch, tmp_victim, dam, SKILL_MIND_BURN);
      } else {
        act("You are able to avoid the flames!\n\r",
            FALSE, ch, 0, tmp_victim, TO_VICT);
        heat_blind(tmp_victim);
      }
    }
  }
}

void mind_teleport(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
  int to_room, iTry = 0;
  extern int top_of_world;      /* ref to the top element of world */
  struct room_data *room;
  
  if (!ch || !victim)
    return;
  
  if (victim != ch)
  {
    if (saves_spell(victim,SAVING_SPELL))
    {
      send_to_char("You can't seem to force them to blink out.\n\r",ch);
      if (IS_NPC(victim))
      {
        if (!victim->specials.fighting)
          set_fighting(victim, ch);
      }
      else
      {
        send_to_char("You feel strange, but the effect fades.\n\r",victim);
      }
      return;
    }
    else
    {
      ch = victim;  /* the character (target) is now the victim */
    }
  }

  if (!IsOnPmp(victim->in_room)) {
    send_to_char("You're on an extra-dimensional plane!\n\r", ch);
    return;
  }

  
  do 
  {
    to_room = number(0, top_of_world);
    room = real_roomp(to_room);
    if (room)
    {
      if ((IS_SET(room->room_flags, PRIVATE)) ||
          (IS_SET(room->room_flags, TUNNEL)) ||
          (IS_SET(room->room_flags, NO_SUM)) ||
          (IS_SET(room->room_flags, NO_MAGIC)) ||
          !IsOnPmp(to_room))
      {
        room = 0;
        iTry++;
      }
    }
    
  } while (!room && iTry < 10);
  
  if (iTry >= 10)
  {
    send_to_char("The skill fails.\n\r", ch);
    return;
  }

  act("$n seems to scatter into tiny particles and is gone!", FALSE, ch,0,0,TO_ROOM);
  char_from_room(ch);
  char_to_room(ch, to_room);
  act("A mass of flying particles form into $n!", FALSE, ch,0,0,TO_ROOM);
  
  do_look(ch, "", 15);
  
  if (IS_SET(real_roomp(to_room)->room_flags, DEATH) && 
      GetMaxLevel(ch) < LOW_IMMORTAL) {
    NailThisSucker(ch);
    return;
  }

  check_falling(ch);

}

        /* astral travel */
#define PROBABILITY_TRAVEL_ENTRANCE   2701
void mind_probability_travel(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
  struct char_data *tmp, *tmp2;
  struct room_data *rp;

if (IS_SET(SystemFlags,SYS_NOASTRAL)) {
        send_to_char("The astral planes are shifting, you cannot!\n",ch);
  return;
 }

  rp = real_roomp(ch->in_room);
  
  for (tmp = rp->people;tmp;tmp=tmp2) {
    tmp2 = tmp->next_in_room;
    if (in_group(ch, tmp) && 
       (!tmp->specials.fighting) &&
       (IS_PC(tmp)||IS_SET(tmp->specials.act,ACT_POLYSELF)) &&
        IS_AFFECTED(tmp,AFF_GROUP)) {
      act("$n wavers as $N sends $m to another plane.", FALSE, tmp, 0, ch, TO_ROOM);
      char_from_room(tmp);
      char_to_room(tmp, PROBABILITY_TRAVEL_ENTRANCE);      
      do_look(tmp, "\0", 15);
      act("$n wavers into existance", FALSE, tmp, 0, 0, TO_ROOM);      
    }
  }  

}

        /* sense DT's */
void mind_danger_sense( byte level, struct char_data *ch,
                        struct char_data *victim, struct obj_data *obj )
{
  struct affected_type af;

  if( !affected_by_spell( victim, SKILL_DANGER_SENSE ) )
  {
    if (ch != victim) 
    {
      act( "$n apre la mente di $N ai pericoli nascosti.", TRUE, ch, 0, 
           victim, TO_NOTVICT );
      act( "$n ti apre la mente ai pericoli nascosti.", TRUE, ch, 0, victim, 
           TO_VICT );
      act( "Apri la mente di $N ai pericoli nascosti.", FALSE, ch, 0, victim, 
           TO_CHAR );
    } 
    else 
    {
      act( "$n sembra essere piu` attento al pericolo.", TRUE, victim, 0, 0,
           TO_ROOM );
      act( "Apri la tua mente ai pericoli nascosti.", TRUE, victim, 0, 0,
           TO_CHAR );
    } 

    af.type      = SKILL_DANGER_SENSE;
    af.duration  = (int)level/10;
    af.modifier  = 0;
    af.location  = APPLY_NONE;
    af.bitvector = 0;
    affect_to_char( victim, &af );
  } 
  else 
  {
    if( ch != victim )
      act( "$N puo` gia` percepire i pericoli nascosti.", FALSE, ch, 0,
           victim, TO_CHAR );
    else
      act( "Puoi gia` percepire i pericoli nascosti.", FALSE, ch, 0, victim,
           TO_CHAR );
  }
}

        /* same as thief spy skil, see into the next room */
void mind_clairvoyance(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
  struct affected_type af;

  if (!affected_by_spell(victim, SKILL_CLAIRVOYANCE) ) {
  if (ch != victim) {
        act("",FALSE,ch,0,victim,TO_CHAR);
        act("",FALSE,ch,0,victim,TO_ROOM);
  } else {
    act("$n meditates for a moment.",TRUE,victim,0,0,TO_ROOM);
    act("You open your mind's eye to nearby visions and sights.",TRUE,victim,0,0,TO_CHAR);
    }

    af.type      = SKILL_CLAIRVOYANCE;
    af.duration  = (level<LOW_IMMORTAL) ? 3 : level;
    af.modifier  = 0;
    af.location  = APPLY_NONE;
    af.bitvector = AFF_SCRYING;
    affect_to_char(victim, &af);
  } else {
          if (ch != victim)
             act("$N can already sense nearby sights.",FALSE,ch,0,victim,TO_CHAR);
              else
             act("You are already clairvoyant.",FALSE,ch,0,victim,TO_CHAR);
    }

}

/* single person attack skill */
void mind_disintegrate( byte level, struct char_data *ch,
                        struct char_data *victim, struct obj_data *obj )
{
  spell_disintegrate(level,ch,victim,obj);
}

/***************************************************************************
 * 
 * if not fighting, shove the mob/pc out a the room if suffcient 
 * level and they do not save, otherwise set fighting. If fighting 
 * then if they fail, treat as bashed and the mobs/pc sits.
 * 
 ***************************************************************************/
        
void mind_telekinesis( byte level, struct char_data *ch,
                       struct char_data *victim, int dir_num )
{

  if (!ch) 
  {
    mudlog( LOG_SYSERR, "!ch in telekenisis");
    return;
  }

  if (!victim)  
  {
    mudlog( LOG_SYSERR, "!victim in telekenisis" );
    return;
  }
 
  /* not fighting, shove him */
  if( !ch->specials.fighting )
  {
    if( saves_spell(victim,SAVING_SPELL) ||
        ( IS_SET(victim->specials.act,ACT_SENTINEL) &&
          IS_SET(victim->specials.act,ACT_HUGE) ) ) 
    {
      /* saved, make fight */            
      act("La tua mente si indebolisce e sei costretto a lasciare $N!", FALSE,
           ch, 0, victim, TO_CHAR );
      act( "$n prova a spostarti, ma la tua mente resiste!", TRUE, ch, 0,
           victim, TO_VICT );
      act( "$n prova a spostare $N senza successo!", TRUE, ch, 0, victim,
           TO_ROOM );
      hit( victim, ch, TYPE_UNDEFINED );
    }
    else
    {
      /* missed save, lets shove'em */
      act( "Alzi $N con la forza della tua mente e l$B sposti lontano!", FALSE,
           ch, 0, victim, TO_CHAR );
      act( "$n ti alza con la forza della sua mente e ti sposta lontano!", 
           TRUE, ch, 0, victim, TO_VICT );
      act( "$n alza $N con la forza della sua mente e l$B sposta lontano!",
           TRUE, ch, 0, victim, TO_ROOM);
      do_move( victim, "\0", dir_num );
    }
  } /* end was not fighting */
  else
  {    
    /* was fighting, bash him */
    if( saves_spell(victim,SAVING_SPELL) ||
        IS_SET(victim->specials.act,ACT_HUGE)) 
    {
      act( "Non riesci a focalizzare la tua mente a sufficienza.", FALSE, ch,
           0, victim, TO_CHAR );
      act( "$n fallisce nello spostarti con la forza della sua mente!", TRUE,
           ch, 0, victim, TO_VICT );
      act( "$n cerca inutilmente di spostare $N con la forza della sua mente",
           TRUE, ch, 0, victim, TO_ROOM );
      /* do nothing */
    }
    else
    {
      /* smack'em to the ground */
      act( "Sbatti $N a terra con il solo pensiero!", FALSE, ch, 0, victim,
           TO_CHAR );
      act( "$n ti alza e ti sbatte a terra con il solo pensiero!", TRUE, ch,
           0, victim, TO_VICT );
      act( "$n sbatte $N a terra con il solo pensiero!", TRUE, ch, 0, victim,
           TO_ROOM);

      GET_POS(victim) = POSITION_SITTING;
      if( !victim->specials.fighting )
        set_fighting( victim,ch );
      WAIT_STATE( victim, PULSE_VIOLENCE * 1 );
    }
  } /* end was fighting */
}

        /* same as fly */
void mind_levitation(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
  struct affected_type af;

  if (!affected_by_spell(victim, SKILL_LEVITATION) ) {
  if (ch != victim) {
        act("You lift $N with a simple thought.",FALSE,ch,0,victim,TO_CHAR);
        act("$N is lifted into the air by a simple thought from $n.",FALSE,ch,0,victim,TO_ROOM);
  } else {
    act("$n lifts $s own body with a thought.",TRUE,victim,0,0,TO_ROOM);
    act("You lift yourself with your mind",TRUE,victim,0,0,TO_CHAR);
    }

    af.type      = SKILL_LEVITATION;
    af.duration  = (int)(level*2)/10;
    af.modifier  = 0;
    af.location  = APPLY_NONE;
    af.bitvector = AFF_FLYING;
    affect_to_char(victim, &af);
  } else {
          if (ch != victim)
             act("$N is already levitating.",FALSE,ch,0,victim,TO_CHAR);
              else
             act("You are already levitating.",FALSE,ch,0,victim,TO_CHAR);
    }
}

        /* healing, 100 points max, cost 100 mana, and stuns the */
        /* psi and lags along time, simular results as mage spell id */
void mind_cell_adjustment(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{

  if (!ch) 
  {
    mudlog( LOG_SYSERR, "!ch in cell_adjustment");
    return;
  }

  if (ch != victim) 
  {
    send_to_char("You cannot use this skill on others.\n\r",ch);
    return;        
  }

  act( "You begin the process of altering your bodies cells.", FALSE, ch, 0,
       victim,TO_CHAR);
  act("$n goes into a deep trance.",FALSE,ch,0,victim,TO_ROOM);

  if (GET_HIT(victim) + 100 > GET_MAX_HIT(victim)) 
  {
    act("You completely heal your body.",FALSE,victim,0,0,TO_CHAR);
    GET_HIT(victim) = GET_MAX_HIT(victim);        
  } else {
         act("You manage to heal some of your body through cell adjustment.",FALSE,victim,0,0,TO_CHAR); 
         GET_HIT(victim) +=100;        
        }
        
   if (GetMaxLevel(ch)<LOW_IMMORTAL) {
        act("You are overcome by exhaustion.",FALSE,ch,0,0,TO_CHAR);
        act("$n slumps to the ground exhausted.",FALSE,ch,0,0,TO_ROOM);        
        WAIT_STATE(ch,PULSE_VIOLENCE*12);
        GET_POS(ch) = POSITION_STUNNED;
      }

}

        /* hide */
void mind_chameleon( byte level, struct char_data *ch,
                     struct char_data *victim, struct obj_data *obj )
{

  if (!ch)
    return;

  if (IS_AFFECTED(ch,AFF_HIDE))
  {
    REMOVE_BIT(ch->specials.affected_by,AFF_HIDE);        
  }

  act( "Nascondi te stess$b alle altre menti.", FALSE, ch, 0, 0, TO_CHAR );
  act( "Il corpo di $n svanisce nei dintorni.", FALSE, ch, 0, 0, TO_ROOM);
  SET_BIT(ch->specials.affected_by,AFF_HIDE);

}

/* strength */
void mind_psi_strength( byte level, struct char_data *ch,
                        struct char_data *victim, struct obj_data *obj )
{
  struct affected_type af;

  if (!victim || !ch)
    return;


  if (!affected_by_spell(victim,SKILL_PSI_STRENGTH))
  {
    act("Ti senti piu` forte.", FALSE, victim,0,0,TO_CHAR);
    act("$n sembra piu` forte.", TRUE, victim, 0, 0, TO_ROOM);
    af.type      = SKILL_PSI_STRENGTH;
    af.duration  = 2*level;
    if (IS_NPC(victim))
    {      
      if (level >= CREATOR)
      {
        af.modifier = 25 - GET_STR(victim);
      }
      else
      {        
        af.modifier = number(1,6);
      }
    }    
    else
    {
      if( HasClass( ch, CLASS_WARRIOR | CLASS_BARBARIAN ) )
        af.modifier = number(1,8);
      else if( HasClass( ch, CLASS_CLERIC | CLASS_THIEF | CLASS_PSI ) )
        af.modifier = number(1,6);
      else 
        af.modifier = number(1,4);
    }
    af.location  = APPLY_STR;
    af.bitvector = 0;
    affect_to_char(victim, &af);
  }
  else
  {
    act( "Non sembra succedere nulla.", FALSE, ch, 0, 0, TO_CHAR );
  }
}

/* long lag time, but after that they get 12 hrs of no hunger/thirst */
void mind_mind_over_body(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
  struct affected_type af;

  if (!affected_by_spell(victim, SKILL_MIND_OVER_BODY) )
  {
    if (ch != victim)
    {
      act("",FALSE,ch,0,victim,TO_CHAR);
      act("",FALSE,ch,0,victim,TO_ROOM);
    }
    else
    {
      act( "$n appare assort$b in meditazione.", TRUE, victim, 0, 0, TO_ROOM );
      act( "Costringi il tuo corpo a non aver bisogno di cibo ed acqua!",
           TRUE, victim, 0,0 , TO_CHAR );
    }

    af.type      = SKILL_MIND_OVER_BODY;
    af.duration  = 12;
    af.modifier  = -1;
    af.location  = APPLY_MOD_THIRST;
    af.bitvector = 0;
    affect_to_char(victim, &af);

    af.type      = SKILL_MIND_OVER_BODY;
    af.duration  = 12;
    af.modifier  = -1;
    af.location  = APPLY_MOD_HUNGER;
    af.bitvector = 0;
    affect_to_char(victim, &af);

    
  }
  else
  {
    if (ch != victim)
      act( "$N non ha bisogno del tuo aiuto.", FALSE, ch, 0, victim, TO_CHAR );
    else
      act( "Il tuo corpo non ha bisogno di mangiare o bere!", FALSE, ch, 0,
           victim, TO_CHAR );
  }
}

/* feeblemind*/
void mind_mind_wipe( byte level, struct char_data *ch,
                     struct char_data *victim, struct obj_data *obj )
{
  spell_feeblemind( level, ch, victim, obj );
}


/* psi protective skill, immune to some psi skills */
void mind_tower_iron_will(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
  struct affected_type af;

  if (!affected_by_spell(victim, SKILL_TOWER_IRON_WILL) )
  {
    if (ch != victim)
    {
      act("",FALSE,ch,0,victim,TO_CHAR);
      act("",FALSE,ch,0,victim,TO_ROOM);
    }
    else
    {
      act( "$n appare assort$b in meditazione.", TRUE, victim, 0, 0, TO_ROOM );
      act( "Alzi una torre di metallo psichico intorno a te!", TRUE, victim,
           0, 0, TO_CHAR );
    }

    af.type      = SKILL_TOWER_IRON_WILL;
    af.duration  = (int)level/10;
    af.modifier  = 0;
    af.location  = APPLY_NONE;
    af.bitvector = 0;
    affect_to_char(victim, &af);
  }
  else
  {
    if (ch != victim)
      act( "$N e` gia` protett$B.",FALSE,ch,0,victim,TO_CHAR);
    else
      act( "Sei gia` protett$b.",FALSE,ch,0,victim,TO_CHAR);
  }
}

/* psi protective skill, immune to feeblemind, etc... */
void mind_mindblank( byte level, struct char_data *ch,
                     struct char_data *victim, struct obj_data *obj )
{
  struct affected_type af;

  if (!affected_by_spell(victim, SKILL_MINDBLANK) )
  {
    if (ch != victim)
    {
      act("",FALSE,ch,0,victim,TO_CHAR);
      act("",FALSE,ch,0,victim,TO_ROOM);
    }
    else
    {
      act( "$n appare assort$b in meditazione.", TRUE, victim, 0, 0, TO_ROOM );
      act( "Mischi i tuoi pensieri svuotando apparentemente la tua mente.",
           TRUE, victim, 0, 0, TO_CHAR );
    }

    af.type      = SKILL_MINDBLANK;
    af.duration  = (int)(level*2)/10;
    af.modifier  = 0;
    af.location  = APPLY_NONE;
    af.bitvector = 0;
    affect_to_char(victim, &af);
  }
  else
  {
    if (ch != victim)
      act( "$N e` gia` protett$B.", FALSE, ch, NULL, victim, TO_CHAR );
    else
      act( "Sei gia` protett$b.", FALSE, ch, NULL, victim, TO_CHAR );
  }

}

        /* same as thief disguise */
void mind_psychic_impersonation( byte level, struct char_data *ch,
                                 struct char_data *victim, 
                                 struct obj_data *obj )
{
  struct affected_type af;
  struct char_data *k;  
    
  if( affected_by_spell( victim, SKILL_PSYCHIC_IMPERSONATION ) ) 
  {
    send_to_char( "Gia` stai provando a impersonare qualcun altro.\n\r", 
                  victim );
    return;
  }

  act( "Modifichi i tuoi lineamenti con la sola concentrazione.", FALSE, ch, 
       NULL, victim, TO_VICT );
  act( "Il lineamenti di $N cambiano sotto i tuoi occhi!", TRUE, ch, NULL,
       victim, TO_NOTVICT );
   
  for( k = character_list; k; k = k->next )
  {
    if( k->specials.hunting == victim )
    {
      k->specials.hunting = NULL;
    }
    if( Hates( k, victim ) ) 
    {
      RemHated( k, victim );
    }
    if( Fears( k, victim ) )
    { 
      RemFeared( k, victim );
    }
  } /* end for */

  af.type = SKILL_PSYCHIC_IMPERSONATION;
  af.duration = (int)( level * 2 ) / 10;
  af.modifier = 0;
  af.location = APPLY_NONE;
  af.bitvector = 0;
  affect_to_char( victim, &af );

}




        /* area effect psionic blast type skill */
void mind_ultra_blast( byte level, struct char_data *ch,
                       struct char_data *victim, struct obj_data *obj)
{
  int dam;
  struct char_data *tmp_victim, *temp;

  assert(ch);
  assert((level >= 1) && (level <= ABS_MAX_LVL)); 

  /* damage = level d4, +level */
  dam = dice(level,4);
  dam +=level;

  act( "You blast out a massive wave of destructive psionic energy!", FALSE, 
       ch,0,victim,TO_CHAR);
  act( "$n blasts out a massive wave of destructive psionic energy!", FALSE,
       ch,0,0,TO_ROOM);
  
  for(tmp_victim = character_list; tmp_victim; tmp_victim = temp)    
  {
    temp = tmp_victim->next;
    if ( (ch->in_room == tmp_victim->in_room) && (ch != tmp_victim))
    {
      if (!in_group(ch,tmp_victim) && !IS_IMMORTAL(tmp_victim)) 
      {
        if (!saves_spell(tmp_victim, SAVING_SPELL)) 
        {

          /* half damage if effected by TOWER OF IRON WILL */
          if (affected_by_spell(tmp_victim,SKILL_TOWER_IRON_WILL)) 
            dam >>=1;
            
          MissileDamage(ch,tmp_victim,dam,SKILL_ULTRA_BLAST);
          /* damage here */
        }
        else
        {
          dam >>=1;  /* half dam */

          /* NO damage if effected by TOWER OF IRON WILL */
          if (affected_by_spell(tmp_victim,SKILL_TOWER_IRON_WILL)) 
            dam =0;

          MissileDamage(ch,tmp_victim,dam,SKILL_ULTRA_BLAST);
        }
      }
      else
        act("You manage to get out of the way of the massive psionic blast!",FALSE, ch, 0, tmp_victim, TO_VICT);         
    }           
  }          /* end for */
}

/* massive single person attack */
void mind_psychic_crush( byte level, struct char_data *ch,
                         struct char_data *victim, struct obj_data *obj )
{
  int dam;

  assert(victim && ch);
  assert((level >= 1) && (level <= ABS_MAX_LVL)); 

  /* damage = level d6, +1 for every two levels of the psionist */

  dam = dice(level,6);
  dam +=(int)level/2;

  if ( saves_spell(victim, SAVING_SPELL) ) 
  {
    dam >>= 1;
    if (affected_by_spell(victim,SKILL_TOWER_IRON_WILL))
      dam =0;
  }

  /* half dam if tower up */
  if (affected_by_spell(victim,SKILL_TOWER_IRON_WILL))
    dam >>=1;

  MissileDamage(ch, victim, dam, SKILL_PSYCHIC_CRUSH);
}


/* increate int,wis or con, reduce the unselected attribs the same */
        
void mind_intensify(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
}


        /* same as cleric COMMAND spell */
void mind_domination(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)

{
}


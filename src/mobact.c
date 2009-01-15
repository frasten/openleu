


#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "protos.h"
#include "cmdid.h"
#include "status.h"
#include "fight.h"


extern struct char_data *character_list;
extern struct index_data *mob_index;
#if HASH
extern struct hash_header room_db;
#else
extern struct room_data *room_db;
#endif
extern struct str_app_type str_app[];
extern struct index_data *mob_index;

extern struct spell_info_type spell_info[];

int top_of_comp = 0;

void mobile_guardian(struct char_data *ch)
{
  struct char_data *targ;
  int found=FALSE;
  
  if (ch->in_room > -1)
  {
    if( !ch->master || !IS_AFFECTED( ch, AFF_CHARM ) )
      return;
    if( ch->in_room == ch->master->in_room && ch->master->specials.fighting )
    { 
      targ = FindAnAttacker( ch->master );
      if( targ )
        found=TRUE;
      
      if (!found) return;
      
      if (!SameRace(targ, ch))
      {
        if( IsHumanoid(ch) )
        {
          act( "$c0009[$c0015$n$c0009] urla 'Non toccare il mio maestro!'", 
               FALSE, ch, 0, 0, TO_ROOM );
        }
        else
        {
          act( "$c0009[$c0015$n$c0009] ruggisce arrabbiato!", 
               FALSE, ch, 0, 0, TO_ROOM );
        }
        if( CAN_SEE( ch, targ ) )
          hit( ch, targ, 0 );
      }
    }
  }
}

void mobile_wander(struct char_data *ch)
{
  int        door, orr;
  struct room_direction_data        *exitp;
  struct room_data        *rp;
  extern int rev_dir[];

  if (GET_POS(ch) != POSITION_STANDING)
    return;

  orr = ch->in_room;

  while(1) {

    door = number(0,8);
    if (door > 5) return;

        /* bug fix john */
if (door == ch->specials.last_direction)
     ch->specials.last_direction = -1;
     
/*     if (door == ch->specials.last_direction)
       continue;
*/       

    exitp = EXIT(ch, door);

    if (!exit_ok(exitp, &rp)) continue;

    if (IS_SET(rp->room_flags, NO_MOB|DEATH))
      continue;

    if (GET_RACE(ch) == RACE_FISH) {
      rp = real_roomp(EXIT(ch, door)->to_room);

      if (rp->sector_type == SECT_UNDERWATER ||
          rp->sector_type == SECT_WATER_NOSWIM ||
          rp->sector_type == SECT_WATER_SWIM) {
        /* then it is ok for the fish to wander there */
      } else {
        return;
      }
    }

    if (IsHumanoid(ch) ? CAN_GO_HUMAN(ch, door) : CAN_GO(ch, door)) {
      if (!IS_SET(ch->specials.act, ACT_STAY_ZONE) ||
          (rp->zone == real_roomp(ch->in_room)->zone)) {
        ch->specials.last_direction = rev_dir[door];
        go_direction(ch, door);
        if (ch->in_room == 0) {
          if (orr != 0) 
          {
            mudlog( LOG_ERROR, "%s just entered void from %d", GET_NAME(ch), orr);
          }
        }
        return;
      }
    }
  }
}

void MobHunt(struct char_data *ch)
{
  int res, k;
  
#if NOTRACK     
  return;    /* too much CPU useage for some machines.  */
#endif
  
  if (ch->persist <= 0)
  {
    res = choose_exit_in_zone(ch->in_room, ch->old_room, 2000);
    if (res > -1)
    {
      go_direction(ch, res);
    }
    else
    {
      if (ch->specials.hunting)
      {
        if (ch->specials.hunting->in_room == ch->in_room)
        {
          if (Hates(ch, ch->specials.hunting) && 
              (!IS_AFFECTED(ch->specials.hunting, AFF_HIDE)))
          {
            if (check_peaceful(ch, "You'd love to tear your quarry to bits, but you just CAN'T\n\r"))
            {
              act( "$n fumes at $N", TRUE, ch, 0, ch->specials.hunting, 
                   TO_ROOM); 
            }
            else
            {
              if (IsHumanoid(ch))
              {
                act("$n screams 'Time to die, $N'", 
                    TRUE, ch, 0, ch->specials.hunting, TO_ROOM); 
              }
              else if (IsAnimal(ch))
              {
                act("$n growls.", TRUE, ch, 0, 0, TO_ROOM);
              }
              MobHit(ch,ch->specials.hunting,0);
              return;
            }
          }
        }
      }
      REMOVE_BIT(ch->specials.act, ACT_HUNTING);
      ch->specials.hunting = 0;
      ch->hunt_dist = 0;
    }
  }
  else if (ch->specials.hunting)
  {
    if (ch->hunt_dist <= 50) 
      ch->hunt_dist = 100;

#if FAST_TRACK
    for (k=1;k<=3 && ch->specials.hunting; k++)
    {
      ch->persist -= 1;
      res = dir_track(ch, ch->specials.hunting);
      if (res!= -1)
      {
        go_direction(ch, res);
      }
      else
      {
        ch->persist = 0;
        ch->specials.hunting = 0;
        ch->hunt_dist = 0;
      }
    } /* end for */
#else    
    for (k=1;k<=1 && ch->specials.hunting; k++)
    {
      ch->persist -= 1;
      res = dir_track(ch, ch->specials.hunting);
      if (res!= -1)
      {
        go_direction(ch, res);
      }
      else
      {
        ch->persist = 0;
        ch->specials.hunting = 0;
        ch->hunt_dist = 0;
      }
    } /* end for */
#endif    
  }
  else
  {
    ch->persist = 0;
  }               
}

int IsMobStartRoom( int room, struct char_data *pMob )
{
  return room == pMob->lStartRoom;
}

int SentinelBackHome( struct char_data *pMob )
{
  int iDir;
  
  if( IS_SET( pMob->specials.act, ACT_SCRIPT ) )
  {
    pMob->lStartRoom = 0;
    return FALSE;
  }
  
  mudlog( LOG_CHECK, "%s is searching path to home", GET_NAME_DESC( pMob ) );
      
  iDir = find_path( pMob->in_room, CASTVF_FP IsMobStartRoom, pMob, -5000, 0 );
  if( iDir >= 0 ) 
  { 
    go_direction( pMob, iDir );
    return TRUE;
  }
  else if( pMob->in_room != pMob->lStartRoom )
  {
    mudlog( LOG_CHECK, "%s cannot find path to home", GET_NAME_DESC( pMob ) );
    act( "$n scompare in una nuvola di fumo.", FALSE, pMob, NULL, NULL, 
         TO_ROOM );
    char_from_room( pMob );
    char_to_room( pMob, pMob->lStartRoom );
    act( "Senti un gran botto ed in una nuvola di fumo appare $n", FALSE,
         pMob, NULL, NULL, TO_ROOM );
    return TRUE;
  }
  
  return FALSE;
}


void MobScavenge(struct char_data *ch)
{
  struct obj_data *best_obj=0, *obj=0;
  int max, cc=0;
  char buf[512];
  
  if ((real_roomp(ch->in_room))->contents && number(0,4)) 
  {
    for( max = -1,best_obj = 0,obj = (real_roomp(ch->in_room))->contents;
         obj; obj = obj->next_content) 
    {
      if (IS_CORPSE(obj)) 
      {
        cc++;
        if (obj->contains) 
        {
          if (IsHumanoid(ch) && !number(0,4)) 
          {
            sprintf(buf, " all %d.corpse", cc);
            if( !special( ch, CMD_GET, buf ) )
              do_get(ch, buf, 0);
            return;
          }
        }
      }
      if (CAN_GET_OBJ(ch, obj)) 
      {
        if (obj->obj_flags.cost > max) 
        {
          best_obj = obj;
          max = obj->obj_flags.cost;
        }
      }
    } /* for */
    
    if (best_obj) 
    {
      if( !special( ch, CMD_GET, fname( best_obj->name ) ) )
      {          
        if (!CheckForAnyTrap(ch, best_obj)) 
        {
          obj_from_room(best_obj);
          obj_to_char(best_obj, ch);
          act("$n gets $p.",FALSE,ch,best_obj,0,TO_ROOM);
        }
      }
      return;
    }
  } 
  else 
  {
    if (IsHumanoid(ch) && real_roomp(ch->in_room)->contents) 
    {
      if (!number(0,4))
      {        
        if( !special( ch, CMD_GET, "all" ) )
        {        
          do_get(ch, "all", 0);
        }
        return;
      }      
    }
  }

#if 0
  if (number(0,3)) 
  {
    for( obj=ch->carrying;obj;obj=obj->next) 
    {
      if (IS_OBJ_STAT(obj, ITEM_NODROP)) 
      {
        do_junk(ch, obj->name, 0);
      }
    }
  }
#endif

  if (!number(0,3)) 
  {
    if (IsHumanoid(ch) && ch->carrying) 
    {
      sprintf(buf, "all");
      do_wear(ch, buf, 0);
    }
  }
}


void check_mobile_activity(int pulse)
{
  register struct char_data *ch;
  struct char_data *pNextChar;
  
  for( ch = character_list; ch; ch = pNextChar )
  {
    pNextChar = ch->next;
    if( IS_MOB(ch) )
    {
      if( ch->specials.tick_to_lag )
      {
        /*do_say(ch, "I'm lagging..", 0); */
        ch->specials.tick_to_lag -= PULSE_VIOLENCE;
      }
      else if( ch->specials.fighting )
      /*do_say(ch, "I got tick", 0); */
#if 0
      if( number(1,20) > GET_DEX(ch) ) 
      { 
        /*do_say(ch, "I fail DEX check :(", 0); */
      } 
      else
#endif
      {
        /*do_say(ch, "Ok, lets kick some ass", 0); */
        mobile_activity(ch);
      }
    }
  }
}

void mobile_activity(struct char_data *ch)
{
  struct char_data *tmp_ch;

  int k;
  extern int no_specials;
  
  if( ch == NULL )
  {
    mudlog( LOG_SYSERR, "ch == NULL in mobile_activity (mobact.c)" );
    return;
  }
  
  if( ch->nMagicNumber != CHAR_VALID_MAGIC )
  {
    mudlog( LOG_SYSERR,
            "Invalid char in mobile_activity (%s).", __FILE__ );
    return;
  }
  
  SetStatus( "mobile_activity started", ch->player.short_descr );

  /* Examine call for special procedure */
  
  /* some status checking for errors */
  if( !real_roomp( ch->in_room ) )
  {
    mudlog( LOG_SYSERR, "Char %s not in correct room.  moving to 50", 
            GET_NAME( ch ) );
    assert( ch->in_room >= 0 ); /* if they are in a - room, assume an error */
    char_from_room( ch );
    char_to_room( ch, 50 );
  }

  /* Execute a script if there is one */

  if( !IS_PC(ch) && IS_SET( ch->specials.act, ACT_SCRIPT ) && 
      !ch->specials.fighting )
  {
    SetStatus( STATUS_CALLINGSCRIPT, NULL );
    DoScript(ch);
    SetStatus( STATUS_ENDSCRIPT, NULL );
    return;
  }
    
#if !defined( EMANUELE )
  if (!IS_SET(ch->specials.act, ACT_SPEC) && mob_index[ch->nr].func) 
  {
    SET_BIT(ch->specials.act,ACT_SPEC);
  }
    
  if ( ( IS_SET( ch->specials.act, ACT_SPEC ) || mob_index[ch->nr].func ) && 
       !no_specials) 
  {
    if (!mob_index[ch->nr].func)     
    {
      mudlog( LOG_ERROR, "Attempting to call a non-existing mob func on %s", 
              GET_NAME(ch) );
      REMOVE_BIT(ch->specials.act, ACT_SPEC);
    } 
    else 
    {
      int nRetVal;
      SetStatus( STATUS_CALLINGSPEC, NULL );
      nRetVal = (*mob_index[ch->nr].func)( ch, 0, "", ch, EVENT_TICK );
      SetStatus( STATUS_ENDSPEC, NULL );
      if( nRetVal )
        return;
    }
  }
#endif

  if( !no_specials ) 
  {                /* do not run these if disabled */
#if defined( EMANUELE )
    if( ch->nr >= 0 && mob_index[ ch->nr ].func )
    {
      int nRetVal;
      SetStatus( STATUS_CALLINGSPEC, NULL );
      nRetVal = (*mob_index[ ch->nr ].func)( ch, 0, "", ch, EVENT_TICK );
      SetStatus( STATUS_ENDSPEC, NULL );
      if( nRetVal )
        return;
    }
#endif

    SetStatus( "Calling ACT routines", NULL );
    if( IS_SET( ch->specials.act, ACT_ARCHER ) )
      if( archer( ch, 0, "", ch, EVENT_TICK ) )
        return;

    if( IS_SET( ch->specials.act, ACT_MAGIC_USER ) ) 
      if( magic_user( ch, 0, "", ch, EVENT_TICK ) ) 
        return;

    if( IS_SET( ch->specials.act, ACT_CLERIC ) )
    {
      int nRetVal;      
      nRetVal = cleric( ch, 0, "", ch, EVENT_TICK );
      SetStatus( "Return from cleric spec proc", NULL );
      if( nRetVal )
        return;
    }

    if (IS_SET(ch->specials.act,ACT_WARRIOR)) 
      if (fighter(ch,0,"",ch,EVENT_TICK)) 
         return;

    if (IS_SET(ch->specials.act,ACT_THIEF)) 
      if (thief(ch,0,"",ch,EVENT_TICK)) 
        return;

    if (IS_SET(ch->specials.act,ACT_DRUID)) 
      if( druid( ch, 0, "", ch, EVENT_TICK ) ) 
        return;

    if( IS_SET( ch->specials.act, ACT_MONK ) ) 
      if (monk(ch,0,"",ch,EVENT_TICK)) 
        return;

    if (IS_SET(ch->specials.act,ACT_BARBARIAN)) 
      if (Barbarian(ch,0,"",ch,EVENT_TICK)) 
        return;

    if( IS_SET( ch->specials.act, ACT_PALADIN ) ) 
      if( Paladin( ch, 0, "", ch, EVENT_TICK ) )
        return;

    if (IS_SET(ch->specials.act,ACT_RANGER)) 
      if (Ranger(ch,0,"",ch,EVENT_TICK)) 
        return;

    if (IS_SET(ch->specials.act,ACT_PSI)) 
      if (Psionist(ch,0,"",ch,EVENT_TICK)) 
        return;
  } /* !no_special */    

  /* check to see if the monster is possessed */
  
  if (AWAKE(ch) && (!ch->specials.fighting) && (!ch->desc) &&
      (!IS_SET(ch->specials.act, ACT_POLYSELF))) 
  {
    SetStatus( "Calling AssistFriend", NULL );
    if( AssistFriend( ch ) )
      return;
    
    
    if( IS_SET( ch->specials.act, ACT_SCAVENGER ) )
    {
      SetStatus( "Calling MobScavenge", NULL );
      MobScavenge(ch);
    }
    
    if( ch->nMagicNumber != CHAR_VALID_MAGIC )
    {
      mudlog( LOG_SYSERR,
              "Invalid char after MobScavenge in mobile_activity (mobact.c)" );
      return;
    }    
    
    
    if( IS_SET( ch->specials.act, ACT_HUNTING ) )
    {
      SetStatus( "Calling MobHunt", NULL );
      MobHunt(ch);
    }
    else if( !IS_SET( ch->specials.act, ACT_SENTINEL ) )
    {
      SetStatus( "Calling mobile_wander", NULL );
      mobile_wander( ch );
    }
    else if( IS_SET( ch->specials.act, ACT_SENTINEL ) && ch->lStartRoom > 0 &&
             ch->lStartRoom != ch->in_room )
    {
      SetStatus( "Calling SentinelBackHome", NULL );
      if( SentinelBackHome( ch ) )
        return;
    }
    
      
    if( ch->nMagicNumber != CHAR_VALID_MAGIC )
    {
      mudlog( LOG_SYSERR,
              "Invalid char after MobHunt or mobile_wander in mobile_activity (mobact.c)" );
      return;
    }    
    
    SetStatus( "Gestione Odio e Paure", NULL );
    if( GET_HIT( ch ) > ( GET_MAX_HIT(ch) / 2 ) ) 
    {
      if( IS_SET( ch->specials.act, ACT_HATEFUL ) ) 
      {
        tmp_ch = FindAHatee(ch);
        if (tmp_ch) 
        {
          if (check_peaceful(ch, "Chiedi al tuo nemico di andare fuori a "
                                 "regolare la questione.\n\r"))
          {
            act( "$c0015[$c0013$n$c0015] ruggisce '$N, che ne dici di uscire "
                 "da qui per regolare i nostri conti in sospeso?'", 
                 TRUE, ch, 0, tmp_ch, TO_ROOM);
          } 
          else 
          {
            if( IsHumanoid( ch ) ) 
            {
              act("$c0009[$c0015$n$c0009] urla 'Ti uccidero`!'",
                  TRUE, ch, 0, 0, TO_ROOM); 
            } 
            else if( IsAnimal( ch ) ) 
            {
              act("$n ruggisce", TRUE, ch, 0, 0, TO_ROOM);
            }
            MobHit(ch,tmp_ch,0);
          }
          if( Hates( ch, tmp_ch) && Fears( ch, tmp_ch ) ) 
          {
            RemHated(ch, tmp_ch);
            RemFeared(ch, tmp_ch);
          }
        }
      }
      if (!ch->specials.fighting) 
      {
        if (IS_SET(ch->specials.act, ACT_AFRAID)) 
        {
          if ((tmp_ch = FindAFearee(ch))!= NULL) 
          {
            do_flee(ch, "", 0);
          }
        }
      }
    } 
    else 
    {
      if (IS_SET(ch->specials.act, ACT_AFRAID)) 
      {
        if ((tmp_ch = FindAFearee(ch))!= NULL) 
        {
          do_flee(ch, "", 0);
        } 
        else 
        {
          if (IS_SET(ch->specials.act, ACT_HATEFUL)) 
          {
            tmp_ch = FindAHatee(ch);
            if (tmp_ch) 
            {
              if( check_peaceful( ch, "Chiedi al tuo nemico di andare fuori a "
                                      "regolare la questione.\n\r" ) ) 
              {
                act("$c0015[$c0013$n$c0015] ruggisce '$N, che ne dici di "
                    "uscire da qui per regolare i nostri conti in sospeso?'", 
                    TRUE, ch, 0, tmp_ch, TO_ROOM);
              } 
              else 
              {
                if (IsHumanoid(ch)) 
                {
                  act("$c0009[$c0015$n$c0009] urla 'Ti uccidero`!'", 
                      TRUE, ch, 0, 0, TO_ROOM); 
                } 
                else if (IsAnimal(ch)) 
                {
                  act("$n ruggisce.", TRUE, ch, 0, 0, TO_ROOM);
                }
                if( hit(ch,tmp_ch,0) == SubjectDead )
                  return;
              }
              if (Hates(ch, tmp_ch) && Fears(ch, tmp_ch)) 
              {
                RemHated(ch, tmp_ch);
                RemFeared(ch, tmp_ch);
              }
            }
          }
        }
      }
    }

    if( ch->nMagicNumber != CHAR_VALID_MAGIC )
    {
      mudlog( LOG_SYSERR,
              "Invalid char after Hates/Fear check in mobile_activity (mobact.c)" );
      return;
    }    
    
    SetStatus( "Gestione Agressivi", NULL );
    if( IS_SET( ch->specials.act, ACT_AGGRESSIVE ) )
    {
      if( ( tmp_ch = FindVictim(ch) ) != NULL )
      {
        if( check_peaceful( ch, "You can't seem to exercise your violent "
                                "tendencies.\n\r" ) )
        {
          act("$n ruggisce impotente.", TRUE, ch, 0, 0, TO_ROOM);
        }
        else
        {
          MobHit(ch, tmp_ch, 0);
        }
        
        return;
      }
    }
    if( IS_SET( ch->specials.act, ACT_META_AGG ) )
    {
      for (k=0;k<=5;k++) 
      {
        tmp_ch = FindMetaVictim(ch);
        if (tmp_ch) 
        {
          if (check_peaceful(ch, "You can't seem to exercise your violent tendencies.\n\r")) 
          {
            act("$n ruggisce impotente.", TRUE, ch, 0, 0, TO_ROOM);
            return;
          }
          MobHit(ch, tmp_ch, 0);
          return;
        }
      }
    }
  
    if( ch->nMagicNumber != CHAR_VALID_MAGIC )
    {
      mudlog( LOG_SYSERR,
              "Invalid char after agressive stuff in mobile_activity (mobact.c)" );
      return;
    }    
    
    if (IS_SET(ch->specials.act, ACT_GUARDIAN)) 
    {
      mobile_guardian(ch);
    }    

    if( ch->nMagicNumber != CHAR_VALID_MAGIC )
    {
      mudlog( LOG_SYSERR,
              "Invalid char after mobile_guardian in mobile_activity (mobact.c)" );
      return;
    }    
    
  } /* If AWAKE(ch)   */

  if (!IS_PC(ch) && !IS_SET(ch->specials.act,ACT_POLYSELF) && 
      ch->specials.fighting) 
  {
    /* here we check for wands/staffs that can be used against the bad guy */
    UseViolentHeldItem(ch);
  }
  
  if (!IS_PC(ch) && ch->specials.fighting) 
  {
    if (IS_AFFECTED(ch->specials.fighting, AFF_FIRESHIELD)) 
    {
      struct char_data *vict;
      vict = FindVictim(ch);
              /* if person we are fighting is protected by fireshield */ 
              /* switch to a new target */
      if (vict && vict != ch->specials.fighting) 
      {
        stop_fighting(ch);
        set_fighting(ch, vict);
      }
    }
  }
}        /* end function */
  
  
int UseViolentHeldItem( struct char_data *ch )
{
  char buf[255];
  char tmp[255];

  if( ch->equipment[ HOLD ] && HasHands( ch ) && ch->specials.fighting ) 
  {
    struct obj_data *obj;
    obj = ch->equipment[ HOLD ];
    if( !obj->obj_flags.value[2] <= 0 ) 
    { /* item has charges */
      if( IS_SET( spell_info[ obj->obj_flags.value[ 3 ] ].targets, 
                  TAR_VIOLENT ) )
      {
        int i, tokillnum = 0;
        one_argument( obj->name, tmp );

        if( isname( GET_NAME( ch ), GET_NAME( ch->specials.fighting ) ) ) 
        {
          /* Questa roba qui sotto, secondo me, non funziona. Ma non dovrebbe
             essere un problema perche` sara` difficile che due con il nome
             uguale si combattano fra loro. Comunque l'if piu` sotto aveva un
             = in meno */
          struct char_data *v;        
          i = 0;
          v = real_roomp( ch->in_room )->people;
          while( v && ch->specials.fighting != v )
          {
            i++;
            v = v->next_in_room; 
          } /* end while */

          if( ch->specials.fighting == v )
            tokillnum = i+1;
          mudlog( LOG_CHECK, "%s attempting to use %s on %d.%s", 
                  GET_NAME( ch ), tmp, tokillnum, 
                  GET_NAME( ch->specials.fighting ) );
        } /* fighting same named mob */

        if( tokillnum > 0 )
          sprintf( buf, "%s %d.%s", tmp, tokillnum,
                   GET_NAME( ch->specials.fighting ) );
        else
          sprintf( buf, "%s %s", tmp, GET_NAME( ch->specials.fighting ) );
        do_use( ch, buf, 0 );

        return TRUE;
      } /* was a violent spell */
    }         /* had charges */
  }         /* was holding */

  return FALSE;
}                 /* end func */

int SameRace( struct char_data *ch1, struct char_data *ch2)
{    
    if ((!ch1) || (!ch2))
      return(FALSE);
    
    if (ch1 == ch2)
      return(TRUE);
    
    if (in_group(ch1,ch2))
      return(TRUE);
    
    if (GET_RACE(ch1) == GET_RACE(ch2)) {
      return(TRUE);
    }
    
    return(FALSE);
}
  
int AssistFriend( struct char_data *ch)
{
  struct char_data *damsel, *targ, *tmp_ch, *next;
  
  damsel = 0;
  targ = 0;
  
  if (check_peaceful(ch, ""))
    return FALSE;
  if (ch->in_room < 0) 
  {
    mudlog( LOG_SYSERR, "Mob %sin negative room", ch->player.name);
    ch->in_room = 0;
    extract_char(ch);
    return TRUE;
  }
  if (ch->in_room == 0) 
    return FALSE;
  
  /*
   * find the people who are fighting
   */
  if( !IS_SET( ch->specials.act, ACT_WIMPY ) )
  {
  
    for( tmp_ch = (real_roomp(ch->in_room))->people; tmp_ch;
         tmp_ch = next ) 
    {
      next = tmp_ch->next_in_room;
    
      if( tmp_ch->specials.fighting && MobFriend( ch, tmp_ch ) && 
          CAN_SEE( ch, tmp_ch ) )
      {
        damsel = tmp_ch;
        break;
      }
    }
  
    if (damsel) 
    {
      /*
       * check if the people in the room are fighting.
       */
      targ = FindAnAttacker( damsel );
      if( targ )
      {
        if( targ->in_room == ch->in_room )
        {
          if( !IS_AFFECTED(ch, AFF_CHARM) || ch->master != targ )
          {
            hit( ch, targ, 0 );
            return TRUE;
          }
        }
      }
    }
  }
  return FALSE;
}
  
void FindABetterWeapon(struct char_data *mob)
{
  struct obj_data *o, *best;
  /*
    pick up and wield weapons
    Similar code for armor, etc.
    */
  
  /* check whether this mob can wield */
  if (!HasHands(mob))
    return;
  
  if (!real_roomp(mob->in_room))
    return;
  
  /* check room */
  best = 0;
  for (o = real_roomp(mob->in_room)->contents; o; o = o->next_content) {
    if (best && IS_WEAPON(o)) {
      if (GetDamage(o,mob) > GetDamage(best,mob)) {
        best = o;
      }
    } else {
      if (IS_WEAPON(o)) {
        best = o;
      }
    }
  }
  /* check inv */
  for (o = mob->carrying; o; o=o->next_content) {
    if (best && IS_WEAPON(o)) {
      if (GetDamage(o,mob) > GetDamage(best,mob)) {
        best = o;
      }
    } else {
      if (IS_WEAPON(o)) {
        best = o;
      }
    }
  }
  
  if (mob->equipment[WIELD]) {
    if (best) {
       if (GetDamage(mob->equipment[WIELD],mob) >= GetDamage(best,mob)) {
          best = mob->equipment[WIELD];
       }
    } else {
      best = mob->equipment[WIELD];
    }
  }

  if (best) {
     if (GetHandDamage(mob) > GetDamage(best, mob)) {
        best = 0;
     }
  } else {
    return;  /* nothing to choose from */
  }

  if (best) {
      /*
        out with the old, in with the new
      */
      if (best->carried_by == mob) {
         if (mob->equipment[WIELD]) {
            do_remove(mob, mob->equipment[WIELD]->name, 0);
         }
         do_wield(mob, best->name, 0);
      } else if (best->equipped_by == mob) {
        /* do nothing */
        return;
      } else {
         do_get(mob, best->name, 0);
      }      
  } else {
    if (mob->equipment[WIELD]) {
      do_remove(mob, mob->equipment[WIELD]->name, 0);
    }
  }
}
      
int GetDamage(struct obj_data *w, struct char_data *ch) 
{
  float ave;
  int iave;
  /*
    return the average damage of the weapon, with plusses.
  */

  ave = w->obj_flags.value[2]/2.0 + 0.5;
    
  ave *=w->obj_flags.value[1];
  
  ave += GetDamBonus(w);
  /*
   * check for immunity:
   */
  iave = (int)ave;
  if (ch->specials.fighting)
  {
    iave = PreProcDam(ch->specials.fighting, ITEM_TYPE(w), iave);
    iave = WeaponCheck(ch, ch->specials.fighting, ITEM_TYPE(w), iave);
  }
  return(iave);
}
      
int GetDamBonus(struct obj_data *w)
{
   int j, tot=0;

    /* return the damage bonus from a weapon */
   for(j=0; j<MAX_OBJ_AFFECT; j++) {
      if (w->affected[j].location == APPLY_DAMROLL || 
          w->affected[j].location == APPLY_HITNDAM) {
          tot += w->affected[j].modifier;        
        }
    }
    return(tot);
}
              
int GetHandDamage(struct char_data *ch) 
{
  float ave;
  int num, size, iave;
  /*
    return the hand damage of the weapon, with plusses.
        dam += dice(ch->specials.damnodice, ch->specials.damsizedice);

    */

  num  = ch->specials.damnodice;
  size = ch->specials.damsizedice;
  
  ave = size/2.0 + 0.5;
    
  ave *= num;  

  /*
   * check for immunity:
   */
  iave = (int)ave;
  if (ch->specials.fighting) {
    iave = PreProcDam(ch->specials.fighting, TYPE_HIT, iave);
    iave = WeaponCheck(ch, ch->specials.fighting, TYPE_HIT, iave);
  }
  return(iave);
}

/*
  check to see if a mob is a friend

*/


int MobFriend( struct char_data *ch, struct char_data *f)
{
  if( SameRace( ch, f ) ) 
  {
    if( IS_GOOD( ch ) ) 
    {
      if( IS_GOOD( f ) ) 
        return TRUE;
    }
    else 
    {
      if( IS_NPC( f ) )
        return TRUE;
    }
  }
  return FALSE;
}

void PulseMobiles(int type)
{
  register struct char_data *ch;
  
  for(ch = character_list; ch; ch = ch->next)
    if(IS_MOB(ch))
      if(mob_index[ch->nr].func)
        (*mob_index[ch->nr].func)(ch, 0, "", ch, type);
} 
 
void DoScript(struct char_data *ch)
{
  int i, check = 0;
  char buf[255], buf2[255], *s;

  strcpy(buf, gpScript_data[ch->script].script[ch->commandp].line);

  if(buf[strlen(buf) - 1] == '\\')
  {
    check = 1;
    buf[strlen(buf) - 1] = '\0';
  }

  strcpy(buf2, buf);
  strtok(buf2, " ");
  s = strtok(NULL, " ");
 

  i = CommandSearch(buf2);

  if(i == -1)
  {
    command_interpreter(ch, buf);
    ch->commandp++;
    return;
  }
  if(s)
    (*gpComp[i].p)(s, ch);
  else
    (*gpComp[i].p)("\0", ch);

  if(*gpScript_data[ch->script].script[ch->commandp].line == '_')
    ch->commandp++;

  if(check)
    DoScript(ch);
}

int CommandSearch(char *arg)
{
 int i;

 for(i = 0; i < top_of_comp; i++)
    if(!strcmp(gpComp[i].arg, arg))
       return(i);

 return(-1);
}

void CommandAssign( char *arg, void (*p)( char *, struct char_data * ) )
{
  if(top_of_comp == 0)
    gpComp = (struct script_com *)malloc(sizeof(struct script_com));
  else
    gpComp = (struct script_com *) realloc(gpComp, 
                               sizeof(struct script_com) * (top_of_comp + 1));
  gpComp[top_of_comp].p = p;
  gpComp[top_of_comp].arg = (char *) malloc(sizeof(char) * (strlen(arg) + 1));
  strcpy(gpComp[top_of_comp].arg, arg);
  top_of_comp++;
}

void CommandSetup()
{
 CommandAssign("noop", noop);
 CommandAssign("goto", sgoto);
 CommandAssign("end", end2);
 CommandAssign("act", do_act);
 CommandAssign("jsr", do_jsr);
 CommandAssign("jmp", do_jmp);
 CommandAssign("rts", do_rts);
 CommandAssign("stop", end2);
}

void noop(char *arg, struct char_data *ch)
{
 int i;

 if(ch->waitp > 0) {
    if(ch->waitp == 1) {
       ch->waitp = 0;
       ch->commandp++;
       return;
     }

    ch->waitp--;
    return;
  }

 if(*arg) {
   i = atoi(arg);

   if(i <= 0) {
     ch->commandp++;
     return;
   }

   ch->waitp = i - 1;
   return;
 }
 ch->commandp++;
 return; 
}

void end2(char *arg, struct char_data *ch)
{
 ch->commandp = 0;
}  

void sgoto(char *arg, struct char_data *ch)
{
  char *p;
  struct char_data *mob;
  int dir, room;

  if(arg) 
  {
    if( *arg == '$' )
    {  /* this is a creature name to follow */
      arg++;
      p = strtok(arg, " ");
      if ((mob = get_char_vis(ch, p)) == NULL) 
      {
        fprintf( stderr, "%s couldn't find mob by name %s\n",                
                 gpScript_data[ch->script].filename, p);
        ch->commandp++;
        return;
      } 
      else 
      {
        room = mob->in_room;
      }
    }
    else 
    {
      room = atoi(arg);
    }
  }
  else 
  {
    mudlog( LOG_ERROR, "Error in script %s, no destination for goto", 
            gpScript_data[ch->script].filename);
    ch->commandp++;
    return;
  }
  if (ch->in_room != room) 
  {
    dir = choose_exit_global(ch->in_room, room, MAX_ROOMS);
    if (dir < 0) 
    {
      do_say(ch, "Woah! Come sono finito qui??", 0);
      do_emote(ch, "scompare di un esplosione di fumo", 0);
      char_from_room(ch);
      char_to_room(ch, room);
      do_emote(ch, "arriva con un gran botto!", 0);
      ch->commandp++;
      return;
    }
    go_direction(ch, dir);
    if (ch->in_room == room)
      ch->commandp++;
    return;
  }
  ch->commandp++;
}

void do_act(char *arg, struct char_data *ch)
{
  int bits;
  if (arg) 
  {
    bits = atoi(arg);
    ch->specials.act = bits;
    if(!IS_SET(ch->specials.act, ACT_SCRIPT))
      SET_BIT(ch->specials.act, ACT_SCRIPT);
    if(!IS_SET(ch->specials.act, ACT_ISNPC))
      SET_BIT(ch->specials.act, ACT_ISNPC);
  }
  ch->commandp++;
  return;
}

void do_jmp(char *arg, struct char_data *ch)
{
 int i;
 char buf[255];

  for(i = 0; strcmp(gpScript_data[ch->script].script[i].line, "end\n"); i++) 
  {
    strcpy(buf,gpScript_data[ch->script].script[i].line);
    if(buf[strlen(buf) - 1] == '\n')
      buf[strlen(buf) - 1] = '\0';

    if(!strncmp(buf, arg, strlen(arg))) 
    {
      ch->commandp = i;
      return;
    }
  }

  mudlog( LOG_ERROR, "Label %s undefined in script assigned to %s. Ignoring.", 
          arg, GET_NAME(ch));

  ch->commandp++;
}

void do_jsr(char *arg, struct char_data *ch)
{
  int i;
  char buf[ 256 ];

  for(i = 0; strcmp(gpScript_data[ch->script].script[i].line, "end\n"); i++) 
  {
    strcpy(buf,gpScript_data[ch->script].script[i].line);
    if(buf[strlen(buf) - 1] == '\n')
       buf[strlen(buf) - 1] = '\0';

    if(!strncmp(buf, arg, strlen(arg))) 
    {
      ch->commandp2 = ch->commandp + 1;
      ch->commandp = i;
      return;
    }
  }

  mudlog( LOG_ERROR, "Label %s undefined in script assigned to %s. Ignoring.", 
          arg, GET_NAME(ch));

  ch->commandp++;
}

void do_rts(char *arg, struct char_data *ch)
{
 ch->commandp = ch->commandp2;
 ch->commandp2 = 0;
}

void MobHit(struct char_data *ch, struct char_data *v, int type)
{
  int base, percent, learned;
  struct obj_data *o;

  if (type != 0)
  {
    hit(ch,v,type);
  }
  else if (ch->equipment[WIELD]) /* backstab thing */
  {
    o = ch->equipment[WIELD];
    if (o->obj_flags.value[3] != 11 && o->obj_flags.value[3] != 1 &&
        o->obj_flags.value[3] != 10)
    {
      hit(ch,v,0);
    }
    else
    {
      if (ch->specials.fighting)
      {
        return;
      }

      if (v->specials.fighting)
      {
        base = 0;
      }
      else
      {
        base = 4;
      }

      learned = 50 + GetMaxLevel(ch);
      percent = number(1,100);
      if (percent > learned)
      {
        if (AWAKE(v))
        {
          AddHated(v, ch);
          damage(ch, v, 0, SKILL_BACKSTAB);
        }
        else
        {
          base += 2;
          AddHated(v, ch);
          GET_HITROLL(ch) += base;
          if( hit(ch,v,SKILL_BACKSTAB) != SubjectDead )
            GET_HITROLL(ch) -= base;
        }
      }
      else
      {
        AddHated(v, ch);
        GET_HITROLL(ch) += base;
        if( hit(ch,v,SKILL_BACKSTAB) != SubjectDead )
          GET_HITROLL(ch) -= base;
      }
    }
  }
  else
  {
    hit(ch,v,0);
  }
}

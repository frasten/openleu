/*
  DaleMUD v2.0        Released 2/1994
  See license.doc for distribution terms.   DaleMUD is based on DIKUMUD
*/

#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "protos.h"
#include "cmdid.h"
#include "fight.h"
#include "status.h"

/*   external vars  */
#if HASH
extern struct hash_header room_db;
#else
extern struct room_data *room_db;
#endif
extern struct char_data *character_list;
extern struct descriptor_data *descriptor_list; 
extern struct index_data *obj_index;
extern int rev_dir[];
extern char *dirs[]; 
extern char *dirsFrom[]; 
extern char *dirsTo[]; 
extern int movement_loss[];
extern char *exits[];

/* Internale function prots */
void DisplayOneMove(struct char_data *ch, int dir, int was_in);
void DisplayMove( struct char_data *ch, int dir, int was_in, int total);


void NotLegalMove(struct char_data *ch)
{
  send_to_char( "Non puoi andare da quella parte...\n\r", ch);
}


int ValidMove( struct char_data *ch, int cmd) 
{
  char tmp[256];
  struct room_direction_data        *exitp;
  
  if( ch == NULL )
  {
    mudlog( LOG_SYSERR, "ch == NULL in ValidMove (act.move.c)" );
    return FALSE;
  }

  exitp = EXIT( ch, cmd );

  if (affected_by_spell(ch, SPELL_WEB)) 
  {
    if (!saves_spell(ch, SAVING_PARA)) 
    {
      send_to_char("Sei intrappolato in una ragnatela appiccicosa!\n\r", ch);
      send_to_char( "I tuoi sforzi non fanno altro che intrappolarti "
                    "maggiormente!\n\r", ch);
      WAIT_STATE(ch, PULSE_VIOLENCE*5);
      if (!IS_PC(ch))
        GET_MOVE(ch)=0;
      return(FALSE);
    } 
    else 
    {
      WAIT_STATE(ch, PULSE_VIOLENCE);
      GET_MOVE(ch)-=50;
      send_to_char("Ti liberi facilmente dalla ragnatela appiccicosa!\n\r", ch);
    }
  }


  if (MOUNTED(ch)) 
  {
    if (GET_POS(MOUNTED(ch)) < POSITION_FIGHTING) 
    {
      send_to_char("La tua cavalcatura deve stare in piedi.\n\r", ch);
      return(FALSE);
    }
    if (ch->in_room != MOUNTED(ch)->in_room) 
    {
      Dismount(ch, MOUNTED(ch), POSITION_STANDING);
    }
  }
#if 0
  if (RIDDEN(ch)) 
  {
    if (ch->in_room != RIDDEN(ch)->in_room) 
    {
      Dismount(RIDDEN(ch), ch, POSITION_STANDING);
    }
  }
#endif
  
  if (!exit_ok(exitp,NULL)) 
  {
    // Se il numero di stanza in quella direzione e` 0 e c'e` una descrizione
    // per l'uscita, allora mostra la descrizione.
    if( exitp && exitp->to_room == 0 && exitp->general_description &&
        *exitp->general_description )
      send_to_char( exitp->general_description, ch );
    else
      NotLegalMove(ch);
    return(FALSE);
  } 
  else if (IS_SET(exitp->exit_info, EX_CLOSED)) 
  {
    if (exitp->keyword) 
    {
      if( !IS_SET(exitp->exit_info, EX_SECRET) &&
          (strcmp(fname(exitp->keyword), "secret"))) 
      {
        sprintf( tmp, "La %s e` chius%c.\n\r", fname(exitp->keyword),
                 IS_SET( exitp->exit_info, EX_MALE ) ? 'o' : 'a' );
        send_to_char(tmp, ch);
        return(FALSE);
      } 
      else 
      {
        NotLegalMove(ch);
        return(FALSE);
      }
    } 
    else 
    {
      NotLegalMove(ch);
      return(FALSE);
    }
  }
  else if( IS_SET( exitp->exit_info, EX_CLIMB ) && 
           !IS_AFFECTED( ch, AFF_FLYING ) )
  {
    send_to_char( "Mi spiace, ma puoi andare da quella parte solo volando od "
                  "arrampicandoti!\n\r", ch );
    return FALSE;
  }
  else 
  {
    struct room_data *rp;
    rp = real_roomp(exitp->to_room);
    if (IS_SET(rp->room_flags, TUNNEL)) 
    {
      if( MobCountInRoom( rp->people ) >= rp->moblim && !IS_IMMORTAL( ch ) )
      {
        send_to_char("Mi spiace, ma non c'e` abbastanza spazio.\n\r", ch);
        return(FALSE);
      }
    }
    if( IS_SET( rp->room_flags, PRIVATE ) )
    {
      if( MobCountInRoom( rp->people ) > 2 )
      {
        send_to_char("Mi spiace, ma il luogo e` privato.\n\r", ch);
        return(FALSE);
      }
    }
    if (IS_SET(rp->room_flags, INDOORS)) 
    {
      if (MOUNTED(ch)) 
      {
        send_to_char( "La tua cavalcatura si rifiuta di entrare in un luogo "
                      "chiuso.\n\r", ch);
        return(FALSE);
      }
    }
    if (IS_SET(rp->room_flags, DEATH)) 
    {
      if (MOUNTED(ch)) 
      {
        send_to_char( "La tua cavalcatura si rifiuta di andare da quella "
                      "parte.\n\r", ch);
        return(FALSE);
      }
    }
    return(TRUE);
  }
}

int RawMove( struct char_data *ch, int dir, int bCheckSpecial )
{
  int need_movement, new_r;
  struct obj_data *obj;
  bool has_boat;
  struct room_data *from_here, *to_here;
  
  int special(struct char_data *ch, int dir, char *arg);

  SetStatus( "RawMove started.", NULL, ch );
  if( bCheckSpecial )
  {
    SetStatus( "Calling special in RawMove" );
    if (special(ch, dir+1, ""))/* Check for special routines(North is 1)*/
    {
      SetStatus( "return 1 in RawMove" );
      
      return(FALSE);
    }
  }
  
  SetStatus( "Calling ValidMove in RawMove" );
  if (!ValidMove(ch, dir)) 
  {
    SetStatus( "return 2 in RawMove" );
    return(FALSE); 
  }

  SetStatus( "Checking charmed in RawMove" );
  
  if( IS_AFFECTED( ch, AFF_CHARM ) && ch->master && 
      ch->in_room == ch->master->in_room ) 
  {
    act("$n scoppia a piangere.", FALSE, ch, 0, 0, TO_ROOM);
    act("Tu scoppi a piangere al pensiero di lasciare $N",
        FALSE, ch, 0, ch->master, TO_CHAR);
    
    SetStatus( "return 3 in RawMove" );
    
    return(FALSE);
  }
  
  SetStatus( "Find rooms in RawMove" );
  from_here = real_roomp(ch->in_room);
  to_here = real_roomp(from_here->dir_option[ dir ]->to_room);
  new_r = from_here->dir_option[ dir ]->to_room;
  
  if( to_here == NULL )
  {
    SetStatus( "Moving in room 0 in RawMove" );
    char_from_room(ch);
    char_to_room(ch, 0);
    
    send_to_char( "Il terreno si scioglie sotto i tuoi piedi come tu cadi nel "
                  "turbinante caos.\n\r",ch);
    do_look(ch, "\0",15);
    if( IS_SET(ch->player.user_flags,SHOW_EXITS) ) 
    {
      act("$c0015-----------------",FALSE,ch,0,0,TO_CHAR);
      do_exits(ch,"",8);
      act("$c0015-----------------",FALSE,ch,0,0,TO_CHAR);
    }
    SetStatus( "return 4 in RawMove" );
    return TRUE;
  }

  if( IS_IMMORTAL(ch) && IS_SET(ch->specials.act, PLR_NOHASSLE) &&
     (!MOUNTED(ch))) 
  {
    SetStatus( "Moving immortal in RawMove" );
    char_from_room(ch);
    char_to_room(ch, new_r);
    do_look(ch, "\0",15);
    if( IS_SET(ch->player.user_flags,SHOW_EXITS) )
    {
      act("$c0015-----------------",FALSE,ch,0,0,TO_CHAR);
      do_exits(ch,"",8);
      act("$c0015-----------------",FALSE,ch,0,0,TO_CHAR);
    }
    SetStatus( "return 5 in RawMove" );
    return(TRUE);
  }

  /*
   *  nail the unlucky with traps.
   */

  if (!MOUNTED(ch)) 
  {
    SetStatus( "CheckingForMovetrap in RawMove" );
    if (CheckForMoveTrap(ch, dir))
    {
      SetStatus( "return 6 in RawMove" );
      return(FALSE);
    }
  } 
  else 
  {
    SetStatus( "CheckForMoveTrap while mounted in RawMove" );
    if (CheckForMoveTrap(MOUNTED(ch), dir))
    {
      SetStatus( "return 7 in RawMove" );
      return(FALSE);
    }
  }

  SetStatus( "Find need_movement in RawMove" );
  if (IS_AFFECTED(ch,AFF_FLYING)) 
  {
    need_movement = 1;
    if (IS_SET(to_here->room_flags, INDOORS))
      need_movement += 2;
  } 
  else if( IS_AFFECTED(ch, AFF_TRAVELLING) && 
           !IS_SET(from_here->room_flags, INDOORS)) 
  {
    need_movement = 1;
  } 
  else 
  {
    need_movement = (movement_loss[from_here->sector_type]+
                     movement_loss[to_here->sector_type]) / 2;
  }

  /*  
   **   Movement in water_nowswim
   */
  
  if ((from_here->sector_type == SECT_WATER_NOSWIM) || 
      (to_here->sector_type == SECT_WATER_NOSWIM)) 
  {
    
    if (!IS_AFFECTED(ch,AFF_FLYING)) 
    {
      if (MOUNTED(ch)) 
      {
        if (!IS_AFFECTED(MOUNTED(ch), AFF_WATERBREATH) &&
            !IS_AFFECTED(MOUNTED(ch), AFF_FLYING)) 
        {
          send_to_char( "La tua cavalcatura ha bisogno di saper volare o "
                        "nuotare per andare\n\rin quella direzione\n\r", ch);
          SetStatus( "return 8 in RawMove" );
          return(FALSE);
        }
      } 
      else 
      {
        has_boat = FALSE;
        /* See if char is carrying a boat */
        for (obj=ch->carrying; obj; obj=obj->next_content)
          if (obj->obj_flags.type_flag == ITEM_BOAT)
            has_boat = TRUE;
        if(IS_IMMORTAL(ch) && IS_SET(ch->specials.act, PLR_NOHASSLE))
          has_boat = TRUE;
        if (!has_boat && !IS_AFFECTED(ch, AFF_WATERBREATH)) 
        {
          send_to_char( "Hai bisogno di una barca per andare in quella "
                        "direzione.\n\r", ch);
          SetStatus( "return 9 in RawMove" );
          return(FALSE);
        }
        if (has_boat)
          need_movement = 1;
      }
    }
  }
  
  /*
   * Movement in SECT_AIR
   */
  if ((from_here->sector_type == SECT_AIR) || 
      (to_here->sector_type == SECT_AIR)) 
  {
    if (!IS_AFFECTED(ch,AFF_FLYING)) 
    {
      if ((!MOUNTED(ch) || !IS_AFFECTED(MOUNTED(ch), AFF_FLYING))) 
      {
        send_to_char("Hai bisogno di volare per andare da quella parte!\n\r",ch);
        SetStatus( "return 10 in RawMove" );
        return(FALSE);
      }
    }
  }
  
  /*
   * Movement in SECT_UNDERWATER
   */
  if( from_here->sector_type == SECT_UNDERWATER || 
      to_here->sector_type == SECT_UNDERWATER )
  {
    if (!IS_AFFECTED(ch,AFF_WATERBREATH)) 
    {
      send_to_char( "Dovresti avere le branchie per andare da quella "
                    "parte!\n\r", ch);
      SetStatus( "return 11 in RawMove" );
      return(FALSE);
    }

    if (MOUNTED(ch)) 
    {
      if (!IS_AFFECTED(MOUNTED(ch),AFF_WATERBREATH)) 
      {
        send_to_char( "La tua cavalcatura dovrebbe avere le branchie per "
                      "andare da quella parte!\n\r",ch);
        SetStatus( "return 12 in RawMove" );
        return(FALSE);
      }
    }
  }

  if ((from_here->sector_type == SECT_TREE) || 
      (to_here->sector_type == SECT_TREE)) 
  {
    if (!IS_AFFECTED(ch,AFF_TREE_TRAVEL)) 
    {
      send_to_char("Vuoi camminare attraverso agli alberi ?\n\r",ch);
      SetStatus( "return 13 in RawMove" );
      return(FALSE);
    }
    if (MOUNTED(ch)) 
    {
      if (!IS_AFFECTED(MOUNTED(ch),AFF_TREE_TRAVEL)) 
      {
        send_to_char( "La tua cavalcatura non puo` camminare attraverso gli "
                      "alberi!\n\r",ch);
        SetStatus( "return 14 in RawMove" );
        return(FALSE);
      }
    }    
  }
  
  if (!MOUNTED(ch)) 
  {
    if (GET_MOVE(ch)<need_movement) 
    {
      act( "Sei esaust$b.", 0, ch, 0, ch, TO_CHAR );
      SetStatus( "return 15 in RawMove" );
      return(FALSE);
    }
  } 
  else if (GET_MOVE(MOUNTED(ch))<need_movement) 
  {
    send_to_char("La tua cavalcatura e` esausta.\n\r", ch);
    SetStatus( "return 16 in RawMove" );
    return(FALSE);
  }
 
  SetStatus( "Loosing movement in RawMove" );
  if (!IS_IMMORTAL(ch) || MOUNTED(ch)) 
  {
    if (IS_NPC(ch)) 
    {
      GET_MOVE(ch) -= 1;
    } 
    else 
    {
      if (MOUNTED(ch)) 
      {
        GET_MOVE(MOUNTED(ch)) -= need_movement;
      } 
      else 
      {
        GET_MOVE(ch) -= need_movement;
      }
    }
  }
  

  if (MOUNTED(ch)) 
  {  
    SetStatus( "Moving while mounted in RawMove" );
    char_from_room(ch);
    char_to_room(ch, new_r);
    char_from_room(MOUNTED(ch));
    char_to_room(MOUNTED(ch), new_r);
  } 
  else 
  {
    SetStatus( "Moving in RawMove" );
    char_from_room(ch);
    char_to_room(ch, new_r);
  }
  SetStatus( "Looking in RawMove" );
  do_look(ch, "\0",15);
  
  if (IS_SET(to_here->room_flags, DEATH) && !IS_IMMORTAL(ch)) 
  {
    SetStatus( "Death room in RawMove" );
    if (MOUNTED(ch))
      NailThisSucker(MOUNTED(ch));
    NailThisSucker(ch);

    SetStatus( "return 17 in RawMove" );
    return(FALSE);
  }

  /*
  **  do something with track 
  */

  SetStatus( "Checking for tracking in RawMove" );
  if (IS_NPC(ch)) 
  {
    if (ch->specials.hunting) 
    {
      if (IS_SET(ch->specials.act, ACT_HUNTING) && ch->desc)
        WAIT_STATE(ch, PULSE_VIOLENCE);
    }
  } 
  else 
  {
    if (ch->specials.hunting) 
    {
      if (IS_SET(ch->specials.act, PLR_HUNTING)) 
      {
        send_to_char("Stai cercando un traccia\n\r", ch);
        WAIT_STATE(ch, PULSE_VIOLENCE);
      }
    }
  } 
  /* show exits */

  SetStatus( "Show exit in RawMove" );
  if( IS_SET(ch->player.user_flags,SHOW_EXITS) )
  {
    act("$c0015-----------------",FALSE,ch,0,0,TO_CHAR);
    do_exits(ch,"",8);
    act("$c0015-----------------",FALSE,ch,0,0,TO_CHAR);
  }

  SetStatus( "last return in RawMove" );
  return(TRUE);
  
}


int MoveOne(struct char_data *ch, int dir, int bCheckSpecial )
{
  int was_in;
  
  was_in = ch->in_room;
  if( RawMove( ch, dir, bCheckSpecial ) ) 
  {  /* no error */
    DisplayOneMove(ch, dir, was_in);
    return TRUE;
  } 
  else
    return FALSE;
  
}

void MoveGroup( struct char_data *ch, int dir)
{
  int was_in;
  struct follow_type *k, *next_dude;
  
  /*
   *   move the leader. (leader never duplicates)
   */
  
  was_in = ch->in_room;
  if( RawMove( ch, dir, TRUE ) ) 
  {  /* no error */
    SetStatus( "DisplayOneMove 1 in MoveGroup" );
    DisplayOneMove(ch, dir, was_in);
    SetStatus( "Checking followers in MoveGroup" );
    if( ch->followers )
    {
      for(k = ch->followers; k; k = next_dude) 
      {
        next_dude = k->next;
        /*
         *  compose a list of followers, w/heaping
         */
        if( was_in == k->follower->in_room &&
            GET_POS(k->follower) >= POSITION_STANDING )
        {
          act("Tu segui $N.", FALSE, k->follower, 0, ch, TO_CHAR);
          if (k->follower->followers) 
          {
            MoveGroup(k->follower, dir);
          } 
          else 
          {
            if( RawMove( k->follower, dir, TRUE ) ) 
            {
              SetStatus( "DisplayOneMove 2 in MoveGroup" );
              DisplayOneMove(k->follower, dir, was_in);
            }
          }
        }
      }
    }
  }
  SetStatus( "return in MoveGroup" );
}

void DisplayOneMove(struct char_data *ch, int dir, int was_in)
{
  DisplayMove(ch, dir, was_in, 1);
}

void DisplayGroupMove(struct char_data *ch, int dir, int was_in, int total)
{
  DisplayMove(ch, dir, was_in, total);
}


void do_move(struct char_data *ch, char *argument, int cmd)
{
  SetStatus( "do_move started", GET_NAME_DESC( ch ), ch );

  if (RIDDEN(ch)) 
  {
    if (RideCheck(RIDDEN(ch), 0)) 
    {
      do_move(RIDDEN(ch), argument, cmd);
      SetStatus( "Returning from move 1", NULL );
      return;
    } 
    else 
    {
      FallOffMount(RIDDEN(ch), ch);
      Dismount(RIDDEN(ch), ch, POSITION_SITTING);
    }
  }
  
  cmd -= 1;
  
  /*
   ** the move is valid, check for follower/master conflicts.
   */
  
  if (ch->attackers > 1) 
  {
    send_to_char( "C'e` troppa gente intorno, non hai spazio per fuggire!\n\r", 
                  ch);
    SetStatus( "Returning from move 2", NULL );
    return;
  }
  
  if (!ch->followers && !ch->master) 
  {
    MoveOne( ch, cmd, TRUE );
  } 
  else 
  {
    if (!ch->followers) 
    {
      MoveOne( ch, cmd, TRUE );
    } 
    else 
    {
      MoveGroup(ch, cmd);
    }
  }
  SetStatus( "Returning from move 3", NULL );
}


/*
 * MoveOne and MoveGroup print messages.  Raw move sends success or failure.
 */


void DisplayMove( struct char_data *ch, int dir, int was_in, int total)
{
  struct char_data *tmp_ch;
  char tmp[256];
  
  for (tmp_ch = real_roomp(was_in)->people; tmp_ch; 
       tmp_ch= tmp_ch->next_in_room) 
  {
    if ((!IS_AFFECTED(ch, AFF_SNEAK)) || (IS_IMMORTAL(tmp_ch))) 
    {
      if ((ch != tmp_ch) && (AWAKE(tmp_ch)) && (CAN_SEE(tmp_ch, ch))) 
      {
        if (!IS_AFFECTED(ch, AFF_SILENCE) || number(0,2)) 
        {
          if (total > 1) 
          {
            if (IS_NPC(ch)) 
            {
              if (IS_AFFECTED(ch,AFF_FLYING)) 
                sprintf( tmp,"%s volano verso %s. [%d]\n\r",
                         ch->player.short_descr, dirsTo[dir], total);
              else
                sprintf( tmp,"%s vanno verso %s. [%d]\n\r",
                         ch->player.short_descr, dirsTo[dir], total);
            }
            else
            {
              if (IS_AFFECTED(ch,AFF_FLYING)) 
                sprintf( tmp,"%s volano verso %s. [%d]\n\r", GET_NAME(ch), 
                         dirsTo[dir], total );
              else
                sprintf( tmp,"%s vanno verso %s. [%d]\n\r", GET_NAME(ch),
                         dirsTo[dir], total);
            }
          }
          else 
          {
            if (IS_NPC(ch)) 
            {
              if (MOUNTED(ch)) 
                sprintf( tmp,"%s va verso %s, cavalcando %s\n\r",
                         ch->player.short_descr, dirsTo[dir], 
                         MOUNTED(ch)->player.short_descr );
              else if (IS_AFFECTED(ch,AFF_FLYING)) 
                sprintf( tmp,"%s vola verso %s.\n\r",
                         ch->player.short_descr, dirsTo[dir] );
              else
                sprintf( tmp,"%s va verso %s.\n\r",ch->player.short_descr,
                         dirsTo[dir]);
            }
            else 
            {
              if (MOUNTED(ch)) 
                sprintf( tmp,"%s va verso %s, cavalcando %s\n\r", GET_NAME(ch), 
                         dirsTo[dir], MOUNTED(ch)->player.short_descr );
              else if (IS_AFFECTED(ch,AFF_FLYING))
                sprintf(tmp,"%s vola verso %s.\n\r", GET_NAME(ch), dirsTo[dir]);
              else
                sprintf( tmp,"%s va verso %s\n\r", GET_NAME(ch), dirsTo[ dir ] );
            }
          }
          if( strlen( tmp ) >= 255 )
            mudlog( LOG_SYSERR, "strlen( tmp ) >= 255" );
          send_to_char(tmp, tmp_ch);
        }
      }
    }
  }
  
  for (tmp_ch = real_roomp(ch->in_room)->people; tmp_ch; 
       tmp_ch = tmp_ch->next_in_room) 
  {
    if (((!IS_AFFECTED(ch, AFF_SNEAK)) || (IS_IMMORTAL(tmp_ch))) &&
        (CAN_SEE(tmp_ch,ch)) &&        (AWAKE(tmp_ch))) 
    {
      if (tmp_ch != ch && (!IS_AFFECTED(ch, AFF_SILENCE) || number(0,2))) 
      {
        if (dir < 6) 
        {
          if (total == 1) 
          {
            if (MOUNTED(ch)) 
              sprintf(tmp, "%s arriva %s, cavalcando %s", 
                      PERS( ch, tmp_ch ), dirsFrom[ rev_dir[ dir ] ], 
                      PERS( MOUNTED( ch ), tmp_ch ) );
            else if (IS_AFFECTED(ch,AFF_FLYING))
              sprintf(tmp, "%s arriva volando %s.", 
                      PERS(ch, tmp_ch),dirsFrom[rev_dir[dir]]);
            else
              sprintf(tmp, "%s arriva %s.", 
                      PERS(ch, tmp_ch),dirsFrom[rev_dir[dir]]);
          }
          else
          {
            if (IS_AFFECTED(ch,AFF_FLYING)) 
              sprintf(tmp, "%s arrivano volando %s.", 
                      PERS(ch, tmp_ch),dirsFrom[rev_dir[dir]]);
            else
              sprintf(tmp, "%s arrivano %s.", 
                      PERS(ch, tmp_ch),dirsFrom[rev_dir[dir]]);
          }
        }
        else   /* dir >= 6 ossia non viene da una direzione particolare */
        {
          if (total == 1) 
          {
            if (MOUNTED(ch)) 
              sprintf(tmp, "Arriva %s cavalcando %s.", 
                      PERS(ch, tmp_ch), PERS(MOUNTED(ch), tmp_ch));
            else if (IS_AFFECTED(ch,AFF_FLYING)) 
              sprintf(tmp, "Arriva %s volando.", PERS(ch, tmp_ch));
            else
              sprintf(tmp, "Arriva %s.", PERS(ch, tmp_ch));
          }
          else 
          {
            if (IS_AFFECTED(ch,AFF_FLYING)) 
              sprintf( tmp, "Arrivano %s volando.", PERS(ch, tmp_ch));
            else
              sprintf( tmp, "Arrivano %s.", PERS(ch, tmp_ch));
          }
        }
        if (total > 1) 
          sprintf(tmp+strlen(tmp), " [%d]", total);
        
        strcat(tmp, "\n\r");
        if( strlen( tmp ) >= 255 )
          mudlog( LOG_SYSERR, "strlen( tmp ) >= 255" );
        send_to_char(tmp, tmp_ch);
      }
    }
  }
}


int AddToCharHeap( struct char_data *heap[50], int *top, int total[50], 
                  struct char_data *k) 
{
  int found, i;
  
  if (*top > 50) 
  {
    return(FALSE);
  } 
  else 
  {
    found = FALSE;
    for (i=0;(i<*top&& !found);i++) 
    {
      if (*top>0) 
      {
        if ((IS_NPC(k)) &&
            (k->nr == heap[i]->nr) &&
            (heap[i]->player.short_descr) &&
            (!strcmp(k->player.short_descr, heap[i]->player.short_descr))) 
        {
          total[i] += 1;
          found=TRUE;
        }
      }          
    }
    if (!found) 
    {
      heap[*top] = k;
      total[*top] = 1;
      *top+=1;
    }
  }
  return(TRUE);
}


int find_door(struct char_data *ch, char *type, char *dir)
{
  char buf[MAX_STRING_LENGTH];
  int door;
  extern char *dirs[];
  struct room_direction_data *exitp;
  
  if( *dir )
  { /* a direction was specified */ 
    if( ( door = search_block( dir, dirs, FALSE ) ) == -1 ) 
    { /* Partial Match */
      sprintf( buf, "'%s' non e` una direzione.\n\r", dir );
      send_to_char( buf, ch);
      return -1;
    }
    exitp = EXIT( ch, door );
    if( exitp && 
        ( IS_SET( exitp->exit_info, EX_ISDOOR ) &&
          ( ( !exitp->keyword && ( !IS_SET( exitp->exit_info, EX_SECRET ) ||
                                   strcmp( type, "secret" ) == 0 ) ) ||
            ( exitp->keyword && isname( type, exitp->keyword ) ) ) ) )
    {
      return( door );
    }
    else
    {
      sprintf( buf, "Non vedo %s verso %s.\n\r", type, dirsTo[ door ] );
      send_to_char( buf, ch );
      return -1;
    }
  } 
  else 
  { 
    if( ( door = search_block( type, dirs, FALSE ) ) != -1 )
    {
      if( ( exitp = EXIT( ch, door ) ) &&
          IS_SET( exitp->exit_info, EX_ISDOOR ) &&
          !IS_SET( exitp->exit_info, EX_SECRET ) )
        return door;
      else
      {
        sprintf( buf, "Non vedi porte verso %s.\n\r", dirsTo[ door ] );
        send_to_char( buf, ch );
        return -1;
      }
    }
    else
    {
      /* try to locate the keyword */
      for( door = 0; door <= 5; door++ )
      {
        if( ( exitp = EXIT( ch, door ) ) &&
            IS_SET( exitp->exit_info, EX_ISDOOR ) &&
            ( ( exitp->keyword && isname( type, exitp->keyword ) ) ||
              ( !exitp->keyword && ( !IS_SET( exitp->exit_info, EX_SECRET ) ||
                                     strcmp( type, "secret" ) == 0 ) ) ) ) 
          return(door);
      }

      sprintf( buf, "Non vedo %s qui.\n\r", type );
      send_to_char(buf, ch);
      return -1;
    }
  }
}


/* remove all necessary bits and send messages */
void open_door(struct char_data *ch, int dir)
{
  struct room_direction_data *exitp, *back;
  struct room_data        *rp;
  char        buf[MAX_INPUT_LENGTH];
  
  rp = real_roomp(ch->in_room);
  if (rp==NULL) 
  {
    mudlog( LOG_SYSERR, "NULL rp in open_door() for %s.", GET_NAME_DESC( ch ) );
    return;
  }
  
  exitp = rp->dir_option[dir];
  
  REMOVE_BIT( exitp->exit_info, EX_CLOSED );
  if( exitp->keyword )
  {
    if( !IS_SET( exitp->exit_info, EX_SECRET ) ) 
    {
      sprintf( buf, "$n apre %s %s", 
               IS_SET( exitp->exit_info, EX_MALE ) ? "un" : "una",
               fname( exitp->keyword ) );
      act( buf, FALSE, ch, 0, 0, TO_ROOM );
    } 
    else 
    {
      act("$n rivela un passaggio segreto !", FALSE, ch, 0, 0, TO_ROOM);
    }
  } 
  else
    act("$n apre la porta.", FALSE, ch, 0, 0, TO_ROOM);
  
  /* now for opening the OTHER side of the door! */
  if( exit_ok( exitp, &rp ) &&
      ( back = rp->dir_option[ rev_dir[ dir ] ] ) &&
      ( back->to_room == ch->in_room ) )
  {
    REMOVE_BIT(back->exit_info, EX_CLOSED);
    if( !IS_SET( back->exit_info, EX_SECRET ) )        
    {
      if( back->keyword )
      {
        sprintf( buf, "%s %s e` apert%c dall'altro lato.\n\r",
                 IS_SET( back->exit_info, EX_MALE ) ? "Il" : "La",
                 fname( back->keyword ), 
                 IS_SET( back->exit_info, EX_MALE ) ? 'o' : 'a' );
        send_to_room(buf, exitp->to_room);
      }
      else
        send_to_room( "La porta e` aperta dall'altro lato.\n\r", 
                      exitp->to_room );
    } 
    else 
    {
      send_to_room( "Un passaggio segreto e` stato aperto dall'altro lato.\n\r", 
                    exitp->to_room);
    }
  }
}
/* remove all necessary bits and send messages */
void raw_open_door(struct char_data *ch, int dir)
{
  struct room_direction_data *exitp, *back;
  struct room_data        *rp;
  char        buf[MAX_INPUT_LENGTH];
  
  rp = real_roomp(ch->in_room);
  if( rp == NULL )
  {
    mudlog( LOG_SYSERR, "NULL rp in raw_open_door() for %s.", 
            GET_NAME_DESC( ch ) );
    return;
  }
  
  exitp = rp->dir_option[dir];
  
  REMOVE_BIT( exitp->exit_info, EX_CLOSED );
  /* now for opening the OTHER side of the door! */
  if( exit_ok( exitp, &rp ) &&
      ( back = rp->dir_option[ rev_dir[ dir ] ] ) &&
      ( back->to_room == ch->in_room ) )    
  {
    REMOVE_BIT( back->exit_info, EX_CLOSED );
    if( !IS_SET( back->exit_info, EX_SECRET ) )
    {
      if( back->keyword )
      {
        sprintf( buf, "%s %s e` apert%c dall'altro lato.\n\r",
                 IS_SET( back->exit_info, EX_MALE ) ? "Il" : "La",
                 fname( back->keyword ), 
                 IS_SET( back->exit_info, EX_MALE ) ? 'o' : 'a' );
        send_to_room( buf, exitp->to_room );
      }
      else
        send_to_room( "La porta e` aperta dall'altro lato.\n\r", 
                      exitp->to_room );
    } 
    else 
    {
      send_to_room( "Un passaggio e` stato aperto dall'altro lato.\n\r",
                    exitp->to_room );
    }
  }                                                 
}

int canDig( struct char_data *ch )
{
  struct obj_data *pObj = ch->equipment[ HOLD ];
  if( pObj && IS_SET( pObj->obj_flags.extra_flags, ITEM_DIG ) )
    return TRUE;
  else if( ( pObj = ch->equipment[ WIELD ] ) &&
           IS_SET( pObj->obj_flags.extra_flags, ITEM_DIG ) )
    return TRUE;
  else
    return FALSE;
}

int canScythe( struct char_data *ch )
{
  struct obj_data *pObj = ch->equipment[ HOLD ];
  if( pObj && IS_SET( pObj->obj_flags.extra_flags, ITEM_SCYTHE ) )
    return TRUE;
  else if( ( pObj = ch->equipment[ WIELD ] ) &&
           IS_SET( pObj->obj_flags.extra_flags, ITEM_SCYTHE ) )
    return TRUE;
  else
    return FALSE;
}

void do_open_exit(struct char_data *ch, char *argument, int cmd)
{
  int door;
  char type[MAX_INPUT_LENGTH], dir[MAX_INPUT_LENGTH], buf[MAX_STRING_LENGTH];
  struct room_direction_data    *exitp,*back;
  struct room_data      *rp;

  char *cmdnameT = NULL;
  char *cmdnameP = NULL;
  char *cmdnameI = NULL;
  /*=FindCommandName(cmd);*/

  switch( cmd )
  {
    case CMD_PULL:
      cmdnameT = "tira";
      cmdnameP = "tiri";
      cmdnameI = "tirare";
      break;
    case CMD_TWIST:
      cmdnameT = "svita";
      cmdnameP = "sviti";
      cmdnameI = "svitare";
      break;
    case CMD_TURN:
      cmdnameT = "gira";
      cmdnameP = "giri";
      cmdnameI = "girare";
      break;
    case CMD_LIFT:
      cmdnameT = "solleva";
      cmdnameP = "sollevi";
      cmdnameI = "sollevare";
      break;
    case CMD_PUSH:
      cmdnameT = "spinge";
      cmdnameP = "spingi";
      cmdnameI = "spingere";
      break;
    case CMD_DIG:
      cmdnameT = "scava";
      cmdnameP = "scavi";
      cmdnameI = "scavare";
      break;
    case CMD_SCYTHE:
      cmdnameT = "taglia";
      cmdnameP = "tagli";
      cmdnameI = "tagliare";
      break;
    default:
      mudlog( LOG_ERROR, "Invalid cmd number (%d) in do_open_exit (act.move.c)", 
              cmd );
      return;
  }

  if( !cmdnameI ) 
  {
    mudlog( LOG_ERROR, "something really wrong happen in do_open_exit, "
                       "cmd:%d\r\n",cmd );
    return;
  }

  argument_interpreter(argument, type, dir);

  if (!*type) 
  {
    sprintf( buf,"Che cosa vuoi %s ?\r\n", cmdnameI );
    *buf=toupper(*buf); /*  ;-) */
    send_to_char(buf,ch);
    return;
  }
  else if( ( door = find_door( ch, type, dir ) ) >= 0 ) 
  {

    /* perhaps it is a something like door */
    exitp = EXIT( ch, door );
    if( !IS_SET( exitp->exit_info, EX_ISDOOR ) )
      send_to_char("Ho paura che questo sia impossibile.\n\r", ch);
    else if( exitp->open_cmd != -1 && cmd == exitp->open_cmd ) 
    {
      if( !IS_SET( exitp->exit_info, EX_CLOSED ) ) 
      {
        if( cmd != CMD_DIG && cmd != CMD_SCYTHE )
        {
          SET_BIT( exitp->exit_info, EX_CLOSED );
          if( IS_SET( exitp->exit_info, EX_SECRET ) ) 
          {
            sprintf(buf,"$n chiude un passaggio segreto verso %s",dirsTo[door]);
            act(buf,0,ch,0,0,TO_ROOM);
          } 
          else 
          {
            sprintf( buf,"$n %s %s $F e chiude il passaggio verso %s", 
                       cmdnameT, 
                       IS_SET( exitp->exit_info, EX_MALE ) ? "il" : "la",
                       dirsTo[ door ] );
            act( buf, 0, ch, 0, exitp->keyword, TO_ROOM);
          }
          sprintf( buf,"Tu %s %s %s e chiudi il passaggio verso %s\r\n", 
                   cmdnameP, IS_SET( exitp->exit_info, EX_MALE ) ? "il" : "la",
                   fname( exitp->keyword ), dirsTo[ door ] );
          send_to_char( buf, ch );
          /* handle backdoor */
          if( exit_ok( exitp, &rp ) && 
              ( back = rp->dir_option[ rev_dir[ door ] ] ) &&
              back->to_room == ch->in_room ) 
          {
            SET_BIT( back->exit_info, EX_CLOSED );
          }
        }
        else
        {
          switch( cmd )
          {
            case CMD_DIG:
              act( "E` inutile scavare ancora.", 0, ch, 0, 0, TO_CHAR );
              break;
            case CMD_SCYTHE:
              act( "Non c'e` piu` nulla da tagliare.", 0, ch, 0, 0, TO_CHAR );
              break;
            default:
              mudlog( LOG_ERROR, "Impossible error with cmd = %d in "
                                 "do_open_exit", cmd );
              break;
          }
        }
      } /* end if !closed */
      else 
      {
        if( ( cmd == CMD_DIG && canDig( ch ) ) ||
            ( cmd == CMD_SCYTHE && canScythe( ch ) ) ||
            ( cmd != CMD_DIG && cmd != CMD_SCYTHE ) )
        {
          raw_open_door( ch, door );
          if( IS_SET( exitp->exit_info, EX_SECRET ) ) 
          {
            sprintf(buf,"$n apre un passaggio verso %s", dirsTo[door]);
            act( buf, 0, ch, 0, 0, TO_ROOM );
          } 
          else 
          {
            sprintf( buf,"$n %s %s $F ed apre un passaggio verso %s", cmdnameT,
                     IS_SET( exitp->exit_info, EX_MALE ) ? "il" : "la",
                     dirsTo[door]);
            act(buf, 0, ch, 0, exitp->keyword, TO_ROOM);
          }
          sprintf( buf,"Tu %s %s %s ed apri un passaggio verso %s\r\n", 
                   cmdnameP, IS_SET( exitp->exit_info, EX_MALE ) ? "il" : "la",
                   fname( exitp->keyword ), dirsTo[ door ] );
          send_to_char( buf, ch );
        }
        else
        {
          act( "Forse usando l'attrezzo adatto...", 0, ch, 0, 0, TO_CHAR );
          if( !IS_SET( exitp->exit_info, EX_SECRET ) )
          {
            sprintf( buf, "$n prova a %s %s $F a mani nude.", cmdnameI,
                     IS_SET( exitp->exit_info, EX_MALE ) ? "un" : "una" );
            act( buf, 0, ch, 0, exitp->keyword, TO_ROOM );
          }
        }
      }
    }
    else 
    {
      send_to_char( "No-no! Fallo da qualche altra parte, per favore...\n\r", 
                    ch);
    }
  }
}

void do_open(struct char_data *ch, char *argument, int cmd)
{
  int door;
  char type[MAX_INPUT_LENGTH], dir[MAX_INPUT_LENGTH], buf[MAX_STRING_LENGTH];
  struct obj_data *obj;
  struct char_data *victim;
  struct room_direction_data        *exitp;
  
  argument_interpreter( argument, type, dir );
  
  if( !*type )
    send_to_char("Cosa vuoi aprire ?\n\r", ch);
  else if( generic_find( argument, FIND_OBJ_INV | FIND_OBJ_ROOM, ch, &victim, 
                         &obj ) ) 
  {
    
    /* this is an object */    
    if( obj->obj_flags.type_flag != ITEM_CONTAINER )
      send_to_char( "Non e` un contenitore.\n\r", ch );
    else if( !IS_SET( obj->obj_flags.value[1], CONT_CLOSED ) )
      send_to_char( "Ma e` gia` aperto !\n\r", ch );
    else if( !IS_SET( obj->obj_flags.value[1], CONT_CLOSEABLE ) )
      send_to_char( "Non puoi farlo.\n\r", ch );
    else if( IS_SET( obj->obj_flags.value[1], CONT_LOCKED ) )
      send_to_char( "Sembra chiuso a chiave.\n\r", ch );
    else
    {
      REMOVE_BIT( obj->obj_flags.value[1], CONT_CLOSED );
      send_to_char( "Ok.\n\r", ch );
      act( "$n apre $p.", FALSE, ch, obj, 0, TO_ROOM );
    }
  } 
  else if( ( door = find_door( ch, type, dir ) ) >= 0 ) 
  {
    
    /* perhaps it is a door */
    exitp = EXIT( ch, door );
    if( !IS_SET( exitp->exit_info, EX_ISDOOR ) )
      send_to_char( "Ho paura che questo sia impossibile.\n\r", ch );
    else if( !IS_SET( exitp->exit_info, EX_CLOSED ) )
    {
      sprintf( buf, "E` gia` apert%c !\n\r", 
               IS_SET( exitp->exit_info, EX_MALE ) ? 'o' : 'a' );
      send_to_char( buf, ch );
    }
    else if( IS_SET( exitp->exit_info, EX_LOCKED ) )
    {
      sprintf( buf, "Sembra chius%c a chiave !\n\r", 
               IS_SET( exitp->exit_info, EX_MALE ) ? 'o' : 'a' );
      send_to_char( buf, ch );
    }
    else if( exitp->open_cmd == -1 || exitp->open_cmd == cmd ) 
    {
      open_door(ch, door);
      send_to_char( "Ok.\n\r", ch );
    }
    else 
    {
      sprintf( buf, "Non puoi APRIRL%c !\n\r", 
               IS_SET( exitp->exit_info, EX_MALE ) ? 'O' : 'A' );
      send_to_char( buf, ch );
    }
  }
}


void do_close(struct char_data *ch, char *argument, int cmd)
{
  int door;
  char type[MAX_INPUT_LENGTH], dir[MAX_INPUT_LENGTH], buf[MAX_STRING_LENGTH];
  struct room_direction_data *back, *exitp;
  struct obj_data *obj;
  struct char_data *victim;
  struct room_data        *rp;
  
  
  argument_interpreter( argument, type, dir );
  
  if( !*type )
    send_to_char( "Cosa vuoi chiudere?\n\r", ch);
  else if( generic_find( argument, FIND_OBJ_INV | FIND_OBJ_ROOM,
                         ch, &victim, &obj ) )
  {
    
    /* this is an object */
    
    if (obj->obj_flags.type_flag != ITEM_CONTAINER)
      send_to_char("Non e` un contenitore.\n\r", ch);
    else if (IS_SET(obj->obj_flags.value[1], CONT_CLOSED))
      send_to_char("Ma e` gia` aperto !\n\r", ch);
    else if (!IS_SET(obj->obj_flags.value[1], CONT_CLOSEABLE))
      send_to_char("Questo e` impossibile.\n\r", ch);
    else
    {
      SET_BIT(obj->obj_flags.value[1], CONT_CLOSED);
      send_to_char("Ok.\n\r", ch);
      act("$n chiude $p.", FALSE, ch, obj, 0, TO_ROOM);
    }
  } 
  else if( ( door = find_door( ch, type, dir ) ) >= 0 ) 
  {
    
    /* Or a door */
    exitp = EXIT(ch, door);
    if( !IS_SET( exitp->exit_info, EX_ISDOOR ) )
      send_to_char("Questo e` assurdo.\n\r", ch);
    else if( IS_SET( exitp->exit_info, EX_CLOSED ) )
    {
      sprintf( buf, "E` gia` chius%c !\n\r", 
               IS_SET( exitp->exit_info, EX_MALE ) ? 'o' : 'a' );
      send_to_char( buf, ch );
    }
    else if( exitp->open_cmd != -1) 
    {
      sprintf( buf, "Non puoi CHIUDERL%c !\n\r", 
               IS_SET( exitp->exit_info, EX_MALE ) ? 'O' : 'A' );
      send_to_char( buf, ch );
    }
    else 
    {
      SET_BIT(exitp->exit_info, EX_CLOSED);
      if( exitp->keyword )
      {
        if( !IS_SET( exitp->exit_info, EX_SECRET ) ) 
        {
          sprintf( buf, "$n chiude %s %s verso %s", 
                   IS_SET( exitp->exit_info, EX_MALE ) ? "un" : "una",
                   fname( exitp->keyword ), dirsTo[ door ] );
          act( buf, FALSE, ch, 0, 0, TO_ROOM );
        } 
        else 
        {
          act("$n chiude un passaggio segreto.", FALSE, ch, 0, 0, TO_ROOM);
        }
      }
      else
      {
        sprintf( buf, "$n chiude la porta verso %s.", dirsTo[ door ] );
        act( buf, FALSE, ch, 0, 0, TO_ROOM);
      }
      send_to_char("Ok.\n\r", ch);

      /* now for closing the other side, too */
      if( exit_ok( exitp, &rp ) &&
          (back = rp->dir_option[rev_dir[door]]) &&
          (back->to_room == ch->in_room) ) 
      {
        SET_BIT(back->exit_info, EX_CLOSED);
        if( back->keyword )
        {
          if( !IS_SET( back->exit_info, EX_SECRET ) )
          {
            sprintf( buf, "%s %s verso %s si chiude silenziosamente.\n\r", 
                     IS_SET( back->exit_info, EX_MALE ) ? "Il" : "La",
                     back->keyword, dirsTo[ rev_dir[ door ] ] );
            send_to_room(buf, exitp->to_room);
          }
        }
        else
        {
          sprintf( buf, "La porta verso %s si chiude silenziosamente.\n\r", 
                   dirsTo[ rev_dir[ door ] ] );
          send_to_room( buf, exitp->to_room ); 
        }
      }
    }
  }
  else
    send_to_char( "Non posso chiudere niente del genere.\n\r", ch);
}


int has_key(struct char_data *ch, int key)
{
  struct obj_data *o;
  
  for (o = ch->carrying; o; o = o->next_content)
    if( o->item_number >= 0 && obj_index[o->item_number].iVNum == key )
      return(1);
  
  if( ch->equipment[HOLD] && ch->equipment[HOLD]->item_number >= 0 &&
      obj_index[ch->equipment[HOLD]->item_number].iVNum == key )
      return(1);
  
  return(0);
}


void raw_unlock_door( struct char_data *ch, 
                     struct room_direction_data *exitp, int door)
{
  struct room_data *rp;
  struct room_direction_data *back;

  REMOVE_BIT(exitp->exit_info, EX_LOCKED);
  /* now for unlocking the other side, too */
  rp = real_roomp(exitp->to_room);
  if (rp && (back = rp->dir_option[rev_dir[door]]) &&
      back->to_room == ch->in_room) 
  {
    REMOVE_BIT(back->exit_info, EX_LOCKED);
  } 
  else 
  {
    mudlog( LOG_ERROR, "Inconsistent door locks in rooms %ld->%ld", 
            ch->in_room, exitp->to_room);
  }
}

void raw_lock_door( struct char_data *ch, 
                   struct room_direction_data *exitp, int door)
{
  struct room_data *rp;
  struct room_direction_data *back;

  SET_BIT(exitp->exit_info, EX_LOCKED);
  /* now for locking the other side, too */
  rp = real_roomp(exitp->to_room);
  if (rp &&
      (back = rp->dir_option[rev_dir[door]]) &&
      back->to_room == ch->in_room) 
  {
    SET_BIT(back->exit_info, EX_LOCKED);
  } 
  else 
  {
    mudlog( LOG_ERROR, "Inconsistent door locks in rooms %ld->%ld", 
            ch->in_room, exitp->to_room);
  }
}

void do_lock(struct char_data *ch, char *argument, int cmd)
{
  int door;
  char type[MAX_INPUT_LENGTH], dir[MAX_INPUT_LENGTH];
  struct room_direction_data *exitp;
  struct obj_data *obj;
  struct char_data *victim;
  
  argument_interpreter(argument, type, dir);
  
  if (!*type)
    send_to_char("Cosa vuoi chiudere a chiave ?\n\r", ch);
  else if (generic_find(argument, FIND_OBJ_INV | FIND_OBJ_ROOM,
                        ch, &victim, &obj)) 
  {
    
    /* this is an object */
    
    if (obj->obj_flags.type_flag != ITEM_CONTAINER)
      send_to_char("Non e` un contenitore.\n\r", ch);
    else if (!IS_SET(obj->obj_flags.value[1], CONT_CLOSED))
      send_to_char("Forse dovresti chiuderlo, prima...\n\r", ch);
    else if (obj->obj_flags.value[2] < 0)
      send_to_char("Questa cosa non puo` essere chiusa a chiave.\n\r", ch);
    else if (!has_key(ch, obj->obj_flags.value[2]))
      send_to_char("Sembra che tu non abbia la chiave giusta.\n\r", ch);        
    else if (IS_SET(obj->obj_flags.value[1], CONT_LOCKED))
      send_to_char("E` gia` chiuso a chiave.\n\r", ch);
    else 
    {
      SET_BIT(obj->obj_flags.value[1], CONT_LOCKED);
      send_to_char("*Cluck*\n\r", ch);
      act("$n chiude a chiave $p.", FALSE, ch, obj, 0, TO_ROOM);
    }
  } 
  else if ((door = find_door(ch, type, dir)) >= 0) 
  {
    
    /* a door, perhaps */
    exitp = EXIT(ch, door);
    
    if (!IS_SET(exitp->exit_info, EX_ISDOOR))
      send_to_char("Questo e` assurdo.\n\r", ch);
    else if (!IS_SET(exitp->exit_info, EX_CLOSED))
      send_to_char("Prima devi chiuderla.\n\r", ch);
    else if (exitp->key < 0)
      send_to_char("Non sembra esserci nessuna toppa per la chiave.\n\r", ch);
    else if (!has_key(ch, exitp->key))
      send_to_char("Non hai la chiave giusta.\n\r", ch);
    else if (IS_SET(exitp->exit_info, EX_LOCKED))
      send_to_char("E` gia chiusa a chiave!\n\r", ch);
    else 
    {
      if (exitp->keyword && strcmp("secret", fname(exitp->keyword)))
        act("$n chiude a chiave il $F.", 0, ch, 0,  exitp->keyword, TO_ROOM);
      else
        act("$n chiude a chiave la porta.", FALSE, ch, 0, 0, TO_ROOM);
      send_to_char("*Click*\n\r", ch);

      raw_lock_door(ch, exitp, door);
    }
  }
}

void do_unlock(struct char_data *ch, char *argument, int cmd)
{
  int door;
  char type[MAX_INPUT_LENGTH], dir[MAX_INPUT_LENGTH];
  struct room_direction_data *exitp;
  struct obj_data *obj;
  struct char_data *victim;
  
  argument_interpreter(argument, type, dir);
  
  if (!*type)
    send_to_char("Che cosa vuaoi aprire ?\n\r", ch);
  else if (generic_find(argument, FIND_OBJ_INV | FIND_OBJ_ROOM,
                        ch, &victim, &obj)) 
  {
    
    /* this is an object */
    
    if (obj->obj_flags.type_flag != ITEM_CONTAINER)
      send_to_char("Non e` un contenitore.\n\r", ch);
    else if (obj->obj_flags.value[2] < 0)
      send_to_char("Non ti sembra di trovare un buco per la chiave.\n\r", ch);
    else if (!has_key(ch, obj->obj_flags.value[2]))
      send_to_char("Non hai la chiave giusta.\n\r", ch);        
    else if (!IS_SET(obj->obj_flags.value[1], CONT_LOCKED))
      send_to_char("Non e` chiuso a chiave, dopo tutto.\n\r", ch);
    else
    {
      REMOVE_BIT(obj->obj_flags.value[1], CONT_LOCKED);
      send_to_char("*Click*\n\r", ch);
      act("$n apre $p.", FALSE, ch, obj, 0, TO_ROOM);
    }
  }
  else if ((door = find_door(ch, type, dir)) >= 0) 
  {
    
    /* it is a door */
    exitp = EXIT(ch, door);
    
    if (!IS_SET(exitp->exit_info, EX_ISDOOR))
      send_to_char("Questo e` assurdo.\n\r", ch);
    else if (!IS_SET(exitp->exit_info, EX_CLOSED))
      send_to_char("Ma se non e` nemmeno chiusa!\n\r", ch);
    else if (exitp->key < 0)
      send_to_char("Non mi sembra di vedere buchi per la chiave.\n\r", ch);
    else if (!has_key(ch, exitp->key))
      send_to_char("Non hai la chiave giusta.\n\r", ch);
    else if (!IS_SET(exitp->exit_info, EX_LOCKED))
      send_to_char("Non e` chiusa a chiave, mi sembra...\n\r", ch);
    else 
    {
      if (exitp->keyword && strcmp("secret", fname(exitp->keyword)))
        act("$n apre il $F.", 0, ch, 0, exitp->keyword, TO_ROOM);
      else
        act("$n sblocca la porta.", FALSE, ch, 0, 0, TO_ROOM);
      send_to_char("*click*\n\r", ch);

      raw_unlock_door(ch, exitp, door);
    }
  }
}


void do_pick(struct char_data *ch, char *argument, int cmd)
{
  byte percent;
  int door;
  char type[MAX_INPUT_LENGTH], dir[MAX_INPUT_LENGTH];
  struct room_direction_data *exitp;
  struct obj_data *obj;
  struct char_data *victim;
  
  argument_interpreter(argument, type, dir);
  
  percent=number(1,101); /* 101% is a complete failure */

  if (!ch->skills) 
  {
    send_to_char("Non sei riusito a forzare la serratura.\n\r", ch);
    return;
  }

  if (!HasClass(ch, CLASS_THIEF) && !HasClass(ch, CLASS_MONK)) 
  {
    send_to_char("Non sei un ladro !\n\r", ch);
    return;
  }
  
  if (percent > (ch->skills[SKILL_PICK_LOCK].learned)) 
  {
    send_to_char("Non sei riuscito a forzare la serratura.\n\r", ch);
    LearnFromMistake(ch, SKILL_PICK_LOCK, 0, 90);
    WAIT_STATE(ch, PULSE_VIOLENCE*4);
    return;
  }
  
  if (!*type) 
  {
    send_to_char("Che cosa vuoi forzare ?\n\r", ch);
  } 
  else if (generic_find(argument, FIND_OBJ_INV | FIND_OBJ_ROOM,
                          ch, &victim, &obj)) 
  {
    
    /* this is an object */
    
    if (obj->obj_flags.type_flag != ITEM_CONTAINER)
      send_to_char("Non e` un contenitore.\n\r", ch);
    else if (!IS_SET(obj->obj_flags.value[1], CONT_CLOSED))
      send_to_char("Ma se non e` nemmeno chiuso !\n\r", ch);
    else if (obj->obj_flags.value[2] < 0)
      send_to_char("Non sembra esserci una serratura.\n\r", ch);
    else if (!IS_SET(obj->obj_flags.value[1], CONT_LOCKED))
      send_to_char("Ma non e` chiuso a chiave !\n\r", ch);
    else if (IS_SET(obj->obj_flags.value[1], CONT_PICKPROOF))
      send_to_char( "Non mi pare che tu possa forzare QUESTA serratura.\n\r", 
                    ch);
    else
    {
      REMOVE_BIT(obj->obj_flags.value[1], CONT_LOCKED);
      send_to_char("*Click*\n\r", ch);
      act("$n ha ragione di $p.", FALSE, ch, obj, 0, TO_ROOM);
    }
  }
  else if ((door = find_door(ch, type, dir)) >= 0) 
  {
    exitp = EXIT(ch, door);
    if (!IS_SET(exitp->exit_info, EX_ISDOOR))
      send_to_char("Questo e` assurdo.\n\r", ch);
    else if (!IS_SET(exitp->exit_info, EX_CLOSED))
      send_to_char("Hai appena realizzato che la porta e` aperta.\n\r", ch);
    else if (exitp->key < 0)
      send_to_char("Non ci sono serrature da forzare.\n\r", ch);
    else if (!IS_SET(exitp->exit_info, EX_LOCKED))
      send_to_char("Oh.. non e` chiusa a chiave.\n\r", ch);
    else if (IS_SET(exitp->exit_info, EX_PICKPROOF))
      send_to_char( "Non credo che riuscirai a forzare QUESTA serratura.\n\r", 
                    ch);
    else 
    {
      if (exitp->keyword)
        act("$n forza abilmente la serratura del $F.", 0, ch, 0,
            exitp->keyword, TO_ROOM);
      else
        act("$n forza la serratura.",TRUE,ch,0,0,TO_ROOM);
      send_to_char("La serratura cede velocemente alla tua abilita`.\n\r", ch);
      raw_unlock_door(ch, exitp, door);

    }
  }
}

void do_enter(struct char_data *ch, char *argument, int cmd)
{
  int door;
  char buf[MAX_INPUT_LENGTH], tmp[MAX_STRING_LENGTH];
  struct room_direction_data        *exitp;
  struct room_data        *rp;
  
  one_argument(argument, buf);
  
  if (*buf) { /* an argument was supplied, search for door keyword */
    for (door = 0; door <= 5; door++)
      if (exit_ok(exitp=EXIT(ch, door), NULL) && exitp->keyword &&
          0==str_cmp(exitp->keyword, buf)) 
      {
        do_move(ch, "", ++door);
        return;
      }
    sprintf(tmp, "Non vedo %s qui.\n\r", buf);
    send_to_char(tmp, ch);
  } 
  else if (IS_SET(real_roomp(ch->in_room)->room_flags, INDOORS)) 
  {
    send_to_char("Sei gia` all'interno.\n\r", ch);
  } 
  else 
  {
    /* try to locate an entrance */
    for (door = 0; door <= 5; door++)
      if (exit_ok(exitp=EXIT(ch, door), &rp) &&
          !IS_SET(exitp->exit_info, EX_CLOSED) &&
          IS_SET(rp->room_flags, INDOORS))
      {
        do_move(ch, "", ++door);
        return;
      }
    send_to_char("Non c'e` nulla in cui entrare.\n\r", ch);
  }
}


void do_leave(struct char_data *ch, char *argument, int cmd)
{
  int door;
  struct room_direction_data        *exitp;
  struct room_data        *rp;
  
  if (!IS_SET(RM_FLAGS(ch->in_room), INDOORS))
    send_to_char("Sei gia` all'aperto, dove vuoi andare ?\n\r", ch);
  else    
  {
    for (door = 0; door <= 5; door++)
      if (exit_ok(exitp=EXIT(ch, door), &rp) &&
          !IS_SET(exitp->exit_info, EX_CLOSED) &&
          !IS_SET(rp->room_flags, INDOORS)) 
      {
        do_move(ch, "", ++door);
        return;
      }
    send_to_char("Non vedo uscite verso l'esterno.\n\r", ch);
  }
}


void do_stand(struct char_data *ch, char *argument, int cmd)
{
  /* can't stand while memorizing! */
  if (affected_by_spell(ch,SKILL_MEMORIZE)) 
  {
    affect_from_char(ch,SKILL_MEMORIZE);
    SpellWearOff(SKILL_MEMORIZE,ch);
  } /* end if */

        /* can't stand and meditate! */
        
  if (affected_by_spell(ch,SKILL_MEDITATE)) 
  {
    affect_from_char(ch,SKILL_MEDITATE);
    SpellWearOff(SKILL_MEDITATE,ch);
  } /* end if */
      

  switch(GET_POS(ch)) 
  {
    case POSITION_STANDING: 
      act("Sei gia` in piedi.",FALSE, ch,0,0,TO_CHAR);
      break;
    case POSITION_SITTING:  
      act("Ti alzi.", FALSE, ch,0,0,TO_CHAR);
      act("$n si alza in piedi.",TRUE, ch, 0, 0, TO_ROOM);
      GET_POS(ch) = POSITION_STANDING;
      break;
    case POSITION_RESTING: 
      act("Smetti di riposare e ti alzi.", FALSE, ch,0,0,TO_CHAR);
      act("$n smette di riposare e si alza in piedi.", TRUE, ch, 0, 0, TO_ROOM);
      GET_POS(ch) = POSITION_STANDING;
      break;
    case POSITION_SLEEPING: 
      act("Dovresti svegliarti, prima!", FALSE, ch, 0,0,TO_CHAR);
      break;
    case POSITION_FIGHTING: 
      act("Pensavi di poter combattere da sedut$b ?",FALSE, ch, 0, 0, TO_CHAR);
      break;
    case POSITION_MOUNTED:
      send_to_char("Non mentre cavalchi!\n\r", ch);
      break;
  
    default: 
      act("Smetti di fluttare e posi i piedi in terra.",
          FALSE, ch, 0, 0, TO_CHAR);
      act("$n smette di fluttuare e posa i piedi in terra.",
          TRUE, ch, 0, 0, TO_ROOM);
      break;
  }
}


void do_sit(struct char_data *ch, char *argument, int cmd)
{
  switch(GET_POS(ch)) 
  {
    case POSITION_STANDING:
      act("Ti siedi.", FALSE, ch, 0,0, TO_CHAR);
      act("$n si siede.", FALSE, ch, 0,0, TO_ROOM);
      GET_POS(ch) = POSITION_SITTING;
      break;
    case POSITION_SITTING:
      act( "Sei gia` sedut$b.", FALSE, ch, 0, 0, TO_CHAR );
      break;
    case POSITION_RESTING:
      act("Smetti di riposare e ti siedi.", FALSE, ch,0,0,TO_CHAR);
      act("$n smette di riposare.", TRUE, ch, 0,0,TO_ROOM);
      GET_POS(ch) = POSITION_SITTING;
      break;
    case POSITION_SLEEPING:
      act( "Non credi che dovresti svegliarti, prima ?", FALSE, ch, 0, 0, 
           TO_CHAR);
      break;
    case POSITION_FIGHTING:
      act("Sederti mentre combatti ? ma sei matt$b ?", FALSE, ch,0,0,TO_CHAR);
      break;
    case POSITION_MOUNTED:
      send_to_char("Non mentre cavalchi !\n\r", ch);
      break;
    default : 
      act("Smetti di fluttuare e ti siedi.", FALSE, ch,0,0,TO_CHAR);
      act("$n smette di fluttuare e si siede.", TRUE, ch,0,0,TO_ROOM);
      GET_POS(ch) = POSITION_SITTING;
      break;
  }
}


void do_rest(struct char_data *ch, char *argument, int cmd) 
{
  switch(GET_POS(ch)) 
  {
    case POSITION_STANDING: 
      act( "Ti fermi a riposare le stanche membra.", FALSE, ch, 0, 0, 
           TO_CHAR);
      act("$n si ferma a riposare.", TRUE, ch, 0, 0, TO_ROOM);
      GET_POS(ch) = POSITION_RESTING;
      break;
    case POSITION_SITTING:
      act( "Ti fermi a riposare le stanche membra.", FALSE, ch, 0, 0, 
           TO_CHAR);
      act("$n si ferma a riposare.", TRUE, ch, 0, 0, TO_ROOM);
      GET_POS(ch) = POSITION_RESTING;
      break;
    case POSITION_RESTING:
      act("Stai gia` riposando.", FALSE, ch, 0, 0, TO_CHAR);
      break;
    case POSITION_SLEEPING:
      act("Prima dovresti svegliarti.", FALSE, ch, 0, 0, TO_CHAR);
      break;
    case POSITION_FIGHTING:
      act( "Riposarti mentre combatti ? ma sei matt$b ?", FALSE, ch, 0, 0, 
           TO_CHAR);
      break;
    case POSITION_MOUNTED:
      send_to_char("Non mentre monti!\n\r", ch);
      break;
  
    default:
      act("Smetti di fluttuare e ti fermi a riposare le stanche membra.",
          FALSE, ch, 0, 0, TO_CHAR);
      act( "$n smette di fluttuare e si ferma a riposare.", FALSE, ch, 0,0, 
           TO_ROOM);
      GET_POS(ch) = POSITION_SITTING;
      break;
  }
}


void do_sleep(struct char_data *ch, char *argument, int cmd) 
{
  
  switch(GET_POS(ch)) 
  {
    case POSITION_STANDING : 
    case POSITION_SITTING  :
    case POSITION_RESTING  : 
      send_to_char("Ti metti a dormire.\n\r", ch);
      act("$n si sdraia e si addormenta", TRUE, ch, 0, 0, TO_ROOM);
      GET_POS(ch) = POSITION_SLEEPING;
      break;
    case POSITION_SLEEPING:
      send_to_char("Stai gia` dormendo.\n\r", ch);
      break;
    case POSITION_FIGHTING:
      send_to_char("Pensi che dormire risolva i tuoi problemi ?\n\r", ch);
      break;
    case POSITION_MOUNTED:
      send_to_char("Non mentre cavalchi !\n\r", ch);
      break;
  
    default: 
      act( "Tu smetti di fluttuare e ti addormenti.", FALSE, ch, 0, 0, TO_CHAR);
      act("$n smette di fluttuare e si addormenta.", TRUE, ch, 0, 0, TO_ROOM);
      GET_POS(ch) = POSITION_SLEEPING;
      break;
  }
}


void do_wake(struct char_data *ch, char *argument, int cmd)
{
  struct char_data *tmp_char;
  char arg[MAX_STRING_LENGTH];
  
  
  one_argument(argument,arg);
  if (*arg) 
  {
    if (GET_POS(ch) == POSITION_SLEEPING) 
    {
      act("Non puoi svegliare la gente se tu stess$b stai dormendo !",
          FALSE, ch,0,0,TO_CHAR);
    } 
    else 
    {
      tmp_char = get_char_room_vis(ch, arg);
      if (tmp_char) 
      {
        if (tmp_char == ch) 
        {
          act("Se vuoi svegliare te stess$b batti semplicemente 'wake'",
              FALSE, ch,0,0,TO_CHAR);
        } 
        else 
        {
          if (GET_POS(tmp_char) == POSITION_SLEEPING) 
          {
            if (IS_AFFECTED(tmp_char, AFF_SLEEP)) 
            {
              act("Non puoi svegliare $N!", FALSE, ch, 0, tmp_char, TO_CHAR);
            } 
            else 
            {
              act("Hai svegliato $N.", FALSE, ch, 0, tmp_char, TO_CHAR);
              GET_POS(tmp_char) = POSITION_SITTING;
              act( "Sei stat$b svegliat$b da $n.", FALSE, ch, 0, tmp_char, 
                   TO_VICT);
              act( "$n ha svegliato $N.", TRUE, ch, NULL, tmp_char, 
                   TO_NOTVICT );
            }
          } 
          else if( GET_POS(tmp_char) == POSITION_STUNNED )
          {
            act( "$N e` svenut$B.", FALSE,ch,0,tmp_char, TO_CHAR );
          }
          else if( GET_POS(tmp_char) < POSITION_STUNNED )
          {
            act( "$N e` ridott$B proprio male. Non sembra cosciente.", 
                 FALSE,ch,0,tmp_char, TO_CHAR );
          }
          else
          {
            act("$N e` gia` svegli$B.",FALSE,ch,0,tmp_char, TO_CHAR);
          }
        }
      } 
      else 
      {
        send_to_char("Non mi sembra che quella persona sia qui.\n\r", ch);
      }
    }
  } 
  else 
  {
    if (IS_AFFECTED(ch,AFF_SLEEP)) 
    {
      send_to_char("Non riesci a svegliarti !\n\r", ch);
    } 
    else 
    {
      if (GET_POS(ch) > POSITION_SLEEPING)
        act( "Sei gia` svegli$b...", FALSE, ch, 0, 0, TO_CHAR );
      else 
      {
        act( "Ti sei svegliat$b.", FALSE, ch, 0, 0, TO_CHAR );
        act("$n si e` svegliat$b.", TRUE, ch, 0, 0, TO_ROOM);
        GET_POS(ch) = POSITION_SITTING;
      }
    }
  }
}


void do_follow(struct char_data *ch, char *argument, int cmd)
{
  char name[160];
  struct char_data *leader;
  
  
  only_argument(argument, name);
  
  if (*name) 
  {
    if (!(leader = get_char_room_vis(ch, name))) 
    {
      send_to_char("Non vedo nessuno con quel nome qui !\n\r", ch);
      return;
    }
  } 
  else 
  {
    send_to_char("Chi e` che vuoi seguire ?\n\r", ch);
    return;
  }
  
  if (IS_AFFECTED(ch, AFF_CHARM) && (ch->master)) 
  {
    
    act( "Il tuo unico desiderio e` seguire $N!", FALSE, ch, 0, ch->master, 
         TO_CHAR);
    
  } 
  else 
  { /* Not Charmed follow person */
    
    if (leader == ch) 
    {
      if (!ch->master) 
      {
        send_to_char("Non stai seguendo nessuno.\n\r", ch);
        return;
      }
      stop_follower(ch);
    } 
    else 
    {
      if (circle_follow(ch, leader)) 
      {
        act("Mi spiace, ma quello segue quell'altro, quell'altro segue te...", 
            FALSE, ch, 0, 0, TO_CHAR);
        return;
      }
      if (ch->master)
        stop_follower(ch);
      if(IS_AFFECTED(ch, AFF_GROUP))
        REMOVE_BIT(ch->specials.affected_by, AFF_GROUP);      
      add_follower(ch, leader);
    }
  }
}

void do_run(struct char_data *ch, char *argument, int cmd)
{
  char buff[MAX_INPUT_LENGTH];
  int keyno, was_in;
  struct room_direction_data *exitdata;
  static char *keywords[]= 
  { 
    "north",
    "east",
    "south",
    "west",
    "up",
    "down",
    "\n" 
  };

  only_argument(argument, buff);

  if (!*buff) 
  {
    send_to_char("In quale direzione vuoi correre ?.\n\r", ch);
    return;
  }

  keyno = search_block(buff, keywords, FALSE);

  if( keyno == -1 )
  {
    act( "Mi spiace ma quella non e` una direzione in cui correre.", FALSE,
         ch, 0, 0, TO_CHAR );
    return;
  }

  if(GET_MOVE(ch) <= 20) 
  {
    act( "Sei troppo stanc$b per correre in questo momento.", FALSE,
         ch, 0, 0, TO_CHAR );
    return;
  }
  
  if(IS_AFFECTED(ch, AFF_CHARM) && (ch->master) && 
      (ch->in_room == ch->master->in_room)) 
  {
    act("$n scoppia a piangere.", FALSE, ch, 0, 0, TO_ROOM);
    act("Scoppi a piangere al pensare di correre via da $N",
        FALSE, ch, 0, ch->master, TO_CHAR);
    return;
  }

  if(!CAN_GO(ch, keyno)) 
  {
    send_to_char( "Pensi di poter correre attraverso le cose ? "
                  "Scegli una direzione aperta!", ch);
    return;
  }
  if(!clearpath(ch,ch->in_room,keyno)) 
  {
    send_to_char("Correre in quella direzione e` inutile.\n\r", ch);
    return;
  }
  exitdata = (real_roomp(ch->in_room)->dir_option[keyno]);
  if((exitdata->to_room)==(ch->in_room)) 
  {
    send_to_char("Correre in quella direzione e` inutile.\n\r", ch);
    return;
  }
 
  send_to_char("Alzi i tacchi e corri piu` forte che puoi!\n\r", ch);
  act("Improvisamente $n alza i tacchi e corre piu` veloce che puo`!",
      FALSE,ch,0,0,TO_ROOM);
  was_in=ch->in_room;
  while( CAN_GO( ch, keyno ) && GET_MOVE(ch) > 20 && 
         RawMove( ch, keyno, TRUE ) )
  {
    DisplayOneMove(ch, keyno, was_in);
    GET_MOVE(ch) -= 1;
    was_in=ch->in_room;
  }

  if (GET_MOVE(ch) > 25) 
  {
    act( "$n rallenta fino a fermarsi, esaust$b per la corsa.",
         FALSE,ch,0,0,TO_ROOM);
    send_to_char("Mi spiace, ma non riesci a correre ulteriormente.\n\r", ch);
  } 
  else 
  {
    act( "$n rallenta fino a fermarsi, con il fiatone per la corsa.",
         FALSE,ch,0,0,TO_ROOM);
    act( "Sei troppo esaust$b per correre ancora.", FALSE, ch, 0, 0, TO_CHAR );
  }
}

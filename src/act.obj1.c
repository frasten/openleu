
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "protos.h"
#include "status.h"

/* extern variables */

extern struct str_app_type str_app[];
extern struct descriptor_data *descriptor_list;
                 
/* extern functions */

struct obj_data *create_money( int amount );
char getall(char *name, char *newname);
int getabunch(char *name, char  *newname);
int str_cmp2(const char *arg1, const char *arg2);

/* procedures related to get */
void get( struct char_data *ch, struct obj_data *obj_object, 
          struct obj_data *sub_object ) 
{
  char buffer[ 256 ];

  /* check person to item ego */
  if( CheckEgo( ch, obj_object ) && CheckGetBarbarianOK( ch, obj_object ) )
  {
    if( sub_object ) 
    {
      if( !IS_SET( sub_object->obj_flags.value[ 1 ], CONT_CLOSED ) )
      {
        act( "Prendi $p da $P.", FALSE, ch, obj_object, sub_object, TO_CHAR );
        act( "$n prende $p da $P.", TRUE, ch, obj_object, sub_object, TO_ROOM );
        obj_from_obj( obj_object );
        obj_to_char( obj_object, ch );
      }
      else 
      {
        act( "Prima dovresti aprire $P.", TRUE, ch, 0, sub_object, TO_CHAR );
        return;
      }
    }
    else 
    {
      if( obj_object->in_room == NOWHERE )
      {
        obj_object->in_room = ch->in_room;
      }
      act( "Prendi $p.", FALSE, ch, obj_object, 0, TO_CHAR );
      act( "$n prende $p.", TRUE, ch, obj_object, 0, TO_ROOM );
      obj_from_room( obj_object );
      obj_to_char( obj_object, ch );
    }
    if( obj_object->obj_flags.type_flag == ITEM_MONEY && 
        obj_object->obj_flags.value[0] >= 1 )
    {
      obj_from_char( obj_object );
      if( obj_object->obj_flags.value[0] > 1 )
        sprintf( buffer, "C'erano %d monete.\n\r", 
                 obj_object->obj_flags.value[ 0 ] );
      else
        sprintf( buffer, "C'era una miserabile moneta.\n\r" ); 
      send_to_char( buffer, ch );
      GET_GOLD( ch ) += obj_object->obj_flags.value[ 0 ];
      if( GET_GOLD( ch ) > 500000 && obj_object->obj_flags.value[0] > 100000 ) 
      {
        mudlog( LOG_PLAYERS, "%s just got %d coins",
                GET_NAME(ch),obj_object->obj_flags.value[0]);
      }
      extract_obj(obj_object);
    }
  } 
  else 
  {
     /* failed barb or ego item check */
  }
}

void do_get(struct char_data *ch, const char *argument, int cmd)
{
  char arg1[MAX_STRING_LENGTH];
  char arg2[MAX_STRING_LENGTH];
  char buffer[MAX_STRING_LENGTH];
  struct obj_data *sub_object, *obj_object,*next_obj;
  bool found = FALSE;
  bool fail  = FALSE;
  bool has   = FALSE;                /* :P */
  int type   = 3;
  char newarg[1000];
  int num, p;
  
  SetStatus( "Entered in do_get", GET_NAME_DESC( ch ), (void *) argument );
  
  argument_interpreter(argument, arg1, arg2);
  
  /* get type */
  if (!*arg1) 
  {
    type = 0;
  }
  if (*arg1 && !*arg2) 
  {  /* plain "get all" */
    if (!str_cmp(arg1,"all")) 
    {
      type = 1;
    } 
    else 
    {
      type = 2; /* "get all.item" */
    }
  }
  if (*arg1 && *arg2) 
  {
    if (!str_cmp(arg1,"all")) 
    {   /* "get all all" */
      if (!str_cmp(arg2,"all")) 
      {
        type = 3;
      } 
      else 
      {
        type = 4; /* get all object */
      }
    } 
    else 
    {
      if (!str_cmp(arg2,"all")) 
      { /* "get object all" */
        type = 5;
      } 
      else 
      {
        type = 6;
      }
    }
  }
  
  switch (type) 
  {
    /* get */
   case 0:
    send_to_char( "Cosa vuoi prendere?\n\r", ch); 
    break;
    /* get all */
   case 1:
    sub_object = 0;
    found = FALSE;
    fail = FALSE;
    for( obj_object = real_roomp(ch->in_room)->contents; obj_object;
         obj_object = next_obj) 
    {
      next_obj = obj_object->next_content;
      /* check for a trap (traps fire often) */
      if (CheckForAnyTrap(ch, obj_object)) 
      {
        SetStatus( "Ending after CheckForAnyTrap in do_get", NULL );
        return;
      }
        

      if (CAN_SEE_OBJ(ch,obj_object)) 
      {
        if ((IS_CARRYING_N(ch) + 1) <= CAN_CARRY_N(ch)) 
        {
          if( (IS_CARRYING_W(ch) + obj_object->obj_flags.weight) <= 
              CAN_CARRY_W(ch)) 
          {
            if (CAN_WEAR(obj_object,ITEM_TAKE)) 
            {
              get(ch,obj_object,sub_object);
              found = TRUE;
            } 
            else 
            {
              act( "Non puoi prendere $p.", FALSE, ch, obj_object, NULL,
                   TO_CHAR );
              fail = TRUE;
            }
          } 
          else 
          {
            act( "Non puoi prendere $p, pesa troppo.", FALSE, ch, obj_object,
                 NULL, TO_CHAR );
            fail = TRUE;
          }
        } 
        else 
        {
          act( "Non puoi prendere $p, hai gia` troppa roba in mano.", FALSE,
               ch, obj_object, NULL, TO_CHAR );
          fail = TRUE;
        }
      }
    }
    if( !found && !fail )
    {
      send_to_char("Non c'e nulla da prendere.\n\r", ch );
    } 
    break;
    /* get ??? (something) */
   case 2:
    {
      struct obj_data *pObjList;
        
      sub_object = NULL;
      found = FALSE;
      fail = FALSE;
      if( getall( arg1, newarg ) == TRUE )
      {
        strcpy( arg1, newarg );
        num = -1;
      } 
      else if( ( p = getabunch( arg1, newarg ) ) != 0 ) 
      {
        strcpy( arg1, newarg );
        num = p;
      }
      else 
      {
        num = 1;
      }
      
      pObjList = real_roomp( ch->in_room )->contents;

      while( num != 0 )
      {
        obj_object = get_obj_in_list_vis( ch, arg1, pObjList );
      
        if( obj_object )
        {
          pObjList = obj_object->next_content;
          if( IS_CORPSE( obj_object ) && num != 1 )
          {
            send_to_char( "Puoi prendere un solo corpo alla volta.\n\r", ch );
            SetStatus( "Ending after IS_CORPSE in do_get", NULL );
            return; /* no need for num and fail above I guess */
          }
        
          /* check for trap */
          if( CheckForGetTrap( ch, obj_object ) )
          {
            SetStatus( "Ending after CheckForGetTrap in do_get", NULL );
            return;
          }
        
          if( ( IS_CARRYING_N( ch ) + 1 < CAN_CARRY_N( ch ) ) ) 
          {
            if( ( IS_CARRYING_W( ch ) + obj_object->obj_flags.weight ) <
                CAN_CARRY_W( ch ) )
            {
              if( CAN_WEAR( obj_object, ITEM_TAKE ) )
              {
                get( ch, obj_object, sub_object );
                found = TRUE;
              }
              else 
              {
                act( "Non puoi prendere $p.", FALSE, ch, obj_object, NULL, 
                     TO_CHAR );
                fail = TRUE;
              }
            } 
            else 
            {
              act( "Non riesci a prendere $p, pesa troppo.", FALSE, ch, 
                   obj_object, NULL, TO_CHAR );
              fail = TRUE;
            }
          } 
          else 
          {
            act( "Non riesci a prendere $p, hai troppa roba in mano", FALSE,
                 ch, obj_object, NULL, TO_CHAR );
            fail = TRUE;
            break;
          }
          if( num > 0 ) 
            num--;
        }
        else
          break;
      }
      
      if( !found && !fail )
      {
        sprintf( buffer, "Non c'e` nessun %s qui.\n\r", arg1 );
        send_to_char(buffer, ch);
      }
    }
    
    break;
    /* get all all */

   case  3:
    send_to_char( "Non puoi prendere tutto da tutto.\n\r", ch);
    break;
    /* get all ??? */
   case 4:
    found = FALSE;
    fail  = FALSE; 
    has   = FALSE;
    sub_object = get_obj_vis_accessible( ch, arg2 );
    if( sub_object )
    {
      if( GET_ITEM_TYPE( sub_object ) == ITEM_CONTAINER )
      {
        if( get_obj_in_list_vis( ch, arg2, ch->carrying ) )
        {
          has = TRUE;
        }
        for( obj_object = sub_object->contains;
             obj_object;
             obj_object = next_obj) 
        {
          next_obj = obj_object->next_content;
          
          /* check for trap */
          if( CheckForGetTrap( ch, obj_object ) )
          {
            SetStatus( "Ending after CheckForGetTrap 2 in do_get", NULL );
            return;
          }
            
          if( CAN_SEE_OBJ( ch, obj_object ) ) 
          {
            if( ( IS_CARRYING_N( ch ) + 1 ) < CAN_CARRY_N( ch ) ) 
            {
              if( has || 
                  ( IS_CARRYING_W( ch ) + obj_object->obj_flags.weight ) <
                  CAN_CARRY_W( ch ) ) 
              {
                if( CAN_WEAR( obj_object,ITEM_TAKE ) ) 
                {
                  get( ch, obj_object, sub_object );
                  found = TRUE;
                } 
                else 
                {
                  act( "Non puoi prendere $p.", FALSE, ch, obj_object, NULL,
                       TO_CHAR );
                  fail = TRUE;
                }
              } 
              else 
              {
                act( "Non puoi prendere $p, pesa troppo.", FALSE, ch, 
                     obj_object, NULL, TO_CHAR );
                fail = TRUE;
              }
            } 
            else 
            {
              act( "Non puoi prendere $p, hai troppa roba in mano", FALSE,
                   ch, obj_object, NULL, TO_CHAR );
              fail = TRUE;
              break;              
            }
          }
        }
        if( !found && !fail )
        {
          act( "Non c'e niente in $p.", FALSE, ch, sub_object, NULL, 
               TO_CHAR );
          fail = TRUE;
        }
      } 
      else 
      {
        act( "$p non e` un contenitore.", FALSE, ch, sub_object, NULL,
             TO_CHAR );
        fail = TRUE;
      }
    } 
    else 
    {
      sprintf( buffer,"Non vedi nessun %s.\n\r", arg2);
      send_to_char( buffer, ch );
      fail = TRUE;
    }
    break;
   case 5:
    act( "Puoi prendere gli oggetti da un solo contenitore alla volta.", FALSE,
         ch, 0, 0, TO_CHAR );
    break;
    
    /*  take ??? from ???   (is it??) */
   case 6:
    found = FALSE;
    fail  = FALSE;
    has   = FALSE;
    sub_object = (struct obj_data *) get_obj_vis_accessible( ch, arg2 );
    if( sub_object )
    {
      if( GET_ITEM_TYPE( sub_object ) == ITEM_CONTAINER )
      {
        struct obj_data *pObjList;
        
        if( get_obj_in_list_vis( ch, arg2, ch->carrying ) )
          has=TRUE;
        if( getall( arg1, newarg ) == TRUE )
        {
          num = -1;
          strcpy( arg1, newarg );
        } 
        else if( ( p = getabunch( arg1, newarg ) ) != 0 ) 
        {
          num = p;                     
          strcpy( arg1, newarg );
        } 
        else 
        {
          num = 1;
        }
        
        pObjList = sub_object->contains;
        
        while( num != 0 )
        {
          obj_object = get_obj_in_list_vis( ch, arg1, pObjList );

          if( obj_object )
          {
            pObjList = obj_object->next_content;
            if( CheckForInsideTrap( ch, sub_object ) )
            {
              SetStatus( "Ending after CheckForInsideTrap in do_get", NULL );
              return;
            }
            
            if( ( IS_CARRYING_N( ch ) + 1 < CAN_CARRY_N( ch ) ) )
            {
              if( has || 
                  ( IS_CARRYING_W( ch ) + obj_object->obj_flags.weight ) < 
                    CAN_CARRY_W( ch ) )
              {
                if( CAN_WEAR( obj_object, ITEM_TAKE ) )
                {
                  get( ch, obj_object, sub_object );
                  found = TRUE;
                } 
                else 
                {
                  act( "Non puoi prendere $p.", TRUE, ch, obj_object, 0, 
                       TO_CHAR );
                  fail = TRUE;
                }
              } 
              else 
              {
                act( "Non puoi prendere $p, pesa troppo.", TRUE,
                     ch, obj_object, 0, TO_CHAR );
                fail = TRUE;
              }
            } 
            else 
            {
              act( "Non puoi prendere $p, hai gia` roba in mano.", TRUE, ch, 
                   obj_object, NULL, TO_CHAR );
              fail = TRUE;
              break;
            }
            
            if( num > 0 )
              num--;
          }
          else
            break;
        }
        
        if( !fail && !found ) 
        {
          sprintf( buffer, "$p non contiene nessun %s.", arg1 );
          act( buffer, TRUE, ch, sub_object, 0, TO_CHAR );
        }
      } 
      else 
      {
        act( "$p non e` un contenitore.", TRUE, ch, sub_object, 0, TO_CHAR );
        fail = TRUE;
      }
    } 
    else 
    {
      sprintf( buffer, "Non vedi nessun %s.", arg2 );
      act( buffer, TRUE, ch, 0, 0, TO_CHAR );
      fail = TRUE;
    }
    break;
  }
  SetStatus( "Before saving PC in do_get", NULL );
#if NODUPLICATES
  if( found && IS_PC( ch ) )
    do_save(ch, "", 0);
#endif
  SetStatus( "Returning from do_get", NULL );
}



void do_drop(struct char_data *ch, const char *argument, int cmd) 
{
  char arg[MAX_INPUT_LENGTH];
  char arg2[ MAX_INPUT_LENGTH ];
  int amount;
  char buffer[MAX_STRING_LENGTH];
  struct obj_data *tmp_object;
  struct obj_data *next_obj;
  bool test = FALSE;
  char newarg[1000];
  int num, p;
  
#if 0
  char *s;
  s = one_argument( argument, arg );
  if( is_number( arg ) )
  {
    amount = atoi( arg );
    strcpy( arg, s );
    
    if( 0 != str_cmp( "coins", arg ) && 0 != str_cmp( "coin", arg ) )
    {
      send_to_char( "Sorry, you can't do that (yet)...\n\r",ch);
      return;
    }
    
    if( amount < 0 )
    {
      send_to_char( "Sorry, you can't do that!\n\r", ch );
      return;
    }
    if( GET_GOLD( ch ) < amount )
    {
      send_to_char( "You haven't got that many coins!\n\r", ch );
      return;
    }
    send_to_char("OK.\n\r",ch);
    if(amount==0)
      return;
    
    act("$n drops some gold.", FALSE, ch, 0, 0, TO_ROOM);
    tmp_object = create_money(amount);
    obj_to_room(tmp_object,ch->in_room);
    GET_GOLD(ch)-=amount;
    return;
  } 
  else 
  {
    only_argument(argument, arg);
  }
#endif
  
  argument_interpreter(argument, arg, arg2);
  
  if( *arg && *arg2 )
  {
    if( is_number( arg ) && ( str_cmp2( "coin", arg2 ) == 0 ||
                              str_cmp2( "monet", arg2 ) == 0 ) )
    {
      if( ( amount = atoi( arg ) ) > 0 )
      {
        if( GET_GOLD( ch ) < amount )
        {
          send_to_char( "Non hai tutte quelle monete!\n\r", ch );
        }
        else
        {
          
          tmp_object = create_money( amount );
          obj_to_room( tmp_object, ch->in_room );
          GET_GOLD( ch ) -= amount;
          if( amount == 1 )
          {
            send_to_char( "Posi una moneta.\n\r", ch );
            act( "$n posa una moneta.", TRUE, ch, 0, 0, TO_ROOM );
          }
          else
          {
            send_to_char( "Posi alcune monete.\n\r", ch );
            act( "$n posa alcune monete.", TRUE, ch, 0, 0, TO_ROOM );
          }
        }
      }
      else
      {
        send_to_char( "Non hai ben chiaro il valore dell'oro.\n\r", ch );
      }
    }
    else
    {
      send_to_char( "Non puoi farlo.\n\r", ch );
    }
    
    return;
  }
  else if( *arg )
  {
    if( !str_cmp( arg, "all" ) ) 
    {
      for(tmp_object = ch->carrying;
          tmp_object;
          tmp_object = next_obj)
      {
        next_obj = tmp_object->next_content;
        if( !IS_OBJ_STAT( tmp_object, ITEM_NODROP ) || IS_IMMORTAL( ch ) )
        {
          obj_from_char(tmp_object);
          obj_to_room(tmp_object,ch->in_room);
          check_falling_obj(tmp_object, ch->in_room);
          test = TRUE;
        }
        else
        {
          if( CAN_SEE_OBJ( ch, tmp_object ) )
          {
            act( "Non puoi posare $p", FALSE, ch, tmp_object, NULL, 
                 TO_CHAR );
            test = TRUE;
          }
        }
      }
      if( !test )
      {
        send_to_char( "Non hai nulla da posare.\n\r", ch);
      }
      else 
      {
        act( "Posi tutto quello che hai.", FALSE, ch, 0, 0, TO_CHAR );
        act( "$n posa tutto quello che ha.", TRUE, ch, 0, 0, TO_ROOM );
      }
#if NODUPLICATES
      do_save(ch, "", 0);
#endif
    }
    else
    {
      struct obj_data *pObjList;
      int bFound = FALSE;
      /* &&&&&& */
      if( getall( arg, newarg ) == TRUE )
      {
        num = -1;
        strcpy(arg,newarg);
      } 
      else if( ( p = getabunch( arg,newarg ) ) != 0 )
      {
        num = p;                     
        strcpy(arg,newarg);
      }
      else 
      {
        num = 1;  
      }
      
      pObjList = ch->carrying;
      
      while( num != 0 )
      {
        tmp_object = get_obj_in_list_vis( ch, arg, pObjList );
          
        if( tmp_object )
        {
          pObjList = tmp_object->next_content;
          if( !IS_OBJ_STAT( tmp_object, ITEM_NODROP ) || IS_IMMORTAL( ch ) )
          {
            act( "Posi $p.", FALSE, ch, tmp_object, NULL, TO_CHAR );
            act("$n posa $p.", TRUE, ch, tmp_object, 0, TO_ROOM );
            obj_from_char( tmp_object );
            obj_to_room( tmp_object,ch->in_room );

            check_falling_obj( tmp_object, ch->in_room );

          }
          else
          {
            act( "Non puoi posare $p.", FALSE, ch, tmp_object, NULL, TO_CHAR );
          }
          if (num > 0) 
            num--;
          bFound = TRUE;
        }
        else
          break;
      }
      if( !bFound )
      {        
        sprintf( buffer,"Non hai nessun %s.\n\r", arg );
        send_to_char( buffer, ch );
      }
#if NODUPLICATES
      else
      {        
        do_save(ch, "", 0);
      }
#endif
    }
  }
  else
  {
    send_to_char( "Cosa vuoi posare?\n\r", ch );
  }
}



void do_put(struct char_data *ch, const char *argument, int cmd)
{
  char buffer[256];
  char arg1[128];
  char arg2[128];
  struct obj_data *obj_object;
  struct obj_data *sub_object;
  struct char_data *tmp_char;
  int bits;
  char newarg[100];
  int num, p;
  
  argument_interpreter(argument, arg1, arg2);
  
  if (*arg1) 
  {
    if (*arg2) 
    {      
      if (getall(arg1,newarg)==TRUE) 
      {
        num = -1;
        strcpy(arg1,newarg);
      } 
      else if ((p = getabunch(arg1,newarg))!=0) 
      {
        num = p;                     
        strcpy(arg1,newarg);
      } 
      else 
      {
        num = 1;  
      }
      
      if (!strcmp(arg1,"all")) 
      {
        
        send_to_char( "Non puoi farlo.\n\r",ch);
        return;
        
      } 
      else 
      {
        while (num != 0) 
        {
#if 1
          bits = generic_find(arg1, FIND_OBJ_INV,
                              ch, &tmp_char, &obj_object);
#else
          obj_object = get_obj_in_list_vis(ch, arg1, ch->carrying);
#endif
          
          if( obj_object )
          {
            if( IS_OBJ_STAT( obj_object, ITEM_NODROP ) && !IS_IMMORTAL( ch ) )
            {
              act( "Non puoi mettere $p da nessuna parte.", FALSE, ch, 
                   obj_object, NULL, TO_CHAR );
              return;
            }
            bits = generic_find( arg2, FIND_OBJ_INV | FIND_OBJ_ROOM,
                                 ch, &tmp_char, &sub_object );
            if( sub_object )
            {
              if( GET_ITEM_TYPE( sub_object ) == ITEM_CONTAINER )
              {
                if( !IS_SET( sub_object->obj_flags.value[ 1 ], CONT_CLOSED ) )
                {
                  if( obj_object == sub_object )
                  {
                    act( "Giocherelli un po` con $p ma senza risultato.", 
                         FALSE, ch, obj_object, NULL, TO_CHAR );
                      
                    return;
                  }
                  if (((sub_object->obj_flags.weight) + 
                       (obj_object->obj_flags.weight)) <
                      (sub_object->obj_flags.value[0])) 
                  {
                    act( "Metti $p in $P",TRUE, ch, obj_object, sub_object, 
                         TO_CHAR);
                    if (bits==FIND_OBJ_INV) 
                    {
                      obj_from_char(obj_object);
                      obj_to_obj(obj_object, sub_object);
                    } 
                    else 
                    {
                      obj_from_room(obj_object);
                      obj_to_obj(obj_object, sub_object);
                    }
                    
                    act( "$n mette $p in $P",TRUE, ch, obj_object, sub_object, 
                         TO_ROOM);
                    num--;
                  } 
                  else 
                  {
                    if (singular(sub_object))
                      send_to_char( "Non c'entra.\n\r", ch );
                    else 
                      send_to_char( "Non c'entrano.\n\r", ch );
                    num = 0;
                  }
                } 
                else 
                {
                  if (singular(obj_object))
                    send_to_char("Sembra chiuso.\n\r", ch);
                  else 
                    send_to_char("Sembrano chiusi.\n\r", ch);
                  num = 0;
                }
              } 
              else 
              {
                act( "$p non e` un contenitore.", FALSE, ch, sub_object, NULL,
                     TO_CHAR );
                num = 0;
              }
            } 
            else 
            {
              sprintf( buffer, "Non hai nessun %s.\n\r", arg2 );
              send_to_char(buffer, ch);
              num = 0;
            }
          } 
          else 
          {
            if( num > 0 || num == -1 ) 
            {
              sprintf(buffer, "Non hai nessun %s.\n\r", arg1);
              send_to_char(buffer, ch);
            }
            num = 0;
          }
        }
#if   NODUPLICATES
        do_save(ch, "", 0);
#endif
      }
    } 
    else 
    {
      sprintf(buffer, "Dove vorresti mettere un %s?\n\r", arg1);
      send_to_char(buffer, ch);
    }
  } 
  else 
  {
    send_to_char("Cosa vuoi mettere e dove?\n\r",ch);
  }
}

int newstrlen(char *p) 
{
  int i;

  for(i=0;i<10&&*p;i++,p++);
  return(i);
}



void do_give(struct char_data *ch, const char *argument, int cmd)
{
  char obj_name[200], vict_name[80], buf[132];
  char arg[80], newarg[100];
  int amount, num, p, count;
  struct char_data *vict;
  struct obj_data *obj;
  
  argument = one_argument( argument, obj_name );
  if( is_number( obj_name ) )
  {
    if( newstrlen( obj_name ) >= 10 )
      obj_name[ 10 ] = '\0';
    amount = atoi( obj_name );
    argument = one_argument( argument, arg );
    if( str_cmp2( "coin", arg ) && str_cmp2( "monet", arg ) )
    {
      send_to_char( "Eh?\n\r",ch);
      return;
    }
    if( amount <= 0 )
    {
      send_to_char( "Non hai ben chiaro il valore dell'oro.\n\r", ch );
      return;
    }
    if( GET_GOLD( ch ) < amount &&
        ( IS_NPC( ch ) || GetMaxLevel( ch ) < DEMIGOD ) )
    { 
      send_to_char( "Non hai tutti quei soldi.\n\r", ch );
      return;
    }

    argument = one_argument( argument, vict_name );

    if( !*vict_name )
    {
      send_to_char( "A chi vuoi dare delle monete?\n\r", ch );
    }
    else if( !( vict = get_char_room_vis( ch, vict_name ) ) )
    {
      send_to_char( "Non vedi nessuno con quel nome.\n\r", ch );
    }
    else
    {
      if( amount == 1 )
      {    
        act( "$n ti da` una moneta d'oro.", FALSE, ch, NULL, vict, TO_VICT );
        act( "Dai una moneta $N.", FALSE, ch, NULL, vict, TO_CHAR );
      }
      else
      {
        sprintf( buf, "$n ti da` %d monete d'oro.", amount );
        act( buf, FALSE, ch, NULL, vict, TO_VICT );
        sprintf( buf, "Dai %d monete d'oro a $N.", amount );
        act( buf, FALSE, ch, NULL, vict, TO_CHAR );
      }
        
      act( "$n da` alcune monete a $N.", TRUE, ch, 0, vict, TO_NOTVICT );
      if( IS_NPC( ch ) || GetMaxLevel( ch ) < DEMIGOD )
        GET_GOLD( ch ) -= amount;
      GET_GOLD( vict ) += amount;
      save_char( ch, AUTO_RENT );
      if( GET_GOLD(vict) > 500000 && amount > 100000 )
      {
        mudlog( LOG_PLAYERS, "%s gave %d coins to %s", GET_NAME( ch ), 
                amount, GET_NAME( vict ) );
      }
    }
    
    return;
  }
  else
  {
    argument=one_argument(argument, vict_name);
    
    
    if( !*obj_name || !*vict_name )
    {
      send_to_char( "Vuoi dare cosa a chi?\n\r", ch);
      return;
    }
    /* &&&& */
    if( getall( obj_name, newarg ) == TRUE )
    {
      num = -1;
      strcpy( obj_name, newarg );
    }
    else if( ( p = getabunch( obj_name, newarg ) ) != 0 )
    {
      num = p;
      strcpy( obj_name, newarg );
    }
    else
    {
      num = 1;  
    }

    count = 0;
    if( ( vict = get_char_room_vis( ch, vict_name ) ) )
    {
      while( num != 0 )
      {
        if( !( obj = get_obj_in_list_vis( ch, obj_name, ch->carrying ) ) )
        {
          if( count == 0 )
            send_to_char( "Non sembra che tu abbia nulla del genere.\n\r", 
                          ch );
          return;
        }
        if( IS_OBJ_STAT(obj, ITEM_NODROP ) && !IS_IMMORTAL( ch ) )
        {
          if (singular(obj))
            send_to_char( "Non puoi lasciarlo andare.\n\r", ch );
          else
            send_to_char( "Non puoi lasciarli andare.\n\r", ch );

          return;
        }
        else
        {
          if( !CheckGiveBarbarianOK( ch, vict, obj ) || 
              !CheckEgoGive( ch, vict, obj ) )
          {
            return;
          }               
          if (vict == ch) 
          {
            send_to_char("Ok.\n\r", ch);
            return;
          }
      
          if ((1+IS_CARRYING_N(vict)) > CAN_CARRY_N(vict))
          {
            act( "$N sembra avere le mani piene.", 0, ch, 0, vict, TO_CHAR );
            return;
          }
          if( obj->obj_flags.weight + IS_CARRYING_W( vict ) > 
              CAN_CARRY_W( vict ) )
          {
            act( "$N non riesce a tenere $p, pesa troppo.", 0, ch, obj, vict, 
                 TO_CHAR);
            return;
          }
          act("$n da` $p a $N.", 1, ch, obj, vict, TO_NOTVICT);
          act("$n ti da` $p.", 0, ch, obj, vict, TO_VICT);
          act("Dai $p a $N", 0, ch, obj, vict, TO_CHAR);
          obj_from_char(obj);
          obj_to_char(obj, vict);
      
          if (num > 0) 
            num--;
          count++;
        }
      }
#if   NODUPLICATES
      do_save(ch, "", 0);
      do_save(vict, "", 0);
#endif
    }
    else
    {
      send_to_char( "Non c'e` nessuno con quel nome qui.\n\r", ch );
    }
  } 
}



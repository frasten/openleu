/*
  DaleMUD v2.0  Released 2/1994
  See license.doc for distribution terms.   DaleMUD is based on DIKUMUD
*/

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <ctype.h>
#include <stdlib.h>

#include "protos.h"

/* extern variables */

extern struct str_app_type str_app[];
extern struct descriptor_data *descriptor_list;
extern char *drinks[];
extern int drink_aff[][3];
extern struct spell_info_type spell_info[];


void weight_change_object(struct obj_data *obj, int weight)
{
  struct obj_data *tmp_obj;
  struct char_data *tmp_ch;

  if( GET_OBJ_WEIGHT(obj) + weight < 1 )
  {
    weight = 0 - (GET_OBJ_WEIGHT(obj) -1);
    mudlog( LOG_ERROR, "Bad weight change on %s, carried by %s.",
            obj->name, 
            obj->carried_by ? GET_NAME_DESC( obj->carried_by ) : "none" );
  }
  
  if (obj->in_room != NOWHERE) 
  {
    GET_OBJ_WEIGHT(obj) += weight;
  } 
  else if( ( tmp_ch = obj->carried_by ) ) 
  {
    obj_from_char(obj);
    GET_OBJ_WEIGHT(obj) += weight;
    obj_to_char(obj, tmp_ch);
  } 
	else if( (tmp_obj = obj->in_obj) ) 
	{
    obj_from_obj(obj);
    GET_OBJ_WEIGHT(obj) += weight;
    obj_to_obj(obj, tmp_obj);
  } 
	else 
	{
    mudlog( LOG_ERROR, "Unknown attempt to subtract weight from an object." );
  }
}



void name_from_drinkcon(struct obj_data *obj)
{
  int i;
  char *new_name;
  
  for(i=0; (*((obj->name)+i)!=' ') && (*((obj->name)+i)!='\0'); i++)  ;
  
  if (*((obj->name)+i)==' ') {
    new_name=strdup((obj->name)+i+1);
    free(obj->name);
    obj->name=new_name;
  }
}



void name_to_drinkcon(struct obj_data *obj,int type)
{
  char *new_name;
  extern char *drinknames[];
  
  CREATE(new_name,char,strlen(obj->name)+strlen(drinknames[type])+2);
  sprintf(new_name,"%s %s",drinknames[type],obj->name);
  free(obj->name);
  obj->name=new_name;
}



void do_drink(struct char_data *ch, const char *argument, int cmd)
{
  char buf[255];
  struct obj_data *temp;
  struct affected_type af;
  int amount;
  
  
  only_argument(argument,buf);
  
  if(!(temp = get_obj_in_list_vis(ch,buf,ch->carrying)))
  {
    act("You can't find it!",FALSE,ch,0,0,TO_CHAR);
    return;
  }
  
  if (temp->obj_flags.type_flag!=ITEM_DRINKCON)
  {
    act("You can't drink from that!",FALSE,ch,0,0,TO_CHAR);
    return;
  }

  if((GET_COND(ch,DRUNK)>15)&&(GET_COND(ch,THIRST)>0))
  {
    /* The pig is drunk */
    act("You're just sloshed.", FALSE, ch, 0, 0, TO_CHAR);
    act("$n is looks really drunk.", TRUE, ch, 0, 0, TO_ROOM);
    return;
  }
  
  if((GET_COND(ch,FULL)>20)&&(GET_COND(ch,THIRST)>0)) /* Stomach full */
  {
    act("Your stomach can't contain anymore!",FALSE,ch,0,0,TO_CHAR);
    return;
  }
  
  if( temp->obj_flags.type_flag == ITEM_DRINKCON )
  {
    if (temp->obj_flags.value[1]>0)  /* Not empty */
    {
      sprintf(buf,"$n drinks %s from $p",drinks[temp->obj_flags.value[2]]);
      act(buf, TRUE, ch, temp, 0, TO_ROOM);
      sprintf(buf,"You drink the %s.\n\r",drinks[temp->obj_flags.value[2]]);
      send_to_char(buf,ch);
      
      if (drink_aff[temp->obj_flags.value[2]][DRUNK] > 0 )
        amount = (25-GET_COND(ch,THIRST))/
          drink_aff[temp->obj_flags.value[2]][DRUNK];
      else
        amount = number(3,10);
      
      amount = MIN(amount,temp->obj_flags.value[1]);

      gain_condition(ch,DRUNK,(int)((int)drink_aff
                 [temp->obj_flags.value[2]][DRUNK]*amount)/4);
      
      gain_condition(ch,FULL,(int)((int)drink_aff
                           [temp->obj_flags.value[2]][FULL]*amount)/4);
      
      gain_condition(ch,THIRST,(int)((int)drink_aff
                     [temp->obj_flags.value[2]][THIRST]*amount)/4);
      
      if(GET_COND(ch,DRUNK)>10)
        act("You feel drunk.",FALSE,ch,0,0,TO_CHAR);
      
      if(GET_COND(ch,THIRST)>20)
        act("You do not feel thirsty.",FALSE,ch,0,0,TO_CHAR);
      
      if(GET_COND(ch,FULL)>20)
        act("You are full.",FALSE,ch,0,0,TO_CHAR);

      /* The shit was poisoned ! */      
      if(IS_SET(temp->obj_flags.value[3],DRINK_POISON))
      {
        act("Oops, it tasted rather strange ?!!?",FALSE,ch,0,0,TO_CHAR);
        act("$n chokes and utters some strange sounds.",
            TRUE,ch,0,0,TO_ROOM);
        af.type = SPELL_POISON;
        af.duration = amount*3;
        af.modifier = 0;
        af.location = APPLY_NONE;
        af.bitvector = AFF_POISON;
        affect_join(ch,&af, FALSE, FALSE);
      }
      
      /* Subtract amount, if not a never-emptying container */      
      if(!IS_SET(temp->obj_flags.value[3],DRINK_PERM))
      {
        weight_change_object(temp, - amount);
        temp->obj_flags.value[1] -= amount;
      }
      
      if(!temp->obj_flags.value[1])
      { 
        /* The last bit */
        /* empty the container, and no longer poison. */
        temp->obj_flags.value[2]=0;
        temp->obj_flags.value[3]=0;
        name_from_drinkcon(temp);

        if( temp->obj_flags.value[0] < 20 )
        {
          extract_obj(temp);  /* get rid of it */
        }
      }
    }
    else
      act( "Non c'e` nulla in $p.", FALSE, ch, temp, 0, TO_CHAR );
  }
}



void do_eat(struct char_data *ch, const char *argument, int cmd)
{
        char buf[100];
        int j, num;
        struct obj_data *temp;
        struct affected_type af;

        one_argument(argument,buf);

        if(!(temp = get_obj_in_list_vis(ch,buf,ch->carrying)))  {
                act("You can't find it!",FALSE,ch,0,0,TO_CHAR);
                return;
        }

        if((temp->obj_flags.type_flag != ITEM_FOOD) && 
           (GetMaxLevel(ch) < DEMIGOD)) {
            act("Your stomach refuses to eat that!?!",FALSE,ch,0,0,TO_CHAR);
                return;
        }

        if(GET_COND(ch,FULL)>20) /* Stomach full */     {       
                act("You are to full to eat more!",FALSE,ch,0,0,TO_CHAR);
                return;
        }

        act("$n eats $p",TRUE,ch,temp,0,TO_ROOM);
        act("You eat the $o.",FALSE,ch,temp,0,TO_CHAR);

        gain_condition(ch,FULL,temp->obj_flags.value[0]);

        if(GET_COND(ch,FULL)>20)
                act("You are full.",FALSE,ch,0,0,TO_CHAR);

        for(j=0; j<MAX_OBJ_AFFECT; j++)
            if (temp->affected[j].location == APPLY_EAT_SPELL) {
                   num = temp->affected[j].modifier;

/* hit 'em with the spell */

                   ((*spell_info[num].spell_pointer)
                         (6, ch, "", SPELL_TYPE_POTION, ch, 0));
                 }

        if(temp->obj_flags.value[3] && (GetMaxLevel(ch) < LOW_IMMORTAL)) {
           act("That tasted rather strange !!",FALSE,ch,0,0,TO_CHAR);
           act("$n coughs and utters some strange sounds.",
               FALSE,ch,0,0,TO_ROOM);

                af.type = SPELL_POISON;
                af.duration = temp->obj_flags.value[0]*2;
                af.modifier = 0;
                af.location = APPLY_NONE;
                af.bitvector = AFF_POISON;
                affect_join(ch,&af, FALSE, FALSE);
        }

        extract_obj(temp);
}


void do_pour(struct char_data *ch, const char *argument, int cmd)
{
  char arg1[132];
  char arg2[132];
  char buf[256];
  struct obj_data *from_obj;
  struct obj_data *to_obj;
  int temp;
  
  argument_interpreter(argument, arg1, arg2);
  
  if(!*arg1) /* No arguments */
  {
    act("What do you want to pour from?",FALSE,ch,0,0,TO_CHAR);
    return;
  }
  
  if(!(from_obj = get_obj_in_list_vis(ch,arg1,ch->carrying)))
  {
    act("You can't find it!",FALSE,ch,0,0,TO_CHAR);
    return;
  }
  
  if(from_obj->obj_flags.type_flag!=ITEM_DRINKCON)
  {
    act("You can't pour from that!",FALSE,ch,0,0,TO_CHAR);
    return;
  }
  
  if(from_obj->obj_flags.value[1]==0)
  {
    act("$p is empty.",FALSE, ch,from_obj, 0,TO_CHAR);
    return;
  }
  
  if(!*arg2)
  {
    act("Where do you want it? Out or in what?",FALSE,ch,0,0,TO_CHAR);
    return;
  }
  
  if(!str_cmp(arg2,"out"))
  {
    act("$n svuota $p", TRUE, ch,from_obj,0,TO_ROOM);
    act("Hai svuotato $p.", FALSE, ch,from_obj,0,TO_CHAR);
    
    weight_change_object(from_obj, -from_obj->obj_flags.value[1]);
    
    from_obj->obj_flags.value[1]=0;
    from_obj->obj_flags.value[2]=0;
    from_obj->obj_flags.value[3]=0;
    name_from_drinkcon(from_obj);
    
    return;
    
  }
  
  if(!(to_obj = get_obj_in_list_vis(ch,arg2,ch->carrying)))
  {
    act("You can't find it!",FALSE,ch,0,0,TO_CHAR);
    return;
  }
  
  if(to_obj->obj_flags.type_flag!=ITEM_DRINKCON)
  {
    act("You can't pour anything into that.",FALSE,ch,0,0,TO_CHAR);
    return;
  }
  
  if((to_obj->obj_flags.value[1]!=0)&&
     (to_obj->obj_flags.value[2]!=from_obj->obj_flags.value[2]))
  {
    act("There is already another liquid in it!",FALSE,ch,0,0,TO_CHAR);
    return;
  }
  
  if(!(to_obj->obj_flags.value[1]<to_obj->obj_flags.value[0]))
  {
    act("There is no room for more.",FALSE,ch,0,0,TO_CHAR);
    return;
  }
  
  sprintf(buf,"You pour the %s into the %s.",
          drinks[from_obj->obj_flags.value[2]],arg2);
  send_to_char(buf,ch);
  
  /* New alias */
  if (to_obj->obj_flags.value[1]==0) 
    name_to_drinkcon(to_obj,from_obj->obj_flags.value[2]);
  
  /* First same type liq. */
  to_obj->obj_flags.value[2]=from_obj->obj_flags.value[2];
    
  /*
   * Il nuovo, migliore modo di farlo... :)
   */
  temp = MIN( from_obj->obj_flags.value[1], to_obj->obj_flags.value[0] -
                                            to_obj->obj_flags.value[1] );
  
  /* Se lo oggetto from_obj e` di tipo DRINK_PERM, in questo modo puo` essere
   * svuotato. Devo decidere se e` una feature od un bug :) Per il momento
   * rimane cosi`, perche` ha vantaggi e svantaggi per il giocatore.
   */
  from_obj->obj_flags.value[1] -= temp;
  to_obj->obj_flags.value[1] += temp;
  weight_change_object( to_obj, temp );
  weight_change_object( from_obj, -temp );
  

  /* Se il from_obj era avvelenato, ovviemente anche il to_obj sara` 
   * avvelenato.
   */
  if( IS_SET( from_obj->obj_flags.value[3], DRINK_POISON ) )
    SET_BIT( to_obj->obj_flags.value[3], DRINK_POISON );
  
  if( !from_obj->obj_flags.value[1] )
  {
    from_obj->obj_flags.value[2]=0;
    from_obj->obj_flags.value[3]=0;
    name_from_drinkcon(from_obj);
  }
  
  return;
}

void do_sip(struct char_data *ch, const char *argument, int cmd)
{
  struct affected_type af;
  char arg[MAX_STRING_LENGTH];
  char buf[MAX_STRING_LENGTH];
  struct obj_data *temp;
  
  one_argument(argument,arg);
  
  if(!(temp = get_obj_in_list_vis(ch,arg,ch->carrying)))    {
      act("You can't find it!",FALSE,ch,0,0,TO_CHAR);
      return;
    }
  
  if(temp->obj_flags.type_flag!=ITEM_DRINKCON)    {
      act("You can't sip from that!",FALSE,ch,0,0,TO_CHAR);
      return;
    }
  
  if(GET_COND(ch,DRUNK)>10) /* The pig is drunk ! */    {
      act("You simply fail to reach your mouth!",FALSE,ch,0,0,TO_CHAR);
      act("$n tries to sip, but fails!",TRUE,ch,0,0,TO_ROOM);
      return;
    }
  
  if(!temp->obj_flags.value[1])  /* Empty */    {
      act("But there is nothing in it?",FALSE,ch,0,0,TO_CHAR);
      return;
    }
  
  act("$n sips from the $o",TRUE,ch,temp,0,TO_ROOM);
  sprintf(buf,"It tastes like %s.\n\r",drinks[temp->obj_flags.value[2]]);
  send_to_char(buf,ch);
  
  gain_condition(ch,DRUNK,(int)(drink_aff[temp->obj_flags.value[2]]
                                [DRUNK]/4));
  
  gain_condition(ch,FULL,(int)(drink_aff[temp->obj_flags.value[2]][FULL]/4));
  
  gain_condition(ch,THIRST,(int)(drink_aff[temp->obj_flags.value[2]]
                                 [THIRST]/4));


  if(GET_COND(ch,DRUNK)>10)
    act("You feel drunk.",FALSE,ch,0,0,TO_CHAR);
  
  if(GET_COND(ch,THIRST)>20)
    act("You do not feel thirsty.",FALSE,ch,0,0,TO_CHAR);
  
  if(GET_COND(ch,FULL)>20)
    act("You are full.",FALSE,ch,0,0,TO_CHAR);
  
  if(IS_SET(temp->obj_flags.value[3],DRINK_POISON)
     && !IS_AFFECTED(ch,AFF_POISON))
  {
    /* The shit was poisoned ! */    
    act("But it also had a strange taste!",FALSE,ch,0,0,TO_CHAR);
      
    af.type = SPELL_POISON;
    af.duration = 3;
    af.modifier = 0;
    af.location = APPLY_NONE;
    af.bitvector = AFF_POISON;
    affect_to_char(ch,&af);
  }
  
  if(!IS_SET(temp->obj_flags.value[3],DRINK_PERM))
  {
    temp->obj_flags.value[1]--;
    weight_change_object(temp, -1); /* Subtract one unit, unless permanent */
  }
  
  if(!temp->obj_flags.value[1])  /* The last bit */
  {
    temp->obj_flags.value[2]=0;
    temp->obj_flags.value[3]=0;
    name_from_drinkcon(temp);
  }
  
  return;
  
}


void do_taste(struct char_data *ch, const char *argument, int cmd)
{
  struct affected_type af;
  char arg[80];
  struct obj_data *temp;
  
  one_argument(argument,arg);
  
  if(!(temp = get_obj_in_list_vis(ch,arg,ch->carrying)))    {
      act("You can't find it!",FALSE,ch,0,0,TO_CHAR);
      return;
    }
  
  if(temp->obj_flags.type_flag==ITEM_DRINKCON)    {
      do_sip(ch,argument,0);
      return;
    }
  
  if(!(temp->obj_flags.type_flag==ITEM_FOOD))    {
      act("Taste that?!? Your stomach refuses!",FALSE,ch,0,0,TO_CHAR);
      return;
    }
  
  act("$n tastes the $o", FALSE, ch, temp, 0, TO_ROOM);
  act("You taste the $o", FALSE, ch, temp, 0, TO_CHAR);
  
  gain_condition(ch,FULL,1);
  
  if(GET_COND(ch,FULL)>20)
    act("You are full.",FALSE,ch,0,0,TO_CHAR);
  
  if(temp->obj_flags.value[3]&&!IS_AFFECTED(ch,AFF_POISON))     {
      act("Ooups, it did not taste good at all!",FALSE,ch,0,0,TO_CHAR);
      
      af.type = SPELL_POISON;
      af.duration = 2;
      af.modifier = 0;
      af.location = APPLY_NONE;
      af.bitvector = AFF_POISON;
      affect_to_char(ch,&af);
    }
  
  temp->obj_flags.value[0]--;
  
  if(!temp->obj_flags.value[0]) {       /* Nothing left */
      act("There is nothing left now.",FALSE,ch,0,0,TO_CHAR);
      extract_obj(temp);
    }
  
  return;

}



/* functions related to wear */

void perform_wear( struct char_data *ch, struct obj_data *obj_object,
                   long keyword)
{
  switch(keyword) 
  {
  case 0 :
    act("$n accende ed usa $p.", FALSE, ch, obj_object,0,TO_ROOM);
    break;
  case 1 : 
    act("$n indossa $p su un dito.", TRUE, ch, obj_object,0,TO_ROOM);
    break;
  case 2 : 
    act("$n mette $p intorno al collo.", TRUE, ch, obj_object,0,TO_ROOM);
    break;
  case 3 : 
    act("$n indossa $p sul corpo.", TRUE, ch, obj_object,0,TO_ROOM);
    break;
  case 4 : 
    act("$n mette $p in testa.",TRUE, ch, obj_object,0,TO_ROOM);
    break;
  case 5 : 
    act("$n indossa $p sulle gambe.",TRUE, ch, obj_object,0,TO_ROOM);
    break;
  case 6 : 
    act("$n si mette $p ai piedi.",TRUE, ch, obj_object,0,TO_ROOM);
    break;
  case 7 : 
    act("$n mette $p sulle mani.",TRUE, ch, obj_object,0,TO_ROOM);
    break;
  case 8 : 
    act("$n indossa $p sulle braccia.",TRUE, ch, obj_object,0,TO_ROOM);
    break;
  case 9 : 
    act("$n si avvolge con $p.",TRUE, ch, obj_object,0,TO_ROOM);
    break;
  case 10 : 
    act("$n avvolge $p alla vita.",TRUE, ch, obj_object,0,TO_ROOM);
    break;
  case 11 : 
    act("$n mette $p intorno ad un polso.",TRUE, ch, obj_object,0,TO_ROOM);
    break;
  case 12 : 
    act("$n impugna $p.",TRUE, ch, obj_object,0,TO_ROOM);
    break;
  case 13 :
    act("$n prende $p in mano.",TRUE, ch, obj_object,0,TO_ROOM);
    break;
  case 14 :
    act("$n utilizza $p.", TRUE, ch, obj_object,0,TO_ROOM);
    break;
  case 15:
    act("$n si mette $p sulle spalle.", TRUE, ch, obj_object,0,TO_ROOM);
    break;
  case 16:
    act("$n appende $p ad un orecchio.", TRUE, ch, obj_object,0,TO_ROOM);
    break;
  case 17:
    act("$n si mette $p sugli occhi.", TRUE, ch, obj_object,0,TO_ROOM);
    break;
  }
}


int IsRestricted(int Mask, int Class)
{
  long i;

  for (i = CLASS_MAGIC_USER; i<= CLASS_PSI; i*=2) 
  {
    if (IS_SET(i, Mask) && (!IS_SET(i, Class))) 
    {
      Mask -= i;
    } /* if */
  } /* for */
     
  if (Mask == Class) 
    return(TRUE);

  return(FALSE);
}


void wear(struct char_data *ch, struct obj_data *obj_object, long keyword)
{
  char buffer[MAX_STRING_LENGTH];
  int BitMask;
  struct room_data *rp;

  
  if (!IS_IMMORTAL(ch)) 
  {

    BitMask = GetItemClassRestrictions(obj_object);
    /* only class items */

    if (IS_SET(obj_object->obj_flags.extra_flags,ITEM_ONLY_CLASS)) 
    {
      if (!OnlyClass(ch,BitMask)) 
      { 
        /* check here only for class restricts */
        if( obj_object->szForbiddenWearToChar && 
            *obj_object->szForbiddenWearToChar )
        {
          act( obj_object->szForbiddenWearToChar, FALSE, ch, obj_object, NULL,
               TO_CHAR );
        }
        else
        {
          sprintf( buffer, "Non sei la persona adatta ad usare %s.\n\r",
                   obj_object->short_description );
          send_to_char( buffer, ch);
        }
        if( obj_object->szForbiddenWearToRoom &&
            *obj_object->szForbiddenWearToRoom )
        {
          act( obj_object->szForbiddenWearToRoom, TRUE, ch, obj_object, NULL,
               TO_ROOM );
        }
        return;
      }
    }
    else if (IsRestricted(BitMask, ch->player.iClass) && IS_PC(ch)) 
    /* not only-class, okay to check normal anti-settings */
    {
      if( obj_object->szForbiddenWearToChar && 
          *obj_object->szForbiddenWearToChar )
      {
        act( obj_object->szForbiddenWearToChar, FALSE, ch, obj_object, NULL,
             TO_CHAR );
      }
      else
      {
        sprintf( buffer, "Ti e` proibito usare %s.\n\r", 
                 obj_object->short_description );
        send_to_char( buffer, ch);
      }
      if( obj_object->szForbiddenWearToRoom &&
          *obj_object->szForbiddenWearToRoom )
      {
        act( obj_object->szForbiddenWearToRoom, TRUE, ch, obj_object, NULL,
             TO_ROOM );
      }
      return;
    }
    
   }


  if( anti_barbarian_stuff( obj_object ) && 
      GET_LEVEL( ch, BARBARIAN_LEVEL_IND ) != 0 &&
      !IS_IMMORTAL( ch ) ) 
  {
    act( "Eck! Non questo! Perpcepisci la magia su $p e lo "
         "getti via schifato!", FALSE, ch, obj_object, NULL, TO_CHAR );
    act( "$n rabbrividisce e getta $p!", FALSE, ch, obj_object, 0, TO_ROOM );
    obj_from_char( obj_object );
    obj_to_room( obj_object, ch->in_room );
    return;
  }

  if( IS_SET( obj_object->obj_flags.extra_flags, ITEM_ANTI_MEN ) )
  {
    if (GET_SEX(ch) != SEX_FEMALE) 
    {
      act( "Solo le femmine possono utilizzare $p.", FALSE, ch, obj_object,
           NULL, TO_CHAR );
      return;
    }
  }

  if( IS_SET(obj_object->obj_flags.extra_flags, ITEM_ANTI_WOMEN)) 
  {
    if (GET_SEX(ch) != SEX_MALE) 
    {
      act( "Solo i maschi possono utilizzare $p.", FALSE, ch, obj_object,
           NULL, TO_CHAR );
      return;
    }
  }


  rp = real_roomp(ch->in_room);

  if (!IsHumanoid(ch)) 
  {
    if ((keyword != 13) || (!HasHands(ch))) 
    {
      act( "Non puoi indossare $p! E` per umanoidi.", FALSE, ch, obj_object,
           NULL, TO_CHAR );
      return;
    }
  }
  
  switch(keyword) 
  {
   case 0:  /* LIGHT SOURCE */
    if (ch->equipment[WEAR_LIGHT]) 
    {
      send_to_char("Stai gia` usando una sorgente di luce.\n\r", ch);
    }
    else if( ch->equipment[WIELD] && ch->equipment[WIELD]->obj_flags.weight > 
             str_app[STRENGTH_APPLY_INDEX(ch)].wield_w) 
    {
      send_to_char( "Non puoi impugnare un'arma a due mani ed usare una "
                    "sorgente di luce.\n\r", ch );
    } 
    else if ((ch->equipment[WIELD] && ch->equipment[HOLD])) 
    {
      send_to_char("Hai solo due mani.\n\r", ch);
    }
    else if( rp->sector_type == SECT_UNDERWATER &&
             obj_object->obj_flags.value[2] != -1) 
    {
      send_to_char("Non puoi accenderla sott'acqua!\n\r", ch);
    }
    else 
    {
      if( obj_object->obj_flags.value[2])
      {
        real_roomp(ch->in_room)->light++;
        if( real_roomp(ch->in_room)->light < 1 )
          real_roomp(ch->in_room)->light = 1;
      }
      send_to_char("Ok.\n\r", ch);
      perform_wear(ch,obj_object,keyword);
      obj_from_char(obj_object);
      equip_char(ch,obj_object, WEAR_LIGHT);
    }
    break;
    
   case 1: 
      if (CAN_WEAR(obj_object,ITEM_WEAR_FINGER)) 
      {
        if ((ch->equipment[WEAR_FINGER_L]) && (ch->equipment[WEAR_FINGER_R])) 
        {
          send_to_char( "Sta gia` indossando qualcosa sulle tue dita.\n\r", ch);
        }
        else 
        {
          perform_wear(ch,obj_object,keyword);
          if (ch->equipment[WEAR_FINGER_L]) 
          {
            act( "Indossi $p su un dito della mano destra.",
                 FALSE, ch, obj_object, NULL, TO_CHAR );
            obj_from_char(obj_object);
            equip_char(ch, obj_object, WEAR_FINGER_R);
          }
          else 
          {
            act( "Indossi $p su un dito della mano sinistra.",
                     FALSE, ch, obj_object, NULL, TO_CHAR );
            obj_from_char(obj_object);
            equip_char(ch, obj_object, WEAR_FINGER_L);
          }
        }
      }
      else 
      {
        send_to_char("Non puoi indossarlo sulle tue dita.\n\r", ch);
      }
      break;

    case 2: 
      if (CAN_WEAR(obj_object,ITEM_WEAR_NECK)) 
      {
        if ((ch->equipment[WEAR_NECK_1]) && (ch->equipment[WEAR_NECK_2])) 
        {
          send_to_char( "Non puoi mettere nient'altro intorno al collo.\n\r", 
                        ch);
        }
        else 
        {
          act( "Ti metti $p intorno al collo.",
               FALSE, ch, obj_object, NULL, TO_CHAR );
          perform_wear(ch,obj_object,keyword);
          if (ch->equipment[WEAR_NECK_1]) 
          {
            obj_from_char(obj_object);
            equip_char(ch, obj_object, WEAR_NECK_2);
          }
          else 
          {
            obj_from_char(obj_object);
            equip_char(ch, obj_object, WEAR_NECK_1);
          }
        }
      }
      else 
      {
        send_to_char( "Non puoi metterlo intorno al collo.\n\r", ch);
      }
      break;

    case 3: 
      if (CAN_WEAR(obj_object,ITEM_WEAR_BODY)) 
      {
        if (ch->equipment[WEAR_BODY]) 
        {
          send_to_char( "Stai gia` indossando qualcosa sul corpo.\n\r", ch);
        }
        else 
        {
          act( "Indossi $p.",
               FALSE, ch, obj_object, NULL, TO_CHAR );
          perform_wear(ch,obj_object,keyword);
          obj_from_char(obj_object);
          equip_char(ch,  obj_object, WEAR_BODY);
        }
      }
      else 
      {
        send_to_char("Non puoi indossarlo sul corpo.\n\r", ch);
      }
      break;

    case 4: 
      if (CAN_WEAR(obj_object,ITEM_WEAR_HEAD)) 
      {
        if (ch->equipment[WEAR_HEAD]) 
        {
          send_to_char("Hai gia qualcosa in testa.\n\r", ch);
        }
        else 
        {
          act( "Ti metti $p in testa.",
              FALSE, ch, obj_object, NULL, TO_CHAR );
          perform_wear(ch,obj_object,keyword);
          obj_from_char(obj_object);
          equip_char(ch, obj_object, WEAR_HEAD);
        } 
      }
      else 
      {
        send_to_char( "Non puoi metterlo in testa.\n\r", ch);
      }
      break;

    case 5: 
      if (CAN_WEAR(obj_object,ITEM_WEAR_LEGS)) 
      {
        if (ch->equipment[WEAR_LEGS]) 
        {
          send_to_char( "Hai gia` qualcosa suelle tue gambe.\n\r", ch);
        }
        else 
        {
          act( "Indossi $p sulle gambe.",
               FALSE, ch, obj_object, NULL, TO_CHAR );
          perform_wear(ch,obj_object,keyword);
          obj_from_char(obj_object);
          equip_char(ch, obj_object, WEAR_LEGS);
        } 
      }
      else 
      {
        send_to_char("Non puoi metterlo sulle gambe.\n\r", ch);
      }
      break;

    case 6: 
      if (CAN_WEAR(obj_object,ITEM_WEAR_FEET)) 
      {
        if (ch->equipment[WEAR_FEET]) 
        {
          send_to_char("Hai gia` un qualche tipo di calzatura.\n\r", ch);
        }
        else 
        {
          act( "Ti metti $p ai piedi.",
               FALSE, ch, obj_object, NULL, TO_CHAR );
          perform_wear(ch,obj_object,keyword);
          obj_from_char(obj_object);
          equip_char(ch, obj_object, WEAR_FEET);
        } 
      }
      else 
      {
        send_to_char( "Non mi sembra una calzatura.\n\r", ch);
      }
      break;
 
    case 7: 
      if (CAN_WEAR(obj_object,ITEM_WEAR_HANDS)) 
      {
        if (ch->equipment[WEAR_HANDS]) 
        {
          send_to_char("Indossi gia` qualcosa sulle tue mani.\n\r", ch);
        }
        else 
        {
          act( "Metti $p sulle mani.",
               FALSE, ch, obj_object, NULL, TO_CHAR );
          perform_wear(ch,obj_object,keyword);
          obj_from_char(obj_object);
          equip_char(ch, obj_object, WEAR_HANDS);
        }
      }
      else 
      {
        send_to_char("Non vedo come possa indossarlo sulle mani.\n\r", ch);
      }
      break;

    case 8: 
      if (CAN_WEAR(obj_object,ITEM_WEAR_ARMS)) 
      {
        if (ch->equipment[WEAR_ARMS]) 
        {
          send_to_char("Hai gia` qualcosa sulle braccia.\n\r", ch);
        }
        else 
        {
          act( "Indossi $p sulle braccia.",
               FALSE, ch, obj_object, NULL, TO_CHAR );
          perform_wear(ch,obj_object,keyword);
          obj_from_char(obj_object);
          equip_char(ch, obj_object, WEAR_ARMS);
        } 
      }
      else 
      {
        send_to_char("Non puoi metterlo sulle braccia.\n\r", ch);
      }
      break;

    case 9: 
      if (CAN_WEAR(obj_object,ITEM_WEAR_ABOUT)) 
      {
        if (ch->equipment[WEAR_ABOUT]) 
        {
          send_to_char("Indossi gia` qualcosa intorno al tuo corpo.\n\r", ch);
        }
        else 
        {
          act( "Ti avvolgi con $p.",
               FALSE, ch, obj_object, NULL, TO_CHAR );
          perform_wear(ch,obj_object,keyword);
          obj_from_char(obj_object);
          equip_char(ch, obj_object, WEAR_ABOUT);
        }
      }
      else 
      {
        send_to_char("Non puo` essere indossato intorno al corpo.\n\r", ch);
      }
      break;
 
    case 10: 
      if (CAN_WEAR(obj_object,ITEM_WEAR_WAISTE)) 
      {
        if (ch->equipment[WEAR_WAISTE]) 
        {
          send_to_char( "Hai gia qualcosa intorno alla vita.\n\r", ch );
        }
        else 
        {
          act( "Ti metti $p alla vita.",
               FALSE, ch, obj_object, NULL, TO_CHAR );
          perform_wear(ch,obj_object,keyword);
          obj_from_char(obj_object);
          equip_char(ch,  obj_object, WEAR_WAISTE);
        } 
      }
      else 
      {
        send_to_char("Non vedo come possa mettorlo intorno alla vita.\n\r", ch);
      }
      break;

    case 11: 
      if (CAN_WEAR(obj_object,ITEM_WEAR_WRIST)) 
      {
        if ((ch->equipment[WEAR_WRIST_L]) && (ch->equipment[WEAR_WRIST_R])) 
        {
          send_to_char( "Hai gia` oggetti intorno a tutti e due i polsi.\n\r", 
                        ch);
        }
        else 
        {
          perform_wear(ch,obj_object,keyword);
          obj_from_char(obj_object);
          if (ch->equipment[WEAR_WRIST_L]) 
          {
            act( "Indossi $p intorno al polso destro.\n\r", 
                 FALSE, ch, obj_object, NULL, TO_CHAR );
            equip_char(ch,  obj_object, WEAR_WRIST_R);
          }
          else 
          {
            act( "Indossi $p intorno al polso sinistro.\n\r", 
                 FALSE, ch, obj_object, NULL, TO_CHAR );
            equip_char(ch, obj_object, WEAR_WRIST_L);
          }
        }
      }
      else 
      {
        send_to_char( "Non puoi metterlo intorno ai polsi.\n\r", ch );
      }
      break;

    case 12:
      if (CAN_WEAR(obj_object,ITEM_WIELD)) 
      {
        if (ch->equipment[WIELD]) 
        {
          send_to_char("Stai gia` impugnando qualcosa.\n\r", ch);
        }
        else if (ch->equipment[WEAR_LIGHT] && ch->equipment[HOLD]) 
        {
          send_to_char("Devi prima togliere qualcosa dalle tue mani.\n\r", ch);
        }
        else 
        {
          if( GET_OBJ_WEIGHT(obj_object) >
              str_app[STRENGTH_APPLY_INDEX(ch)].wield_w) 
          {
            send_to_char( "E` troppo pesante per usarlo con una mano sola..\n\r",
                          ch);
            if( GET_OBJ_WEIGHT(obj_object) < 
                (3*str_app[STRENGTH_APPLY_INDEX(ch)].wield_w)/2) 
            {
              send_to_char("Ma puoi usarlo con due mani.\n\r", ch);
              if (ch->equipment[WEAR_SHIELD]) 
              {
                send_to_char("Se rimuovi il tuo scudo.\n\r", ch);
              } /* wearing shield */
              else if (ch->equipment[HOLD] || ch->equipment[WEAR_LIGHT])
              {
               send_to_char( "Se rimuovi quello che hai nell'altra mano.\n\r", 
                             ch);
              } /* holding light type */
              else 
              {
                perform_wear(ch,obj_object,keyword);
                obj_from_char(obj_object);
                equip_char(ch, obj_object, WIELD);
              } /*  fine, wear it */
            } /* strong enough */
            else
            {
              send_to_char( "Sei troppo debole anche per usarlo a due mani.\n\r",
                            ch);
            } /* to weak to wield two handed */
          } 
          else 
          {
            act( "Impugni $p.",
                 FALSE, ch, obj_object, NULL, TO_CHAR );
            perform_wear(ch,obj_object,keyword);
            obj_from_char(obj_object);
            equip_char(ch, obj_object, WIELD);
          }
        }
      }
      else 
      {
        send_to_char("Non e` una cosa che puo` essere impugnata.\n\r", ch);
      }
      break;
    
    case 13:
      if (CAN_WEAR(obj_object,ITEM_HOLD)) 
      {
        if (ch->equipment[HOLD]) 
        {
          send_to_char("Stai gia` usando qualcosa.\n\r", ch);
        }
        else if( ch->equipment[WIELD] &&
                 ch->equipment[WIELD]->obj_flags.weight >
                 str_app[STRENGTH_APPLY_INDEX(ch)].wield_w )
        {
          send_to_char( "Non puoi impugnare un'arma a due mani e usare "
                        "qualcos'altro.\n\r", ch);
        }
        else if (ch->equipment[WEAR_LIGHT] && ch->equipment[WIELD])
        {
          send_to_char("Hai tutte e due le mani occupate!\n\r", ch);
        }
        else 
        {
          if (CAN_WEAR(obj_object,ITEM_WIELD))
          {
            if (ch->equipment[WEAR_SHIELD]) 
            {
              send_to_char( "Non puoi usare uno scudo, un'arma e tenere "
                            "un'altra cosa contemporaneamente!\n\r", ch);
              return;
            }

            if( GET_OBJ_WEIGHT(obj_object) > 
                str_app[STRENGTH_APPLY_INDEX(ch)].wield_w/2) 
            {
              send_to_char( "Quest'arma e` troppo pesante da tenere.\n\r", 
                            ch);
              return;
            }
          }
          else if (ch->equipment[WIELD]) 
          {
            if( GET_OBJ_WEIGHT(obj_object) > 
                str_app[STRENGTH_APPLY_INDEX(ch)].wield_w) 
            {
              send_to_char( "Questo oggetto e` troppo pesante da tenere.\n\r", 
                            ch);
              return;
            }
          }
          act( "Prendi $p in mano.",
               FALSE, ch, obj_object, NULL, TO_CHAR );
          perform_wear(ch,obj_object,keyword);
          obj_from_char(obj_object);
          equip_char(ch, obj_object, HOLD);
        }
      }
      else 
      {
        send_to_char("Non e` possibile tenerlo.\n\r", ch);
      }
      break;

    case 14: 
      if (CAN_WEAR(obj_object,ITEM_WEAR_SHIELD)) 
      {
        if ((ch->equipment[WEAR_SHIELD])) 
        {
          send_to_char("Stai gia` usando uno scudo.\n\r", ch);
        }
        else if( ch->equipment[WIELD] && 
                 ch->equipment[WIELD]->obj_flags.weight >
                 str_app[STRENGTH_APPLY_INDEX(ch)].wield_w) 
        {
          send_to_char( "Non puoi impugnare un'arma a due mani ed usare uno "
                        "scudo.\n\r", ch);
        }
        else if( ch->equipment[HOLD] && 
                 CAN_WEAR(ch->equipment[HOLD],ITEM_WIELD))
        {
          send_to_char("Non puoi usare uno scudo e tenere un'arma!\n\r", ch);
          return;
        }
        else
        {
          perform_wear(ch,obj_object,keyword);
          act( "Usi $p.", FALSE, ch, obj_object, NULL, TO_CHAR );
          obj_from_char(obj_object);
          equip_char(ch, obj_object, WEAR_SHIELD);
        }
      }
      else 
      {
        send_to_char("Non sembra uno scudo.\n\r", ch);
      }
      break;

    case 15:
#if 0
      if( CAN_WEAR(obj_object, ITEM_WEAR_BACK) && 
          obj_object->obj_flags.type_flag == ITEM_CONTAINER) 
#else
      if( CAN_WEAR( obj_object, ITEM_WEAR_BACK ) ) 
#endif
      {
        if (ch->equipment[WEAR_BACK]) 
        {
          send_to_char( "Hai gia` qualcosa sulle tue spalle.\n\r", ch);
        }
        else 
        {
          act( "Ti metti $p sulle spalle.",
               FALSE, ch, obj_object, NULL, TO_CHAR );
          perform_wear(ch,obj_object,keyword);
          obj_from_char(obj_object);
          equip_char(ch,  obj_object, WEAR_BACK);
        }
      }
      else 
      {
        send_to_char( "Non vedo come puoi metterlo sulle spalle.\n\r", ch);
      }
      break;

    case 16:
      if (CAN_WEAR(obj_object,ITEM_WEAR_EAR)) 
      {
        if ((ch->equipment[WEAR_EAR_L]) && (ch->equipment[WEAR_EAR_R])) 
        {
          send_to_char( "Hai gia` qualcosa su entrambe le orecchie.\n\r", ch);
        } 
        else 
        {
          perform_wear(ch,obj_object,keyword);
          obj_from_char(obj_object);
          if (ch->equipment[WEAR_EAR_L]) 
          {
            sprintf( buffer, "Appendi %s alla tua orecchia destra.\n\r",
                     obj_object->short_description);
            send_to_char(buffer, ch);
            equip_char(ch,  obj_object, WEAR_EAR_R);
          } 
          else 
          {
            sprintf( buffer, "Appendi %s alla tua orecchia sinistra.\n\r",
                     obj_object->short_description);
            send_to_char(buffer, ch);
            equip_char(ch, obj_object, WEAR_EAR_L);
          }
        }
      } 
      else 
      {
        send_to_char("Non puoi appenderlo alle orecchie.\n\r", ch);
      }
      break;
 
    case 17:
      if (CAN_WEAR(obj_object,ITEM_WEAR_EYE)) 
      {
        if (ch->equipment[WEAR_EYES]) 
        {
          send_to_char("Hai gia` qualcosa sui tuoi occhi.\n\r", ch);
        }
        else 
        {
          act( "Ti metti $p davanti agli occhi.",
               FALSE, ch, obj_object, NULL, TO_CHAR );
          perform_wear(ch,obj_object,keyword);
          obj_from_char(obj_object);
          equip_char(ch,  obj_object, WEAR_EYES);
        } 
      }
      else 
      {
        send_to_char("No, non sugli occhi.\n\r", ch);
      }
      break;

    case -1: 
      sprintf( buffer,"Indossare %s ? dove?.\n\r", 
               obj_object->short_description);
      send_to_char(buffer, ch);
      break;
    case -2: 
      sprintf( buffer,"Non puoi indossare %s.\n\r", 
               obj_object->short_description);
      send_to_char(buffer, ch);
      break;
    default: 
      mudlog( LOG_ERROR, "Unknown type called in wear." );
      break;
  }
}


void do_wear(struct char_data *ch, const char *argument, int cmd) 
{
  char arg1[MAX_INPUT_LENGTH];
  char arg2[MAX_INPUT_LENGTH];
  char buf[256];
  char buffer[MAX_INPUT_LENGTH];
  struct obj_data *obj_object, *next_obj;
  int keyword;
  const char *keywords[] = {
    "finger",
    "neck",
    "body",
    "head",
    "legs",
    "feet",
    "hands",
    "arms",
    "about",
    "waist",
    "wrist",
    "wield",
    "hold",
    "shield",
    "back",
    "ears",
    "eye",
    "\n"
    };
  
  argument_interpreter(argument, arg1, arg2);
  if (*arg1) 
  {
    if (!strcmp(arg1,"all")) 
    {
      
      for (obj_object = ch->carrying; obj_object; obj_object = next_obj) 
      {
        next_obj = obj_object->next_content;
        keyword = -2;
        
        if (CAN_WEAR(obj_object,ITEM_HOLD)) 
          keyword = 13;
        if (CAN_WEAR(obj_object,ITEM_WEAR_SHIELD)) 
          keyword = 14;
        if (CAN_WEAR(obj_object,ITEM_WEAR_FINGER)) 
          keyword = 1;
        if (CAN_WEAR(obj_object,ITEM_WEAR_NECK)) 
          keyword = 2;
        if (CAN_WEAR(obj_object,ITEM_WEAR_WRIST)) 
          keyword = 11;
        if (CAN_WEAR(obj_object,ITEM_WEAR_WAISTE)) 
          keyword = 10;
        if (CAN_WEAR(obj_object,ITEM_WEAR_ARMS)) 
          keyword = 8;
        if (CAN_WEAR(obj_object,ITEM_WEAR_HANDS)) 
          keyword = 7;
        if (CAN_WEAR(obj_object,ITEM_WEAR_FEET)) 
          keyword = 6;
        if (CAN_WEAR(obj_object,ITEM_WEAR_LEGS)) 
          keyword = 5;
        if (CAN_WEAR(obj_object,ITEM_WEAR_ABOUT)) 
          keyword = 9;
        if (CAN_WEAR(obj_object,ITEM_WEAR_HEAD)) 
          keyword = 4;
        if (CAN_WEAR(obj_object,ITEM_WEAR_BODY)) 
          keyword = 3;
        if (CAN_WEAR(obj_object,ITEM_WIELD)) 
          keyword = 12;
        if (CAN_WEAR(obj_object,ITEM_WEAR_BACK) &&
            obj_object->obj_flags.type_flag==ITEM_CONTAINER) 
          keyword=15;
        if (CAN_WEAR(obj_object,ITEM_WEAR_EYE)) 
          keyword = 17;
        if (CAN_WEAR(obj_object,ITEM_WEAR_EAR)) 
          keyword = 16;


        if (keyword != -2) 
        {
          wear(ch, obj_object, keyword);
        }
      }
    }
    else 
    {
      obj_object = get_obj_in_list_vis(ch, arg1, ch->carrying);
      if (obj_object) 
      {
        if (*arg2) 
        {
          keyword = search_block(arg2, keywords, FALSE); /* Partial Match */
          if (keyword == -1) 
          {
            sprintf(buf, "Non puoi indossare nulla su un %s.\n\r", arg2);
            send_to_char(buf, ch);
          } 
          else 
          {
            wear(ch, obj_object, keyword+1);
          }
        } 
        else 
        {
          keyword = -2;
          if (CAN_WEAR(obj_object,ITEM_HOLD)) 
            keyword = 13;
          if (CAN_WEAR(obj_object,ITEM_WEAR_SHIELD)) 
            keyword = 14;
          if (CAN_WEAR(obj_object,ITEM_WEAR_FINGER)) 
            keyword = 1;
          if (CAN_WEAR(obj_object,ITEM_WEAR_NECK)) 
            keyword = 2;
          if (CAN_WEAR(obj_object,ITEM_WEAR_WRIST)) 
            keyword = 11;
          if (CAN_WEAR(obj_object,ITEM_WEAR_WAISTE))
            keyword = 10;
          if (CAN_WEAR(obj_object,ITEM_WEAR_ARMS)) 
            keyword = 8;
          if (CAN_WEAR(obj_object,ITEM_WEAR_HANDS))
            keyword = 7;
          if (CAN_WEAR(obj_object,ITEM_WEAR_FEET)) 
            keyword = 6;
          if (CAN_WEAR(obj_object,ITEM_WEAR_LEGS)) 
            keyword = 5;
          if (CAN_WEAR(obj_object,ITEM_WEAR_ABOUT))
            keyword = 9;
          if (CAN_WEAR(obj_object,ITEM_WEAR_HEAD)) 
            keyword = 4;
          if (CAN_WEAR(obj_object,ITEM_WEAR_BODY)) 
            keyword = 3;
          if (CAN_WEAR(obj_object,ITEM_WEAR_BACK) &&
             obj_object->obj_flags.type_flag==ITEM_CONTAINER)
            keyword=15;
          if (CAN_WEAR(obj_object,ITEM_WEAR_EYE)) 
            keyword = 17;
          if (CAN_WEAR(obj_object,ITEM_WEAR_EAR)) 
            keyword = 16;

          wear(ch, obj_object, keyword);
        }
      } 
      else 
      {
        sprintf(buffer, "Non sembra che tu abbia un '%s'.\n\r",arg1);
        send_to_char(buffer,ch);
      }
    }
  } 
  else 
  {
    send_to_char("Che cosa vuoi indossare?\n\r", ch);
  }
}


void do_wield(struct char_data *ch, const char *argument, int cmd) 
{
  char arg1[MAX_INPUT_LENGTH];
  char arg2[MAX_INPUT_LENGTH];
  char buffer[MAX_INPUT_LENGTH];
  struct obj_data *obj_object;
  int keyword = 12;
  
  argument_interpreter(argument, arg1, arg2);
  if (*arg1) 
  {
    obj_object = get_obj_in_list_vis(ch, arg1, ch->carrying);
    if (obj_object) 
    {
      wear(ch, obj_object, keyword);
    } 
    else 
    {
      sprintf(buffer, "Non sembra che ti abbia un '%s'.\n\r",arg1);
      send_to_char(buffer,ch);
    }
  } 
  else 
  {
    send_to_char("Cosa vuoi impugnare?\n\r", ch);
  }
}


void do_grab(struct char_data *ch, const char *argument, int cmd)
{
  char arg1[128];
  char arg2[128];
  char buffer[256];
  struct obj_data *obj_object;
  
  argument_interpreter(argument, arg1, arg2);
  
  if (*arg1) 
  {
    obj_object = get_obj_in_list(arg1, ch->carrying);
    if (obj_object) 
    {
      if (obj_object->obj_flags.type_flag == ITEM_LIGHT)
        wear(ch, obj_object, WEAR_LIGHT);
      else
        wear(ch, obj_object, 13);
    } 
    else 
    {
      sprintf(buffer, "Non sembra che tu abbia un '%s'.\n\r",arg1);
      send_to_char(buffer,ch);
    }
  } 
  else 
  {
    send_to_char("Cosa vuoi usare?\n\r", ch);
  }
}


void do_remove(struct char_data *ch, const char *argument, int cmd)
{
  char arg1[128],*T,*P;
  char buffer[256];
  int Rem_List[20],Num_Equip;
  struct obj_data *obj_object;
  int j;
  
  one_argument(argument, arg1);
  
  if( *arg1 )
  {
    if (!strcmp(arg1,"all")) 
    {
      for (j=0;j<MAX_WEAR;j++) 
      {
        if (CAN_CARRY_N(ch) != IS_CARRYING_N(ch)) 
        {
          if (ch->equipment[j]) 
          {
            if ((obj_object = unequip_char(ch,j))!=NULL) 
            {
              obj_to_char(obj_object, ch);
              
              act("Smetti di usare $p.",FALSE,ch,obj_object,0,TO_CHAR);
              if (obj_object->obj_flags.type_flag == ITEM_LIGHT)
                if( obj_object->obj_flags.value[ 2 ] )
                  real_roomp(ch->in_room)->light--;

            }
          }
        } 
        else 
        {
          send_to_char("Non puoi trasportare altro.\n\r",ch);
          j = MAX_WEAR;
        }
      }
      act( "$n smette di usare il suo equipaggiamento.", TRUE, ch, NULL,
           NULL, TO_ROOM );
      return;
    }
    if( isdigit( arg1[ 0 ] ) )           
    {
      /* PAT-PAT-PAT */
      
      /* Make a list of item numbers for stuff to remove */
      
      for( Num_Equip = j = 0; j< MAX_WEAR; j++ )
      {
        if (CAN_CARRY_N(ch) != IS_CARRYING_N(ch))               
        {
          if( ch->equipment[ j ] ) 
            Rem_List[Num_Equip++] = j;
        }
      }
      
      T = arg1;
      
      while( isdigit( *T ) && ( *T != '\0' ) )           
      {
        P = T;
        if( strchr( T, ',' ) )              
        {
          P = strchr( T, ',' );
          *P = '\0';
        }
        if( atoi( T ) > 0 && atoi( T ) <= Num_Equip )
        {
          if (CAN_CARRY_N(ch) != IS_CARRYING_N(ch))
          {
            j = Rem_List[ atoi( T ) - 1 ];
            if( ch->equipment[ j ] )
            {
              if( ( obj_object = unequip_char( ch, j ) ) != NULL )
              {
                obj_to_char(obj_object, ch);
                act("Smetti di usare $p.",FALSE,ch,obj_object,0,TO_CHAR);
                act("$n smette di usare $p.",TRUE,ch,obj_object,0,TO_ROOM);

                if (obj_object->obj_flags.type_flag == ITEM_LIGHT)
                  if (obj_object->obj_flags.value[2]) 
                    real_roomp(ch->in_room)->light--;

              }
            }
          } 
          else
          {
            send_to_char("Non riesci a trasportare altra roba.\r",ch);
            j = MAX_WEAR;
          }
        } 
        else  
        {
          sprintf(buffer,"Sembra che tu non abbia un %s\n\r",T);
          send_to_char(buffer,ch);
        }
        if (T != P) 
          T = P + 1;
        else 
          *T = '\0';
      }
    }
    else 
    {
      obj_object = get_object_in_equip_vis(ch, arg1, ch->equipment, &j);
      if( obj_object )
      {
        if( IS_OBJ_STAT(obj_object,ITEM_NODROP) && !IS_IMMORTAL( ch ) )
        {
          send_to_char( "Non puoi lasciarlo andare, deve essere stregato!\n\r",
                        ch);
          return;
        }
        if (CAN_CARRY_N(ch) != IS_CARRYING_N(ch)) 
        {
          
          obj_to_char(unequip_char(ch, j), ch);
          
          if (obj_object->obj_flags.type_flag == ITEM_LIGHT)
            if (obj_object->obj_flags.value[2])
              real_roomp(ch->in_room)->light--;
          
          act("Smetti di usare $p.",FALSE,ch,obj_object,0,TO_CHAR);
          act("$n smette di usare $p.",TRUE,ch,obj_object,0,TO_ROOM);
          
        } 
        else 
        {
          send_to_char("Non puoi trasportare altra roba.\n\r", ch);
        }
      } 
      else 
      {
        send_to_char("Non lo stai usando.\n\r", ch);
      }
    }
  }
  else 
  {
    send_to_char("Cosa vuoi rimuovere?\n\r", ch);
  }

  check_falling(ch);
}

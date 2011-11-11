

/*
**  0 = Mage, 1 = cleric, 3 = thief, 2 = fighter
*/

/*
**  
*/

#include <stdio.h>
#include <string.h>

#include "protos.h"


/* extern variables */

extern struct room_data *world;
extern struct descriptor_data *descriptor_list;
extern struct room_data *world;
extern struct dex_app_type dex_app[];



int GetClassLevel(struct char_data *ch, int iClass)
{

  if (IS_SET(ch->player.iClass, iClass)) {
    return(GET_LEVEL(ch, CountBits(iClass)-1));
  }
  return(0);
}

int CountBits(int iClass)
{
  if (iClass == 1) return(1);
  if (iClass == 2) return(2);
  if (iClass == 4) return(3);
  if (iClass == 8) return(4);
  if (iClass == 16) return(5);
  if (iClass == 32) return(6);
  if (iClass == 64) return(7);
  if (iClass == 128) return(8);
  if (iClass == 256) return(9);
  if (iClass == 512) return(10);
  if (iClass == 1024) return(11);
  if (iClass == 2048) return(12);

  mudlog( LOG_SYSERR, "Unrecognized class in CountBits (multiclass.c).");
  return 0;
}

int OnlyClass( struct char_data *pChar, int nClass )
{
  int i;

  for( i = 1; i <= CLASS_PSI; i *= 2 )
  {
    if( GetClassLevel( pChar, i ) != 0 )
      if( !( i & nClass ) )
        return FALSE;
  }
  return TRUE;

}


int HasClass(struct char_data *ch, int iClass)
{

  if (IS_NPC(ch) && !IS_SET(ch->specials.act,ACT_POLYSELF))
  {
    /* Se non e` specificata una classe particolare, restituisce TRUE, visto
     * che gli NPC non sono di tutte e nessuna classe. */
    if( !IS_SET( iClass, CLASS_MONK) || !IS_SET( iClass, CLASS_DRUID) ||
        !IS_SET( iClass, CLASS_BARBARIAN) || !IS_SET( iClass, CLASS_SORCERER) ||
        !IS_SET( iClass, CLASS_PALADIN) || !IS_SET( iClass, CLASS_RANGER) ||
        !IS_SET( iClass, CLASS_PSI ) )
      return(TRUE);
    /* Se invece la creatura ha una classe particolare (diversa dal WARRIOR)
     * fa lo stesso controllo dei giocatori. */
  } /* was NPC */

  if (IS_SET(ch->player.iClass, iClass))
    return(TRUE);

  return FALSE;
}

int HowManyClasses(struct char_data *ch)
{
  short i, tot=0;

  for (i=0;i<MAX_CLASS;i++) 
  {
    if (GET_LEVEL(ch, i)) 
    {
      tot++;
    }
  } /* end for */
  
  if (tot) 
    return(tot);
    
  else 
  {

  
    if (IS_SET(ch->player.iClass, CLASS_MAGIC_USER)) 
      tot++;

    if (IS_SET(ch->player.iClass, CLASS_WARRIOR)) 
      tot++;
      
    if (IS_SET(ch->player.iClass, CLASS_THIEF))
      tot++;

    if (IS_SET(ch->player.iClass, CLASS_CLERIC))
      tot++;

    if (IS_SET(ch->player.iClass, CLASS_DRUID))
      tot++;

    if (IS_SET(ch->player.iClass, CLASS_MONK))
      tot++;

    if (IS_SET(ch->player.iClass, CLASS_BARBARIAN))
      tot++;

    if (IS_SET(ch->player.iClass, CLASS_SORCERER))
      tot++;

    if (IS_SET(ch->player.iClass, CLASS_PALADIN))
      tot++;

    if (IS_SET(ch->player.iClass, CLASS_RANGER))
      tot++;

    if (IS_SET(ch->player.iClass, CLASS_PSI))
      tot++;

   }
  
  return(tot);
}


int BestFightingClass(struct char_data *ch)
{


 if (GET_LEVEL(ch, WARRIOR_LEVEL_IND)) 
   return(WARRIOR_LEVEL_IND);
 if (GET_LEVEL(ch, PALADIN_LEVEL_IND)) 
   return(PALADIN_LEVEL_IND);   

 if (GET_LEVEL(ch, BARBARIAN_LEVEL_IND)) 
   return(BARBARIAN_LEVEL_IND);   
 if (GET_LEVEL(ch, RANGER_LEVEL_IND)) 
   return(RANGER_LEVEL_IND);   

 if (GET_LEVEL(ch, CLERIC_LEVEL_IND))
   return(CLERIC_LEVEL_IND);
 if (GET_LEVEL(ch, DRUID_LEVEL_IND))
   return(DRUID_LEVEL_IND);
 if (GET_LEVEL(ch, MONK_LEVEL_IND)) 
   return(MONK_LEVEL_IND);
 if (GET_LEVEL(ch, THIEF_LEVEL_IND)) 
   return(THIEF_LEVEL_IND);

 if (GET_LEVEL(ch, PSI_LEVEL_IND)) 
   return(PSI_LEVEL_IND);
 if (GET_LEVEL(ch, MAGE_LEVEL_IND)) 
   return(MAGE_LEVEL_IND);
 if (GET_LEVEL(ch, SORCERER_LEVEL_IND)) 
   return(SORCERER_LEVEL_IND);
 
  mudlog( LOG_SYSERR, "Character %s has no recognized class.", GET_NAME( ch ) );
  assert(0);

  return(1);
}

int BestThiefClass(struct char_data *ch)
{

 if (GET_LEVEL(ch, THIEF_LEVEL_IND)) 
   return(THIEF_LEVEL_IND);
 if (GET_LEVEL(ch, MONK_LEVEL_IND)) 
   return(MONK_LEVEL_IND);
 if (GET_LEVEL(ch, PSI_LEVEL_IND)) 
   return(PSI_LEVEL_IND);

 if (GET_LEVEL(ch, MAGE_LEVEL_IND)) 
   return(MAGE_LEVEL_IND);
 if (GET_LEVEL(ch, SORCERER_LEVEL_IND)) 
   return(SORCERER_LEVEL_IND);

 if (GET_LEVEL(ch, WARRIOR_LEVEL_IND)) 
   return(WARRIOR_LEVEL_IND);
 if (GET_LEVEL(ch, BARBARIAN_LEVEL_IND)) 
   return(BARBARIAN_LEVEL_IND);
 if (GET_LEVEL(ch, RANGER_LEVEL_IND)) 
   return(RANGER_LEVEL_IND);   
 if (GET_LEVEL(ch, PALADIN_LEVEL_IND)) 
   return(PALADIN_LEVEL_IND);   
   
 if (GET_LEVEL(ch, DRUID_LEVEL_IND)) 
   return(DRUID_LEVEL_IND);
 if (GET_LEVEL(ch, CLERIC_LEVEL_IND)) 
   return(CLERIC_LEVEL_IND);
 
  mudlog( LOG_SYSERR, "Character %s has no recognized class.", GET_NAME( ch ) );
  assert(0);

  return(1);
}

int BestMagicClass(struct char_data *ch)
{

  if (GET_LEVEL(ch, MAGE_LEVEL_IND)) 
    return(MAGE_LEVEL_IND);
  if (GET_LEVEL(ch, SORCERER_LEVEL_IND)) 
    return(SORCERER_LEVEL_IND);
  if (GET_LEVEL(ch, DRUID_LEVEL_IND)) 
    return(DRUID_LEVEL_IND);
  if (GET_LEVEL(ch, CLERIC_LEVEL_IND)) 
    return(CLERIC_LEVEL_IND);

  if (GET_LEVEL(ch, PALADIN_LEVEL_IND)) 
    return(PALADIN_LEVEL_IND);   
  if (GET_LEVEL(ch, PSI_LEVEL_IND)) 
    return(PSI_LEVEL_IND);   

  if (GET_LEVEL(ch, RANGER_LEVEL_IND)) 
    return(RANGER_LEVEL_IND);   
  
  if (GET_LEVEL(ch, THIEF_LEVEL_IND)) 
    return(THIEF_LEVEL_IND);
  if (GET_LEVEL(ch, WARRIOR_LEVEL_IND)) 
    return(WARRIOR_LEVEL_IND);
  if (GET_LEVEL(ch, BARBARIAN_LEVEL_IND)) 
    return(BARBARIAN_LEVEL_IND);
  
  if (GET_LEVEL(ch, MONK_LEVEL_IND)) 
    return(MONK_LEVEL_IND);
 
  mudlog( LOG_SYSERR, "Character %s has no recognized class.", 
          GET_NAME( ch ) );
  ch->player.iClass = 4;

  return(1);
}

int GetALevel(struct char_data *ch, int which)
{
  byte ind[ MAX_CLASS ], k;
  int i, j;

  for( i = 0; i < MAX_CLASS; i++ ) 
    ind[i] = GET_LEVEL(ch,i);

  /* chintzy sort. (just to prove that I did learn something in college) */

  for( i = 0; i < MAX_CLASS-1; i++ )
  {
    for( j = i + 1 ;j < MAX_CLASS; j++ ) 
    {
      if( ind[j] > ind[i] )
      {
        k = ind[i];
        ind[i] = ind[j];
        ind[j] = k;
      }
    }
  }

  if( which > -1 && which < 4 ) 
    return( ind[ which ] );

  return 0;
}

int GetSecMaxLev(struct char_data *ch)
{
   return(GetALevel(ch, 2));
}

int GetThirdMaxLev(struct char_data *ch)
{
   return(GetALevel(ch, 3));
}

int GetMaxLevel(struct char_data *ch)
{
  register int max=0, i;

  for (i=0; i< MAX_CLASS; i++) {
    if (GET_LEVEL(ch, i) > max)
      max = GET_LEVEL(ch,i);
  }

  return(max);
}

int GetTotLevel(struct char_data *ch)
{
  int max=0, i;

  for (i=0; i< MAX_CLASS; i++)
      max += GET_LEVEL(ch,i);

  return(max);

}

void StartLevels(struct char_data *ch)
{

  if (IS_SET(ch->player.iClass, CLASS_MAGIC_USER)) {
    advance_level(ch, MAGE_LEVEL_IND);
  }

  if (IS_SET(ch->player.iClass, CLASS_SORCERER)) {
    advance_level(ch, SORCERER_LEVEL_IND);
  }
  if (IS_SET(ch->player.iClass, CLASS_CLERIC)) {
    advance_level(ch, CLERIC_LEVEL_IND);
  }
  if (IS_SET(ch->player.iClass, CLASS_WARRIOR)) {
    advance_level(ch, WARRIOR_LEVEL_IND);
  }
  if (IS_SET(ch->player.iClass, CLASS_THIEF)) {
    advance_level(ch, THIEF_LEVEL_IND);
  }
  if (IS_SET(ch->player.iClass, CLASS_DRUID)) {
    advance_level(ch,DRUID_LEVEL_IND);
  }
  if (IS_SET(ch->player.iClass, CLASS_MONK)) {
    advance_level(ch, MONK_LEVEL_IND);
  }
  if (IS_SET(ch->player.iClass, CLASS_BARBARIAN)) {
    advance_level(ch, BARBARIAN_LEVEL_IND);
  }  
  if (IS_SET(ch->player.iClass, CLASS_PALADIN)) {
    advance_level(ch, PALADIN_LEVEL_IND);
  }    
  if (IS_SET(ch->player.iClass, CLASS_RANGER)) {
    advance_level(ch, RANGER_LEVEL_IND);
  }  
  if (IS_SET(ch->player.iClass, CLASS_PSI)) {
    advance_level(ch, PSI_LEVEL_IND);
  }  

}



int BestClassIND(struct char_data *ch)
{

  int max=0, iClass=0, i;

  for (i=0; i< MAX_CLASS; i++)
    if (max < GET_LEVEL(ch,i)) {
      max = GET_LEVEL(ch, i);
      iClass = i;
    }

  assert(max > 0);
  return(iClass);

}

int BestClassBIT(struct char_data *ch)
{
  int max=0, iClass=0, i;

  for (i=0; i< MAX_CLASS; i++)
    if (max < GET_LEVEL(ch,i)) 
    {
      max = GET_LEVEL(ch, i);
      iClass = i;
    }

  assert(max > 0);

  switch(iClass) 
  {
  case MAGE_LEVEL_IND:
    return(1);
    break;
  case CLERIC_LEVEL_IND:
    return(2);
    break;
  case WARRIOR_LEVEL_IND:
    return(4);
    break;
  case THIEF_LEVEL_IND:
    return(8);
    break; 
  case DRUID_LEVEL_IND:
    return(16);
    break;
  case MONK_LEVEL_IND:
    return(32);
    break;
  case BARBARIAN_LEVEL_IND:
    return(64);
    break;
  case SORCERER_LEVEL_IND:
    return(128);
    break;   
  case PALADIN_LEVEL_IND:
    return(256);
    break;      
  case RANGER_LEVEL_IND:
    return(512);
    break;   
  case PSI_LEVEL_IND: 
    return(1024);
    break;
  default:  
    mudlog( LOG_SYSERR, "Error in BestClassBIT"); 
    break; 
 } /* switch */
 
  return(iClass);
}

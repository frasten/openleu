
#include <stdio.h>
#include <string.h>

#include "protos.h"

/* uses */

extern struct time_info_data time_info;
extern struct weather_data weather_info;
extern struct room_data *room_db;
extern long SystemFlags;

/*In this part. */
void SaveTheWorld();
void weather_and_time(int mode);
void another_hour(int mode);
void weather_change(void);
void GetMonth( int month);
void ChangeWeather( int change);
void switch_light(byte why);  /* -DM 7/16/92 */
void PulseMobiles(int type);

/* what stage is moon in?  (1 - 32) */
unsigned char moontype;   

int gSeason;   /* global variable --- the season */
int gMoonSet = 3;
int gSunRise = 5;
int gSunSet = 18;
int gMoonRise = 22;

int gLightLevel = 4;  /* defaults to sunlight */

/* Here comes the code */

void weather_and_time(int mode)
{
  another_hour(mode);
  if(mode)
    weather_change();
}

void another_hour(int mode)
{
  char moon[20], buf[100];
  int tmp, i;
  
  time_info.hours++;
  
  tmp = time_info.hours;
  
  if (mode) 
  {
    
    /* as a test, save a piece of the world every mud hour */
    SaveTheWorld();
    if (tmp == 0) 
    {
      for (i=0;i<29;i++)   /* save the rest of the world automatically */
        SaveTheWorld();
    }
    if (tmp == gMoonRise) 
    {
      if (moontype < 4) 
      {
        strcpy(moon, "nuova");
      } else if (moontype < 12) {
        strcpy(moon, "crescente");
      } else if (moontype < 20) {
        strcpy(moon, "piena");
      } else  if (moontype < 28) {
        strcpy(moon, "calante");
      } else {
        strcpy(moon, "nuova");
      }
      switch_light(MOON_RISE);
      sprintf(buf,"La luna %s comincia a sorgere dall'orizzonte.\n\r",moon);
      send_to_outdoor(buf);
      if((moontype > 16) && (moontype < 22)) 
      {
        gLightLevel++;   /* brighter during these moons */
      }
    }
    if (tmp == gSunRise && !IS_SET(SystemFlags,SYS_ECLIPS)) 
    {
      weather_info.sunlight = SUN_RISE;
      send_to_outdoor("Il sole comincia a sorgere dall'orizzonte.\n\r");
    }
    if (tmp == gSunRise+1&& !IS_SET(SystemFlags,SYS_ECLIPS)) 
    {
      weather_info.sunlight = SUN_LIGHT;
      switch_light(SUN_LIGHT);
      send_to_outdoor( "Il giorno e` iniziato e la luce del sole comincia a "
                       "splendere.\n\r");
    }
    if (tmp == gSunSet && !IS_SET(SystemFlags,SYS_ECLIPS)) 
    {
      weather_info.sunlight = SUN_SET;
      send_to_outdoor( "Il sole cala lentamente nell'orizzonte orientale.\n\r");
    }
    if (tmp == gSunSet+1) 
    {
      weather_info.sunlight = SUN_DARK;
      switch_light(SUN_DARK);
      send_to_outdoor("La notte e` iniziata stendendo il suo velo oscuro.\n\r");
    }
    if (tmp == gMoonSet) 
    {
      if((moontype > 15) && (moontype < 25)) 
      {
        switch_light(MOON_SET);
        send_to_outdoor( "Un velo di oscurita` cala nuovamente appena la luna "
                         "tramonta.\n\r");
      } 
      else 
      {
        send_to_outdoor("La luna tramonta lentamente.\n\r");
      }
      
    }
    if (tmp == 12) 
    {
      send_to_outdoor("Il sole e` esattamente sopra di te.\n\r");
    }
    
    if (time_info.hours > 23)  /* Changed by HHS due to bug ???*/ 
    {
      time_info.hours -= 24;
      time_info.day++;
      switch(time_info.day) 
      {
      case 0:
      case 6:
      case 13:
      case 20:
      case 27:
      case 34:
        PulseMobiles(EVENT_WEEK);
        break;
      }  
      /* check the season */
      ChangeSeason(time_info.month);
      
      moontype++;
      if (moontype > 32)
        moontype = 1;
      
      if (time_info.day>34)  
      {
        time_info.day = 0;
        time_info.month++;
        GetMonth(time_info.month);
        PulseMobiles(EVENT_MONTH);
        
        if(time_info.month>16)
        {
          time_info.month = 0;
          time_info.year++;
        }
      }
      
      ChangeSeason(time_info.month);      
    }
  }
}

void ChangeSeason(int month)
{
  extern int gSeason;
  switch (month){
  case 0:
  case 1:
  case 2:
  case 3:
  case 16:
    gSunRise = 9;  /* very late  */
    gSunSet = 16;  /* very early */
    gSeason = SEASON_WINTER;
    break;
  case 4:
  case 5:
  case 6:
  case 7:
    gSunRise = 7;  /* late  */
    gSunSet = 18;  /* early */
    gSeason = SEASON_SPRING;
    break;
  case 8:
  case 9:
  case 10:
  case 11:
    gSunRise = 5;  /* early  */
    gSunSet = 20;  /* late   */
    gSeason = SEASON_SUMMER;
    break;
  case 12:
  case 13:
  case 14:
  case 15:
    gSunRise = 7;  /* late  */
    gSunSet = 18;  /* early */
    gSeason = SEASON_FALL;
    break;
  default:
    gSeason = SEASON_WINTER;
    gSunRise = 9;  /* very late  */
    gSunSet = 16;  /* very early */
    break;
  }
}

void weather_change()
{
  int diff, change;
  
  if((time_info.month>=9)&&(time_info.month<=16))
    diff=(weather_info.pressure>985 ? -2 : 2);
  else
    diff=(weather_info.pressure>1015? -2 : 2);
  
  weather_info.change += (dice(1,4)*diff+dice(2,6)-dice(2,6));
  
  weather_info.change = MIN(weather_info.change,12);
  weather_info.change = MAX(weather_info.change,-12);
  
  weather_info.pressure += weather_info.change;
  
  weather_info.pressure = MIN(weather_info.pressure,1040);
  weather_info.pressure = MAX(weather_info.pressure,960);
  
  change = 0;
  
  switch(weather_info.sky)
  {
    case SKY_CLOUDLESS :
    {
      if (weather_info.pressure<990)
        change = 1;
      else if (weather_info.pressure<1010)
        if(dice(1,4)==1)
          change = 1;
      break;
    }
    case SKY_CLOUDY :
    {
      if (weather_info.pressure<970)
        change = 2;
      else if (weather_info.pressure<990)
        if(dice(1,4)==1)
          change = 2;
        else
          change = 0;
      else if (weather_info.pressure>1030)
        if(dice(1,4)==1)
          change = 3;
      
      break;
    }
    case SKY_RAINING :
    {
      if (weather_info.pressure<970)
        if(dice(1,4)==1)
          change = 4;
        else
          change = 0;
      else if (weather_info.pressure>1030)
        change = 5;
      else if (weather_info.pressure>1010)
        if(dice(1,4)==1)
          change = 5;
      
      break;
    }
    case SKY_LIGHTNING :
    {
      if (weather_info.pressure>1010)
        change = 6;
      else if (weather_info.pressure>990)
        if(dice(1,4)==1)
          change = 6;
      
      break;
    }
    default : 
    {
      change = 0;
      weather_info.sky=SKY_CLOUDLESS;
      break;
    }
  }
  
  ChangeWeather(change);
  
}

void ChangeWeather( int change)
{

  if (change < 0)
    change = 0;
  if (change > 7)
    change = 6;
   
  switch(change)
  {
    case 0 : 
      break;
    case 1 :
    {
      send_to_outdoor("Il cielo si sta annuvolando.\n\r");
      weather_info.sky=SKY_CLOUDY;
      break;
    }
    case 2 :
    {
      if ((time_info.month > 3) && (time_info.month < 14)) 
      {
        send_to_desert( "Un forte vento  comincia a soffiare attraverso "
                        "il paese.\n\r");
        send_to_arctic("Comincia a nevicare.\n\r");
        send_to_out_other("Inizia a piovere.\n\r");
      } 
      else 
      {
        send_to_desert( "Un forte vento freddo comincia a soffiare "
                        "attraverso il paese.\n\r" );
        send_to_arctic( "Comincia a nevicare forte.\n\r" );
        send_to_out_other( "Comincia a nevicare.\n\r" );
      }
      weather_info.sky=SKY_RAINING;
      break;
    }
    case 3 :
    {
      send_to_outdoor("Le nuvole se ne stanno andando.\n\r");
      weather_info.sky=SKY_CLOUDLESS;
      break;
    }
    case 4 :
    {
      if ((time_info.month > 3) && (time_info.month < 14)) 
      {
        send_to_desert( "Sei nel mezzo di in una tempesta di sabbia.\n\r");
        send_to_arctic( "Sei nel mezzo di una tempesta di neve.\n\r");
        send_to_out_other( "Sei nel mezzo di un accecante temporale.\n\r");
      } 
      else  
      {
        send_to_desert( "Sei nel mezzo di una tempesta di sabbia.\n\r");
        send_to_arctic( "Sei nel mezzo di una tempesta di neve.\n\r");
        send_to_out_other( "Sei nel mezzo di una tormenta. \n\r" );
      }
      weather_info.sky=SKY_LIGHTNING;
      break;
    }
    case 5 :
    {
      if ((time_info.month > 3) && (time_info.month < 14))  
      {
        send_to_desert( "La tempesta di sabbia si placa lentamente.\n\r");
        send_to_arctic( "La tempesta di neve si calma lentamente.\n\r");
        send_to_out_other( "La tormenta di pioggia cala piano piano.\n\r" );
      } 
      else  
      {
        send_to_desert( "La tempesta di sabbia si placa lentamente.\n\r");
        send_to_arctic( "Ha semmo di nevicare.\n\r");
        send_to_out_other( "Ha smesso di nevicare.\n\r");
      }
      weather_info.sky=SKY_CLOUDY;
      break;
    }
    case 6 :
    {
      if ((time_info.month > 3) && (time_info.month < 14)) 
      {
        send_to_desert( "La tempesta si e` placata, ma il vento soffia "
                        "ancora forte.\n\r");
        send_to_arctic( "La tempesta di neve si e` calmata ma nevica "
                        "ancora.\n\r");
        send_to_out_other( "Il temporale e` finito, ma piove ancora.\n\r");
      } 
      else 
      {
        send_to_desert( "La tempesta si e` placata, ma il vento soffia "
                        "ancora forte.\n\r");
        send_to_arctic( "La tempesta di neve si e` calmata ma nevica "
                        "ancora.\n\r");
        send_to_out_other( "La tormenta si e` calmata, ma nevica ancora.\n\r");
      }
      weather_info.sky=SKY_RAINING;
      break;
    }
    default : break;
  }
}

void GetMonth( int month)
{
  if (month < 0)
     return;

  if (month <= 1) 
  {
    send_to_outdoor(" Si gela qui fuori.\n\r");
  }
  else if (month <=2) 
  {
    send_to_outdoor(" Fa molto freddo.\n\r");
  }
  else if (month <=3) 
  {
    send_to_outdoor(" Oggi e` un po' meno freddo.\n\r");
  }
  else if (month == 4) 
  {
    send_to_outdoor(" Cominciano a sbocciare i fiori.\n\r");
    PulseMobiles(EVENT_SPRING);
  }
  else if (month == 8) 
  {
    send_to_outdoor(" C'e` un'afa qui fuori.\n\r");
    PulseMobiles(EVENT_SUMMER);
  }
  else if (month == 12) 
  {
    send_to_outdoor(" Comincia ad alzarsi il vento.\n\r");
    PulseMobiles(EVENT_FALL);
  }
  else if (month == 13) 
  {
    send_to_outdoor(" Comincia a far freddo.\n\r"); 
  }
  else if (month == 14) 
  {
    send_to_outdoor(" La foglie cominciano a cambiare colore.\n\r");
  }
  else if (month == 15) 
  {
    send_to_outdoor(" Fa veramente freddo.\n\r");
  }
  else if (month == 16) 
  {
    send_to_outdoor(" Si gela qui fuori.\n\r");
    PulseMobiles(EVENT_WINTER);
  }
}

void switch_light(byte why)
{
 extern int gLightLevel;

 switch(why) 
 {
 case MOON_SET:
   mudlog( LOG_CHECK, "setting all rooms to dark" );
   gLightLevel = 0;
   break;
 case SUN_LIGHT:
   mudlog( LOG_CHECK, "setting all rooms to light" );
   gLightLevel = 4;
   break;
 case SUN_DARK:
   mudlog( LOG_CHECK, "setting all rooms to dark" );
   gLightLevel = 0;
   break;
 case MOON_RISE:
   mudlog( LOG_CHECK, "setting all non-forest to light" );
   gLightLevel = 1;
   break;
 default:
   mudlog( LOG_SYSERR, "Unknown switch on switch_light: %s",why );
   break;
 }
}

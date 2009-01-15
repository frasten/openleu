/*
*** DaleMUD        ANSI_PARSER.C
***                Parser ansi colors for act();
*/


#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

#include "ansi.h"
#include "protos.h"

extern long SystemFlags;

/*        
 *   $CMBFG, where M is modier, B is back group color and FG is fore 
 *   $C0001 would be normal, black back, red fore.
 *   $C1411 would be bold, blue back, light yellow fore 
 */    

char *ansi_parse(char *code )
{
  char m[ 10 ], b[ 10 ],f[ 10 ];
  static char szResult[ 256 ];

  if (!code)
    return(NULL);

  if (IS_SET(SystemFlags,SYS_NOANSI))
    return(NULL);

#if defined( EMANUELE )
  /* do modifier */
  switch( code[0] )
  {
   case '0':
    sprintf( m,"%s",ANSI_MOD_NORMAL);
    break;
   case '1':
    sprintf(m,"%s",ANSI_MOD_BOLD);
    break;
   case '2':
    sprintf(m,"%s",ANSI_MOD_FAINT);
    break;
    /* not used in ansi that I know of */
   case '3':
    sprintf(m,"%s",ANSI_MOD_NORMAL);
    break;
   case '4':
    sprintf(m,"%s",ANSI_MOD_UNDERLINE);
    break;
   case '5':
    sprintf(m,"%s",ANSI_MOD_BLINK);
    break;
   case '6':
    sprintf(m,"%s",ANSI_MOD_REVERSE);
    break;

   default:
    sprintf(m,"%s",ANSI_MOD_NORMAL);
    break;
  }

  /* do back ground color */
  switch(code[1])
  {
   case '0':
    sprintf(b,"%s",ANSI_BK_BLACK);
    break;
   case '1':
    sprintf(b,"%s",ANSI_BK_RED);
    break;
   case '2':
    sprintf(b,"%s",ANSI_BK_GREEN);
    break;
   case '3':
    sprintf(b,"%s",ANSI_BK_BROWN);
    break;
   case '4':
    sprintf(b,"%s",ANSI_BK_BLUE);
    break;
   case '5':
    sprintf(b,"%s",ANSI_BK_MAGENTA);
    break;
   case '6':
    sprintf(b,"%s",ANSI_BK_CYAN);
    break;
   case '7':
    sprintf(b,"%s",ANSI_BK_LT_GRAY);
    break;
   default:
    sprintf(b,"%s",ANSI_BK_BLACK);
    break;
  }
          
  /* do foreground color */
  switch(code[2])
  {
   case '0':
    switch(code[3])
    {                  /* 00-09 */
     case '0':
      sprintf(f,"%s",ANSI_FG_BLACK);
      break;
     case '1':
      sprintf(f,"%s",ANSI_FG_RED);
      break;
     case '2':
      sprintf(f,"%s",ANSI_FG_GREEN);
      break;
     case '3':
      sprintf(f,"%s",ANSI_FG_BROWN);
      break;
     case '4':
      sprintf(f,"%s",ANSI_FG_BLUE);
      break;
     case '5':
      sprintf(f,"%s",ANSI_FG_MAGENTA);
      break;
     case '6':
      sprintf(f,"%s",ANSI_FG_CYAN);
      break;
     case '7':
      sprintf(f,"%s",ANSI_FG_LT_GRAY);
      break;
     case '8':
      sprintf(f,"%s",ANSI_FG_DK_GRAY);
      break;
     case '9':
      sprintf(f,"%s",ANSI_FG_LT_RED);
      break;
     default:
      sprintf(f,"%s",ANSI_FG_DK_GRAY);
      break;
    }
    break;

   case '1':
    switch(code[3])
    {                  /* 10-15 */
     case '0':
      sprintf(f,"%s",ANSI_FG_LT_GREEN);
      break;
     case '1':
      sprintf(f,"%s",ANSI_FG_YELLOW);
      break;
     case '2':
      sprintf(f,"%s",ANSI_FG_LT_BLUE);
      break;
     case '3':
      sprintf(f,"%s",ANSI_FG_LT_MAGENTA);
      break;
     case '4':
      sprintf(f,"%s",ANSI_FG_LT_CYAN);
      break;
     case '5':
      sprintf(f,"%s",ANSI_FG_WHITE);
      break;
     default:
      sprintf(f,"%s",ANSI_FG_LT_GREEN);
      break;
    }
    break;

   default :
    sprintf(f,"%s",ANSI_FG_LT_RED);
    break;                                  
  }
          

  sprintf( szResult, "\033[%s;%s;%sm", m, b, f );
  return szResult;
#else
  /* do modifier */
  switch(code[0])
  {
   case '0':
    sprintf(m,"%s",MOD_NORMAL);
    break;
   case '1':
    sprintf(m,"%s",MOD_BOLD);
    break;
   case '2':
    sprintf(m,"%s",MOD_FAINT);
    break;
    /* not used in ansi that I know of */
   case '3':
    sprintf(m,"%s",MOD_NORMAL);
    break;
   case '4':
    sprintf(m,"%s",MOD_UNDERLINE);
    break;
   case '5':
    sprintf(m,"%s",MOD_BLINK);
    break;
   case '6':
    sprintf(m,"%s",MOD_REVERSE);
    break;

   default:
    sprintf(m,"%s",MOD_NORMAL);
    break;
  }

  /* do back ground color */
  switch(code[1])
  {
   case '0':
    sprintf(b,"%s",BK_BLACK);
    break;
   case '1':
    sprintf(b,"%s",BK_RED);
    break;
   case '2':
    sprintf(b,"%s",BK_GREEN);
    break;
   case '3':
    sprintf(b,"%s",BK_BROWN);
    break;
   case '4':
    sprintf(b,"%s",BK_BLUE);
    break;
   case '5':
    sprintf(b,"%s",BK_MAGENTA);
    break;
   case '6':
    sprintf(b,"%s",BK_CYAN);
    break;
   case '7':
    sprintf(b,"%s",BK_LT_GRAY);
    break;
   default:
    sprintf(b,"%s",BK_BLACK);
    break;
  }
          
  /* do foreground color */
  switch(code[2])
  {
   case '0':
    switch(code[3])
    {                  /* 00-09 */
     case '0':
      sprintf(f,"%s",FG_BLACK);
      break;
     case '1':
      sprintf(f,"%s",FG_RED);
      break;
     case '2':
      sprintf(f,"%s",FG_GREEN);
      break;
     case '3':
      sprintf(f,"%s",FG_BROWN);
      break;
     case '4':
      sprintf(f,"%s",FG_BLUE);
      break;
     case '5':
      sprintf(f,"%s",FG_MAGENTA);
      break;
     case '6':
      sprintf(f,"%s",FG_CYAN);
      break;
     case '7':
      sprintf(f,"%s",FG_LT_GRAY);
      break;
     case '8':
      sprintf(f,"%s",FG_DK_GRAY);
      break;
     case '9':
      sprintf(f,"%s",FG_LT_RED);
      break;
     default:
      sprintf(f,"%s",FG_DK_GRAY);
      break;
    }
    break;

   case '1':
    switch(code[3])
    {                  /* 10-15 */
     case '0':
      sprintf(f,"%s",FG_LT_GREEN);
      break;
     case '1':
      sprintf(f,"%s",FG_YELLOW);
      break;
     case '2':
      sprintf(f,"%s",FG_LT_BLUE);
      break;
     case '3':
      sprintf(f,"%s",FG_LT_MAGENTA);
      break;
     case '4':
      sprintf(f,"%s",FG_LT_CYAN);
      break;
     case '5':
      sprintf(f,"%s",FG_WHITE);
      break;
     default:
      sprintf(f,"%s",FG_LT_GREEN);
      break;
    }
    break;

   default :
    sprintf(f,"%s",FG_LT_RED);
    break;                                  
  }
          

  sprintf( szResult, "%s%s%s", m, b, f );
  return szResult;
#endif
}


#include <stdio.h>
#include <string.h>

/* ATTENZIONE Controllare che corrispondano con i valori definiti in structs.h
   ed in utility.c */
void mudlog( unsigned uType, char *szString, ... );
#define LOG_CHECK    0x0002

int SecCheck(char *arg, char *site)
{
  char buf[ 255 ];
  FILE *f1;

  if(!(f1 = fopen(buf, "rt"))) 
  {
    mudlog( LOG_CHECK, "Unable to open security file for %s.", arg);
    return(-1);
  }

  fgets(buf, 250, f1);
  fclose(f1);

  if(!*buf) 
  {
    mudlog( LOG_CHECK, "Security file for %s empty.", arg);
    return(-1);
  }

  if( buf[ strlen( buf ) - 1 ] == '\n')
    buf[ strlen( buf ) - 1 ] = '\0';

  if( !strncmp( site, buf, strlen( buf ) ) ) 
  {
    return(1);
  }
  mudlog( LOG_CHECK, "Site %s and %s don't match for %s. Booting.", site, 
          buf, arg);

  return(0);
}


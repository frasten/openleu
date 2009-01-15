
#include <signal.h>
#include <stdio.h>
#include <sys/time.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

#include "protos.h"

#include "status.h"

char gszMudStatus[ 100 ] = "";
char gszName[ 100 ] = "";
void *gpGeneric = NULL;


extern struct descriptor_data *descriptor_list;

void checkpointing( int dummy );
void shutdown_request( int dummy );
void logsig( int dummy );
void hupsig( int dummy );
void badcrash( int dummy );

void PrintStatus()
{
  mudlog( LOG_SYSERR | LOG_SILENT, "Mud status when crashed: '%s'",
          gszMudStatus );
  mudlog( LOG_SYSERR | LOG_SILENT, "  Last Name '%s'", gszName );
}

void SetStatus( const char *szStatus, char *szString, void *pGeneric )
{
  if( szStatus )
  {
    strncpy( gszMudStatus, szStatus, sizeof( gszMudStatus ) );
    gszMudStatus[ sizeof( gszMudStatus ) - 1 ] = 0;
  }
  
  if( szString )
  {
    strncpy( gszName, szString, sizeof( gszName ) );
    gszName[ sizeof( gszName ) - 1 ] = 0;
  }

  if( pGeneric )
  {
    gpGeneric = pGeneric;
  }
}

void signal_setup()
{
  struct itimerval itime;
  struct timeval interval;

  signal(SIGUSR2, shutdown_request);

  /* just to be on the safe side: */

  signal(SIGHUP, hupsig);
  signal(SIGPIPE, SIG_IGN);
  signal(SIGINT, hupsig);
  signal(SIGALRM, logsig);
  signal(SIGTERM, hupsig);
#if 0
  signal( SIGSEGV, badcrash );
  signal( SIGBUS, badcrash );
#endif

  /* set up the deadlock-protection */

  interval.tv_sec = 300;    /* 5 minutes */
  interval.tv_usec = 0;
  itime.it_interval = interval;
  itime.it_value = interval;
  if( setitimer( ITIMER_VIRTUAL, &itime, 0 ) < 0 )
    perror( "Setting Virtual timer in signal_setup" );
  else if( signal( SIGVTALRM, checkpointing ) == SIG_ERR )
    perror( "Calling 'signal' in signal_setup" );
}

void checkpointing( int dummy )
{
  extern int tics;
        
  if (!tics) 
  {
    mudlog( LOG_SYSERR, "CHECKPOINT shutdown: tics not updated" );
    PrintStatus();
  
    abort();
  }
  else
  {
    mudlog( LOG_CHECK, "CHECKPOINT: tics updated" );
    tics = 0;
  }
  if( signal( SIGVTALRM, checkpointing ) == SIG_ERR )
    perror( "Calling 'signal' in checkpointing" );
}

void shutdown_request( int dummy )
{
  extern int mudshutdown;

  mudlog( LOG_CHECK, "Received USR2 - shutdown request");
  mudshutdown = 1;
}


/* kick out players etc */
void hupsig( int dummy )
{
  int i;
  extern int mudshutdown, rebootgame;

  mudlog( LOG_CHECK, "Received SIGHUP, SIGINT, or SIGTERM. Shutting down");

  raw_force_all("return");
  raw_force_all("save");
  for (i=0;i<30;i++) 
  {
    SaveTheWorld();
  }
  mudshutdown = rebootgame = 1;
}

void logsig( int dummy )
{
  mudlog( LOG_CHECK, "Signal received. Ignoring." );
  signal( SIGALRM, logsig );
}

#if 1
void badcrash( int dummy )
{
  static int graceful_tried = 0;
  struct descriptor_data *desc;

  mudlog( LOG_CHECK, 
          "SIGSEGV or SIGBUS received. Trying to shut down gracefully.");
  
  PrintStatus();
  
  if( !graceful_tried )
  {
#if 0
    close(mother_desc);
#endif
    mudlog( LOG_CHECK, "Trying to close all sockets.");
    graceful_tried = 1;
    for( desc = descriptor_list; desc; desc = desc->next )
      close( desc->descriptor );
  }
  abort();
}
#endif                                             

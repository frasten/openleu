/*************************************************************************
 * lucertole.c contiene le procedure speciali per la foresta delle 
 * lucertole di Benem
 * **********************************************************************/
#if !defined( _LUCERTOLE_H )
#define _LUCERTOLE_H

int Lizardman( struct char_data *ch, int cmd, const char *arg, 
               struct char_data *mob, int type );
int lizardman_shaman( struct char_data *ch, int cmd, const char *arg, 
                      struct char_data *mob, int type );
int village_woman( struct char_data *ch, int cmd, const char *arg, 
                   struct char_data *mob, int type );

int snake_avt( struct char_data *ch, int cmd, const char *arg,
               struct char_data *mob, int type );

int snake_avt2( struct char_data *ch, int cmd, const char *arg, 
                struct char_data *mob, int type );

int virgin_sac( struct char_data *ch, int cmd, const char *arg, 
                struct char_data *mob, int type );

int snake_guardian( struct char_data *ch, int cmd, const char *arg, 
                    struct char_data *mob, int type );

int CapannaVillaggio( struct char_data *pChar, int iCmd, const char *szArgument,
                      struct room_data *pRoom, int iType );

int ColloSerpente( struct char_data *pChar, int iCmd, const char *szArgument,
                   struct room_data *pRoom, int iType );

int Rampicante( struct char_data *pChar, int iCmd, const char *szArgument,
                struct room_data *pRoom, int iType );

#endif


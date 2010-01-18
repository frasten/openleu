/***************************************************************************
  carceri.c contiene per procedure speciale per la zona delle carceri.
***************************************************************************/
#if !defined(_CARCERI_H)
#define _CARCERI_H

int Minicius( struct char_data *pChar, int nCmd, const char *szArg,
              struct char_data *pMob, int nType );
              
int VermeDellaMorte( struct char_data *pChar, int nCmd, const char *szArg,
                     struct char_data *pMob, int nType );

int KyussSon( struct char_data *pChar, int nCmd, const char *szArg,
              struct char_data *pMob, int nType );
              
int Piovra( struct char_data *pChar, int nCmd, const char *szArg,
            struct char_data *pMob, int nType );

int Moribondo( struct char_data *pChar, int nCmd, const char *szArg,
                struct char_data *pMob, int nType );

#endif

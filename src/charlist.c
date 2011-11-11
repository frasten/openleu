#include <stdlib.h>

#include "charlist.h"

int IsInList( CharElem *pElem, void *pWho )
{
  CharElem *pCurr;
  for( pCurr = pElem; pCurr; pCurr = pCurr->pNext )
    if( pCurr->pWho == pWho )
      return 1;
  return 0;
}

CharElem *InsertInList( CharElem **pElem, void *pWho, int nTimer )
{
  CharElem *pCurr;

  pCurr = (CharElem *)calloc( 1, sizeof( CharElem ) );
  pCurr->pNext = *pElem;
  pCurr->pWho = pWho;
  pCurr->nTimer = nTimer;
  pCurr->nIntData = 0;
  *pElem = pCurr;
  return pCurr;
}

CharElem *InsertInListInt( CharElem **pElem, void *pWho, int nTimer, 
                           int nData )
{
  CharElem *pCurr;

  pCurr = (CharElem *)calloc( 1, sizeof( CharElem ) );
  pCurr->pNext = *pElem;
  pCurr->pWho = pWho;
  pCurr->nTimer = nTimer;
  pCurr->nIntData = nData;
  *pElem = pCurr;
  return pCurr;
}

int GetIntData( CharElem *pElem, void *pWho )
{
  CharElem *pCurr;

  for( pCurr = pElem; pCurr; pCurr = pCurr->pNext )
    if( pCurr->pWho == pWho )
      return pCurr->nIntData;
      
  return 0;
}

void SetIntData( CharElem **pElem, void *pWho, int nData )
{
  CharElem *pCurr;

  for( pCurr = *pElem; pCurr; pCurr = pCurr->pNext )
    if( pCurr->pWho == pWho )
      break;
  if( !pCurr )
    pCurr = InsertInList( pElem, pWho, 0 );
    
  pCurr->nIntData = nData;
}

int AddIntData( CharElem **pElem, void *pWho, int nData )
{
  CharElem *pCurr;

  for( pCurr = *pElem; pCurr; pCurr = pCurr->pNext )
    if( pCurr->pWho == pWho )
      break;
  if( !pCurr )
    pCurr = InsertInList( pElem, pWho, 0 );

  pCurr->nIntData += nData;
  return pCurr->nIntData;
}

int SumIntData( CharElem *pElem )
{
  int nSum = 0;
  CharElem *pCurr;

  for( pCurr = pElem; pCurr; pCurr = pCurr->pNext )
    nSum += pCurr->nIntData;
    
  return nSum;
}

int SumIntDataNeg( CharElem *pElem )
{
  int nSum = 0;
  CharElem *pCurr;

  for( pCurr = pElem; pCurr; pCurr = pCurr->pNext )
    if( pCurr->nIntData < 0 )
      nSum -= pCurr->nIntData;
    
  return nSum;
}

int SumIntDataPos( CharElem *pElem )
{
  int nSum = 0;
  CharElem *pCurr;

  for( pCurr = pElem; pCurr; pCurr = pCurr->pNext )
    if( pCurr->nIntData > 0 )
      nSum += pCurr->nIntData;
    
  return nSum;
}

void UpdateList( CharElem **pElem )
{
  CharElem *pNextCurr;
  CharElem *pCurr;
    
  for( pCurr = *pElem; pCurr; pCurr = pNextCurr )
  {
    pNextCurr = pCurr->pNext;
    if( pCurr->nTimer > 0 )
    {
      pCurr->nTimer--;
      if( pCurr->nTimer == 0 )
      {
        if( pCurr == *pElem )
          *pElem = pCurr->pNext;
        else
        {
          CharElem *pTmp;
          for( pTmp = *pElem; pTmp && pTmp->pNext != pCurr;
               pTmp = pTmp->pNext );
          if( pTmp )
            pTmp->pNext = pCurr->pNext;
        }
        free( pCurr );
      }
    }
  }
}

void RemoveFromList( CharElem **pElem, void *pWho )
{
  CharElem *pCurr = 0;
  
  if( pWho == (*pElem)->pWho )
  {
    pCurr = *pElem;
    *pElem = pCurr->pNext;
  }
  else
  {
    CharElem *pTmp;
    for( pTmp = *pElem; pTmp && pTmp->pNext && pTmp->pNext->pWho != pWho;
         pTmp = pTmp->pNext );
    if( pTmp )
    {
      pCurr = pTmp->pNext;
      pTmp->pNext = pCurr->pNext;
    }
  }
  if( pCurr )
    free( pCurr );
}

void FreeList( CharElem **pElem )
{
  CharElem *pCurr = NULL;
  
  while( *pElem != NULL )
  {
    pCurr = *pElem;
    *pElem = pCurr->pNext;
    free( pCurr );
  }
}

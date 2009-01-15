#if !defined( _CHARLIST_H )
#define _CHARLIST_H

typedef struct tagCharElem
{
  void *pWho;
  struct tagCharElem *pNext;
  int nTimer;
  int nIntData;
} CharElem;

int IsInList( CharElem *pElem, void *pWho );
CharElem *InsertInList( CharElem **pElem, void *pWho, int nTimer );
CharElem *InsertInListInt( CharElem **pElem, void *pWho, int nTimer,
													 int nData );
int GetIntData( CharElem *pElem, void *pWho );
void SetIntData( CharElem **pElem, void *pWho, int nData );
int AddIntData( CharElem **pElem, void *pWho, int nData );
int SumIntData( CharElem *pElem );
int SumIntDataPos( CharElem *pElem );
int SumIntDataNeg( CharElem *pElem );
void UpdateList( CharElem **pElem );
void RemoveFromList( CharElem **pElem, void *pWho );
void FreeList( CharElem **pElem );

#endif

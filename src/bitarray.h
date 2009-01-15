#if !defined( _BITARRAY_H )
#define _BITARRAY_H

#define BYTESIZE 8

class BitArray
{
  unsigned char *achArray;
 public:
  BitArray( int iSize )
  {
    achArray = new unsigned char[ iSize / BYTESIZE + 1 ];
  };
  
  void ClearBit( int iIndex ) 
  {
    achArray[ iIndex / BYTESIZE ] &= ~( 1 << ( iIndex % BYTESIZE ) );
  };
  
  void SetBit( int iIndex )
  {
    achArray[ iIndex / BYTESIZE ] |=  1 << ( iIndex % BYTESIZE );
  };
  
  bool IsSetBit( int iIndex )
  {
    return achArray[ iIndex / BYTESIZE ] & (  1 << ( iIndex % BYTESIZE ) );
  };
  
  void ToggleBit( int iIndex )
  {
    achArray[ iIndex / BYTESIZE ] ^=  1 << ( iIndex % BYTESIZE );
  };
  
  ~BitArray()
  {
    if( achArray )
      delete achArray;
  };
};
  


#endif
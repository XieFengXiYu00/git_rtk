
#include "sys.h"
#include "CRC16.h"
//CRC16_CCITT的函数调用方法，已测试；
//1.检测的数据为char型，函数调用输出为short型
//函数步骤如下：
//1.根据CRC16的标准选择初值CRCIn的值。
//
//2.将数据的第一个字节与CRCIn高8位异或。
//
//3.判断最高位，若该位为 0 左移一位，若为 1 左移一位再与多项式Hex码异或。
//
//4.重复3直至8位全部移位计算结束。
//
//5.重复将所有输入数据操作完成以上步骤，所得16位数即16位CRC校验码。

unsigned short CRC16_CCITT(unsigned char *puchMsg, unsigned int usDataLen)
{
  unsigned short wCRCin = 0x0000;
  unsigned short wCPoly = 0x1021;
  unsigned char wChar = 0;
  int i;
  while (usDataLen--) 	
  {
        wChar = *(puchMsg++);
        InvertUint8(&wChar,&wChar);
        wCRCin ^= (wChar << 8);
        for(i = 0;i < 8;i++)
        {
          if(wCRCin & 0x8000)
            wCRCin = (wCRCin << 1) ^ wCPoly;
          else
            wCRCin = wCRCin << 1;
        }
  }
  InvertUint16(&wCRCin,&wCRCin);
  return (wCRCin) ;
}
void InvertUint8(unsigned char *DesBuf, unsigned char *SrcBuf)
 {
     int i;
    unsigned char temp = 0;
     for(i = 0; i < 8; i++)
    {
         if(SrcBuf[0] & (1 << i))
        {
             temp |= 1<<(7-i);
          }
      }
      DesBuf[0] = temp;
 }
/***************************************************************************************************
  * @Brief    double byte data inversion        
  * @Param    
  *            @DesBuf: destination buffer
  *            @SrcBuf: source buffer
  * @RetVal    None
  * @Note      (MSB)0101_0101_1010_1010 ---> 0101_0101_1010_1010(LSB)
  **************************************************************************************************
  */
  void InvertUint16(unsigned short *DesBuf, unsigned short *SrcBuf)  
  {  
      int i;  
     unsigned short temp = 0;    
     
     for(i = 0; i < 16; i++)  
     {  
         if(SrcBuf[0] & (1 << i))
         {          
             temp |= 1<<(15 - i);  
         }
     }  
     DesBuf[0] = temp;  
 }















#include "sys.h"
#include "CRC16.h"
//CRC16_CCITT�ĺ������÷������Ѳ��ԣ�
//1.��������Ϊchar�ͣ������������Ϊshort��
//�����������£�
//1.����CRC16�ı�׼ѡ���ֵCRCIn��ֵ��
//
//2.�����ݵĵ�һ���ֽ���CRCIn��8λ���
//
//3.�ж����λ������λΪ 0 ����һλ����Ϊ 1 ����һλ�������ʽHex�����
//
//4.�ظ�3ֱ��8λȫ����λ���������
//
//5.�ظ��������������ݲ���������ϲ��裬����16λ����16λCRCУ���롣

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














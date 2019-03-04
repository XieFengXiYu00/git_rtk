#ifndef __CRC16_H
#define __CRC16_H	 
#include "sys.h"

unsigned short CRC16_CCITT(unsigned char *puchMsg, unsigned int usDataLen);
void InvertUint8(unsigned char *DesBuf, unsigned char *SrcBuf);
void InvertUint16(unsigned short *DesBuf, unsigned short *SrcBuf);

#endif
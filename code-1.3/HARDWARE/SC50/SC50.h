#ifndef __SC50_H
#define __SC50_H	 
#include "sys.h" 


//¶Ë¿Ú¶¨Òå

#define SDA(x)			GPIO_Pin_Set(GPIOB,PIN6,x)		// SDA
#define BUSY(x)			GPIO_Pin_Set(GPIOB,PIN7,x)		// BUSY


void SC50X0B_Init(void);


void SC50X0B_SDA(unsigned char data);

#endif


















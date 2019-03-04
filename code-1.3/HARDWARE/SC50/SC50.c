#include "SC50.h" 
#include "delay.h" 



void SC50X0B_Init(void)
{    	 
	RCC->AHB1ENR|=1<<1;	//使能PORTB时钟 
	GPIO_Set(GPIOB,PIN6|PIN7,GPIO_MODE_OUT,GPIO_OTYPE_PP,GPIO_SPEED_100M,GPIO_PUPD_PU); //PB0,PB1设置
	
	SDA(1);
	BUSY(1);
	
}



void SC50X0B_SDA(unsigned char data)
{
	unsigned char i;
	SDA(0);
	delay_ms(5);
	BUSY(1);
	for(i=0;i < 8;i++)
 { 
	 SDA(1); 
	 if(data & 0x01)
	 {
	 delay_us(1500); 
	 SDA(0);
	 delay_us(500); 
	 }
	 else
	 { 
	 delay_us(500); 
	 SDA(0);
	 delay_us(1500); 
	 } 
	 data >>= 1;
 } 
 SDA(1);
 BUSY(0);
 delay_us(500);

} 









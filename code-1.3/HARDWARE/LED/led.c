#include "led.h" 
#include "delay.h" 
//////////////////////////////////////////////////////////////////////////////////	 





void LED_Init(void)
{    	 
	RCC->AHB1ENR|=1<<1;	//使能PORTB时钟 
	GPIO_Set(GPIOB,PIN0|PIN1,GPIO_MODE_OUT,GPIO_OTYPE_PP,GPIO_SPEED_100M,GPIO_PUPD_PU); //PB0,PB1设置
	
//	GPIO_Set(GPIOB,PIN6|PIN7,GPIO_MODE_OUT,GPIO_OTYPE_PP,GPIO_SPEED_100M,GPIO_PUPD_PU); //PB0,PB1设置
//	SDA(1);
//	BUSY(1);
	
	
	LED0(1);			//关闭DS0
	LED1(1);			//关闭DS1
}











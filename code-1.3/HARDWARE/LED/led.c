#include "led.h" 
#include "delay.h" 
//////////////////////////////////////////////////////////////////////////////////	 





void LED_Init(void)
{    	 
	RCC->AHB1ENR|=1<<1;	//ʹ��PORTBʱ�� 
	GPIO_Set(GPIOB,PIN0|PIN1,GPIO_MODE_OUT,GPIO_OTYPE_PP,GPIO_SPEED_100M,GPIO_PUPD_PU); //PB0,PB1����
	
//	GPIO_Set(GPIOB,PIN6|PIN7,GPIO_MODE_OUT,GPIO_OTYPE_PP,GPIO_SPEED_100M,GPIO_PUPD_PU); //PB0,PB1����
//	SDA(1);
//	BUSY(1);
	
	
	LED0(1);			//�ر�DS0
	LED1(1);			//�ر�DS1
}











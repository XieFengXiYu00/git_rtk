#include "wkup.h"
#include "SC50.h"
#include "led.h"
#include "delay.h"	
#include "rtc.h"
#include "usart.h"
#include "sys.h"
//////////////////////////////////////////////////////////////////////////////////	 
//������ֻ��ѧϰʹ�ã�δ��������ɣ��������������κ���;
//ALIENTEK STM32������
//�������� ����	   
//����ԭ��@ALIENTEK
//������̳:www.openedv.com
//��������:2016/7/13
//�汾��V1.0
//��Ȩ���У�����ؾ���
//Copyright(C) ������������ӿƼ����޹�˾ 2014-2024
//All rights reserved									  
//////////////////////////////////////////////////////////////////////////////////
			 
//ϵͳ�������ģʽ
void Sys_Enter_Standby(void)
{		
	u32 tempreg;			//��ʱ�洢�Ĵ���ֵ��
	//�ر���������(����ʵ�����д) 
 	RCC->AHB1RSTR|=0X01FE;	//��λ��GPIOA���������IO��	
  	while(WKUP_KD);			//�ȴ�WK_UP�����ɿ�(����RTC�ж�ʱ,�����WK_UP�ɿ��ٽ������)
 	RCC->AHB1RSTR|=1<<0;	//��λGPIOA
	
	//STM32F4/F7,��������RTC����жϺ�,�����ȹر�RTC�ж�,�����жϱ�־λ,Ȼ����������
	//RTC�ж�,�ٽ������ģʽ�ſ�����������,�����������.
	RCC->APB1ENR|=1<<28;	//ʹ�ܵ�Դʱ�� 
	PWR->CR1|=1<<8;			//���������ʹ��(RTC+SRAM) 
	//�ر�RTC�Ĵ���д����
	RTC->WPR=0xCA;
	RTC->WPR=0x53; 
	tempreg=RTC->CR&(0X0F<<12);//��¼ԭ����RTC�ж�����
	RTC->CR&=~(0XF<<12);	//�ر�RTC�����ж�
	RTC->ISR&=~(0X3F<<8);	//�������RTC�жϱ�־.  
	PWR->CR2|=1<<0;      	//���Wake-up ��־
	RTC->CR|=tempreg;		//��������RTC�ж� 
	RTC->WPR=0xFF;			//ʹ��RTC�Ĵ���д����  
	Sys_Standby();			//�������ģʽ
}
//���WKUP�ŵ��ź�
//����ֵ1:��������3s����
//      0:����Ĵ���	
u8 Check_WKUP(void) 
{
	u8 t=0;
	u8 tx=0;	//��¼�ɿ��Ĵ���
	LED0(0); 	//����DS0 
	while(1)
	{
		if(WKUP_KD)//�Ѿ�������
		{
			t++;
			tx=0;
		}else 
		{
			tx++; 
			if(tx>3)	//����90ms��û��WKUP�ź�
			{
				LED0(1); 
				return 0;//����İ���,���´�������
			}
		}
		delay_ms(30);
		if(t>=100)		//���³���3����
		{
			LED0(0);;	//����DS0 
			return 1;	//����3s������
		}
	}
} 



//�ж�,��⵽PA0�ŵ�һ��������.	  
//�ж���0���ϵ��жϼ��
void EXTI0_IRQHandler(void)
{ 
#if SYSTEM_SUPPORT_OS 		//���SYSTEM_SUPPORT_OSΪ�棬����Ҫ֧��OS.
	
	OSIntEnter();    
	
#endif
	EXTI->PR=1<<0;  //���LINE0�ϵ��жϱ�־λ  
  delay_ms(10); //����	
	if(Check_WKUP())//�ػ�?
	{
		SC50X0B_SDA(0x04);	
    Sys_Enter_Standby();  		
	}
#if SYSTEM_SUPPORT_OS 	//���SYSTEM_SUPPORT_OSΪ�棬����Ҫ֧��OS.
	
	OSIntExit(); 
	
#endif	
} 
//PA0 WKUP���ѳ�ʼ��
void WKUP_Init(void)
{	  		  
	RCC->AHB1ENR|=1<<0;    		//ʹ��PORTAʱ��	   
	GPIO_Set(GPIOA,PIN0,GPIO_MODE_IN,0,0,GPIO_PUPD_PD);	//PA0���� 
	//(����Ƿ���������)��    		 
    if(Check_WKUP()==0)
	{ 
 		Sys_Enter_Standby();	//���ǿ���,�������ģʽ  
	}
	SC50X0B_SDA(0x02);
	Ex_NVIC_Config(GPIO_A,0,RTIR);	//PA0�����ش���	    										   		   
	MY_NVIC_Init(2,2,EXTI0_IRQn,2);	//��ռ2�������ȼ�2����2
}

















#ifndef __LED_H
#define __LED_H	 
#include "sys.h" 
//////////////////////////////////////////////////////////////////////////////////	 
//本程序只供学习使用，未经作者许可，不得用于其它任何用途
//ALIENTEK STM32开发板
//LED驱动代码	   
//正点原子@ALIENTEK
//技术论坛:www.openedv.com
//创建日期:2016/7/11
//版本：V1.0
//版权所有，盗版必究。
//Copyright(C) 广州市星翼电子科技有限公司 2014-2024
//All rights reserved									  
////////////////////////////////////////////////////////////////////////////////// 	 

//LED端口定义
#define LED0(x)			GPIO_Pin_Set(GPIOB,PIN1,x)		// DS0
#define LED1(x)			GPIO_Pin_Set(GPIOB,PIN0,x)		// DS1 


void LED_Init(void);	//初始化

#endif


















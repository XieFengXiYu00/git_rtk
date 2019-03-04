#ifndef __USART_H
#define __USART_H 
#include "sys.h"
#include "stdio.h"	  
//////////////////////////////////////////////////////////////////////////////////	   
//本程序只供学习使用，未经作者许可，不得用于其它任何用途
//ALIENTEK STM32开发板
//串口1初始化 
//正点原子@ALIENTEK
//技术论坛:www.openedv.com
//创建日期:2016/7/8
//版本：V1.1
//版权所有，盗版必究。
//Copyright(C) 广州市星翼电子科技有限公司 2014-2024
//All rights reserved
//********************************************************************************
//修改说明 
//V1.1 20160708 
//移植自F429的例程,针对F7做了相应修改
////////////////////////////////////////////////////////////////////////////////// 

#define USART_REC_LEN  			800  	//定义最大接收字节数 200

#define EN_USART1_RX 			1		//使能（1）/禁止（0）串口1
#define EN_USART2_RX 	 		1			//使能（1）/禁止（0）串口2
#define EN_USART3_RX 			1		//使能（1）/禁止（0）串口2
#define EN_UART4_RX 			1		//使能（1）/禁止（0）串口2




extern u16 USART2_RX_NUM;         		//串口2接收数组数量
extern u16 USART3_RX_NUM;         		//串口3接收数组数量
extern u16 UART4_RX_NUM;         			//串口4接收数组数量


extern u8  USART_RX_BUF[USART_REC_LEN]; //接收缓冲,最大USART_REC_LEN个字节.末字节为换行符 
extern u16 USART_RX_STA;         		//接收状态标记	


void uart1_init(u32 pclk2,u32 bound);
void uart2_init(u32 pclk1,u32 bound);
void uart3_init(u32 pclk1,u32 bound);
void uart4_init(u32 pclk1,u32 bound);
void uart5_init(u32 pclk1,u32 bound);
//void uart6_init(u32 pclk1,u32 bound);
//void uart7_init(u32 pclk1,u32 bound);
//void uart8_init(u32 pclk1,u32 bound);
#endif	   

















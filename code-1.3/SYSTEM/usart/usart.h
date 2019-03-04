#ifndef __USART_H
#define __USART_H 
#include "sys.h"
#include "stdio.h"	  
//////////////////////////////////////////////////////////////////////////////////	   
//������ֻ��ѧϰʹ�ã�δ��������ɣ��������������κ���;
//ALIENTEK STM32������
//����1��ʼ�� 
//����ԭ��@ALIENTEK
//������̳:www.openedv.com
//��������:2016/7/8
//�汾��V1.1
//��Ȩ���У�����ؾ���
//Copyright(C) ������������ӿƼ����޹�˾ 2014-2024
//All rights reserved
//********************************************************************************
//�޸�˵�� 
//V1.1 20160708 
//��ֲ��F429������,���F7������Ӧ�޸�
////////////////////////////////////////////////////////////////////////////////// 

#define USART_REC_LEN  			800  	//�����������ֽ��� 200

#define EN_USART1_RX 			1		//ʹ�ܣ�1��/��ֹ��0������1
#define EN_USART2_RX 	 		1			//ʹ�ܣ�1��/��ֹ��0������2
#define EN_USART3_RX 			1		//ʹ�ܣ�1��/��ֹ��0������2
#define EN_UART4_RX 			1		//ʹ�ܣ�1��/��ֹ��0������2




extern u16 USART2_RX_NUM;         		//����2������������
extern u16 USART3_RX_NUM;         		//����3������������
extern u16 UART4_RX_NUM;         			//����4������������


extern u8  USART_RX_BUF[USART_REC_LEN]; //���ջ���,���USART_REC_LEN���ֽ�.ĩ�ֽ�Ϊ���з� 
extern u16 USART_RX_STA;         		//����״̬���	


void uart1_init(u32 pclk2,u32 bound);
void uart2_init(u32 pclk1,u32 bound);
void uart3_init(u32 pclk1,u32 bound);
void uart4_init(u32 pclk1,u32 bound);
void uart5_init(u32 pclk1,u32 bound);
//void uart6_init(u32 pclk1,u32 bound);
//void uart7_init(u32 pclk1,u32 bound);
//void uart8_init(u32 pclk1,u32 bound);
#endif	   

















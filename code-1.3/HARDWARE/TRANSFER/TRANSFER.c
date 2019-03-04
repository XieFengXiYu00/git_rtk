#include "TRANSFER.h" 
#include "DMA.h" 
#include "SC50.h"
#include "sys.h"
#include "delay.h"
#include "usart.h"
#include "includes.h"

extern OS_EVENT * msg_broad;			//�㲥�����ָ�룻

#define USART_REC_LEN  			800  	//�����������ֽ��� 200
extern u8  USART2_RX_BUF[USART_REC_LEN]; //���ջ���,���USART_REC_LEN���ֽ�
extern u8  USART3_RX_BUF[USART_REC_LEN]; //���ջ���,���USART_REC_LEN���ֽ�
extern u8  UART4_RX_BUF[USART_REC_LEN]; //���ջ���,���USART_REC_LEN���ֽ�

void BaseTransfer(void){
    int voice_flag;
		static u8 times_flag=0;//��̬�ֲ���¼���봫������ѭ���Ĵ���

		
		if(USART2_RX_NUM>0)//ת���忨����
		{	
			
		if(times_flag==0)//����������ѹ����
		{
		times_flag=1;	
		voice_flag=4;
		OSMboxPost(msg_broad,(void*)voice_flag);//��վ��λ������������ʾ
		delay_ms(20);	
		
		}	
		
			DMA_TxConfig(DMA1_Stream4,4,(u32)&UART4->TDR,(u32)USART2_RX_BUF,USART2_RX_NUM);//�忨COM2����2����ת������������4
			MYDMA_Enable(DMA1_Stream4,USART2_RX_NUM);//��ʼһ��DMA����	  
			while(1)
			{
			if(DMA1->HISR&(1<<5))	
			 {
				
				DMA1->HIFCR|=1<<5;	
				break; 
			 }

			}	
			
			//USART2_flag=0; 
			USART2_RX_NUM=0;

		}
		delay_ms(30); 
}

void RoverTransfer(void){
    int voice_flag;
		static u8 times_flag=0;//��̬�ֲ���¼���봫������ѭ���Ĵ���

		
		if(USART2_RX_NUM>0)//ת���忨����
		{	
			
			if(times_flag==0)//����������ѹ����
			{
			times_flag=1;	
			voice_flag=7;
			OSMboxPost(msg_broad,(void*)voice_flag);//�ƶ�վ������·������ʾ
			delay_ms(20);	
			
			}	
		
			DMA_TxConfig(DMA1_Stream3,4,(u32)&USART3->TDR,(u32)USART2_RX_BUF,USART2_RX_NUM);//�忨COM2����2����ת��������3������   
			MYDMA_Enable(DMA1_Stream3,USART2_RX_NUM);//��ʼһ��DMA����	  
			while(1)
			{
			if(DMA1->LISR&(1<<27))	
			 {
				
				DMA1->LIFCR|=1<<27;	
				break; 
			 }

			}	
			
 
			USART2_RX_NUM=0;

		}
		if(UART4_RX_NUM>0)//����4����RTCM����ת��������5�忨COM1
		{
			DMA_TxConfig(DMA1_Stream7,4,(u32)&UART5->TDR,(u32)UART4_RX_BUF,UART4_RX_NUM);
			MYDMA_Enable(DMA1_Stream7,UART4_RX_NUM);//��ʼһ��DMA����	  
			while(1)
			{
			if(DMA1->HISR&(1<<27))	
			 {
				
				DMA1->HIFCR|=1<<27;	
				break; 
			 }

			}				

			UART4_RX_NUM=0;		
		
		}
		delay_ms(30); 
}









#include "TRANSFER.h" 
#include "DMA.h" 
#include "SC50.h"
#include "sys.h"
#include "delay.h"
#include "usart.h"
#include "includes.h"

extern OS_EVENT * msg_broad;			//广播邮箱块指针；

#define USART_REC_LEN  			800  	//定义最大接收字节数 200
extern u8  USART2_RX_BUF[USART_REC_LEN]; //接收缓冲,最大USART_REC_LEN个字节
extern u8  USART3_RX_BUF[USART_REC_LEN]; //接收缓冲,最大USART_REC_LEN个字节
extern u8  UART4_RX_BUF[USART_REC_LEN]; //接收缓冲,最大USART_REC_LEN个字节

void BaseTransfer(void){
    int voice_flag;
		static u8 times_flag=0;//静态局部记录进入传输任务循环的次数

		
		if(USART2_RX_NUM>0)//转发板卡数据
		{	
			
		if(times_flag==0)//触发语音电压报警
		{
		times_flag=1;	
		voice_flag=4;
		OSMboxPost(msg_broad,(void*)voice_flag);//基站定位已锁定语音提示
		delay_ms(20);	
		
		}	
		
			DMA_TxConfig(DMA1_Stream4,4,(u32)&UART4->TDR,(u32)USART2_RX_BUF,USART2_RX_NUM);//板卡COM2串口2数据转发至数传串口4
			MYDMA_Enable(DMA1_Stream4,USART2_RX_NUM);//开始一次DMA传输	  
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
		static u8 times_flag=0;//静态局部记录进入传输任务循环的次数

		
		if(USART2_RX_NUM>0)//转发板卡数据
		{	
			
			if(times_flag==0)//触发语音电压报警
			{
			times_flag=1;	
			voice_flag=7;
			OSMboxPost(msg_broad,(void*)voice_flag);//移动站蓝牙链路语音提示
			delay_ms(20);	
			
			}	
		
			DMA_TxConfig(DMA1_Stream3,4,(u32)&USART3->TDR,(u32)USART2_RX_BUF,USART2_RX_NUM);//板卡COM2串口2数据转发至串口3蓝牙端   
			MYDMA_Enable(DMA1_Stream3,USART2_RX_NUM);//开始一次DMA传输	  
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
		if(UART4_RX_NUM>0)//串口4数传RTCM数据转发至串口5板卡COM1
		{
			DMA_TxConfig(DMA1_Stream7,4,(u32)&UART5->TDR,(u32)UART4_RX_BUF,UART4_RX_NUM);
			MYDMA_Enable(DMA1_Stream7,UART4_RX_NUM);//开始一次DMA传输	  
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









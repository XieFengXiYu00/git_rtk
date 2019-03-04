#include "sys.h"
#include "usart.h"	  
////////////////////////////////////////////////////////////////////////////////// 	 
//如果使用ucos,则包括下面的头文件即可.
#if SYSTEM_SUPPORT_OS
#include "includes.h"					//ucos 使用	  
#endif
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

//加入以下代码,支持printf函数,而不需要选择use MicroLIB	  
#if 1
#pragma import(__use_no_semihosting)  
//解决HAL库使用时,某些情况可能报错的bug
int _ttywrch(int ch)    
{
    ch=ch;
	return ch;
}
//标准库需要的支持函数                 
struct __FILE 
{ 
	int handle; 
	/* Whatever you require here. If the only file you are using is */ 
	/* standard output using printf() for debugging, no file handling */ 
	/* is required. */ 
}; 
/* FILE is typedef’ d in stdio.h. */ 
FILE __stdout;       
//定义_sys_exit()以避免使用半主机模式    
void _sys_exit(int x) 
{ 
	x = x; 
} 
//重定义fputc函数 
int fputc(int ch, FILE *f)
{      
	while((USART1->ISR&0X40)==0);//循环发送,直到发送完毕   
	USART1->TDR = (u8) ch;      
	return ch;
}
#endif 
//end
//////////////////////////////////////////////////////////////////

#if EN_USART1_RX   //如果使能了接收
//串口1中断服务程序
//注意,读取USARTx->SR能避免莫名其妙的错误   	
u8 USART_RX_BUF[USART_REC_LEN];     //接收缓冲,最大USART_REC_LEN个字节.
//接收状态
//bit15，	接收完成标志
//bit14，	接收到0x0d
//bit13~0，	接收到的有效字节数目
u16 USART_RX_STA=0;       //接收状态标记	  
  
void USART1_IRQHandler(void)
{
	u8 res;	
#if SYSTEM_SUPPORT_OS 		//如果SYSTEM_SUPPORT_OS为真，则需要支持OS.
	OSIntEnter();    
#endif
	if(USART1->ISR&(1<<5))//接收到数据
	{	 
		res=USART1->RDR; 
		if((USART_RX_STA&0x8000)==0)//接收未完成
		{
			if(USART_RX_STA&0x4000)//接收到了0x0d
			{
				if(res!=0x0a)USART_RX_STA=0;//接收错误,重新开始
				else USART_RX_STA|=0x8000;	//接收完成了 
			}else //还没收到0X0D
			{	
				if(res==0x0d)USART_RX_STA|=0x4000;
				else
				{
					USART_RX_BUF[USART_RX_STA&0X3FFF]=res;
					USART_RX_STA++;
					if(USART_RX_STA>(USART_REC_LEN-1))USART_RX_STA=0;//接收数据错误,重新开始接收	  
				}		 
			}
		}  		 									     
	} 
#if SYSTEM_SUPPORT_OS 	//如果SYSTEM_SUPPORT_OS为真，则需要支持OS.
	OSIntExit();  											 
#endif
} 
#endif


//////////////////////////////////////////////////////////////////

//#if EN_USART2_RX   //如果使能了接收
////串口2中断服务程序
////注意,读取USARTx->SR能避免莫名其妙的错误   	
//u8  USART2_RX_BUF[USART_REC_LEN]; //接收缓冲,最大USART_REC_LEN个字节
//u16 USART2_RX_NUM;         		//接收数组数量
//u8  USART2_flag;

//void USART2_IRQHandler(void)
//{
//	u8 res;	
//#if SYSTEM_SUPPORT_OS 		//如果SYSTEM_SUPPORT_OS为真，则需要支持OS.
//	OSIntEnter();    
//#endif
//	if(USART2->ISR&(1<<5))//接收到数据
//	{	 
//		res=USART2->RDR;
//    USART2_RX_BUF[USART2_RX_NUM++]=res;

//	}
//  else if(USART2->ISR&(1<<4))//不是第一次接收到完整一帧
//	{
//		USART2->ICR|=1<<4;
//		USART2_flag=1;

//	}

//	
//#if SYSTEM_SUPPORT_OS 	//如果SYSTEM_SUPPORT_OS为真，则需要支持OS.
//	OSIntExit();  											 
//#endif
//} 
//#endif
#if EN_USART2_RX   //如果使能了接收
u16 USART2_RX_NUM;
void USART2_IRQHandler(void)
{
  // u16 USART2_RX_NUM; 
	#if SYSTEM_SUPPORT_OS 		//如果SYSTEM_SUPPORT_OS为真，则需要支持OS.
	OSIntEnter();    
#endif
	if(USART2->ISR&(1<<4))//接收到数据
	{	 
		USART2->ICR|=1<<4;    //清除空闲中断标志位
		USART2->RDR;          //读取接收寄存器 清除接收中断标志位 
		
    SCB_CleanInvalidateDCache();

		USART2_RX_NUM=USART_REC_LEN-DMA1_Stream5->NDTR;	//得到当前还剩余多少个数据

		DMA1_Stream5->CR&=~(1<<0); 	//关闭DMA传输 
		while(DMA1_Stream5->CR&0X1);//确保DMA可以被设置	
		DMA1_Stream5->NDTR = USART_REC_LEN;		//DMA 存储器0地址 
		DMA1_Stream5->CR|=1<<0;		//开启DMA传输
								     
	} 
#if SYSTEM_SUPPORT_OS 	//如果SYSTEM_SUPPORT_OS为真，则需要支持OS.
	OSIntExit();  											 
#endif
} 
#endif	


#if EN_USART3_RX   //如果使能了接收
//串口1中断服务程序
//注意,读取USARTx->SR能避免莫名其妙的错误   	
u16 USART3_RX_NUM=0;       //接收数目
u8  USART3_flag;
u8  USART3_Start_flag;

void USART3_IRQHandler(void)
{
//	u16 USART3_RX_NUM;
#if SYSTEM_SUPPORT_OS 		//如果SYSTEM_SUPPORT_OS为真，则需要支持OS.
	OSIntEnter();    
#endif
	if(USART3->ISR&(1<<4))//接收到数据
	{	 
		USART3->ICR|=1<<4;    //清除空闲中断标志位
		USART3->RDR;          //读取接收寄存器 清除接收中断标志位 
		
    SCB_CleanInvalidateDCache();

		USART3_RX_NUM=USART_REC_LEN-DMA1_Stream1->NDTR;	//得到当前还剩余多少个数据

		DMA1_Stream1->CR&=~(1<<0); 	//关闭DMA传输 
		while(DMA1_Stream1->CR&0X1);//确保DMA可以被设置	
		DMA1_Stream1->NDTR=USART_REC_LEN;		//DMA 存储器0地址 
		DMA1_Stream1->CR|=1<<0;		//开启DMA传输
								     
	} 
//if(USART3->ISR&(1<<3)) UART4->ICR|=1<<3;
	
	
#if SYSTEM_SUPPORT_OS 	//如果SYSTEM_SUPPORT_OS为真，则需要支持OS.
	OSIntExit();  											 
#endif
} 
#endif


#if EN_UART4_RX   //如果使能了接收
//串口1中断服务程序
//注意,读取USARTx->SR能避免莫名其妙的错误   	
u16 UART4_RX_NUM=0;       //接收数目
u8  UART4_flag;

void UART4_IRQHandler(void)
{
//	u8 res;	
#if SYSTEM_SUPPORT_OS 		//如果SYSTEM_SUPPORT_OS为真，则需要支持OS.
	OSIntEnter();    
#endif
	if(UART4->ISR&(1<<4))//接收到数据
	{	 
		UART4->ICR|=1<<4;    //清除空闲中断标志位
		UART4->RDR;          //读取接收寄存器 清除接收中断标志位 
		
    SCB_CleanInvalidateDCache();

		UART4_RX_NUM=USART_REC_LEN-DMA1_Stream2->NDTR;	//得到当前还剩余多少个数据

		DMA1_Stream2->CR&=~(1<<0); 	//关闭DMA传输 
		while(DMA1_Stream2->CR&0X1);//确保DMA可以被设置	
		DMA1_Stream2->NDTR=USART_REC_LEN;		//DMA 存储器0地址 
		DMA1_Stream2->CR|=1<<0;		//开启DMA传输
								     
	} 


	
	
#if SYSTEM_SUPPORT_OS 	//如果SYSTEM_SUPPORT_OS为真，则需要支持OS.
	OSIntExit();  											 
#endif
} 
#endif

//串口5中断服务程序
void UART5_IRQHandler(void)                	
{ 
	

    if(UART5->ISR& 1<<4)//是否进入空闲中断 若是，则表示数据已经接收完成
    {
		UART5->ICR|=1<<4;    //清除空闲中断标志位
		UART5->RDR;          //读取接收寄存器 清除接收中断标志位 
		
	
//		DMA1_Stream0->CR&=~(1<<0); 	//关闭DMA传输 
//		while(DMA1_Stream0->CR&0X1);//确保DMA可以被设置	
//		DMA1_Stream0->NDTR=REC_BUF_SIZE_UART5;		//DMA 存储器0地址 
//		DMA1_Stream0->CR|=1<<0;		//开启DMA传输
		
	}
}








//初始化IO 串口1
//pclk2:PCLK2时钟频率(Mhz)
//bound:波特率 
void uart1_init(u32 pclk2,u32 bound)
{  	 
	u32	temp;	   
	temp=(pclk2*1000000+bound/2)/bound;	//得到USARTDIV@OVER8=0,采用四舍五入计算
	RCC->AHB1ENR|=1<<0;   	//使能PORTA口时钟  
	RCC->APB2ENR|=1<<4;  	//使能串口1时钟 
	GPIO_Set(GPIOA,PIN9|PIN10,GPIO_MODE_AF,GPIO_OTYPE_PP,GPIO_SPEED_50M,GPIO_PUPD_PU);//PA9,PA10,复用功能,上拉输出
 	GPIO_AF_Set(GPIOA,9,7);	//PA9,AF7
	GPIO_AF_Set(GPIOA,10,7);//PA10,AF7  	   
	//波特率设置
 	USART1->BRR=temp; 		//波特率设置@OVER8=0 	
	USART1->CR1=0;		 	//清零CR1寄存器
	USART1->CR1|=0<<28;	 	//设置M1=0
	USART1->CR1|=0<<12;	 	//设置M0=0&M1=0,选择8位字长 
	USART1->CR1|=0<<15; 	//设置OVER8=0,16倍过采样 
	USART1->CR1|=1<<3;  	//串口发送使能 
	
	
	
#if EN_USART1_RX		  	//如果使能了接收
	//使能接收中断 
	USART1->CR1|=1<<2;  	//串口接收使能
	USART1->CR1|=1<<5;    	//接收缓冲区非空中断使能	    	
	MY_NVIC_Init(6,0,USART1_IRQn,4);//组4，最低优先级
#endif
	USART1->CR1|=1<<0;  	//串口使能
}

//初始化IO 串口2
//bound:波特率

void uart2_init(u32 pclk2,u32 bound)
{    
	u32	temp;		
	temp=(pclk2*1000000+bound/2)/bound;	//得到USARTDIV@OVER8=0,采用四舍五入计算
        RCC->APB1ENR|=1<<0;           //??PORTA???  
				RCC->APB1ENR|=1<<17;  	//使能串口2时钟 
				GPIO_Set(GPIOA,PIN2|PIN3,GPIO_MODE_AF,GPIO_OTYPE_PP,GPIO_SPEED_50M,GPIO_PUPD_PU);//PA2,PA3,复用功能，下拉输出
				GPIO_AF_Set(GPIOA,2,7);	//PA9,AF7
				GPIO_AF_Set(GPIOA,3,7);//PA10,AF7  	     
	//波特率设置
	
 	USART2->BRR=temp; 		//波特率设置@OVER8=0 	
	USART2->CR1=0;		 	//清零CR1寄存器
	USART2->CR1|=0<<28;	 	//设置M1=0
	USART2->CR1|=0<<12;	 	//设置M0=0&M1=0,选择8位字长 
	USART2->CR1|=0<<15; 	//设置OVER8=0,16倍过采样 
	USART2->CR1|=1<<3;  	//串口发送使能 
	
	
#if EN_USART2_RX==1		  	//如果使能了接收
	//使能接收中断 
   // USART2->CR1|=1<<2;  	//串口接收使能
	  //USART2CR1=USART2->CR1;
	
    USART2->CR1|=1<<2;  	//串口接收使能
	  USART2->CR1|=1<<4;  	//串口空闲中断
	
		USART2->CR3|=USART_CR3_DMAT;	//串口2 DMA发送使能
    USART2->CR3|=USART_CR3_DMAR;  	//串口2 DMA接收使能
	
	MY_NVIC_Init(7,0,USART2_IRQn,4);//组4，最低优先级
#endif
	USART2->CR1|=1<<0;  	//串口使能  

}
//初始化IO 串口3
//bound:波特率
//初始化IO 串口3 
//bound:波特率
void uart3_init(u32 pclk1,u32 bound)
{	
	//UART 初始化设置
	u32	temp;	   
	temp=(pclk1*1000000+bound/2)/bound;	//得到USARTDIV@OVER8=0,采用四舍五入计算
	RCC->AHB1ENR|=1<<1;   	//使能PORTB口时钟  
	RCC->APB1ENR|=1<<18;  	//使能串口3时钟 
	GPIO_Set(GPIOB,PIN10|PIN11,GPIO_MODE_AF,GPIO_OTYPE_PP,GPIO_SPEED_50M,GPIO_PUPD_PU);//PB10,PB11,复用功能,上拉输出
 	GPIO_AF_Set(GPIOB,10,7);//PB10,AF7
	GPIO_AF_Set(GPIOB,11,7);//PB11,AF7  	   
	
	//波特率设置
 	USART3->BRR=temp; 		//波特率设置@OVER8=0 	
	USART3->CR1=0;		 	//清零CR1寄存器
	USART3->CR1|=0<<28;	 	//设置M1=0
	USART3->CR1|=0<<12;	 	//设置M0=0&M1=0,选择8位字长 
	USART3->CR1|=0<<15; 	//设置OVER8=0,16倍过采样 
	USART3->CR1|=1<<3;  	//串口发送使能 
 
	USART3->CR1|=1<<2;  	         //串口接收使能
	USART3->CR1|= USART_CR1_IDLEIE;  //串口接收空闲中断使能
	USART3->CR1|=1<<5;    //接收缓冲区非空中断使能
	USART3->CR3|=USART_CR3_DMAT;	   //串口DMA发送使能
	USART3->CR3|=USART_CR3_DMAR;    //串口DMA接收使能   	
	
	MY_NVIC_Init(8,0,USART3_IRQn,4);
    
	
	USART3->CR1|=1<<0;  	        //串口3 使能
}


//初始化IO 串口4 
//bound:波特率
void uart4_init(u32 pclk2,u32 bound)
{	
	//UART 初始化设置
	u32	temp;	   
	temp=(pclk2*1000000+bound/2)/bound;	//得到USARTDIV@OVER8=0,采用四舍五入计算
	RCC->AHB1ENR|=1<<2;   	//使能PORTC口时钟  
	RCC->APB1ENR|=1<<19;  	//使能串口4时钟 
	GPIO_Set(GPIOC,PIN10|PIN11,GPIO_MODE_AF,GPIO_OTYPE_PP,GPIO_SPEED_50M,GPIO_PUPD_PU);//PC10,PC11,复用功能,上拉输出
 	GPIO_AF_Set(GPIOC,10,8);//PC10,AF8
	GPIO_AF_Set(GPIOC,11,8);//PC11,AF8  	   
	
	//波特率设置
 	UART4->BRR=temp; 		//波特率设置@OVER8=0 	
	UART4->CR1=0;		 	//清零CR1寄存器
	UART4->CR1|=0<<28;	 	//设置M1=0
	UART4->CR1|=0<<12;	 	//设置M0=0&M1=0,选择8位字长 
	UART4->CR1|=0<<15; 	    //设置OVER8=0,16倍过采样 
	UART4->CR1|=1<<3;  	    //串口发送使能
	
	UART4->CR1|=1<<2;  	    //串口接收使能
	
	UART4->CR3|=USART_CR3_DMAT;	   //串口4 DMA发送使能
	UART4->CR3|=USART_CR3_DMAR;    //串口4 DMA接收使能
 
	UART4->CR1|=1<<5;  	//串口空闲中断	
	UART4->CR1|= USART_CR1_IDLEIE;  //串口接收空闲中断使能
  MY_NVIC_Init(9,0,UART4_IRQn,4);//组4，最低优先级

	UART4->CR1|=1<<0;  	        //串口4 使能
}




//初始化IO 串口5 
//bound:波特率
void uart5_init(u32 pclk1,u32 bound)
{	
	//UART 初始化设置
	u32	temp;	   
	temp=(pclk1*1000000+bound/2)/bound;	//得到USARTDIV@OVER8=0,采用四舍五入计算
	RCC->AHB1ENR|=1<<1;   	//使能PORTB口时钟  
	RCC->APB1ENR|=1<<20;  	//使能串口5时钟 
	GPIO_Set(GPIOB,PIN8|PIN9,GPIO_MODE_AF,GPIO_OTYPE_PP,GPIO_SPEED_50M,GPIO_PUPD_PU);//PB8,PB9,复用功能,上拉输出
 	GPIO_AF_Set(GPIOB,8,7);
	GPIO_AF_Set(GPIOB,9,7);  	   
	
	//波特率设置
 	UART5->BRR=temp; 		//波特率设置@OVER8=0 	
	UART5->CR1=0;		 	//清零CR1寄存器
	UART5->CR1|=0<<28;	 	//设置M1=0
	UART5->CR1|=0<<12;	 	//设置M0=0&M1=0,选择8位字长 
	UART5->CR1|=0<<15; 	    //设置OVER8=0,16倍过采样 
	UART5->CR1|=1<<3;  	    //串口发送使能 
	UART5->CR1|=1<<2;  	    //串口接收使能
	UART5->CR3|=USART_CR3_DMAT;	   //DMA发送使能
	UART5->CR3|=USART_CR3_DMAR;    //DMA接收使能
	
   		
	MY_NVIC_Init(10,0,UART5_IRQn,4);//组4，最低优先级
    
	UART5->CR1|=1<<0;  	        //串口5 使能
}

//初始化IO 串口6 
//bound:波特率
//void uart6_init(u32 pclk2,u32 bound)
//{	
//	//UART 初始化设置
//	u32	temp;	   
//	temp=(pclk2*1000000+bound/2)/bound;	//得到USARTDIV@OVER8=0,采用四舍五入计算
//	RCC->AHB1ENR|=1<<2;   	//使能PORTC口时钟  
//	RCC->APB2ENR|=1<<5;  	//使能串口6时钟 
//	GPIO_Set(GPIOC,PIN6|PIN7,GPIO_MODE_AF,GPIO_OTYPE_PP,GPIO_SPEED_50M,GPIO_PUPD_PU);//PC6,PC7,复用功能,上拉输出
// 	GPIO_AF_Set(GPIOC,6,8);	//PC6,AF8
//	GPIO_AF_Set(GPIOC,7,8);//PC7,AF8 	   
//	
//	//波特率设置
// 	USART6->BRR=temp; 		//波特率设置@OVER8=0 	
//	USART6->CR1=0;		 	//清零CR1寄存器
//	USART6->CR1|=0<<28;	 	//设置M1=0
//	USART6->CR1|=0<<12;	 	//设置M0=0&M1=0,选择8位字长 
//	USART6->CR1|=0<<15; 	    //设置OVER8=0,16倍过采样 
//	USART6->CR1|=1<<3;  	    //串口发送使能 

//	//使能接收中断 
//	USART6->CR1|=1<<2;  	            //串口接收使能
//	USART6->CR1|= USART_CR1_IDLEIE;  //串口接收空闲中断使能
//   	
//	
//	MY_NVIC_Init(11,0,USART6_IRQn,4);//组4，最低优先级
//    
//	USART6->CR3|=USART_CR3_DMAT;	   //串口6 DMA发送使能
//    USART6->CR3|=USART_CR3_DMAR;    //串口6 DMA接收使能
//	
//	USART6->CR1|=1<<0;  	        //串口6 使能
//}

//初始化IO 串口7 
//bound:波特率

//void uart7_init(u32 pclk1,u32 bound)
//{	
//	//UART 初始化设置
//	u32	temp;	   
//	temp=(pclk1*1000000+bound/2)/bound;	//得到USARTDIV@OVER8=0,采用四舍五入计算
//	RCC->AHB1ENR|=1<<4;   	//使能PORTE口时钟  
//	RCC->APB1ENR|=1<<30;  	//使能串口7时钟 
//	GPIO_Set(GPIOE,PIN7|PIN8,GPIO_MODE_AF,GPIO_OTYPE_PP,GPIO_SPEED_50M,GPIO_PUPD_PU);//PE7,PE8,复用功能,上拉输出
// 	GPIO_AF_Set(GPIOE,7,8);	//PE7,AF8
//	GPIO_AF_Set(GPIOE,8,8);//PE8,AF8  	   
//	
//	//波特率设置
// 	UART7->BRR=temp; 		//波特率设置@OVER8=0 	
//	UART7->CR1=0;		 	//清零CR1寄存器
//	UART7->CR1|=0<<28;	 	//设置M1=0
//	UART7->CR1|=0<<12;	 	//设置M0=0&M1=0,选择8位字长 
//	UART7->CR1|=0<<15; 	    //设置OVER8=0,16倍过采样 
//	UART7->CR1|=1<<3;  	    //串口发送使能 

//	//使能接收中断 
//	UART7->CR1|=1<<2;  	            //串口接收使能
//	UART7->CR1|= USART_CR1_IDLEIE;  //串口接收空闲中断使能
//   	
//	
//	MY_NVIC_Init(12,0,UART7_IRQn,4);//组4，最低优先级
//    
//	//UART7->CR3|=USART_CR3_DMAT;	    //串口7 DMA发送使能
//    UART7->CR3|=USART_CR3_DMAR;    //串口7 DMA接收使能
//	
//	UART7->CR1|=1<<0;  	        //串口7 使能
//}

//初始化IO 串口8 
//bound:波特率
//void uart8_init(u32 pclk1,u32 bound)
//{	
//	//UART 初始化设置
//	u32	temp;	   
//	temp=(pclk1*1000000+bound/2)/bound;	//得到USARTDIV@OVER8=0,采用四舍五入计算
//	RCC->AHB1ENR|=1<<4;   	//使能PORTE口时钟  
////	RCC->APB1ENR|=1<<31;  	//使能串口8时钟 (与下条语句的作用一样)
//	RCC->APB1ENR|=0x80000000;//使能串口8时钟 (使用这种方式使能串口8，是为 消除编译器警告)
//	GPIO_Set(GPIOE,PIN0|PIN1,GPIO_MODE_AF,GPIO_OTYPE_PP,GPIO_SPEED_50M,GPIO_PUPD_PU);//PE0,PE1,复用功能,上拉输出
// 	GPIO_AF_Set(GPIOE,0,8);	//PE0,AF8
//	GPIO_AF_Set(GPIOE,1,8);//PE1,AF8  	   
//	
//	//波特率设置
// 	UART8->BRR=temp; 		//波特率设置@OVER8=0 	
//	UART8->CR1=0;		 	//清零CR1寄存器
//	UART8->CR1|=0<<28;	 	//设置M1=0
//	UART8->CR1|=0<<12;	 	//设置M0=0&M1=0,选择8位字长 
//	UART8->CR1|=0<<15; 	    //设置OVER8=0,16倍过采样 
//	UART8->CR1|=1<<3;  	    //串口发送使能 

//	//使能接收中断 
////	UART8->CR1|=1<<2;  	            //串口接收使能
////	UART8->CR1|= USART_CR1_IDLEIE;  //串口接收空闲中断使能
////	UART8->CR1|=1<<5;    	        //接收缓冲区非空中断使能  如果开启则每收到一个字节就会进入一次中断
//	
////	MY_NVIC_Init(13,0,UART8_IRQn,4);//组4，最低优先级
//	
//	UART8->CR1|=1<<0;  	        //串口8 使能
//}











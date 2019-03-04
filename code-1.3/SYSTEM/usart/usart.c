#include "sys.h"
#include "usart.h"	  
////////////////////////////////////////////////////////////////////////////////// 	 
//���ʹ��ucos,����������ͷ�ļ�����.
#if SYSTEM_SUPPORT_OS
#include "includes.h"					//ucos ʹ��	  
#endif
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

//�������´���,֧��printf����,������Ҫѡ��use MicroLIB	  
#if 1
#pragma import(__use_no_semihosting)  
//���HAL��ʹ��ʱ,ĳЩ������ܱ����bug
int _ttywrch(int ch)    
{
    ch=ch;
	return ch;
}
//��׼����Ҫ��֧�ֺ���                 
struct __FILE 
{ 
	int handle; 
	/* Whatever you require here. If the only file you are using is */ 
	/* standard output using printf() for debugging, no file handling */ 
	/* is required. */ 
}; 
/* FILE is typedef�� d in stdio.h. */ 
FILE __stdout;       
//����_sys_exit()�Ա���ʹ�ð�����ģʽ    
void _sys_exit(int x) 
{ 
	x = x; 
} 
//�ض���fputc���� 
int fputc(int ch, FILE *f)
{      
	while((USART1->ISR&0X40)==0);//ѭ������,ֱ���������   
	USART1->TDR = (u8) ch;      
	return ch;
}
#endif 
//end
//////////////////////////////////////////////////////////////////

#if EN_USART1_RX   //���ʹ���˽���
//����1�жϷ������
//ע��,��ȡUSARTx->SR�ܱ���Ī������Ĵ���   	
u8 USART_RX_BUF[USART_REC_LEN];     //���ջ���,���USART_REC_LEN���ֽ�.
//����״̬
//bit15��	������ɱ�־
//bit14��	���յ�0x0d
//bit13~0��	���յ�����Ч�ֽ���Ŀ
u16 USART_RX_STA=0;       //����״̬���	  
  
void USART1_IRQHandler(void)
{
	u8 res;	
#if SYSTEM_SUPPORT_OS 		//���SYSTEM_SUPPORT_OSΪ�棬����Ҫ֧��OS.
	OSIntEnter();    
#endif
	if(USART1->ISR&(1<<5))//���յ�����
	{	 
		res=USART1->RDR; 
		if((USART_RX_STA&0x8000)==0)//����δ���
		{
			if(USART_RX_STA&0x4000)//���յ���0x0d
			{
				if(res!=0x0a)USART_RX_STA=0;//���մ���,���¿�ʼ
				else USART_RX_STA|=0x8000;	//��������� 
			}else //��û�յ�0X0D
			{	
				if(res==0x0d)USART_RX_STA|=0x4000;
				else
				{
					USART_RX_BUF[USART_RX_STA&0X3FFF]=res;
					USART_RX_STA++;
					if(USART_RX_STA>(USART_REC_LEN-1))USART_RX_STA=0;//�������ݴ���,���¿�ʼ����	  
				}		 
			}
		}  		 									     
	} 
#if SYSTEM_SUPPORT_OS 	//���SYSTEM_SUPPORT_OSΪ�棬����Ҫ֧��OS.
	OSIntExit();  											 
#endif
} 
#endif


//////////////////////////////////////////////////////////////////

//#if EN_USART2_RX   //���ʹ���˽���
////����2�жϷ������
////ע��,��ȡUSARTx->SR�ܱ���Ī������Ĵ���   	
//u8  USART2_RX_BUF[USART_REC_LEN]; //���ջ���,���USART_REC_LEN���ֽ�
//u16 USART2_RX_NUM;         		//������������
//u8  USART2_flag;

//void USART2_IRQHandler(void)
//{
//	u8 res;	
//#if SYSTEM_SUPPORT_OS 		//���SYSTEM_SUPPORT_OSΪ�棬����Ҫ֧��OS.
//	OSIntEnter();    
//#endif
//	if(USART2->ISR&(1<<5))//���յ�����
//	{	 
//		res=USART2->RDR;
//    USART2_RX_BUF[USART2_RX_NUM++]=res;

//	}
//  else if(USART2->ISR&(1<<4))//���ǵ�һ�ν��յ�����һ֡
//	{
//		USART2->ICR|=1<<4;
//		USART2_flag=1;

//	}

//	
//#if SYSTEM_SUPPORT_OS 	//���SYSTEM_SUPPORT_OSΪ�棬����Ҫ֧��OS.
//	OSIntExit();  											 
//#endif
//} 
//#endif
#if EN_USART2_RX   //���ʹ���˽���
u16 USART2_RX_NUM;
void USART2_IRQHandler(void)
{
  // u16 USART2_RX_NUM; 
	#if SYSTEM_SUPPORT_OS 		//���SYSTEM_SUPPORT_OSΪ�棬����Ҫ֧��OS.
	OSIntEnter();    
#endif
	if(USART2->ISR&(1<<4))//���յ�����
	{	 
		USART2->ICR|=1<<4;    //��������жϱ�־λ
		USART2->RDR;          //��ȡ���ռĴ��� ��������жϱ�־λ 
		
    SCB_CleanInvalidateDCache();

		USART2_RX_NUM=USART_REC_LEN-DMA1_Stream5->NDTR;	//�õ���ǰ��ʣ����ٸ�����

		DMA1_Stream5->CR&=~(1<<0); 	//�ر�DMA���� 
		while(DMA1_Stream5->CR&0X1);//ȷ��DMA���Ա�����	
		DMA1_Stream5->NDTR = USART_REC_LEN;		//DMA �洢��0��ַ 
		DMA1_Stream5->CR|=1<<0;		//����DMA����
								     
	} 
#if SYSTEM_SUPPORT_OS 	//���SYSTEM_SUPPORT_OSΪ�棬����Ҫ֧��OS.
	OSIntExit();  											 
#endif
} 
#endif	


#if EN_USART3_RX   //���ʹ���˽���
//����1�жϷ������
//ע��,��ȡUSARTx->SR�ܱ���Ī������Ĵ���   	
u16 USART3_RX_NUM=0;       //������Ŀ
u8  USART3_flag;
u8  USART3_Start_flag;

void USART3_IRQHandler(void)
{
//	u16 USART3_RX_NUM;
#if SYSTEM_SUPPORT_OS 		//���SYSTEM_SUPPORT_OSΪ�棬����Ҫ֧��OS.
	OSIntEnter();    
#endif
	if(USART3->ISR&(1<<4))//���յ�����
	{	 
		USART3->ICR|=1<<4;    //��������жϱ�־λ
		USART3->RDR;          //��ȡ���ռĴ��� ��������жϱ�־λ 
		
    SCB_CleanInvalidateDCache();

		USART3_RX_NUM=USART_REC_LEN-DMA1_Stream1->NDTR;	//�õ���ǰ��ʣ����ٸ�����

		DMA1_Stream1->CR&=~(1<<0); 	//�ر�DMA���� 
		while(DMA1_Stream1->CR&0X1);//ȷ��DMA���Ա�����	
		DMA1_Stream1->NDTR=USART_REC_LEN;		//DMA �洢��0��ַ 
		DMA1_Stream1->CR|=1<<0;		//����DMA����
								     
	} 
//if(USART3->ISR&(1<<3)) UART4->ICR|=1<<3;
	
	
#if SYSTEM_SUPPORT_OS 	//���SYSTEM_SUPPORT_OSΪ�棬����Ҫ֧��OS.
	OSIntExit();  											 
#endif
} 
#endif


#if EN_UART4_RX   //���ʹ���˽���
//����1�жϷ������
//ע��,��ȡUSARTx->SR�ܱ���Ī������Ĵ���   	
u16 UART4_RX_NUM=0;       //������Ŀ
u8  UART4_flag;

void UART4_IRQHandler(void)
{
//	u8 res;	
#if SYSTEM_SUPPORT_OS 		//���SYSTEM_SUPPORT_OSΪ�棬����Ҫ֧��OS.
	OSIntEnter();    
#endif
	if(UART4->ISR&(1<<4))//���յ�����
	{	 
		UART4->ICR|=1<<4;    //��������жϱ�־λ
		UART4->RDR;          //��ȡ���ռĴ��� ��������жϱ�־λ 
		
    SCB_CleanInvalidateDCache();

		UART4_RX_NUM=USART_REC_LEN-DMA1_Stream2->NDTR;	//�õ���ǰ��ʣ����ٸ�����

		DMA1_Stream2->CR&=~(1<<0); 	//�ر�DMA���� 
		while(DMA1_Stream2->CR&0X1);//ȷ��DMA���Ա�����	
		DMA1_Stream2->NDTR=USART_REC_LEN;		//DMA �洢��0��ַ 
		DMA1_Stream2->CR|=1<<0;		//����DMA����
								     
	} 


	
	
#if SYSTEM_SUPPORT_OS 	//���SYSTEM_SUPPORT_OSΪ�棬����Ҫ֧��OS.
	OSIntExit();  											 
#endif
} 
#endif

//����5�жϷ������
void UART5_IRQHandler(void)                	
{ 
	

    if(UART5->ISR& 1<<4)//�Ƿ��������ж� ���ǣ����ʾ�����Ѿ��������
    {
		UART5->ICR|=1<<4;    //��������жϱ�־λ
		UART5->RDR;          //��ȡ���ռĴ��� ��������жϱ�־λ 
		
	
//		DMA1_Stream0->CR&=~(1<<0); 	//�ر�DMA���� 
//		while(DMA1_Stream0->CR&0X1);//ȷ��DMA���Ա�����	
//		DMA1_Stream0->NDTR=REC_BUF_SIZE_UART5;		//DMA �洢��0��ַ 
//		DMA1_Stream0->CR|=1<<0;		//����DMA����
		
	}
}








//��ʼ��IO ����1
//pclk2:PCLK2ʱ��Ƶ��(Mhz)
//bound:������ 
void uart1_init(u32 pclk2,u32 bound)
{  	 
	u32	temp;	   
	temp=(pclk2*1000000+bound/2)/bound;	//�õ�USARTDIV@OVER8=0,���������������
	RCC->AHB1ENR|=1<<0;   	//ʹ��PORTA��ʱ��  
	RCC->APB2ENR|=1<<4;  	//ʹ�ܴ���1ʱ�� 
	GPIO_Set(GPIOA,PIN9|PIN10,GPIO_MODE_AF,GPIO_OTYPE_PP,GPIO_SPEED_50M,GPIO_PUPD_PU);//PA9,PA10,���ù���,�������
 	GPIO_AF_Set(GPIOA,9,7);	//PA9,AF7
	GPIO_AF_Set(GPIOA,10,7);//PA10,AF7  	   
	//����������
 	USART1->BRR=temp; 		//����������@OVER8=0 	
	USART1->CR1=0;		 	//����CR1�Ĵ���
	USART1->CR1|=0<<28;	 	//����M1=0
	USART1->CR1|=0<<12;	 	//����M0=0&M1=0,ѡ��8λ�ֳ� 
	USART1->CR1|=0<<15; 	//����OVER8=0,16�������� 
	USART1->CR1|=1<<3;  	//���ڷ���ʹ�� 
	
	
	
#if EN_USART1_RX		  	//���ʹ���˽���
	//ʹ�ܽ����ж� 
	USART1->CR1|=1<<2;  	//���ڽ���ʹ��
	USART1->CR1|=1<<5;    	//���ջ������ǿ��ж�ʹ��	    	
	MY_NVIC_Init(6,0,USART1_IRQn,4);//��4��������ȼ�
#endif
	USART1->CR1|=1<<0;  	//����ʹ��
}

//��ʼ��IO ����2
//bound:������

void uart2_init(u32 pclk2,u32 bound)
{    
	u32	temp;		
	temp=(pclk2*1000000+bound/2)/bound;	//�õ�USARTDIV@OVER8=0,���������������
        RCC->APB1ENR|=1<<0;           //??PORTA???  
				RCC->APB1ENR|=1<<17;  	//ʹ�ܴ���2ʱ�� 
				GPIO_Set(GPIOA,PIN2|PIN3,GPIO_MODE_AF,GPIO_OTYPE_PP,GPIO_SPEED_50M,GPIO_PUPD_PU);//PA2,PA3,���ù��ܣ��������
				GPIO_AF_Set(GPIOA,2,7);	//PA9,AF7
				GPIO_AF_Set(GPIOA,3,7);//PA10,AF7  	     
	//����������
	
 	USART2->BRR=temp; 		//����������@OVER8=0 	
	USART2->CR1=0;		 	//����CR1�Ĵ���
	USART2->CR1|=0<<28;	 	//����M1=0
	USART2->CR1|=0<<12;	 	//����M0=0&M1=0,ѡ��8λ�ֳ� 
	USART2->CR1|=0<<15; 	//����OVER8=0,16�������� 
	USART2->CR1|=1<<3;  	//���ڷ���ʹ�� 
	
	
#if EN_USART2_RX==1		  	//���ʹ���˽���
	//ʹ�ܽ����ж� 
   // USART2->CR1|=1<<2;  	//���ڽ���ʹ��
	  //USART2CR1=USART2->CR1;
	
    USART2->CR1|=1<<2;  	//���ڽ���ʹ��
	  USART2->CR1|=1<<4;  	//���ڿ����ж�
	
		USART2->CR3|=USART_CR3_DMAT;	//����2 DMA����ʹ��
    USART2->CR3|=USART_CR3_DMAR;  	//����2 DMA����ʹ��
	
	MY_NVIC_Init(7,0,USART2_IRQn,4);//��4��������ȼ�
#endif
	USART2->CR1|=1<<0;  	//����ʹ��  

}
//��ʼ��IO ����3
//bound:������
//��ʼ��IO ����3 
//bound:������
void uart3_init(u32 pclk1,u32 bound)
{	
	//UART ��ʼ������
	u32	temp;	   
	temp=(pclk1*1000000+bound/2)/bound;	//�õ�USARTDIV@OVER8=0,���������������
	RCC->AHB1ENR|=1<<1;   	//ʹ��PORTB��ʱ��  
	RCC->APB1ENR|=1<<18;  	//ʹ�ܴ���3ʱ�� 
	GPIO_Set(GPIOB,PIN10|PIN11,GPIO_MODE_AF,GPIO_OTYPE_PP,GPIO_SPEED_50M,GPIO_PUPD_PU);//PB10,PB11,���ù���,�������
 	GPIO_AF_Set(GPIOB,10,7);//PB10,AF7
	GPIO_AF_Set(GPIOB,11,7);//PB11,AF7  	   
	
	//����������
 	USART3->BRR=temp; 		//����������@OVER8=0 	
	USART3->CR1=0;		 	//����CR1�Ĵ���
	USART3->CR1|=0<<28;	 	//����M1=0
	USART3->CR1|=0<<12;	 	//����M0=0&M1=0,ѡ��8λ�ֳ� 
	USART3->CR1|=0<<15; 	//����OVER8=0,16�������� 
	USART3->CR1|=1<<3;  	//���ڷ���ʹ�� 
 
	USART3->CR1|=1<<2;  	         //���ڽ���ʹ��
	USART3->CR1|= USART_CR1_IDLEIE;  //���ڽ��տ����ж�ʹ��
	USART3->CR1|=1<<5;    //���ջ������ǿ��ж�ʹ��
	USART3->CR3|=USART_CR3_DMAT;	   //����DMA����ʹ��
	USART3->CR3|=USART_CR3_DMAR;    //����DMA����ʹ��   	
	
	MY_NVIC_Init(8,0,USART3_IRQn,4);
    
	
	USART3->CR1|=1<<0;  	        //����3 ʹ��
}


//��ʼ��IO ����4 
//bound:������
void uart4_init(u32 pclk2,u32 bound)
{	
	//UART ��ʼ������
	u32	temp;	   
	temp=(pclk2*1000000+bound/2)/bound;	//�õ�USARTDIV@OVER8=0,���������������
	RCC->AHB1ENR|=1<<2;   	//ʹ��PORTC��ʱ��  
	RCC->APB1ENR|=1<<19;  	//ʹ�ܴ���4ʱ�� 
	GPIO_Set(GPIOC,PIN10|PIN11,GPIO_MODE_AF,GPIO_OTYPE_PP,GPIO_SPEED_50M,GPIO_PUPD_PU);//PC10,PC11,���ù���,�������
 	GPIO_AF_Set(GPIOC,10,8);//PC10,AF8
	GPIO_AF_Set(GPIOC,11,8);//PC11,AF8  	   
	
	//����������
 	UART4->BRR=temp; 		//����������@OVER8=0 	
	UART4->CR1=0;		 	//����CR1�Ĵ���
	UART4->CR1|=0<<28;	 	//����M1=0
	UART4->CR1|=0<<12;	 	//����M0=0&M1=0,ѡ��8λ�ֳ� 
	UART4->CR1|=0<<15; 	    //����OVER8=0,16�������� 
	UART4->CR1|=1<<3;  	    //���ڷ���ʹ��
	
	UART4->CR1|=1<<2;  	    //���ڽ���ʹ��
	
	UART4->CR3|=USART_CR3_DMAT;	   //����4 DMA����ʹ��
	UART4->CR3|=USART_CR3_DMAR;    //����4 DMA����ʹ��
 
	UART4->CR1|=1<<5;  	//���ڿ����ж�	
	UART4->CR1|= USART_CR1_IDLEIE;  //���ڽ��տ����ж�ʹ��
  MY_NVIC_Init(9,0,UART4_IRQn,4);//��4��������ȼ�

	UART4->CR1|=1<<0;  	        //����4 ʹ��
}




//��ʼ��IO ����5 
//bound:������
void uart5_init(u32 pclk1,u32 bound)
{	
	//UART ��ʼ������
	u32	temp;	   
	temp=(pclk1*1000000+bound/2)/bound;	//�õ�USARTDIV@OVER8=0,���������������
	RCC->AHB1ENR|=1<<1;   	//ʹ��PORTB��ʱ��  
	RCC->APB1ENR|=1<<20;  	//ʹ�ܴ���5ʱ�� 
	GPIO_Set(GPIOB,PIN8|PIN9,GPIO_MODE_AF,GPIO_OTYPE_PP,GPIO_SPEED_50M,GPIO_PUPD_PU);//PB8,PB9,���ù���,�������
 	GPIO_AF_Set(GPIOB,8,7);
	GPIO_AF_Set(GPIOB,9,7);  	   
	
	//����������
 	UART5->BRR=temp; 		//����������@OVER8=0 	
	UART5->CR1=0;		 	//����CR1�Ĵ���
	UART5->CR1|=0<<28;	 	//����M1=0
	UART5->CR1|=0<<12;	 	//����M0=0&M1=0,ѡ��8λ�ֳ� 
	UART5->CR1|=0<<15; 	    //����OVER8=0,16�������� 
	UART5->CR1|=1<<3;  	    //���ڷ���ʹ�� 
	UART5->CR1|=1<<2;  	    //���ڽ���ʹ��
	UART5->CR3|=USART_CR3_DMAT;	   //DMA����ʹ��
	UART5->CR3|=USART_CR3_DMAR;    //DMA����ʹ��
	
   		
	MY_NVIC_Init(10,0,UART5_IRQn,4);//��4��������ȼ�
    
	UART5->CR1|=1<<0;  	        //����5 ʹ��
}

//��ʼ��IO ����6 
//bound:������
//void uart6_init(u32 pclk2,u32 bound)
//{	
//	//UART ��ʼ������
//	u32	temp;	   
//	temp=(pclk2*1000000+bound/2)/bound;	//�õ�USARTDIV@OVER8=0,���������������
//	RCC->AHB1ENR|=1<<2;   	//ʹ��PORTC��ʱ��  
//	RCC->APB2ENR|=1<<5;  	//ʹ�ܴ���6ʱ�� 
//	GPIO_Set(GPIOC,PIN6|PIN7,GPIO_MODE_AF,GPIO_OTYPE_PP,GPIO_SPEED_50M,GPIO_PUPD_PU);//PC6,PC7,���ù���,�������
// 	GPIO_AF_Set(GPIOC,6,8);	//PC6,AF8
//	GPIO_AF_Set(GPIOC,7,8);//PC7,AF8 	   
//	
//	//����������
// 	USART6->BRR=temp; 		//����������@OVER8=0 	
//	USART6->CR1=0;		 	//����CR1�Ĵ���
//	USART6->CR1|=0<<28;	 	//����M1=0
//	USART6->CR1|=0<<12;	 	//����M0=0&M1=0,ѡ��8λ�ֳ� 
//	USART6->CR1|=0<<15; 	    //����OVER8=0,16�������� 
//	USART6->CR1|=1<<3;  	    //���ڷ���ʹ�� 

//	//ʹ�ܽ����ж� 
//	USART6->CR1|=1<<2;  	            //���ڽ���ʹ��
//	USART6->CR1|= USART_CR1_IDLEIE;  //���ڽ��տ����ж�ʹ��
//   	
//	
//	MY_NVIC_Init(11,0,USART6_IRQn,4);//��4��������ȼ�
//    
//	USART6->CR3|=USART_CR3_DMAT;	   //����6 DMA����ʹ��
//    USART6->CR3|=USART_CR3_DMAR;    //����6 DMA����ʹ��
//	
//	USART6->CR1|=1<<0;  	        //����6 ʹ��
//}

//��ʼ��IO ����7 
//bound:������

//void uart7_init(u32 pclk1,u32 bound)
//{	
//	//UART ��ʼ������
//	u32	temp;	   
//	temp=(pclk1*1000000+bound/2)/bound;	//�õ�USARTDIV@OVER8=0,���������������
//	RCC->AHB1ENR|=1<<4;   	//ʹ��PORTE��ʱ��  
//	RCC->APB1ENR|=1<<30;  	//ʹ�ܴ���7ʱ�� 
//	GPIO_Set(GPIOE,PIN7|PIN8,GPIO_MODE_AF,GPIO_OTYPE_PP,GPIO_SPEED_50M,GPIO_PUPD_PU);//PE7,PE8,���ù���,�������
// 	GPIO_AF_Set(GPIOE,7,8);	//PE7,AF8
//	GPIO_AF_Set(GPIOE,8,8);//PE8,AF8  	   
//	
//	//����������
// 	UART7->BRR=temp; 		//����������@OVER8=0 	
//	UART7->CR1=0;		 	//����CR1�Ĵ���
//	UART7->CR1|=0<<28;	 	//����M1=0
//	UART7->CR1|=0<<12;	 	//����M0=0&M1=0,ѡ��8λ�ֳ� 
//	UART7->CR1|=0<<15; 	    //����OVER8=0,16�������� 
//	UART7->CR1|=1<<3;  	    //���ڷ���ʹ�� 

//	//ʹ�ܽ����ж� 
//	UART7->CR1|=1<<2;  	            //���ڽ���ʹ��
//	UART7->CR1|= USART_CR1_IDLEIE;  //���ڽ��տ����ж�ʹ��
//   	
//	
//	MY_NVIC_Init(12,0,UART7_IRQn,4);//��4��������ȼ�
//    
//	//UART7->CR3|=USART_CR3_DMAT;	    //����7 DMA����ʹ��
//    UART7->CR3|=USART_CR3_DMAR;    //����7 DMA����ʹ��
//	
//	UART7->CR1|=1<<0;  	        //����7 ʹ��
//}

//��ʼ��IO ����8 
//bound:������
//void uart8_init(u32 pclk1,u32 bound)
//{	
//	//UART ��ʼ������
//	u32	temp;	   
//	temp=(pclk1*1000000+bound/2)/bound;	//�õ�USARTDIV@OVER8=0,���������������
//	RCC->AHB1ENR|=1<<4;   	//ʹ��PORTE��ʱ��  
////	RCC->APB1ENR|=1<<31;  	//ʹ�ܴ���8ʱ�� (��������������һ��)
//	RCC->APB1ENR|=0x80000000;//ʹ�ܴ���8ʱ�� (ʹ�����ַ�ʽʹ�ܴ���8����Ϊ ��������������)
//	GPIO_Set(GPIOE,PIN0|PIN1,GPIO_MODE_AF,GPIO_OTYPE_PP,GPIO_SPEED_50M,GPIO_PUPD_PU);//PE0,PE1,���ù���,�������
// 	GPIO_AF_Set(GPIOE,0,8);	//PE0,AF8
//	GPIO_AF_Set(GPIOE,1,8);//PE1,AF8  	   
//	
//	//����������
// 	UART8->BRR=temp; 		//����������@OVER8=0 	
//	UART8->CR1=0;		 	//����CR1�Ĵ���
//	UART8->CR1|=0<<28;	 	//����M1=0
//	UART8->CR1|=0<<12;	 	//����M0=0&M1=0,ѡ��8λ�ֳ� 
//	UART8->CR1|=0<<15; 	    //����OVER8=0,16�������� 
//	UART8->CR1|=1<<3;  	    //���ڷ���ʹ�� 

//	//ʹ�ܽ����ж� 
////	UART8->CR1|=1<<2;  	            //���ڽ���ʹ��
////	UART8->CR1|= USART_CR1_IDLEIE;  //���ڽ��տ����ж�ʹ��
////	UART8->CR1|=1<<5;    	        //���ջ������ǿ��ж�ʹ��  ���������ÿ�յ�һ���ֽھͻ����һ���ж�
//	
////	MY_NVIC_Init(13,0,UART8_IRQn,4);//��4��������ȼ�
//	
//	UART8->CR1|=1<<0;  	        //����8 ʹ��
//}











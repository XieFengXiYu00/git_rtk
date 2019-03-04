#include "sys.h"
#include "delay.h" 
#include "led.h" 
#include "usart.h"
#include "includes.h"
#include "24cxx.h"
#include "wkup.h"
#include "adc.h"
#include "DMA.h"
#include "SC50.h"
#include "TRANSFER.h"
#include "string.h"
#include "rtc.h" 
#include "malloc.h" 
#include "exfuns.h"    
#include "fattester.h"
#include "sdmmc_sdcard.h"
#include "wdg.h"
#include "key.h"


//RTK��վ����
//���
//2019-1-10 1.3��
//LED0������ʾϵͳ�Ƿ�����
//����������̵�0.5������һ��
//
//Transfer����
//����ģʽΪ��վģʽʱ ���û�վ���亯�����忨COM2�Ĵ���2͸��������4������
//����ģʽΪ�ƶ�վģʽʱ �����ƶ�վ���亯�����忨COM2�Ĵ���2͸��������3������
//���Ҵ���4������͸�����忨COM1�Ĵ���5
//
//Updata����
//���մ���1�ġ�Updata��ָ�����E2PROM��ı�־λ���ĴӴ�app��λ��ת��bootloaderl��
//�Ӷ����°汾��
//
//Voice����
//���ݲ�ͬVoice_flag����ģ�鲥����ͬ����
//
//Config_task����
//ʹ����������RTKģʽ�Լ�����ģʽ
//
//��ȡ��ص�ѹ����
//���ö༶��ѹ������������
//
//SdTime����
//SD��time.txt��¼ʱ�䣻��������º����˸
/////////////////////////UCOSII��������///////////////////////////////////


#define START_TASK_PRIO      			20//��ʼ��������ȼ�����Ϊ���
//���������ջ��С
#define START_STK_SIZE  				128
//�����ջ	
OS_STK START_TASK_STK[START_STK_SIZE];
//������
void start_task(void *pdata);	
 
 

#define LED0_TASK_PRIO       			9 
//���������ջ��С
#define LED0_STK_SIZE  		    		128
//�����ջ	
OS_STK LED0_TASK_STK[LED0_STK_SIZE];
//������
void led0_task(void *pdata);

 

#define Transfer_TASK_PRIO       			5 
//���������ջ��С
#define Transfer_STK_SIZE  					256
//�����ջ
OS_STK Transfer_TASK_STK[Transfer_STK_SIZE];
//������
void Transfer_task(void *pdata);


#define Updata_TASK_PRIO       			2
//���������ջ��С
#define Updata_STK_SIZE  					256
//�����ջ
OS_STK Updata_TASK_STK[Updata_STK_SIZE];
//������
void Updata_task(void *pdata);



#define Voice_TASK_PRIO       			3
//���������ջ��С
#define Voice_STK_SIZE  					256
//�����ջ
OS_STK Voice_TASK_STK[Voice_STK_SIZE];
//������
void Voice_task(void *pdata);




//24C02����
//��δ����
//���ȼ��Ѿ���SD��������
#define EEPROM_TASK_PRIO       			8 
//���������ջ��С
#define EEPROM_STK_SIZE  					256
//�����ջ
OS_STK EEPROM_TASK_STK[EEPROM_STK_SIZE];
//������
void EEPROM_task(void *pdata);



#define Config_TASK_PRIO       			4
//���������ջ��С
#define Config_STK_SIZE  					128
//�����ջ
OS_STK Config_TASK_STK[Config_STK_SIZE];
//������
void Config_task(void *pdata);



#define SdTime_TASK_PRIO       			8
//���������ջ��С
#define SdTime_STK_SIZE  					256
//�����ջ
OS_STK SdTime_TASK_STK[SdTime_STK_SIZE];
//������
void SdTime_task(void *pdata);



//��ȡ��ص�ѹ����
//���ö༶��ѹ������������
#define ADC_TASK_PRIO       			6
//���������ջ��С
#define ADC_STK_SIZE  					128
//�����ջ
OS_STK ADC_TASK_STK[ADC_STK_SIZE];
//������
void ADC_task(void *pdata);



OS_EVENT * msg_broad;			//�㲥�����ָ�룻
OS_EVENT * Mode_Flag;		  //�忨ģʽ�ź���ָ��	 



//Ҫд�뵽24c02���ַ�������
//const u8 TEXT_Buffer[]=
//"unlogall \r\n\
//undulation 0.0 \r\n\
//mode base time 60 1.5 2.5 \r\n\
//log com2 rtcm1006 ontime 10 \r\n\
//log com2 rtcm1033 ontime 10 \r\n\
//log com2 rtcm1074 ontime 1 \r\n\
//log com2 rtcm1084 ontime 1 \r\n\
//log com2 rtcm1124 ontime 1 \r\n\
//log com2 rtcm1094 ontime 1 \r\n\
//Saveconfig";
//#define SIZE sizeof(TEXT_Buffer)

#define USART_REC_LEN  			800  	//�����������ֽ��� 200
//u8  USART_RX_BUF[USART_REC_LEN]; //���ջ���,���USART_REC_LEN���ֽ�
u8  USART2_RX_BUF[USART_REC_LEN]; //���ջ���,���USART_REC_LEN���ֽ�
u8  USART3_RX_BUF[USART_REC_LEN]; //���ջ���,���USART_REC_LEN���ֽ�
u8  UART4_RX_BUF[USART_REC_LEN]; //���ջ���,���USART_REC_LEN���ֽ�




int main(void)
 { 
	Stm32_Clock_Init(432,25,2,9);	//����ʱ��,216Mhz
  delay_init(216);				//��ʱ��ʼ�� 
	
	uart1_init(108,115200);//���ӵ��Կ�
	uart2_init(54,115200);//���Ӱ忨COM2
	uart3_init(54,115200);//������������
	uart4_init(54,115200);//��������
	uart5_init(54,115200);//���Ӱ忨COM1
	 
	WWDG_Init(0X7F,0X5F,3);	
	
	DMA_RxConfig(DMA1_Stream5,4,(u32)&USART2->RDR,(u32)USART2_RX_BUF,USART_REC_LEN);
  MYDMA_Enable(DMA1_Stream5,USART_REC_LEN);//��ʼһ��UART2 DMA���ݽ���
		
	
	DMA_RxConfig(DMA1_Stream1,4,(u32)&USART3->RDR,(u32)USART3_RX_BUF,USART_REC_LEN);//DMA1,STEAM1,CH4,����Ϊ����1,�洢��ΪSendBuff,����Ϊ:SEND_BUF_SIZE.
	MYDMA_Enable(DMA1_Stream1,USART_REC_LEN);//��ʼһ��UART3 DMA���ݽ���
	
	DMA_RxConfig(DMA1_Stream2,4,(u32)&UART4->RDR,(u32)UART4_RX_BUF,USART_REC_LEN);//DMA1,STEAM2,CH4,����Ϊ����1,�洢��ΪSendBuff,����Ϊ:SEND_BUF_SIZE.
	MYDMA_Enable(DMA1_Stream2,USART_REC_LEN);//��ʼһ��UART4 DMA���ݽ���
	

	
	AT24CXX_Init();			  //IIC��ʼ��
	Adc_Init(); 				  //��ʼ��ADC	
	LED_Init();						//��ʼ��LEDʱ�� 
  SC50X0B_Init();       //��ʼ������ģ������
  //WKUP_Init();				  //��������ģ�飬debuggerʱҪע�͵�������debugger������	
  SC50X0B_SDA(0x02);    //������ʾ����
	
	my_mem_init(SRAMIN);		//��ʼ���ڲ��ڴ��
	//my_mem_init(SRAMEX);		//��ʼ���ⲿ�ڴ��
	//my_mem_init(SRAMTCM);		//��ʼ��TCM�ڴ��
	
	OSInit();                       //UCOS��ʼ��
		
  SD_Init();

//	while(SD_Init())//��ⲻ��SD��
//	{
////		printf("SD Card Error\r\n!");				
////		printf("Please Check! \r\n");
//		delay_ms(50);		
//	
//		

//	}
	
	exfuns_init();							//Ϊfatfs��ر��������ڴ�				 
  f_mount(fs[0],"0:",1); 					//����SD��	
 
    OSTaskCreateExt((void(*)(void*) )start_task,                //������
                    (void*          )0,                         //���ݸ��������Ĳ���
                    (OS_STK*        )&START_TASK_STK[START_STK_SIZE-1],//�����ջջ��
                    (INT8U          )START_TASK_PRIO,           //�������ȼ�
                    (INT16U         )START_TASK_PRIO,           //����ID����������Ϊ�����ȼ�һ��
                    (OS_STK*        )&START_TASK_STK[0],        //�����ջջ��
                    (INT32U         )START_STK_SIZE,            //�����ջ��С
                    (void*          )0,                         //�û�����Ĵ洢��
                    (INT16U         )OS_TASK_OPT_STK_CHK|OS_TASK_OPT_STK_CLR|OS_TASK_OPT_SAVE_FP);//����ѡ��,Ϊ�˱���������������񶼱��渡��Ĵ�����ֵ
	OSStart(); 						//��ʼ����
}

//��ʼ����
void start_task(void *pdata)
{
	OS_CPU_SR cpu_sr=0;
	
	 msg_broad=OSMboxCreate((void*)0);	//������Ϣ����
	 Mode_Flag=OSMboxCreate((void*)0);
	
	
	pdata=pdata;
	OSStatInit(); 		 	//����ͳ������ 
	OS_ENTER_CRITICAL();  	//�����ٽ���(�ر��ж�)
    //LED0����
    OSTaskCreateExt((void(*)(void*) )led0_task,                 
                    (void*          )0,
                    (OS_STK*        )&LED0_TASK_STK[LED0_STK_SIZE-1],
                    (INT8U          )LED0_TASK_PRIO,            
                    (INT16U         )LED0_TASK_PRIO,            
                    (OS_STK*        )&LED0_TASK_STK[0],         
                    (INT32U         )LED0_STK_SIZE,             
                    (void*          )0,                         
                    (INT16U         )OS_TASK_OPT_STK_CHK|OS_TASK_OPT_STK_CLR|OS_TASK_OPT_SAVE_FP);
										
	//Transfer����
    OSTaskCreateExt((void(*)(void*) )Transfer_task,                 
                    (void*          )0,
                    (OS_STK*        )&Transfer_TASK_STK[Transfer_STK_SIZE-1],
                    (INT8U          )Transfer_TASK_PRIO,            
                    (INT16U         )Transfer_TASK_PRIO,            
                    (OS_STK*        )&Transfer_TASK_STK[0],         
                    (INT32U         )Transfer_STK_SIZE,             
                    (void*          )0,                         
                    (INT16U         )OS_TASK_OPT_STK_CHK|OS_TASK_OPT_STK_CLR|OS_TASK_OPT_SAVE_FP); 

	//Updata����
    OSTaskCreateExt((void(*)(void*) )Updata_task,                 
                    (void*          )0,
                    (OS_STK*        )&Updata_TASK_STK[Transfer_STK_SIZE-1],
                    (INT8U          )Updata_TASK_PRIO,            
                    (INT16U         )Updata_TASK_PRIO,            
                    (OS_STK*        )&Updata_TASK_STK[0],         
                    (INT32U         )Updata_STK_SIZE,             
                    (void*          )0,                         
                    (INT16U         )OS_TASK_OPT_STK_CHK|OS_TASK_OPT_STK_CLR|OS_TASK_OPT_SAVE_FP); 


	//Voice����
    OSTaskCreateExt((void(*)(void*) )Voice_task,                 
                    (void*          )0,
                    (OS_STK*        )&Voice_TASK_STK[Voice_STK_SIZE-1],
                    (INT8U          )Voice_TASK_PRIO,            
                    (INT16U         )Voice_TASK_PRIO,            
                    (OS_STK*        )&Voice_TASK_STK[0],         
                    (INT32U         )Voice_STK_SIZE,             
                    (void*          )0,                         
                    (INT16U         )OS_TASK_OPT_STK_CHK|OS_TASK_OPT_STK_CLR|OS_TASK_OPT_SAVE_FP); 		
										
										
	//Config����
    OSTaskCreateExt((void(*)(void*) )Config_task,                 
                    (void*          )0,
                    (OS_STK*        )&Config_TASK_STK[Config_STK_SIZE-1],
                    (INT8U          )Config_TASK_PRIO,            
                    (INT16U         )Config_TASK_PRIO,            
                    (OS_STK*        )&Config_TASK_STK[0],         
                    (INT32U         )Config_STK_SIZE,             
                    (void*          )0,                         
                    (INT16U         )OS_TASK_OPT_STK_CHK|OS_TASK_OPT_STK_CLR|OS_TASK_OPT_SAVE_FP); 	

	//ADC����
    OSTaskCreateExt((void(*)(void*) )ADC_task,                 
                    (void*          )0,
                    (OS_STK*        )&ADC_TASK_STK[ADC_STK_SIZE-1],
                    (INT8U          )ADC_TASK_PRIO,            
                    (INT16U         )ADC_TASK_PRIO,            
                    (OS_STK*        )&ADC_TASK_STK[0],         
                    (INT32U         )ADC_STK_SIZE,             
                    (void*          )0,                         
                    (INT16U         )OS_TASK_OPT_STK_CHK|OS_TASK_OPT_STK_CLR|OS_TASK_OPT_SAVE_FP); 		




	//SdTime����
    OSTaskCreateExt((void(*)(void*) )SdTime_task,                 
                    (void*          )0,
                    (OS_STK*        )&SdTime_TASK_STK[SdTime_STK_SIZE-1],
                    (INT8U          )SdTime_TASK_PRIO,            
                    (INT16U         )SdTime_TASK_PRIO,            
                    (OS_STK*        )&SdTime_TASK_STK[0],         
                    (INT32U         )SdTime_STK_SIZE,             
                    (void*          )0,                         
                    (INT16U         )OS_TASK_OPT_STK_CHK|OS_TASK_OPT_STK_CLR|OS_TASK_OPT_SAVE_FP); 											

//24C02����
//    OSTaskCreateExt((void(*)(void*) )EEPROM_task,                 
//                    (void*          )0,
//                    (OS_STK*        )&EEPROM_TASK_STK[Transfer_STK_SIZE-1],
//                    (INT8U          )EEPROM_TASK_PRIO,            
//                    (INT16U         )EEPROM_TASK_PRIO,            
//                    (OS_STK*        )&EEPROM_TASK_STK[0],         
//                    (INT32U         )EEPROM_STK_SIZE,             
//                    (void*          )0,                         
//                    (INT16U         )OS_TASK_OPT_STK_CHK|OS_TASK_OPT_STK_CLR|OS_TASK_OPT_SAVE_FP); 											

  OS_EXIT_CRITICAL();             //�˳��ٽ���(���ж�)
	OSTaskSuspend(START_TASK_PRIO); //����ʼ����
}
 
//LED0���� �̵���ʾ��������
void led0_task(void *pdata)
{	 	 
	while(1)
	{
		LED0(0);
		delay_ms(500);
		LED0(1);
		delay_ms(500);
	}
}

//Transfer����
//����ģʽΪ��վģʽʱ ���û�վ���亯�����忨COM2�Ĵ���2͸��������4������
//����ģʽΪ��վģʽʱ �����ƶ�վ���亯�����忨COM2�Ĵ���2͸��������3������
//���Ҵ���4������͸�����忨COM1�Ĵ���5
//ֻ���������ʱ��ʹ����Ӧ�Ĵ��ڣ�������������
void Transfer_task(void *pdata)
{
//	u32 total,free;
	u8 err;



	while(1)
	{
		

	 u32 mode_flag=(u32)OSMboxPend(Mode_Flag,0,&err);     //�����ź��� 
   switch(mode_flag) 
	 {
		 case 1://��վ��������
//			 	USART2->CR1|=1<<2;  	//����2����ʹ��
				BaseTransfer();
				break;
		 case 2://�ƶ�վ��������
        //RoverTransfer(USART2_Start_flag,USART2_flag);�����ڴ���ʱֻ�����ø��Ĳ���ֵ������
//		 		USART2->CR1|=1<<2;  	//����2����ʹ��
//		 		UART4->CR1|=1<<2;  	//����4����ʹ��		 
		    RoverTransfer();
				break;
		 default:mode_flag=2;
		 break;

	 }	

	  delay_ms(20);
}
}

//Updata����
//���մ���1�ġ�Updata��ָ�����E2PROM��ı�־λ���ĴӴ�app��λ��ת��bootloaderl��
//�Ӷ����°汾��
void Updata_task(void *pdata)
{
	
	u8 	CFG_flag;
	u8 appflag[1];
  u8 key;
	

	while(1)
	{
//		if(USART_RX_BUF[0]==0x41&&USART_RX_BUF[1]==0x54&&USART_RX_BUF[2]==0x53){CFG_flag=1;}//
//		else if(USART_RX_BUF[0]==0x42&&USART_RX_BUF[1]==0x41&&USART_RX_BUF[2]==0x53){CFG_flag=2;}
//		else if(USART_RX_BUF[0]==0x52&&USART_RX_BUF[1]==0x4F&&USART_RX_BUF[2]==0x56){CFG_flag=3;}
//		else {CFG_flag=0;}

//   switch(CFG_flag) 
	key=KEY_Scan(0);
	if(key==WKUP_PRES)	//WK_UP��������
	 {

			 
			  appflag[0]=0;
				AT24CXX_Write(0,(u8*)appflag,1);		 				
		 
		    __set_FAULTMASK(1);				
			  NVIC_SystemReset();	

	 }	
 
	  delay_ms(20);
}
}

void Voice_task(void *pdata)
{
	u32 key=0;
	u8 err;
	while(1)
	{
		  key=(u32)OSMboxPend(msg_broad,0,&err);  
		switch (key)
		{
			case 4:
				SC50X0B_SDA(0x06);
				break;
			case 3:
				SC50X0B_SDA(0x01);
				break;			
			case 5:
				SC50X0B_SDA(0x10);
				break;
			case 6:
				SC50X0B_SDA(0x0C);
				break;
			case 7:
				SC50X0B_SDA(0x14);
				break;
			
			default:key=0;
		}

//		  broad_flag=8;//����д�����������������OSMboxPost��ͻ��
//  		OSMboxPost(msg_broad,(void*)broad_flag);
			 delay_ms(10);
	}
	
}


//�忨�Լ�����������
void Config_task(void *pdata)
{
int t;
//int voice_flag;
u8 	CFG_flag;

u8 Config1[]={"++++"};  
//u8 Config2[]={"ATS104="};
u8 Config3[]={"AT&W"};
u8 Config4[]={"ATA"};

const u8 Base_Buffer[]=
"unlogall\r\n\
unlogall\r\n\
undulation 0.0\r\n\
mode base time 60 1.5 2.5\r\n\
log com2 rtcm1006 ontime 10\r\n\
log com2 rtcm1033 ontime 10\r\n\
log com2 rtcm1074 ontime 1\r\n\
log com2 rtcm1084 ontime 1\r\n\
log com2 rtcm1124 ontime 1\r\n\
log com2 rtcm1094 ontime 1\r\n\
Saveconfig\r\n";


const u8 Rover_Buffer[]=
"unlogall\r\n\
unlogall\r\n\
MODE ROVER\r\n\
log com2 gpgga ontime 1\r\n\
Saveconfig\r\n";

	

#define SIZE1 sizeof(Config1)
#define SIZE2 sizeof(Config2)	
#define SIZE3 sizeof(Config3)	
#define SIZE4 sizeof(Config4)
	
#define SIZE5 sizeof(Base_Buffer)	
#define SIZE6 sizeof(Rover_Buffer)
	
	while(1)
	{
	 int mode_flag;
//   USART2->CR1|=0<<2;//
//	 UART4->CR1|=1<<2;	
		
		
		if(USART3_RX_BUF[0]==0x41&&USART3_RX_BUF[1]==0x54&&USART3_RX_BUF[2]==0x53){CFG_flag=1;}//�����յ�֡ͷΪATָ��
		else if(USART3_RX_BUF[0]==0x42&&USART3_RX_BUF[1]==0x41&&USART3_RX_BUF[2]==0x53){CFG_flag=2;}//�����յ�֡ͷΪBASEָ��
		else if(USART3_RX_BUF[0]==0x52&&USART3_RX_BUF[1]==0x4F&&USART3_RX_BUF[2]==0x56){CFG_flag=3;}//�����յ�֡ͷΪROVERָ��
		else {CFG_flag=0;}
		

		switch (CFG_flag)
		{
		case 1://ת������������������
		{	

			
			for(t=0;t<3;t++)
			{
				UART4->TDR=Config1[t];
				while((UART4->ISR&0X40)==0);
			}
			delay_ms(1000);	
			for(t=0;t<USART3_RX_NUM;t++)//�����ַ������ӦС�ڵ���10λ
			{
				UART4->TDR=USART3_RX_BUF[t];
				while((UART4->ISR&0X40)==0);
			}	
			for(t=0;t<SIZE3;t++)//��������
			{
				UART4->TDR=Config3[t];
				while((UART4->ISR&0X40)==0);
			}	
			for(t=0;t<SIZE4;t++)//�˳�����ģʽ
			{
				UART4->TDR=Config4[t];
				while((UART4->ISR&0X40)==0);
			}	
			break;

		}
		
		case 2://ת���������û�վ����
		{	

			
			for(t=0;t<SIZE5;t++)
			{
				USART2->TDR=Base_Buffer[t];
				while((USART2->ISR&0X40)==0);
			}
		
			mode_flag=1;
			break;
 
		}	

		case 3://ת�����������ƶ�վ����
		{	

			
			for(t=0;t<SIZE6;t++)
			{
				USART2->TDR=Rover_Buffer[t];//����3��������ת�����忨COM2����2
				while((USART2->ISR&0X40)==0);
			}

			mode_flag=2;
			break;   

		}
	 default:CFG_flag=0;
					break;
	}		
			//USART3_flag=0; 
			USART3_RX_NUM=0;
	    memset(USART3_RX_BUF,0,20);//���ͨ�������������飬�����޶�����20���ֽ�
      //USART2->CR1|=0<<2;

		  delay_ms(50);   
      OSMboxPost(Mode_Flag,(void*)mode_flag);


	}
}



//��ѹ����ʵ��
void ADC_task(void *pdata)
{
int voice_flag;
u16 adcx;
static int adc_flag=0;//��̬�ֲ����ö༶��ѹ����
int t;
	while(1)
	{
		for(t=0;t<15;t++)delay_ms(1000);
						
		adcx=Get_Adc_Average(ADC_CH5,20);
		if(adcx<5000&&adc_flag==0)//����������ѹ����
		{
		adc_flag=1;	
		voice_flag=6;
		OSMboxPost(msg_broad,(void*)voice_flag);
		 delay_ms(10);
		}	

	}
		
}


void SdTime_task(void *pdata)
{
	u8 led1sta=1;
	u8 hour,min,sec,ampm;
//	u8 year,month,date,week;
	u8 tbuf[40];
  u32 total,free;
	u8 t=0; 
	u8 test;
	u8 sdfull = 0;
	int i;
	
	char pathi[3]="";
  char path[20]="RTK/time_";
//	char *lastfile="";
//	u16 lstfullnum;
	
	
	test=exf_getfree("0",&total,&free);	
	printf("exf_getfree: %d\r\n",test);
	printf("remain: %d\r\n",free);
	mf_mkdir("0:RTK");
	
//	f_open(file,"RTK/time.txt",FA_OPEN_ALWAYS);
//	f_close(file);
	f_open(file,"RTK/time_num.txt",FA_OPEN_ALWAYS);
	f_close(file);		
		
	for(i=1;i<100;i++)					//ɨ��SD����Ŀ¼��ֱ��������û�е���ţ�������forѭ��
	{

		memcpy(path,"RTK/time_",20);
		sprintf(pathi,"%d",i);		//����ת�ַ���
		strcat(path,pathi);
		strcat(path,".txt");           //ƴ���ַ���
		if(i>50)
		{sdfull = 1;}
		if(f_open(file,path,0x00)!=0)
		{	
			break;
			
		}
		f_close(file);
	}
	if(sdfull==0)
	{
		f_open(file,path,FA_CREATE_NEW); //�½����û�г��ֵ���� txt�ļ�
		//printf("new SD path---------------------%s\r\n",path);
		f_close(file);
		f_open(file,"RTK/time_num.txt",FA_WRITE);
		//f_lseek(file,f_size(file));
		f_write(file,path,14,&bw);
		f_close(file);
	}
	

while(1) 
{		
	
		t++;
		if((t%1000)==0)	//ÿ1000ms����һ����ʾ����
		{
			
			
		 f_open(file,path,FA_WRITE);
			
		  RTC_Get_Time(&hour,&min,&sec,&ampm);
			sprintf((char*)tbuf,"Time:%02d:%02d:%02d\r\n",hour,min,sec);
			f_lseek(file,f_size(file));
      f_write(file,tbuf,15,&bw);
//			RTC_Get_Date(&year,&month,&date,&week);
//			sprintf((char*)tbuf,"Date:20%02d-%02d-%02d",year,month,date); 
//			sprintf((char*)tbuf,"Week:%d",week);
	    f_close(file);
	  	delay_ms(200);		
		  LED1(led1sta^=1);//DS0��˸
		 
		} 

	}	 
}

		




















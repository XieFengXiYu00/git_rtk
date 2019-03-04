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


//RTK基站代码
//金杰
//2019-1-10 1.3版
//LED0任务，提示系统是否运行
//正常情况下绿灯0.5秒亮灭一次
//
//Transfer任务
//配置模式为基站模式时 调用基站传输函数，板卡COM2的串口2透传至串口4数传口
//配置模式为移动站模式时 调用移动站传输函数，板卡COM2的串口2透传至串口3蓝牙口
//并且串口4数传口透传至板卡COM1的串口5
//
//Updata任务
//接收串口1的“Updata”指令，根据E2PROM里的标志位更改从此app软复位跳转到bootloaderl，
//从而更新版本。
//
//Voice任务
//根据不同Voice_flag语音模块播报不同语音
//
//Config_task任务
//使用蓝牙配置RTK模式以及数传模式
//
//读取电池电压任务
//设置多级电压语音报警功能
//
//SdTime任务
//SD卡time.txt记录时间；正常情况下红灯闪烁
/////////////////////////UCOSII任务设置///////////////////////////////////


#define START_TASK_PRIO      			20//开始任务的优先级设置为最低
//设置任务堆栈大小
#define START_STK_SIZE  				128
//任务堆栈	
OS_STK START_TASK_STK[START_STK_SIZE];
//任务函数
void start_task(void *pdata);	
 
 

#define LED0_TASK_PRIO       			9 
//设置任务堆栈大小
#define LED0_STK_SIZE  		    		128
//任务堆栈	
OS_STK LED0_TASK_STK[LED0_STK_SIZE];
//任务函数
void led0_task(void *pdata);

 

#define Transfer_TASK_PRIO       			5 
//设置任务堆栈大小
#define Transfer_STK_SIZE  					256
//任务堆栈
OS_STK Transfer_TASK_STK[Transfer_STK_SIZE];
//任务函数
void Transfer_task(void *pdata);


#define Updata_TASK_PRIO       			2
//设置任务堆栈大小
#define Updata_STK_SIZE  					256
//任务堆栈
OS_STK Updata_TASK_STK[Updata_STK_SIZE];
//任务函数
void Updata_task(void *pdata);



#define Voice_TASK_PRIO       			3
//设置任务堆栈大小
#define Voice_STK_SIZE  					256
//任务堆栈
OS_STK Voice_TASK_STK[Voice_STK_SIZE];
//任务函数
void Voice_task(void *pdata);




//24C02任务
//暂未启用
//优先级已经被SD任务用了
#define EEPROM_TASK_PRIO       			8 
//设置任务堆栈大小
#define EEPROM_STK_SIZE  					256
//任务堆栈
OS_STK EEPROM_TASK_STK[EEPROM_STK_SIZE];
//任务函数
void EEPROM_task(void *pdata);



#define Config_TASK_PRIO       			4
//设置任务堆栈大小
#define Config_STK_SIZE  					128
//任务堆栈
OS_STK Config_TASK_STK[Config_STK_SIZE];
//任务函数
void Config_task(void *pdata);



#define SdTime_TASK_PRIO       			8
//设置任务堆栈大小
#define SdTime_STK_SIZE  					256
//任务堆栈
OS_STK SdTime_TASK_STK[SdTime_STK_SIZE];
//任务函数
void SdTime_task(void *pdata);



//读取电池电压任务
//设置多级电压语音报警功能
#define ADC_TASK_PRIO       			6
//设置任务堆栈大小
#define ADC_STK_SIZE  					128
//任务堆栈
OS_STK ADC_TASK_STK[ADC_STK_SIZE];
//任务函数
void ADC_task(void *pdata);



OS_EVENT * msg_broad;			//广播邮箱块指针；
OS_EVENT * Mode_Flag;		  //板卡模式信号量指针	 



//要写入到24c02的字符串数组
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

#define USART_REC_LEN  			800  	//定义最大接收字节数 200
//u8  USART_RX_BUF[USART_REC_LEN]; //接收缓冲,最大USART_REC_LEN个字节
u8  USART2_RX_BUF[USART_REC_LEN]; //接收缓冲,最大USART_REC_LEN个字节
u8  USART3_RX_BUF[USART_REC_LEN]; //接收缓冲,最大USART_REC_LEN个字节
u8  UART4_RX_BUF[USART_REC_LEN]; //接收缓冲,最大USART_REC_LEN个字节




int main(void)
 { 
	Stm32_Clock_Init(432,25,2,9);	//设置时钟,216Mhz
  delay_init(216);				//延时初始化 
	
	uart1_init(108,115200);//连接调试口
	uart2_init(54,115200);//连接板卡COM2
	uart3_init(54,115200);//连接蓝牙数据
	uart4_init(54,115200);//连接数传
	uart5_init(54,115200);//连接板卡COM1
	 
	WWDG_Init(0X7F,0X5F,3);	
	
	DMA_RxConfig(DMA1_Stream5,4,(u32)&USART2->RDR,(u32)USART2_RX_BUF,USART_REC_LEN);
  MYDMA_Enable(DMA1_Stream5,USART_REC_LEN);//开始一次UART2 DMA数据接收
		
	
	DMA_RxConfig(DMA1_Stream1,4,(u32)&USART3->RDR,(u32)USART3_RX_BUF,USART_REC_LEN);//DMA1,STEAM1,CH4,外设为串口1,存储器为SendBuff,长度为:SEND_BUF_SIZE.
	MYDMA_Enable(DMA1_Stream1,USART_REC_LEN);//开始一次UART3 DMA数据接收
	
	DMA_RxConfig(DMA1_Stream2,4,(u32)&UART4->RDR,(u32)UART4_RX_BUF,USART_REC_LEN);//DMA1,STEAM2,CH4,外设为串口1,存储器为SendBuff,长度为:SEND_BUF_SIZE.
	MYDMA_Enable(DMA1_Stream2,USART_REC_LEN);//开始一次UART4 DMA数据接收
	

	
	AT24CXX_Init();			  //IIC初始化
	Adc_Init(); 				  //初始化ADC	
	LED_Init();						//初始化LED时钟 
  SC50X0B_Init();       //初始化语音模块引脚
  //WKUP_Init();				  //待机唤醒模块，debugger时要注释掉，否则debugger会闪退	
  SC50X0B_SDA(0x02);    //开机提示语音
	
	my_mem_init(SRAMIN);		//初始化内部内存池
	//my_mem_init(SRAMEX);		//初始化外部内存池
	//my_mem_init(SRAMTCM);		//初始化TCM内存池
	
	OSInit();                       //UCOS初始化
		
  SD_Init();

//	while(SD_Init())//检测不到SD卡
//	{
////		printf("SD Card Error\r\n!");				
////		printf("Please Check! \r\n");
//		delay_ms(50);		
//	
//		

//	}
	
	exfuns_init();							//为fatfs相关变量申请内存				 
  f_mount(fs[0],"0:",1); 					//挂载SD卡	
 
    OSTaskCreateExt((void(*)(void*) )start_task,                //任务函数
                    (void*          )0,                         //传递给任务函数的参数
                    (OS_STK*        )&START_TASK_STK[START_STK_SIZE-1],//任务堆栈栈顶
                    (INT8U          )START_TASK_PRIO,           //任务优先级
                    (INT16U         )START_TASK_PRIO,           //任务ID，这里设置为和优先级一样
                    (OS_STK*        )&START_TASK_STK[0],        //任务堆栈栈底
                    (INT32U         )START_STK_SIZE,            //任务堆栈大小
                    (void*          )0,                         //用户补充的存储区
                    (INT16U         )OS_TASK_OPT_STK_CHK|OS_TASK_OPT_STK_CLR|OS_TASK_OPT_SAVE_FP);//任务选项,为了保险起见，所有任务都保存浮点寄存器的值
	OSStart(); 						//开始任务
}

//开始任务
void start_task(void *pdata)
{
	OS_CPU_SR cpu_sr=0;
	
	 msg_broad=OSMboxCreate((void*)0);	//创建消息邮箱
	 Mode_Flag=OSMboxCreate((void*)0);
	
	
	pdata=pdata;
	OSStatInit(); 		 	//开启统计任务 
	OS_ENTER_CRITICAL();  	//进入临界区(关闭中断)
    //LED0任务
    OSTaskCreateExt((void(*)(void*) )led0_task,                 
                    (void*          )0,
                    (OS_STK*        )&LED0_TASK_STK[LED0_STK_SIZE-1],
                    (INT8U          )LED0_TASK_PRIO,            
                    (INT16U         )LED0_TASK_PRIO,            
                    (OS_STK*        )&LED0_TASK_STK[0],         
                    (INT32U         )LED0_STK_SIZE,             
                    (void*          )0,                         
                    (INT16U         )OS_TASK_OPT_STK_CHK|OS_TASK_OPT_STK_CLR|OS_TASK_OPT_SAVE_FP);
										
	//Transfer任务
    OSTaskCreateExt((void(*)(void*) )Transfer_task,                 
                    (void*          )0,
                    (OS_STK*        )&Transfer_TASK_STK[Transfer_STK_SIZE-1],
                    (INT8U          )Transfer_TASK_PRIO,            
                    (INT16U         )Transfer_TASK_PRIO,            
                    (OS_STK*        )&Transfer_TASK_STK[0],         
                    (INT32U         )Transfer_STK_SIZE,             
                    (void*          )0,                         
                    (INT16U         )OS_TASK_OPT_STK_CHK|OS_TASK_OPT_STK_CLR|OS_TASK_OPT_SAVE_FP); 

	//Updata任务
    OSTaskCreateExt((void(*)(void*) )Updata_task,                 
                    (void*          )0,
                    (OS_STK*        )&Updata_TASK_STK[Transfer_STK_SIZE-1],
                    (INT8U          )Updata_TASK_PRIO,            
                    (INT16U         )Updata_TASK_PRIO,            
                    (OS_STK*        )&Updata_TASK_STK[0],         
                    (INT32U         )Updata_STK_SIZE,             
                    (void*          )0,                         
                    (INT16U         )OS_TASK_OPT_STK_CHK|OS_TASK_OPT_STK_CLR|OS_TASK_OPT_SAVE_FP); 


	//Voice任务
    OSTaskCreateExt((void(*)(void*) )Voice_task,                 
                    (void*          )0,
                    (OS_STK*        )&Voice_TASK_STK[Voice_STK_SIZE-1],
                    (INT8U          )Voice_TASK_PRIO,            
                    (INT16U         )Voice_TASK_PRIO,            
                    (OS_STK*        )&Voice_TASK_STK[0],         
                    (INT32U         )Voice_STK_SIZE,             
                    (void*          )0,                         
                    (INT16U         )OS_TASK_OPT_STK_CHK|OS_TASK_OPT_STK_CLR|OS_TASK_OPT_SAVE_FP); 		
										
										
	//Config任务
    OSTaskCreateExt((void(*)(void*) )Config_task,                 
                    (void*          )0,
                    (OS_STK*        )&Config_TASK_STK[Config_STK_SIZE-1],
                    (INT8U          )Config_TASK_PRIO,            
                    (INT16U         )Config_TASK_PRIO,            
                    (OS_STK*        )&Config_TASK_STK[0],         
                    (INT32U         )Config_STK_SIZE,             
                    (void*          )0,                         
                    (INT16U         )OS_TASK_OPT_STK_CHK|OS_TASK_OPT_STK_CLR|OS_TASK_OPT_SAVE_FP); 	

	//ADC任务
    OSTaskCreateExt((void(*)(void*) )ADC_task,                 
                    (void*          )0,
                    (OS_STK*        )&ADC_TASK_STK[ADC_STK_SIZE-1],
                    (INT8U          )ADC_TASK_PRIO,            
                    (INT16U         )ADC_TASK_PRIO,            
                    (OS_STK*        )&ADC_TASK_STK[0],         
                    (INT32U         )ADC_STK_SIZE,             
                    (void*          )0,                         
                    (INT16U         )OS_TASK_OPT_STK_CHK|OS_TASK_OPT_STK_CLR|OS_TASK_OPT_SAVE_FP); 		




	//SdTime任务
    OSTaskCreateExt((void(*)(void*) )SdTime_task,                 
                    (void*          )0,
                    (OS_STK*        )&SdTime_TASK_STK[SdTime_STK_SIZE-1],
                    (INT8U          )SdTime_TASK_PRIO,            
                    (INT16U         )SdTime_TASK_PRIO,            
                    (OS_STK*        )&SdTime_TASK_STK[0],         
                    (INT32U         )SdTime_STK_SIZE,             
                    (void*          )0,                         
                    (INT16U         )OS_TASK_OPT_STK_CHK|OS_TASK_OPT_STK_CLR|OS_TASK_OPT_SAVE_FP); 											

//24C02任务
//    OSTaskCreateExt((void(*)(void*) )EEPROM_task,                 
//                    (void*          )0,
//                    (OS_STK*        )&EEPROM_TASK_STK[Transfer_STK_SIZE-1],
//                    (INT8U          )EEPROM_TASK_PRIO,            
//                    (INT16U         )EEPROM_TASK_PRIO,            
//                    (OS_STK*        )&EEPROM_TASK_STK[0],         
//                    (INT32U         )EEPROM_STK_SIZE,             
//                    (void*          )0,                         
//                    (INT16U         )OS_TASK_OPT_STK_CHK|OS_TASK_OPT_STK_CLR|OS_TASK_OPT_SAVE_FP); 											

  OS_EXIT_CRITICAL();             //退出临界区(开中断)
	OSTaskSuspend(START_TASK_PRIO); //挂起开始任务
}
 
//LED0任务 绿灯提示程序运行
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

//Transfer任务
//配置模式为基站模式时 调用基站传输函数，板卡COM2的串口2透传至串口4数传口
//配置模式为基站模式时 调用移动站传输函数，板卡COM2的串口2透传至串口3蓝牙口
//并且串口4数传口透传至板卡COM1的串口5
//只在有任务的时候使能相应的串口，否则容易乱码
void Transfer_task(void *pdata)
{
//	u32 total,free;
	u8 err;



	while(1)
	{
		

	 u32 mode_flag=(u32)OSMboxPend(Mode_Flag,0,&err);     //请求信号量 
   switch(mode_flag) 
	 {
		 case 1://基站传输任务
//			 	USART2->CR1|=1<<2;  	//串口2接收使能
				BaseTransfer();
				break;
		 case 2://移动站传输任务
        //RoverTransfer(USART2_Start_flag,USART2_flag);参数在传递时只能引用更改参数值！！！
//		 		USART2->CR1|=1<<2;  	//串口2接收使能
//		 		UART4->CR1|=1<<2;  	//串口4接收使能		 
		    RoverTransfer();
				break;
		 default:mode_flag=2;
		 break;

	 }	

	  delay_ms(20);
}
}

//Updata任务
//接收串口1的“Updata”指令，根据E2PROM里的标志位更改从此app软复位跳转到bootloaderl，
//从而更新版本。
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
	if(key==WKUP_PRES)	//WK_UP按键按下
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

//		  broad_flag=8;//这样写与另外两个任务里的OSMboxPost冲突。
//  		OSMboxPost(msg_broad,(void*)broad_flag);
			 delay_ms(10);
	}
	
}


//板卡以及数传的配置
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
		
		
		if(USART3_RX_BUF[0]==0x41&&USART3_RX_BUF[1]==0x54&&USART3_RX_BUF[2]==0x53){CFG_flag=1;}//蓝牙收到帧头为AT指令
		else if(USART3_RX_BUF[0]==0x42&&USART3_RX_BUF[1]==0x41&&USART3_RX_BUF[2]==0x53){CFG_flag=2;}//蓝牙收到帧头为BASE指令
		else if(USART3_RX_BUF[0]==0x52&&USART3_RX_BUF[1]==0x4F&&USART3_RX_BUF[2]==0x56){CFG_flag=3;}//蓝牙收到帧头为ROVER指令
		else {CFG_flag=0;}
		

		switch (CFG_flag)
		{
		case 1://转发蓝牙配置数传命令
		{	

			
			for(t=0;t<3;t++)
			{
				UART4->TDR=Config1[t];
				while((UART4->ISR&0X40)==0);
			}
			delay_ms(1000);	
			for(t=0;t<USART3_RX_NUM;t++)//网络地址数长度应小于等于10位
			{
				UART4->TDR=USART3_RX_BUF[t];
				while((UART4->ISR&0X40)==0);
			}	
			for(t=0;t<SIZE3;t++)//保存配置
			{
				UART4->TDR=Config3[t];
				while((UART4->ISR&0X40)==0);
			}	
			for(t=0;t<SIZE4;t++)//退出配置模式
			{
				UART4->TDR=Config4[t];
				while((UART4->ISR&0X40)==0);
			}	
			break;

		}
		
		case 2://转发蓝牙配置基站命令
		{	

			
			for(t=0;t<SIZE5;t++)
			{
				USART2->TDR=Base_Buffer[t];
				while((USART2->ISR&0X40)==0);
			}
		
			mode_flag=1;
			break;
 
		}	

		case 3://转发蓝牙配置移动站命令
		{	

			
			for(t=0;t<SIZE6;t++)
			{
				USART2->TDR=Rover_Buffer[t];//串口3蓝牙命令转发至板卡COM2串口2
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
	    memset(USART3_RX_BUF,0,20);//清空通过蓝牙配置数组，这里限定长度20个字节
      //USART2->CR1|=0<<2;

		  delay_ms(50);   
      OSMboxPost(Mode_Flag,(void*)mode_flag);


	}
}



//电压报警实验
void ADC_task(void *pdata)
{
int voice_flag;
u16 adcx;
static int adc_flag=0;//静态局部配置多级电压报警
int t;
	while(1)
	{
		for(t=0;t<15;t++)delay_ms(1000);
						
		adcx=Get_Adc_Average(ADC_CH5,20);
		if(adcx<5000&&adc_flag==0)//触发语音电压报警
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
		
	for(i=1;i<100;i++)					//扫描SD卡根目录，直到出现了没有的序号，则跳出for循环
	{

		memcpy(path,"RTK/time_",20);
		sprintf(pathi,"%d",i);		//整型转字符串
		strcat(path,pathi);
		strcat(path,".txt");           //拼接字符串
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
		f_open(file,path,FA_CREATE_NEW); //新建这个没有出现的序号 txt文件
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
		if((t%1000)==0)	//每1000ms更新一次显示数据
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
		  LED1(led1sta^=1);//DS0闪烁
		 
		} 

	}	 
}

		




















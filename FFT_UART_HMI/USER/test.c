#include "sys.h"
#include "usart.h"		
#include "delay.h"
#include "led.h"
#include "adc.h"
#include "dma.h"
#include "tim.h"
#include "key.h"
#include "hmi.h"
#include "includes.h" 
#include "encoder.h"

/////////////////////////UCOSII��������///////////////////////////////////
//START ����
//�����������ȼ�
#define START_TASK_PRIO		4		//��ʼ��������ȼ�����Ϊ���
//���������ջ��С
#define START_STK_SIZE		36
//�����ջ	
OS_STK START_TASK_STK[START_STK_SIZE];
//������
void start_task(void *pdata);	

// ������
#define MAIN_TASK_PRIO		0												// �����������ȼ�
#define MAIN_STK_SIZE			256											// ���������ջ��С
__align(8) OS_STK MAIN_TASK_STK[MAIN_STK_SIZE];		// �����ջ������8�ֽڶ���
void main_task(void *pdata);											// ������

// �û����봦������
#define USER_INPUT_TASK_PRIO	1										// �����������ȼ�
#define USER_INPUT_STK_SIZE		30									// ���������ջ��С
OS_STK USER_INPUT_TASK_STK[USER_INPUT_STK_SIZE];	// �����ջ	
void user_input_task(void *pdata);								// ������

// LED����			
#define LED_TASK_PRIO			2												// �����������ȼ�
#define LED_STK_SIZE			40											// ���������ջ��С
OS_STK LED_TASK_STK[LED_STK_SIZE];								// �����ջ	
void led_task(void *pdata);												// ������
			
// ��������			
#define TEST_TASK_PRIO		3												// �����������ȼ�
#define TEST_STK_SIZE			64											// ���������ջ��С
OS_STK TEST_TASK_STK[TEST_STK_SIZE];							// �����ջ	
void test_task(void *pdata);											// ������

OS_EVENT * msg_key1;			// ����1�����¼���ָ��
OS_EVENT * msg_key2;			// ����2�����¼���ָ��
OS_EVENT * msg_key3;			// ����3�����¼���ָ��
OS_EVENT * msg_key_enc;		// ���������������¼���ָ��
OS_EVENT * msg_enc_delta;		// ������λ�������¼���ָ��
OS_EVENT * msg_cpuload;			// CPU���������¼���ָ��

char text_log[32];				// ������־

// ������
int main(void)
{	
	Stm32_Clock_Init(9);		// ϵͳʱ������
	delay_init(72);					// ��ʱ��ʼ��
	UART1_Init(72, 115200);	// ����1��ʼ��Ϊ115200
	LED_Init();							// ��ʼ��LED
	ADC1_Init();						// ��ʼ��ADC1
	KEY_Init();							// ������ʼ������
	Encoder_Init();					// ��ʼ��������
	
	DMA1_CH1_Init((uint32_t)&ADC1->DR, (uint32_t)&adc_buf);		// ��ʼ��DMA1ͨ��1(ADC1)
	DMA1_CH4_Init((uint32_t)&USART1->DR, (uint32_t)&tx_buff);	// ��ʼ��DMA1ͨ��4(UART1_TX)
	DMA1_CH5_Init((uint32_t)&USART1->DR, (uint32_t)&rx_buff);	// ��ʼ��DMA1ͨ��5(UART1_RX)
	
	DMA1_CH1_TC_CallBack(ADC1_DMA_CallBack);			// ����DMA��������жϻص�����
	DMA1_CH4_TC_CallBack(UART1_TX_DMA_CallBack);	// ����DMA��������жϻص�����
	
	DMA1_CH1_Start(NPT);
	DMA1_CH5_Start(UART1_RX_BUFF_SIZE);
	
	TIM3_TRGO_Init();				// ��ʱ����ʼ��
	TIM3_TRGO_Freq(20000);	// ��ʱ������Ƶ��20kHz
	
	// ������ʼ����
	OSInit();
	OSTaskCreate(start_task,(void *)0,(OS_STK *)&START_TASK_STK[START_STK_SIZE-1],START_TASK_PRIO );
	OSStart();
}

// ��ʼ����
void start_task(void *pdata)
{
	OS_CPU_SR cpu_sr=0;
	
	msg_key1 = OSMboxCreate((void*)0);			// ������Ϣ����
	msg_key2 = OSMboxCreate((void*)0);			// ������Ϣ����
	msg_key3 = OSMboxCreate((void*)0);			// ������Ϣ����
	msg_key_enc = OSMboxCreate((void*)0);		// ������Ϣ����
	msg_enc_delta = OSMboxCreate((void*)0);	// ������Ϣ����
	msg_cpuload = OSMboxCreate((void*)0);		// ������Ϣ����
	
	OSStatInit();							// ��ʼ��ͳ������.�������ʱ1��������	
 	OS_ENTER_CRITICAL();			// �����ٽ���(�޷����жϴ��)
	
// 	OSTaskCreate(led_task,(void *)0,(OS_STK *)&LED_TASK_STK[LED_STK_SIZE-1], LED_TASK_PRIO);
//	OSTaskCreate(test_task,(void *)0,(OS_STK *)&TEST_TASK_STK[TEST_STK_SIZE-1], TEST_TASK_PRIO);
	
	OSTaskCreateExt(	main_task,
										(void *)0,
										(OS_STK *)&MAIN_TASK_STK[MAIN_STK_SIZE-1], 
										MAIN_TASK_PRIO,
										0,
										(OS_STK *)&MAIN_TASK_STK[0], 
										MAIN_STK_SIZE,
										(void *)0,
										OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR
									);
										
	OSTaskCreateExt(	user_input_task,	
										(void *)0,
										(OS_STK *)&USER_INPUT_TASK_STK[USER_INPUT_STK_SIZE-1], 
										USER_INPUT_TASK_PRIO,
										1,
										(OS_STK *)&USER_INPUT_TASK_STK[0], 
										USER_INPUT_STK_SIZE,
										(void *)0,
										OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR
									);
										
	OSTaskCreateExt(	led_task,	
										(void *)0,
										(OS_STK *)&LED_TASK_STK[LED_STK_SIZE-1], 
										LED_TASK_PRIO,
										2,
										(OS_STK *)&LED_TASK_STK[0], 
										LED_STK_SIZE,
										(void *)0,
										OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR
									);
										
	OSTaskCreateExt(	test_task,
										(void *)0,
										(OS_STK *)&TEST_TASK_STK[TEST_STK_SIZE-1], 
										TEST_TASK_PRIO,
										3,
										(OS_STK *)&TEST_TASK_STK[0], 
										TEST_STK_SIZE,
										(void *)0,
										OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR
									);
	
 	OSTaskSuspend(START_TASK_PRIO);	//������ʼ����.
	OS_EXIT_CRITICAL();				//�˳��ٽ���(���Ա��жϴ��)
}

// ������
void main_task(void *pdata)
{
	while(1)
	{
		HMI_StateMachine();		// HMIϵͳ״̬��
		delay_ms(1);
	}
}

// �û���������
void user_input_task(void *pdata)
{
	while(1)
	{
		KEY_StateMachine(&KEY1_Obj, KEY1_Pin);				// ����1״̬��
		KEY_StateMachine(&KEY2_Obj, KEY2_Pin);				// ����2״̬��
		KEY_StateMachine(&KEY3_Obj, KEY3_Pin);				// ����3״̬��
		KEY_StateMachine(&KEY_Enc_Obj, KEY_Enc_Pin);	// ����������״̬��
		
		OSMboxPost(msg_key1, (void*)KEY_Get_Action(&KEY1_Obj));			//���Ͱ���1������Ϣ
		OSMboxPost(msg_key2, (void*)KEY_Get_Action(&KEY2_Obj));			//���Ͱ���2������Ϣ
		OSMboxPost(msg_key3, (void*)KEY_Get_Action(&KEY3_Obj));			//���Ͱ���3������Ϣ
		OSMboxPost(msg_key_enc, (void*)KEY_Get_Action(&KEY_Enc_Obj));	//���ͱ���������������Ϣ
		
		OSMboxPost(msg_enc_delta, (void*)GetEncDelta());	//���ͱ�����λ��������Ϣ
		
		delay_ms(10);
	}
}


// LED����
void led_task(void *pdata)
{
	while(1)
	{
		LED = ~LED;
		delay_ms(100);
	}
}

// �������񣬻�ȡCPU�����ʣ��Լ�������ջ�ռ�ʹ�����
void test_task(void *pdata)
{
//	OS_STK_DATA StackData;	
	
	while(1)
	{
		OSMboxPost(msg_cpuload, (void*)OSCPUUsage);	//����CPU������Ϣ
		delay_ms(500);
		
//		OSTaskStkChk(MAIN_TASK_PRIO, &StackData);
//		sprintf(text_log, "MAIN Stack:%d %d\r\n", StackData.OSUsed, StackData.OSFree);
//		UART1_TX_Bytes((uint8_t*)text_log, strlen(text_log));
//		
//		OSTaskStkChk(USER_INPUT_TASK_PRIO, &StackData);
//		sprintf(text_log, "USER_INPUT Stack:%d %d\r\n", StackData.OSUsed, StackData.OSFree);
//		UART1_TX_Bytes((uint8_t*)text_log, strlen(text_log));
//		
//		OSTaskStkChk(LED_TASK_PRIO, &StackData);
//		sprintf(text_log, "LED Stack:%d %d\r\n", StackData.OSUsed, StackData.OSFree);
//		UART1_TX_Bytes((uint8_t*)text_log, strlen(text_log));
//		
//		OSTaskStkChk(TEST_TASK_PRIO, &StackData);
//		sprintf(text_log, "TEST Stack:%d %d\r\n", StackData.OSUsed, StackData.OSFree);
//		UART1_TX_Bytes((uint8_t*)text_log, strlen(text_log));
	}
}

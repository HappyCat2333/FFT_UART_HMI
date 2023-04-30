#include "encoder.h"

// �����������������
KEY_Object_T KEY_Enc_Obj;

// ��ʼ��������
void Encoder_Init(void)
{
	// ����������IO����
	RCC->APB2ENR|=1<<2;    		// ʹ��PORTAʱ��	 
	GPIOA->CRL&=0XFFFFFF0F; 	// PA1����
	GPIOA->CRL|=0X00000080;	
  GPIOA->ODR|=1<<1;					// PA1��������
	// ����������IO����
	RCC->APB2ENR|=1<<3;    		// ʹ��PORTBʱ��	 
	GPIOB->CRL&=0X00FFFFFF; 	// PB6/7����
	GPIOB->CRL|=0X88000000;	
  GPIOB->ODR|=3<<6;					// PB6/7��������
	
	RCC->APB1ENR |= 1<<2;	// TIM4ʱ��ʹ��
	TIM4->ARR = 0xFFFF;		// �趨�������Զ���װֵ   
	TIM4->PSC = 0;  			// Ԥ��Ƶ��
	TIM4->CR1 &= ~(uint16_t)(1<<4);    // ��ʱ��4���ϼ���
	TIM4->SMCR |= (1<<0);	// ������ģʽ1
	TIM4->CCER |= (1<<0);	// ͨ��1����ʹ��
	TIM4->CCER |= (1<<4);	// ͨ��2����ʹ��
	TIM4->CCMR1 |= (1<<0);	// IC1ӳ��TI1
	TIM4->CCMR1 |= (1<<8);	// IC2ӳ��TI2
	TIM4->CCMR1 |= (6<<4);	// ���벶��1�˲�������
	
	TIM4->CNT = 0;				// ��ռ�����
	TIM4->CR1 |= 0x01;    // ʹ�ܶ�ʱ��4
	
	KEY_Enc_Obj.state = KEY_Up;	// ��ʼ����������������״̬
	KEY_Enc_Obj.action = KEY_Action_None;	// ��ʼ����������
}


int16_t GetEncDelta(void)
{
	static int8_t last_pos = 0;
	int8_t current_pos = 0;
	int8_t retval = 0;
	
	current_pos = (int8_t)(TIM4->CNT >> 1);
	
	retval = (current_pos - last_pos);
	
	last_pos = current_pos;
	
	return retval;
}



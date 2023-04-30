#ifndef __HMI_H
#define __HMI_H

#include "config.h"
#include <stdint.h>

/* HMI״̬ö�ٶ��� */
typedef enum
{
	HMI_Start = 0,		/* ��ʼ���� */
	HMI_Menu,					/* �˵����� */
	HMI_Spectrum,			/* Ƶ���ǽ��� */
	HMI_Setting,			/* ���ý��� */
	HMI_Default				/* ��ʼ״̬ */
}HMI_STATE_E;

/* Ƶ������ϵͳ״̬ö�ٶ��� */
typedef enum
{
	FFT_SUB_SendCMD = 0,	/* ͸�������״̬ */
	FFT_SUB_SendData,			/* ͸�����ݷ���״̬ */
	FFT_SUB_Default,			/* ��ʼ״̬ */
	FFT_SUB_Exit					/* �˳�״̬ */
}FFT_SUB_STATE_E;

/* �˵���ϵͳ״̬ö�ٶ��� */
typedef enum
{
	Menu_SUB_FFT = 0,			/* Ƶ����ѡ��״̬ */
	Menu_SUB_Setting,			/* ����ѡ��״̬ */
	Menu_SUB_Default,			/* ��ʼ״̬ */
	Menu_SUB_Exit					/* �˳�״̬ */
}Menu_SUB_STATE_E;

// Ƶ������ϵͳ����ṹ�����Ͷ���
typedef struct
{
	FFT_SUB_STATE_E state;	// ��ϵͳ״̬
	uint8_t exit_signal;		// ��ϵͳ�˳��ź�
	uint8_t cursor_update;	// �α����״̬
}FFT_SUB_Object_T;


// �˵���ϵͳ����ṹ�����Ͷ���
typedef struct
{
	Menu_SUB_STATE_E state;	// ��ϵͳ״̬
	uint8_t exit_signal;		// ��ϵͳ�˳��ź�
}Menu_SUB_Object_T;

extern HMI_STATE_E hmi_state;
extern uint32_t lBufInArray[NPT];				/* FFT ������������� */
extern uint32_t lBufOutArray[NPT/2];		/* FFT ������������ */
extern uint32_t lBufMagArray[NPT/2];		/* ��г�������ķ�ֵ */

void HMI_StateMachine(void);						// HMIϵͳ״̬��
void FFT_SUB_StateMachine(FFT_SUB_Object_T* sub);		// Ƶ������ϵͳ״̬��
void Menu_SUB_StateMachine(Menu_SUB_Object_T* sub);	// �˵���ϵͳ״̬��
void GetPowerMag(void);
void Creat_Single(void);
uint16_t GetMaxIndex(void);		// ��ȡ�����ȶ�ӦƵ��

#endif


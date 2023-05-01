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
	SUB_FFT_SendCMD = 0,	/* ͸�������״̬ */
	SUB_FFT_SendData,			/* ͸�����ݷ���״̬ */
	SUB_FFT_Default,			/* ��ʼ״̬ */
	SUB_FFT_Exit					/* �˳�״̬ */
}SUB_FFT_STATE_E;

/* �˵���ϵͳ״̬ö�ٶ��� */
typedef enum
{
	SUB_Menu_FFT = 0,			/* Ƶ����ѡ��״̬ */
	SUB_Menu_Setting,			/* ����ѡ��״̬ */
	SUB_Menu_Default,			/* ��ʼ״̬ */
	SUB_Menu_Exit					/* �˳�״̬ */
}SUB_Menu_STATE_E;

/* ������ϵͳ״̬ö�ٶ��� */
typedef enum
{
	SUB_Set_Sampling_Item = 0,	/* ����Ƶ����Ŀ״̬ */
	SUB_Set_Sampling_Selet,			/* ����Ƶ��ѡ��״̬ */
	SUB_Set_Window_Item,				/* ��������Ŀ״̬ */
	SUB_Set_Window_Selet,				/* ������ѡ��״̬ */
	SUB_Set_Default,						/* ��ʼ״̬ */
	SUB_Set_Exit								/* �˳�״̬ */
}SUB_Set_STATE_E;


// Ƶ������ϵͳ����ṹ�����Ͷ���
typedef struct
{
	SUB_FFT_STATE_E state;	// ��ϵͳ״̬
	uint8_t exit_signal;		// ��ϵͳ�˳��ź�
	uint8_t cursor_update;	// �α����״̬
}SUB_FFT_Object_T;


// �˵���ϵͳ����ṹ�����Ͷ���
typedef struct
{
	SUB_Menu_STATE_E state;	// ��ϵͳ״̬
	uint8_t exit_signal;		// ��ϵͳ�˳��ź�
}SUB_Menu_Object_T;


// ������ϵͳ����ṹ�����Ͷ���
typedef struct
{
	SUB_Set_STATE_E state;	// ��ϵͳ״̬
	uint8_t exit_signal;		// ��ϵͳ�˳��ź�
}SUB_Set_Object_T;


extern HMI_STATE_E hmi_state;
extern uint32_t lBufInArray[NPT];				/* FFT ������������� */
extern uint32_t lBufOutArray[NPT/2];		/* FFT ������������ */
extern uint32_t lBufMagArray[NPT/2];		/* ��г�������ķ�ֵ */

void HMI_StateMachine(void);						// HMIϵͳ״̬��
void SUB_FFT_StateMachine(SUB_FFT_Object_T* sub);		// Ƶ������ϵͳ״̬��
void SUB_Menu_StateMachine(SUB_Menu_Object_T* sub);	// �˵���ϵͳ״̬��
void SUB_Set_StateMachine(SUB_Set_Object_T* sub);		// ������ϵͳ״̬��
void GetPowerMag(void);
void Creat_Single(void);
uint16_t GetMaxIndex(void);		// ��ȡ�����ȶ�ӦƵ��

#endif


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

/* ��ϵͳ״̬ö�ٶ��� */
typedef enum
{
	SUB_SendCMD = 0,	/* ͸�������״̬ */
	SUB_SendData,			/* ͸�����ݷ���״̬ */
	SUB_Default,			/* ��ʼ״̬ */
	SUB_Exit					/* �˳�״̬ */
}SUB_STATE_E;

// ��ϵͳ����ṹ�����Ͷ���
typedef struct
{
	SUB_STATE_E state;		// ��ϵͳ״̬
	uint8_t exit_signal;	// ��ϵͳ�˳��ź�
	uint8_t cursor_update;	// �α����״̬
}SUB_Object_T;

extern HMI_STATE_E hmi_state;
extern uint32_t lBufInArray[NPT];				/* FFT ������������� */
extern uint32_t lBufOutArray[NPT/2];		/* FFT ������������ */
extern uint32_t lBufMagArray[NPT/2];		/* ��г�������ķ�ֵ */


void HMI_StateMachine(void);							// HMIϵͳ״̬��
void SUB_StateMachine(SUB_Object_T* sub);	// ��ϵͳ״̬��
void GetPowerMag(void);
void Creat_Single(void);

#endif


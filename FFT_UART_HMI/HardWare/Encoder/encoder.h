#ifndef __ENCODER_H
#define __ENCODER_H

#include "sys.h"
#include "key.h"

#define KEY_Enc_Pin	PAin(1)	// ��������������

// �����������������
extern KEY_Object_T KEY_Enc_Obj;

void Encoder_Init(void);		// ��ʼ��������
int16_t GetEncDelta(void);		// ��ȡ����������ֵ


#endif


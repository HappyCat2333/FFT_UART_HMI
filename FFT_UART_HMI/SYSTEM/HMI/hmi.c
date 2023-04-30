#include "hmi.h"
#include "key.h"
#include "adc.h"
#include "dma.h"
#include "usart.h"
#include <stdio.h>
#include <string.h>
#include "stm32_dsp.h"
#include "table_fft.h"
#include "math.h"
#include "tim.h"
#include "includes.h"

extern OS_EVENT * msg_key1;				// ����1�����¼���ָ��
extern OS_EVENT * msg_key2;				// ����2�����¼���ָ��
extern OS_EVENT * msg_key3;				// ����3�����¼���ָ��
extern OS_EVENT * msg_key_enc;		// ���������������¼���ָ��
extern OS_EVENT * msg_enc_delta;	// ������λ�������¼���ָ��
extern OS_EVENT * msg_cpuload;		// CPU���������¼���ָ��

uint32_t lBufInArray[NPT];			/* FFT ������������� */
uint32_t lBufOutArray[NPT/2];		/* FFT ������������ */
uint32_t lBufMagArray[NPT/2];		/* ��г�������ķ�ֵ */

HMI_STATE_E hmi_state = HMI_Default;
FFT_SUB_Object_T sub_fft;		// Ƶ������ϵͳ����
Menu_SUB_Object_T sub_menu;	// �˵���ϵͳ����

static char hmi_cmd[32];
static uint8_t send_data[256];
static float sampling_freq = 20000;	// ����Ƶ�ʣ���λHz
static int16_t npt_pos = 0;					// FFT�����λ��
static float npt_freq = 0;					// FFT������ӦƵ��
static float npt_power = 0;					// FFT������Ӧ��ֵ

static uint8_t err;
static int16_t enc_dalta = 0;
//static uint8_t x_zoom = 1;			// x������
static uint8_t y_zoom = 1;			// y������

// HMIϵͳ״̬��
void HMI_StateMachine(void)
{
	static uint16_t cnt = 0;
	
	switch(hmi_state)
	{
		case HMI_Start: {		// ��ʼ����
			if (500 == cnt)	// ��ʱ500ms
			{
				hmi_state = HMI_Menu;	// �л����˵�����
				sub_menu.state = Menu_SUB_Default;
				sprintf(hmi_cmd, "page 1\xFF\xFF\xFF");
				UART1_TX_Bytes((uint8_t*)hmi_cmd, strlen(hmi_cmd));
			}
			if (0 != (uint32_t)OSMboxPend(msg_cpuload, 1, &err))
			{
				sprintf(hmi_cmd, "cpu.txt=\"CPU:%u%%\"\xFF\xFF\xFF", OSCPUUsage);
				UART1_TX_Bytes((uint8_t*)hmi_cmd, strlen(hmi_cmd));
			}
			cnt++;
		} break;
		
		case HMI_Menu: {		// �˵�����
			if (KEY_Action_Down == (uint32_t)OSMboxPend(msg_key_enc, 1, &err))	// �������������¶���
			{
				if (Menu_SUB_FFT == sub_menu.state)
				{
					hmi_state = HMI_Spectrum;	// �л���Ƶ���ǽ���
					sprintf(hmi_cmd, "page 2\xFF\xFF\xFF");
					UART1_TX_Bytes((uint8_t*)hmi_cmd, strlen(hmi_cmd));
					sub_fft.state = FFT_SUB_Default;
					sub_fft.exit_signal = 0;
					sub_fft.cursor_update = 0;
					npt_pos = 0;
					sprintf(hmi_cmd, "f1.txt=\"%.2f\"\xFF\xFF\xFF", sampling_freq/2000/256*40);
					UART1_TX_Bytes((uint8_t*)hmi_cmd, strlen(hmi_cmd));
					sprintf(hmi_cmd, "f2.txt=\"%.2f\"\xFF\xFF\xFF", sampling_freq/2000/256*40*2);
					UART1_TX_Bytes((uint8_t*)hmi_cmd, strlen(hmi_cmd));
					sprintf(hmi_cmd, "f3.txt=\"%.2f\"\xFF\xFF\xFF", sampling_freq/2000/256*40*3);
					UART1_TX_Bytes((uint8_t*)hmi_cmd, strlen(hmi_cmd));
					sprintf(hmi_cmd, "f4.txt=\"%.2f\"\xFF\xFF\xFF", sampling_freq/2000/256*40*4);
					UART1_TX_Bytes((uint8_t*)hmi_cmd, strlen(hmi_cmd));
					sprintf(hmi_cmd, "f5.txt=\"%.2f\"\xFF\xFF\xFF", sampling_freq/2000/256*40*5);
					UART1_TX_Bytes((uint8_t*)hmi_cmd, strlen(hmi_cmd));
					sprintf(hmi_cmd, "f6.txt=\"%.2f\"\xFF\xFF\xFF", sampling_freq/2000/256*40*6);
					UART1_TX_Bytes((uint8_t*)hmi_cmd, strlen(hmi_cmd));
				}
				else if (Menu_SUB_Setting == sub_menu.state)
				{
					hmi_state = HMI_Setting;	// �л������ý���
					sprintf(hmi_cmd, "page 3\xFF\xFF\xFF");
					UART1_TX_Bytes((uint8_t*)hmi_cmd, strlen(hmi_cmd));
				}
			}
			else
			{
				Menu_SUB_StateMachine(&sub_menu);	// �˵���ϵͳ״̬��
			}
			
			if (0 != (uint32_t)OSMboxPend(msg_cpuload, 1, &err))
			{
				sprintf(hmi_cmd, "cpu.txt=\"CPU:%u%%\"\xFF\xFF\xFF", OSCPUUsage);
				UART1_TX_Bytes((uint8_t*)hmi_cmd, strlen(hmi_cmd));
			}
		} break;
		
		case HMI_Spectrum: {	// Ƶ���ǽ���
			if (KEY_Action_Down == (uint32_t)OSMboxPend(msg_key_enc, 1, &err))	// �������������¶���
			{
				sub_fft.exit_signal = 1;		// ����Ƶ������ϵͳ��ֹ�ź�
			}
			else if (FFT_SUB_Exit == sub_fft.state)	// ��ϵͳ�˳�
			{
				hmi_state = HMI_Menu;	// �л����˵�����
				sub_menu.state = Menu_SUB_Default;
				sprintf(hmi_cmd, "page 1\xFF\xFF\xFF");
				UART1_TX_Bytes((uint8_t*)hmi_cmd, strlen(hmi_cmd));
			}
			else
			{
				FFT_SUB_StateMachine(&sub_fft);	// Ƶ������ϵͳ״̬��
			}
		} break;
		
		case HMI_Setting: {	// ���ý���
			enc_dalta = (int32_t)OSMboxPend(msg_enc_delta, 1, &err);
			
			if (KEY_Action_Down == (uint32_t)OSMboxPend(msg_key_enc, 1, &err))	// �������������¶���
			{
				TIM3_TRGO_Freq(sampling_freq);		// ����TIM3����Ƶ��
				
				hmi_state = HMI_Menu;	// �л����˵�����
				sub_menu.state = Menu_SUB_Default;
				sprintf(hmi_cmd, "page 1\xFF\xFF\xFF");
				UART1_TX_Bytes((uint8_t*)hmi_cmd, strlen(hmi_cmd));
			}
			else if (0 != enc_dalta)	// ������������Ϊ0������ť�����ı�
			{
				sampling_freq += (float)enc_dalta * 1000;
				
				// ����Ƶ���޷�
				if (sampling_freq > MAX_SAM_FREQ)				// ����Ƶ������
				{
					sampling_freq = MAX_SAM_FREQ;			
				}
				else if (sampling_freq < MIN_SAM_FREQ)	// ����Ƶ������
				{
					sampling_freq = MIN_SAM_FREQ;
				}
				
				sprintf(hmi_cmd, "n0.val=%d\xFF\xFF\xFF", (int32_t)(sampling_freq/1000));
				UART1_TX_Bytes((uint8_t*)hmi_cmd, strlen(hmi_cmd));
				
				if (0 != (uint32_t)OSMboxPend(msg_cpuload, 1, &err))
				{
					sprintf(hmi_cmd, "cpu.txt=\"CPU:%u%%\"\xFF\xFF\xFF", OSCPUUsage);
					UART1_TX_Bytes((uint8_t*)hmi_cmd, strlen(hmi_cmd));
				}
			}
		} break;
		
		default: {					// ��ʼ״̬
			hmi_state = HMI_Start;		// �л�����ʼ����
			sprintf(hmi_cmd, "page 0\xFF\xFF\xFF");
			UART1_TX_Bytes((uint8_t*)hmi_cmd, strlen(hmi_cmd));
		} break;
	}
}


// Ƶ������ϵͳ״̬��
void FFT_SUB_StateMachine(FFT_SUB_Object_T* sub)
{
	uint8_t rx_data[32];
	uint32_t temp = 0;
	uint16_t i;
	
	switch(sub->state)
	{
		case FFT_SUB_SendCMD: {		// ͸�������״̬
			if (UART1_RX_Bytes(rx_data, uart_rx_len))	// ���յ���ʾ����Ӧ��
			{
				if (0 == memcmp(rx_data, "\xFE\xFF\xFF\xFF", 4))	// �Ƿ�Ϊ͸������
				{
					sub->state = FFT_SUB_SendData;	// �л�����������״̬
					if (0 != sub->cursor_update)	//����α�����
					{
						sub->cursor_update = 0;
						
						memset(send_data, 0x00, sizeof(send_data));
						send_data[2*npt_pos] = SCOPE_HIGH;
					}
					else	//���Ƶ������
					{
						for(i = 0; i < NPT; i++)
						{
							/******************************************************************
								������Ϊ��Ƭ����ADCֻ�ܲ����ĵ�ѹ ������Ҫǰ����ֱ��ƫִ
								����ֱ��ƫִ�� ����ϼ�ȥ2048��һ�� �ﵽ�������ڲ�����Ŀ��	
							******************************************************************/
							lBufInArray[i] = (uint32_t)(((int16_t)adc_buf[i])-2048) << 16;
						}
//						Creat_Single();
//						cr4_fft_1024_stm32(lBufOutArray, lBufInArray, NPT);		//FFT�任
						cr4_fft_256_stm32(lBufOutArray, lBufInArray, NPT);
						GetPowerMag();																					//ȡֱ��������Ӧ��ADֵ
						for(i=0; i<NPT/2; i++)
						{
							temp = lBufMagArray[i]/25 * y_zoom;
							if (temp < 256)
							{
								send_data[2*i] = (uint8_t)temp;
							}
							else
							{
								send_data[2*i] = 255;
							}
							send_data[2*i+1] = 0;
						}
					}
					UART1_TX_Bytes(send_data, NPT);
				}
			}
		} break;
		
		case FFT_SUB_SendData: {	// ͸�����ݷ���״̬
			if (UART1_RX_Bytes(rx_data, uart_rx_len))	// ���յ���ʾ����Ӧ��
			{
				if (0 == memcmp(rx_data, "\xFD\xFF\xFF\xFF", 4))	// �Ƿ�Ϊ͸�����
				{
					sub->state = FFT_SUB_Default;	// �л�����ʼ״̬
					DMA1_CH1_Start(NPT);	// ����DMA1 CH1
				}
			}
		} break;
		
		case FFT_SUB_Default: {		// ��ʼ״̬
			if (0 != sub->exit_signal)		// ���յ���ֹ�ź�
			{
				sub->state = FFT_SUB_Exit;			// ��ϵͳ�˳�
				sub->exit_signal = 0;				// �˳���ɣ���ֹ�ź���0
			}
			else if (0 != adc_dma_flag)
			{
				sub->state = FFT_SUB_SendCMD;		// �л�����������״̬
				adc_dma_flag = 0;						// ������ɱ�־��0
				DMA1_CH1_Stop();						// DMA1ͨ��1ֹͣ
				// ��������͸������
				sprintf(hmi_cmd, "addt s0.id,0,%d\xFF\xFF\xFF", NPT);
				UART1_TX_Bytes((uint8_t*)hmi_cmd, strlen(hmi_cmd));
			}
			else
			{
				enc_dalta = (int32_t)OSMboxPend(msg_enc_delta, 1, &err);
				if (0 != enc_dalta)	// ������������Ϊ0
				{
					sub->state = FFT_SUB_SendCMD;		// �л�����������״̬
					sub->cursor_update = 1;			// �α����
					
					npt_pos += enc_dalta;				// npt�����ӱ���������
					
					if (npt_pos > (NPT/2)-1)		// npt���������NPT/2 - 1
					{
						npt_pos = 0;							// npt������0
					}
					else if (npt_pos < 0)				// npt���������0
					{
						npt_pos = (NPT/2) - 1;		// npt������(NPT/2)
					}
					// ��������͸������
					sprintf(hmi_cmd, "addt s0.id,1,%d\xFF\xFF\xFF", NPT);
					UART1_TX_Bytes((uint8_t*)hmi_cmd, strlen(hmi_cmd));
				}
				else if (KEY_Action_Down == (uint32_t)OSMboxPend(msg_key3, 1, &err))	// ����3���¶���
				{
					sub->state = FFT_SUB_SendCMD;		// �л�����������״̬
					sub->cursor_update = 1;			// �α����
					
					npt_pos = GetMaxIndex();		// ��ȡ�����ȶ�ӦƵ��
					// ��������͸������
					sprintf(hmi_cmd, "addt s0.id,1,%d\xFF\xFF\xFF", NPT);
					UART1_TX_Bytes((uint8_t*)hmi_cmd, strlen(hmi_cmd));
				}
				else if (KEY_Action_Down == (uint32_t)OSMboxPend(msg_key2, 1, &err))	// ����2���¶���
				{
					y_zoom ++;	// �ı�Y������
					if (y_zoom > 3)
					{
						y_zoom = 1;
					}
					sprintf(hmi_cmd, "t6.txt=\"Y Zoom:%u.0\"\xFF\xFF\xFF", y_zoom);
					UART1_TX_Bytes((uint8_t*)hmi_cmd, strlen(hmi_cmd));
					
					sprintf(hmi_cmd, "v1.txt=\"%.2f\"\xFF\xFF\xFF", 3.3/y_zoom/4);
					UART1_TX_Bytes((uint8_t*)hmi_cmd, strlen(hmi_cmd));
					sprintf(hmi_cmd, "v2.txt=\"%.2f\"\xFF\xFF\xFF", 3.3/y_zoom/4*2);
					UART1_TX_Bytes((uint8_t*)hmi_cmd, strlen(hmi_cmd));
					sprintf(hmi_cmd, "v3.txt=\"%.2f\"\xFF\xFF\xFF", 3.3/y_zoom/4*3);
					UART1_TX_Bytes((uint8_t*)hmi_cmd, strlen(hmi_cmd));
					sprintf(hmi_cmd, "v4.txt=\"%.2f\"\xFF\xFF\xFF", 3.3/y_zoom/4*4);
					UART1_TX_Bytes((uint8_t*)hmi_cmd, strlen(hmi_cmd));
				}
				else
				{
					npt_freq = sampling_freq / NPT * npt_pos;						// FFT������ӦƵ��
					npt_power = (float)lBufMagArray[npt_pos]/4095*3.3;	// FFT������Ӧ��ֵ
					
					sprintf(hmi_cmd, "t2.txt=\"%.2f\"\xFF\xFF\xFF", npt_freq/1000);
					UART1_TX_Bytes((uint8_t*)hmi_cmd, strlen(hmi_cmd));
					sprintf(hmi_cmd, "t4.txt=\"%.2f\"\xFF\xFF\xFF", npt_power);
					UART1_TX_Bytes((uint8_t*)hmi_cmd, strlen(hmi_cmd));
					
					if (0 != (uint32_t)OSMboxPend(msg_cpuload, 1, &err))
					{
						sprintf(hmi_cmd, "cpu.txt=\"CPU:%u%%\"\xFF\xFF\xFF", OSCPUUsage);
						UART1_TX_Bytes((uint8_t*)hmi_cmd, strlen(hmi_cmd));
					}
				}
			}
		} break;
		
		case FFT_SUB_Exit: {		// �˳�״̬
		} break;
	}
}

// �˵���ϵͳ״̬��
void Menu_SUB_StateMachine(Menu_SUB_Object_T* sub)
{
	switch(sub->state)
	{
		case Menu_SUB_FFT: {			// Ƶ����ѡ��״̬
			enc_dalta = (int32_t)OSMboxPend(msg_enc_delta, 1, &err);
			if (0 < enc_dalta) // ��ת
			{
				sub->state = Menu_SUB_Setting;
				sprintf(hmi_cmd, "p0.pic=1\xFF\xFF\xFF");
				UART1_TX_Bytes((uint8_t*)hmi_cmd, strlen(hmi_cmd));
				sprintf(hmi_cmd, "p1.pic=4\xFF\xFF\xFF");
				UART1_TX_Bytes((uint8_t*)hmi_cmd, strlen(hmi_cmd));
			}
			else if (0 > enc_dalta) // ��ת
			{
				sub->state = Menu_SUB_Default;
				sprintf(hmi_cmd, "p0.pic=1\xFF\xFF\xFF");
				UART1_TX_Bytes((uint8_t*)hmi_cmd, strlen(hmi_cmd));
				sprintf(hmi_cmd, "p1.pic=3\xFF\xFF\xFF");
				UART1_TX_Bytes((uint8_t*)hmi_cmd, strlen(hmi_cmd));
			}
		} break;
		
		case Menu_SUB_Setting: {	// ����ѡ��״̬
			enc_dalta = (int32_t)OSMboxPend(msg_enc_delta, 1, &err);
			if (0 < enc_dalta) // ��ת
			{
				sub->state = Menu_SUB_Default;
				sprintf(hmi_cmd, "p0.pic=1\xFF\xFF\xFF");
				UART1_TX_Bytes((uint8_t*)hmi_cmd, strlen(hmi_cmd));
				sprintf(hmi_cmd, "p1.pic=3\xFF\xFF\xFF");
				UART1_TX_Bytes((uint8_t*)hmi_cmd, strlen(hmi_cmd));
			}
			else if (0 > enc_dalta) // ��ת
			{
				sub->state = Menu_SUB_FFT;
				sprintf(hmi_cmd, "p0.pic=2\xFF\xFF\xFF");
				UART1_TX_Bytes((uint8_t*)hmi_cmd, strlen(hmi_cmd));
				sprintf(hmi_cmd, "p1.pic=3\xFF\xFF\xFF");
				UART1_TX_Bytes((uint8_t*)hmi_cmd, strlen(hmi_cmd));
			}
		} break;
		
		case Menu_SUB_Default: {	// ��ʼ״̬
			enc_dalta = (int32_t)OSMboxPend(msg_enc_delta, 1, &err);
			if (0 != enc_dalta)	// ������������Ϊ0
			{
				if (0 < enc_dalta) // ��ת
				{
					sub->state = Menu_SUB_FFT;
					sprintf(hmi_cmd, "p0.pic=2\xFF\xFF\xFF");
					UART1_TX_Bytes((uint8_t*)hmi_cmd, strlen(hmi_cmd));
					sprintf(hmi_cmd, "p1.pic=3\xFF\xFF\xFF");
					UART1_TX_Bytes((uint8_t*)hmi_cmd, strlen(hmi_cmd));
				}
				else if (0 > enc_dalta) // ��ת
				{
					sub->state = Menu_SUB_Setting;
					sprintf(hmi_cmd, "p0.pic=1\xFF\xFF\xFF");
					UART1_TX_Bytes((uint8_t*)hmi_cmd, strlen(hmi_cmd));
					sprintf(hmi_cmd, "p1.pic=4\xFF\xFF\xFF");
					UART1_TX_Bytes((uint8_t*)hmi_cmd, strlen(hmi_cmd));
				}
			}
		} break;
		
		case Menu_SUB_Exit: {			// �˳�״̬
		} break;
	}
}

//��ȡFFT���г������
void GetPowerMag(void)
{
	int16_t lX,lY;
	float X,Y,Mag;
	int16_t i;
	for(i=0; i<NPT/2; i++)
	{
		lX  = (lBufOutArray[i] << 16) >> 16;
		lY  = (lBufOutArray[i] >> 16);
	
		//����32768�ٳ�65536��Ϊ�˷��ϸ������������
		X = NPT * ((float)lX) / 32768;
		Y = NPT * ((float)lY) / 32768;
		Mag = sqrt(X * X + Y * Y)*1.0/ NPT;
		if(i == 0)
		{
			lBufMagArray[i] = (uint32_t)(Mag * 32768);
		}
		else
		{
			lBufMagArray[i] = (uint32_t)(Mag * 65536);
		}
	}
}

/************FFT���*****************/
//������ ����һ���ź�
void Creat_Single(void)
{
	u16 i = 0;
	float fx=0.0;
	
	for(i=0; i<NPT; i++)
	{
		fx = 2048+2048*sin(PI2 * i * 500.0 / Fs)+
				 1024*sin(PI2 * i * 3000.0 / Fs)+
				 512*sin(PI2 * i * 5000.0 / Fs);
		lBufInArray[i] = ((signed short)fx) << 16;	
	}
}

// ��ȡ�����ȶ�ӦƵ��
uint16_t GetMaxIndex(void)
{
	uint16_t i, max_index = 0;
	uint32_t max_val = 0;
	
	for (i = 0; i < NPT/2; i++)
	{
		if (lBufMagArray[i] > max_val)
		{
			max_val = lBufMagArray[i];
			max_index = i;
		}
	}
	return max_index;
}

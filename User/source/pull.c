#include "pull.h"
/************************************�������������ݴ���************************************/
/************************************�������������ݴ���************************************/

uint16_t PullBuffer_old[adc_avg][adc_channel]={0};
uint16_t PullBuffer[adc_avg][adc_channel]={0};
uint16_t ADC_Temp[adc_channel]={0};
float ADC_Real[adc_channel]={0};
uint16_t Pull_Value[adc_channel]={0};
uint8_t PullData[2*adc_channel]={0};


void Pull_Data_Handle(void)//�������ݴ�����
{
	uint8_t i= 0, k=0;
	memcpy(PullBuffer,PullBuffer_old,sizeof(PullBuffer_old));//��ADC���ݷ����м�Ĵ���
	memcpy(ADC_Temp,PullBuffer,sizeof(adc_channel));//��ʹ����ֵƽ���˲�����
//	ADC_average_filter(PullBuffer,ADC_Temp);//��ֵƽ���˲�����
	for(i=0;i<adc_channel;i++)
	{
		ADC_Real[i] = (float)(ADC_Temp[i] * 3.3)/4095;//����adc�ɼ����ĵ�ѹֵ
		//�����Ӧ��������0-20mv���Զ�Ӧ0-3�֣����߳�150����ϵ�������kg��Ϊ150*1000��
		//�Ŵ�102��
		Pull_Value[i] = (uint16_t)(ADC_Real[i]*150*1000/102);
		PullData[k]=(uint8_t)Pull_Value[i];//�Ͱ�λ
	  PullData[k+1]=Pull_Value[i] >> 8;//�߰�λ
		if(k<10)
		{ 
			k+=2;
		}
		else//������ֵ�����㣬��ֹ�������
		{
				k=0;
		}
	}
	
}

/*
void ADC_average_filter(uint16_t adc_arr[adc_avg][adc_channel], uint16_t Filter_out[adc_channel])//��ֵƽ���˲�����
{
	uint8_t i,j,row,col =0;
	uint32_t temp = 0;
	uint32_t sample_sum[adc_channel]={0};
	for(col=0;col<adc_channel;col++)//��5��adcͨ������������5��
	{
		for(i=0;i<adc_avg-1;i++)//�������ÿ����С�����������
			{
				for(j=0;j<adc_avg-1;j++)
				{
					if(adc_arr[j][col] > adc_arr[j+1][col])
					{
						temp = adc_arr[j][col];
						adc_arr[j][col] = adc_arr[j+1][col];
						adc_arr[j+1][col] = temp;				
					}
				}
			}
	}
	for(col=0;col<adc_channel;col++)//����5��adcͨ������ֵƽ����
	{
		for(row=min_size;row<adc_avg-max_size;row++)//ȥ��min_size����Сֵ��max_size�����ֵ
		{
			sample_sum[col] += adc_arr[row][col];
		}
		Filter_out[col] = sample_sum[col]/(adc_avg-min_size-max_size);//ȥ����ֵȡƽ��
	}
}
*/


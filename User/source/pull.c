#include "pull.h"
/************************************拉力传感器数据处理************************************/
/************************************拉力传感器数据处理************************************/

uint16_t PullBuffer_old[adc_avg][adc_channel]={0};
uint16_t PullBuffer[adc_avg][adc_channel]={0};
uint16_t ADC_Temp[adc_channel]={0};
float ADC_Real[adc_channel]={0};
uint16_t Pull_Value[adc_channel]={0};
uint8_t PullData[2*adc_channel]={0};


void Pull_Data_Handle(void)//拉力数据处理函数
{
	uint8_t i= 0, k=0;
	memcpy(PullBuffer,PullBuffer_old,sizeof(PullBuffer_old));//将ADC数据放入中间寄存器
	memcpy(ADC_Temp,PullBuffer,sizeof(adc_channel));//不使用中值平均滤波函数
//	ADC_average_filter(PullBuffer,ADC_Temp);//中值平均滤波函数
	for(i=0;i<adc_channel;i++)
	{
		ADC_Real[i] = (float)(ADC_Temp[i] * 3.3)/4095;//计算adc采集到的电压值
		//计算对应的拉力（0-20mv线性对应0-3吨，两者成150倍关系，换算成kg则为150*1000）
		//放大102倍
		Pull_Value[i] = (uint16_t)(ADC_Real[i]*150*1000/102);
		PullData[k]=(uint8_t)Pull_Value[i];//低八位
	  PullData[k+1]=Pull_Value[i] >> 8;//高八位
		if(k<10)
		{ 
			k+=2;
		}
		else//超过阈值则清零，防止数组溢出
		{
				k=0;
		}
	}
	
}

/*
void ADC_average_filter(uint16_t adc_arr[adc_avg][adc_channel], uint16_t Filter_out[adc_channel])//中值平均滤波函数
{
	uint8_t i,j,row,col =0;
	uint32_t temp = 0;
	uint32_t sample_sum[adc_channel]={0};
	for(col=0;col<adc_channel;col++)//有5个adc通道，即数组有5列
	{
		for(i=0;i<adc_avg-1;i++)//对数组的每列由小到大进行排序
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
	for(col=0;col<adc_channel;col++)//计算5个adc通道的中值平均数
	{
		for(row=min_size;row<adc_avg-max_size;row++)//去掉min_size个最小值、max_size个最大值
		{
			sample_sum[col] += adc_arr[row][col];
		}
		Filter_out[col] = sample_sum[col]/(adc_avg-min_size-max_size);//去掉最值取平均
	}
}
*/


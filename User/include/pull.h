#include "string.h"
#include "adc.h"

#define adc_channel 5

/*��ֵ�˲�����*/
#define adc_avg 1    //ƽ���ɼ�adc_avg��
#define min_size 0   //ȥ��min_size����Сֵ
#define max_size 0   //ȥ��max_size�����ֵ

void Pull_Data_Handle(void);
void ADC_average_filter(uint16_t adc_arr[adc_avg][adc_channel], uint16_t Filter_out[adc_channel]);//��ֵƽ���˲�����

#include "string.h"
#include "adc.h"

#define adc_channel 5

/*中值滤波参数*/
#define adc_avg 1    //平均采集adc_avg次
#define min_size 0   //去掉min_size个最小值
#define max_size 0   //去掉max_size个最大值

void Pull_Data_Handle(void);
void ADC_average_filter(uint16_t adc_arr[adc_avg][adc_channel], uint16_t Filter_out[adc_channel]);//中值平均滤波函数

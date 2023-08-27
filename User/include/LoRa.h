#include "usart.h"
#include "string.h"

#define MAX_Num_of_Relay 8//最大中继个数
#define Size_of_Relay 19//中继坐标大小（经度10字节，纬度9字节）
#define Earth_R 6371.0

typedef struct relay_info_array//存放中继经纬度信息
{
	uint8_t zhongji_long[10];
	uint8_t zhongji_lat[9];
	double longtitude;
	double latitude;	
}Relay_t;


void LoRa_Handle(void);//接受数据处理函数
void char2double(Relay_t* prelay);
void Data_Upstream(void);
double tran2rad(double degree);
double distance_cal(Relay_t*prelay,Relay_t*ppresent);
void LoRa_Operation(void);


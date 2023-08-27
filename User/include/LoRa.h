#include "usart.h"
#include "string.h"

#define MAX_Num_of_Relay 8//����м̸���
#define Size_of_Relay 19//�м������С������10�ֽڣ�γ��9�ֽڣ�
#define Earth_R 6371.0

typedef struct relay_info_array//����м̾�γ����Ϣ
{
	uint8_t zhongji_long[10];
	uint8_t zhongji_lat[9];
	double longtitude;
	double latitude;	
}Relay_t;


void LoRa_Handle(void);//�������ݴ�����
void char2double(Relay_t* prelay);
void Data_Upstream(void);
double tran2rad(double degree);
double distance_cal(Relay_t*prelay,Relay_t*ppresent);
void LoRa_Operation(void);


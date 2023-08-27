#include "battery.h"

/************************************�������ģ�����ݴ���************************************/
/************************************�������ģ�����ݴ���************************************/

//�������ģ�����ݱ�ͷ0x68��0x0E��0x20
//�������ģ�鷢�Ͷ�������16�ֽ�/֡

uint8_t BatteryBuffer_old[32]={0};//�������ݽ��ռĴ���
uint8_t BatteryBuffer[16]={0};//�м�Ĵ���
//��ص����ݣ�����Ϊ�������ٷֱȣ�����ѹ�߰�λ����ѹ�ڰ�λ
uint8_t BatteryData[3] = {0};

void Battery_Data_Handle(void)//�������ݴ����� 
{
	uint8_t BatteryTemp[32]={0};//�м�Ĵ�����������֡���ݣ��Ա��жϱ�ͷ��ȡ��������һ֡����
	uint8_t BatteryClear[16]={0};//����Ĵ���
	uint8_t  i=0;
	memcpy(&BatteryTemp[0],&BatteryBuffer_old[0],sizeof(uint8_t)*32);
	
	//�������ģ��ԭʼ���ݣ���ʱ���ܴ������Ĳ��ǰ��ֲ��ϵ�����˳��
	//���Խ���32�ֽڣ���֡�����ݣ��ҵ�֡ͷ0x68��0x0E��0x20��Ű�������һ�����ݷ���BatteryBuffer��	
	for(i=0;i<16;i++)
	{

		if(BatteryTemp[i]==0x68 && BatteryTemp[i+1]==0x0E && BatteryTemp[i+2]==0x20)
			{
				memcpy(&BatteryBuffer[0],&BatteryTemp[i],sizeof(uint8_t)*16);
			}
	}
	if(BatteryBuffer[0]!=0x68)//������ǵ�һ֡���ݣ�������
	{
	
		memcpy(&BatteryBuffer[0],&BatteryClear[0],sizeof(uint8_t)*16);//����
	}
	else
	{
		BatteryData[0]=BatteryBuffer[3];//��ص������ٷֱȣ�
		BatteryData[1]=BatteryBuffer[8];//��ص�ѹ�߰�λ
		BatteryData[2]=BatteryBuffer[9];//��ص�ѹ�ڰ�λ
	}
}

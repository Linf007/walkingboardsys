#include "battery.h"

/************************************电量检测模块数据处理************************************/
/************************************电量检测模块数据处理************************************/

//电量检测模块数据报头0x68、0x0E、0x20
//电量检测模块发送定长数据16字节/帧

uint8_t BatteryBuffer_old[32]={0};//缓冲数据接收寄存器
uint8_t BatteryBuffer[16]={0};//中间寄存器
//电池的数据；依此为电量（百分比）、电压高八位、电压第八位
uint8_t BatteryData[3] = {0};

void Battery_Data_Handle(void)//电量数据处理函数 
{
	uint8_t BatteryTemp[32]={0};//中间寄存器，接收两帧数据，以便判断报头提取出完整的一帧数据
	uint8_t BatteryClear[16]={0};//清零寄存器
	uint8_t  i=0;
	memcpy(&BatteryTemp[0],&BatteryBuffer_old[0],sizeof(uint8_t)*32);
	
	//电量检测模块原始数据，有时可能传回来的不是按手册上的数据顺序
	//所以接受32字节（两帧）数据，找到帧头0x68、0x0E、0x20后才把完整的一整数据放入BatteryBuffer里	
	for(i=0;i<16;i++)
	{

		if(BatteryTemp[i]==0x68 && BatteryTemp[i+1]==0x0E && BatteryTemp[i+2]==0x20)
			{
				memcpy(&BatteryBuffer[0],&BatteryTemp[i],sizeof(uint8_t)*16);
			}
	}
	if(BatteryBuffer[0]!=0x68)//如果不是第一帧数据，则清零
	{
	
		memcpy(&BatteryBuffer[0],&BatteryClear[0],sizeof(uint8_t)*16);//清零
	}
	else
	{
		BatteryData[0]=BatteryBuffer[3];//电池电量（百分比）
		BatteryData[1]=BatteryBuffer[8];//电池电压高八位
		BatteryData[2]=BatteryBuffer[9];//电池电压第八位
	}
}

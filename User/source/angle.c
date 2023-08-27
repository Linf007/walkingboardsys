#include "angle.h"

/************************************倾角传感器数据处理************************************/
/************************************倾角传感器数据处理************************************/

//报头0x55,标识位：角加速度0x51、角速度0x52、角度0x53
//倾角传感器发送定长数据33字节/帧，其中11字节为角加速度数据、11字节为角速度数据、11字节为角度数据
//只需要用到角度数据

float X=0,Y=0,Z=0;//倾角传感器数据，分别有角度滚转角（x 轴）,俯仰角（y 轴）,偏航角（z 轴）
uint8_t AngleReal[11]={0};//倾角角度
uint8_t AngleBuffer_old[66]={0};//缓冲数据接收寄存器(两帧数据)
uint8_t AngleBuffer[33]={0};//倾角数据寄存器（一帧完整的数据）
uint8_t AngleData[angle_data_len]={0};


void Angle_Data_Handle(void)//倾角数据处理函数
{
		uint8_t AngleTemp[66]={0};
		uint8_t AngleClear[66]={0};//清零寄存器
		uint8_t  i=0;
		//先把接收的数据放入中间寄存器
		memcpy(&AngleTemp[0],&AngleBuffer_old[0],sizeof(uint8_t)*66);
		//倾角传感器原始数据，有时可能传回来的不是按手册上的数据顺序
		//所以接受66字节（两帧）数据，找到帧头0x55后才把完整的一整数据放入AngleBuffer里		
		for(i=0;i<11;i++)
		{
			if(AngleTemp[i]==0x55)//找出报头，提取一帧完整数据到AngleBuffer
			{
				memcpy(&AngleBuffer[0],&AngleTemp[i],sizeof(uint8_t)*33);
			}
		}
    if(AngleBuffer[0]!=0x55)//如果不是第一帧数据，则清零
		{
		memcpy(&AngleBuffer[0],&AngleClear,sizeof(uint8_t)*33);
		}
	  else//数据正常发送时，开始接收
	  {
			if(AngleBuffer[1]==0x51)//判断标识位，我们只需要角度数据（即0x53之后的11位数据）
			{
				memcpy(&AngleReal[0],&AngleBuffer[22],sizeof(uint8_t)*11);
			}
			else if(AngleBuffer[1]==0x52)
			{
				memcpy(&AngleReal[0],&AngleBuffer[11],sizeof(uint8_t)*11);
			}
			else if(AngleBuffer[1]==0x53)
			{
				memcpy(&AngleReal[0],&AngleBuffer[0],sizeof(uint8_t)*11);
			}
			else{}
	   }	

		//角度计算，具体公式参考模块手册维特智能JY62
		X=(float)(((short)AngleReal[3]<<8)|AngleReal[2])/32768*180*100;//乘100传过去除1000使用
		Y=(float)(((short)AngleReal[5]<<8)|AngleReal[4])/32768*180*100;
		Z=(float)(((short)AngleReal[7]<<8)|AngleReal[6])/32768*180*100;
		AngleData[0]=(uint8_t)X;//低八位
		AngleData[1]=X/256;//高八位
		AngleData[2]=(uint8_t)Y;
		AngleData[3]=Y/256;
		AngleData[4]=(uint8_t)Z;
		AngleData[5]=Z/256;

}


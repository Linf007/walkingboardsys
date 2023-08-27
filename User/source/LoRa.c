#include "LoRa.h"
#include "pull.h"
#include "main.h"
#include "math.h"
#include "cmsis_os.h"


/************************************上位机与下位机通讯************************************/
/************************************上位机与下位机通讯************************************/


/*PC与MCU通信：*/
#define Enddata  0x3E;//帧结束位
#define TOOLADDR 0x40;//MCU地址
//握手协议+校准命令应答//再发送一次，模块从待机状态进入工作状态
uint8_t Answer_OR[8]={0x55,0x10,0x40,0x08,0x00,0x00,0x00,0x3E};
uint8_t Reset_OR[8]={0x55,0x10,0x40,0x08,0x01,0x01,0x01,0x3E};//复位指示

uint8_t Data_TX_Head[3]={0x55,0x11,0x40};//发送数据应答头
uint8_t All_Data_TX[60] = {0};

uint8_t CmdData_Rx[MAX_Num_of_Relay*Size_of_Relay]={0};//上位机命令寄存器
uint8_t lora_id = 0x01;//设置LoRa模块的NetID//
uint8_t LoRa_ID_Config[8] = {0xC0, 0x00, 0x05, 0x12, 0x34, 0x01, 0x62, 0x1C};//只改变NETID---LoRa_ID_Config[5]

uint8_t all_relay_info[MAX_Num_of_Relay*Size_of_Relay]={0};//所有中继位置信息
Relay_t Relay_Buff[MAX_Num_of_Relay]={0};//每个中继的经纬度信息
Relay_t Present_posi={0};//当前走板经纬度
double distances=0;//走板与中继的距离

extern uint8_t BatteryData[3];//处理好的电量数据
extern uint8_t AngleData[6];//处理好的倾角数据
extern uint8_t GnssData[30];//处理好的GNSS数据
extern uint8_t PullData[2*adc_channel];//处理好的拉力数据（分高低位）
extern EventGroupHandle_t Upstream_Event_Handle;

/***********************************************处理上位机命令函数*********************************************/

void LoRa_Handle(void)
{
	if(CmdData_Rx[0]==0x55&&CmdData_Rx[1]==0x30&&CmdData_Rx[2]==0x20&&CmdData_Rx[3]==0x08&&CmdData_Rx[7]==0x3E)
	{
		if(CmdData_Rx[4]==0x01)//接收到握手命令
		{
			HAL_UART_Transmit_IT(&huart4,Answer_OR,8);//握手应答
			xEventGroupSetBits(Upstream_Event_Handle,Handshake_Event);
		}
		else if(CmdData_Rx[4]==0x10)//复位命令
		{
			Reset_Exec();//软件复位					
		}
		else if(CmdData_Rx[4]==0xFF)//开机命令
		{
			Reset_Exec();
		}
		else if(CmdData_Rx[4]==0xC3)
		{
			HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_RESET);//LoRa进入配置模式
			LoRa_ID_Config[5]=CmdData_Rx[5];//配置NetID
			lora_id=0x01;//避免修改中继信息
			HAL_Delay(100);
			HAL_UART_Transmit_IT(&huart4,LoRa_ID_Config,8);//设置网络ID			
		}
		else if(CmdData_Rx[4]==0xAA)
		{
			Data_Upstream();
		}
	}
	
	else if(CmdData_Rx[0]==0xC1&&CmdData_Rx[7]==0x1C)//LoRa的NetID配置成功
	{		
		HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_SET);//GPIO控制三极管，PB12置'1'则三极管导通，LoRa设置为透传模式
		switch (lora_id)//修改NETID后清除所过中继的信息
		{
			case 2:memset(&Relay_Buff[0],0,sizeof(Relay_Buff[0]));break;
			case 3:memset(&Relay_Buff[1],0,sizeof(Relay_Buff[1]));break;
			case 4:memset(&Relay_Buff[2],0,sizeof(Relay_Buff[2]));break;
			case 5:memset(&Relay_Buff[3],0,sizeof(Relay_Buff[3]));break;
			case 6:memset(&Relay_Buff[4],0,sizeof(Relay_Buff[4]));break;
			case 7:memset(&Relay_Buff[5],0,sizeof(Relay_Buff[5]));break;
			case 8:memset(&Relay_Buff[6],0,sizeof(Relay_Buff[6]));break;
			case 9:memset(&Relay_Buff[7],0,sizeof(Relay_Buff[7]));break;
			default:break;
		}
			
	}
	//接收中继经纬度信息
	if(((CmdData_Rx[3]==0xFE)&&(CmdData_Rx[12]==0xFE)))//||((CmdData_Rx[1]==0x00)&&(CmdData_Rx[2]==0x00)&&(CmdData_Rx[7]==0x00)))
	{

		memcpy(&all_relay_info,&CmdData_Rx,MAX_Num_of_Relay*Size_of_Relay);
		if(all_relay_info[3]==0xFE&&all_relay_info[12]==0xFE)
		{
			memcpy(&Relay_Buff[0].zhongji_long,&all_relay_info[0],10);
			memcpy(&Relay_Buff[0].zhongji_lat,&all_relay_info[10],9);
		}
		if(all_relay_info[22]==0xFE&&all_relay_info[31]==0xFE)
		{
			memcpy(&Relay_Buff[1].zhongji_long,&all_relay_info[19],10);
			memcpy(&Relay_Buff[1].zhongji_lat,&all_relay_info[29],9);
		}		
		if(all_relay_info[41]==0xFE&&all_relay_info[50]==0xFE)
		{
			memcpy(&Relay_Buff[2].zhongji_long,&all_relay_info[38],10);
			memcpy(&Relay_Buff[2].zhongji_lat,&all_relay_info[48],9);
		}	
		if(all_relay_info[60]==0xFE&&all_relay_info[69]==0xFE)
		{
			memcpy(&Relay_Buff[3].zhongji_long,&all_relay_info[57],10);
			memcpy(&Relay_Buff[3].zhongji_lat,&all_relay_info[67],9);
		}	
		if(all_relay_info[79]==0xFE&&all_relay_info[88]==0xFE)
		{
			memcpy(&Relay_Buff[4].zhongji_long,&all_relay_info[76],10);
			memcpy(&Relay_Buff[4].zhongji_lat,&all_relay_info[86],9);
		}	
		if(all_relay_info[98]==0xFE&&all_relay_info[107]==0xFE)
		{
			memcpy(&Relay_Buff[5].zhongji_long,&all_relay_info[95],10);
			memcpy(&Relay_Buff[5].zhongji_lat,&all_relay_info[105],9);
		}	
		if(all_relay_info[117]==0xFE&&all_relay_info[126]==0xFE)
		{
			memcpy(&Relay_Buff[6].zhongji_long,&all_relay_info[114],10);
			memcpy(&Relay_Buff[6].zhongji_lat,&all_relay_info[124],9);
		}		
		for(int k=0;k<MAX_Num_of_Relay;k++)
		{
			char2double(&Relay_Buff[k]);//转换数据类型
		}
		xEventGroupSetBits(Upstream_Event_Handle,Received_Relay_Event);
	}
}

/*********************************************char转换为double型*******************************************/
/*
@parameter1：中继坐标结构体
*/
void char2double(Relay_t* prelay)
{
	prelay->latitude=(prelay->zhongji_lat[0])*10+(prelay->zhongji_lat[1])+(double)(prelay->zhongji_lat[3])*0.1+(double)(prelay->zhongji_lat[4])*0.01+(double)(prelay->zhongji_lat[5])*0.001+\
											(double)(prelay->zhongji_lat[6])*0.0001+(double)(prelay->zhongji_lat[7])*0.00001+(double)(prelay->zhongji_lat[8])*0.000001;
	prelay->longtitude=(prelay->zhongji_long[0])*100+(prelay->zhongji_long[1])*10+(prelay->zhongji_long[2])+(double)(prelay->zhongji_long[4])*0.1+(double)(prelay->zhongji_long[5])*0.01+\
												(double)(prelay->zhongji_long[6])*0.001+(double)(prelay->zhongji_long[7])*0.0001+(double)(prelay->zhongji_long[8])*0.00001+(double)(prelay->zhongji_long[9])*0.000001;
}

/************************************************打包上传数据**********************************************/
void Data_Upstream(void)
{
	memcpy(&All_Data_TX,&Data_TX_Head,sizeof(uint8_t)*3);//头数据
	All_Data_TX[3]=50;//此帧数据长度
	memcpy(&All_Data_TX[4],&PullData,sizeof(uint8_t)*10);//ADC数据
	memcpy(&All_Data_TX[14],&AngleData,sizeof(uint8_t)*4);//倾角数据
	memcpy(&All_Data_TX[18],&BatteryData,sizeof(uint8_t)*1);//电量数据
	memcpy(&All_Data_TX[19],&GnssData[0],sizeof(uint8_t)*30);//GNSS数据
	All_Data_TX[49]=Enddata;//帧结束
	HAL_UART_Transmit_IT(&huart4,All_Data_TX,50);	
}

/***********************************************角度转换为弧度*********************************************/
/*
return：弧度
@parameter1：角度
*/
double tran2rad(double degree)
{
	double Rad_result;
	Rad_result=degree*3.1415926/180;
	return Rad_result;
}



/**************************************************距离计算************************************************/
/*
return：两个坐标点距离
@parameter1：中继坐标结构体（地址）
@parameter2：走板坐标结构体（地址）
*/
double distance_cal(Relay_t*prelay,Relay_t*ppresent)
{
	double x1,x2,y1,y2,Distance_result;
	x1=tran2rad(ppresent->longtitude);
	y1=tran2rad(ppresent->latitude);
	x2=tran2rad(prelay->longtitude);
	y2=tran2rad(prelay->latitude);
	Distance_result=Earth_R*acos(cos(y1)*cos(y2)*cos(x1-x2)+sin(y1)*sin(y2));//距离计算公式
	return Distance_result;
}

/**********************************************配置LoRa及数据上传*******************************************/
void LoRa_Operation(void)
{
	for(int i=0;i<MAX_Num_of_Relay;i++)
	{
		distances=distance_cal(&Relay_Buff[i],&Present_posi);//走板与中继的距离
		if((distances>0)&&(distances<50))
		{
			HAL_GPIO_WritePin(LoRa_Config_GPIO_Port,LoRa_Config_Pin,GPIO_PIN_RESET);//LoRa进入配置模式
			osDelay(100);
			lora_id=i+2;
			LoRa_ID_Config[5]=lora_id;
			HAL_UART_Transmit_IT(&huart4,LoRa_ID_Config,8);//配置LORa
		}
		else
		{		
			Data_Upstream();//数据打包上传			
		}
	}
	
}



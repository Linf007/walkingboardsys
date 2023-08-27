#include "LoRa.h"
#include "pull.h"
#include "main.h"
#include "math.h"
#include "cmsis_os.h"


/************************************��λ������λ��ͨѶ************************************/
/************************************��λ������λ��ͨѶ************************************/


/*PC��MCUͨ�ţ�*/
#define Enddata  0x3E;//֡����λ
#define TOOLADDR 0x40;//MCU��ַ
//����Э��+У׼����Ӧ��//�ٷ���һ�Σ�ģ��Ӵ���״̬���빤��״̬
uint8_t Answer_OR[8]={0x55,0x10,0x40,0x08,0x00,0x00,0x00,0x3E};
uint8_t Reset_OR[8]={0x55,0x10,0x40,0x08,0x01,0x01,0x01,0x3E};//��λָʾ

uint8_t Data_TX_Head[3]={0x55,0x11,0x40};//��������Ӧ��ͷ
uint8_t All_Data_TX[60] = {0};

uint8_t CmdData_Rx[MAX_Num_of_Relay*Size_of_Relay]={0};//��λ������Ĵ���
uint8_t lora_id = 0x01;//����LoRaģ���NetID//
uint8_t LoRa_ID_Config[8] = {0xC0, 0x00, 0x05, 0x12, 0x34, 0x01, 0x62, 0x1C};//ֻ�ı�NETID---LoRa_ID_Config[5]

uint8_t all_relay_info[MAX_Num_of_Relay*Size_of_Relay]={0};//�����м�λ����Ϣ
Relay_t Relay_Buff[MAX_Num_of_Relay]={0};//ÿ���м̵ľ�γ����Ϣ
Relay_t Present_posi={0};//��ǰ�߰徭γ��
double distances=0;//�߰����м̵ľ���

extern uint8_t BatteryData[3];//����õĵ�������
extern uint8_t AngleData[6];//����õ��������
extern uint8_t GnssData[30];//����õ�GNSS����
extern uint8_t PullData[2*adc_channel];//����õ��������ݣ��ָߵ�λ��
extern EventGroupHandle_t Upstream_Event_Handle;

/***********************************************������λ�������*********************************************/

void LoRa_Handle(void)
{
	if(CmdData_Rx[0]==0x55&&CmdData_Rx[1]==0x30&&CmdData_Rx[2]==0x20&&CmdData_Rx[3]==0x08&&CmdData_Rx[7]==0x3E)
	{
		if(CmdData_Rx[4]==0x01)//���յ���������
		{
			HAL_UART_Transmit_IT(&huart4,Answer_OR,8);//����Ӧ��
			xEventGroupSetBits(Upstream_Event_Handle,Handshake_Event);
		}
		else if(CmdData_Rx[4]==0x10)//��λ����
		{
			Reset_Exec();//�����λ					
		}
		else if(CmdData_Rx[4]==0xFF)//��������
		{
			Reset_Exec();
		}
		else if(CmdData_Rx[4]==0xC3)
		{
			HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_RESET);//LoRa��������ģʽ
			LoRa_ID_Config[5]=CmdData_Rx[5];//����NetID
			lora_id=0x01;//�����޸��м���Ϣ
			HAL_Delay(100);
			HAL_UART_Transmit_IT(&huart4,LoRa_ID_Config,8);//��������ID			
		}
		else if(CmdData_Rx[4]==0xAA)
		{
			Data_Upstream();
		}
	}
	
	else if(CmdData_Rx[0]==0xC1&&CmdData_Rx[7]==0x1C)//LoRa��NetID���óɹ�
	{		
		HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_SET);//GPIO���������ܣ�PB12��'1'�������ܵ�ͨ��LoRa����Ϊ͸��ģʽ
		switch (lora_id)//�޸�NETID����������м̵���Ϣ
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
	//�����м̾�γ����Ϣ
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
			char2double(&Relay_Buff[k]);//ת����������
		}
		xEventGroupSetBits(Upstream_Event_Handle,Received_Relay_Event);
	}
}

/*********************************************charת��Ϊdouble��*******************************************/
/*
@parameter1���м�����ṹ��
*/
void char2double(Relay_t* prelay)
{
	prelay->latitude=(prelay->zhongji_lat[0])*10+(prelay->zhongji_lat[1])+(double)(prelay->zhongji_lat[3])*0.1+(double)(prelay->zhongji_lat[4])*0.01+(double)(prelay->zhongji_lat[5])*0.001+\
											(double)(prelay->zhongji_lat[6])*0.0001+(double)(prelay->zhongji_lat[7])*0.00001+(double)(prelay->zhongji_lat[8])*0.000001;
	prelay->longtitude=(prelay->zhongji_long[0])*100+(prelay->zhongji_long[1])*10+(prelay->zhongji_long[2])+(double)(prelay->zhongji_long[4])*0.1+(double)(prelay->zhongji_long[5])*0.01+\
												(double)(prelay->zhongji_long[6])*0.001+(double)(prelay->zhongji_long[7])*0.0001+(double)(prelay->zhongji_long[8])*0.00001+(double)(prelay->zhongji_long[9])*0.000001;
}

/************************************************����ϴ�����**********************************************/
void Data_Upstream(void)
{
	memcpy(&All_Data_TX,&Data_TX_Head,sizeof(uint8_t)*3);//ͷ����
	All_Data_TX[3]=50;//��֡���ݳ���
	memcpy(&All_Data_TX[4],&PullData,sizeof(uint8_t)*10);//ADC����
	memcpy(&All_Data_TX[14],&AngleData,sizeof(uint8_t)*4);//�������
	memcpy(&All_Data_TX[18],&BatteryData,sizeof(uint8_t)*1);//��������
	memcpy(&All_Data_TX[19],&GnssData[0],sizeof(uint8_t)*30);//GNSS����
	All_Data_TX[49]=Enddata;//֡����
	HAL_UART_Transmit_IT(&huart4,All_Data_TX,50);	
}

/***********************************************�Ƕ�ת��Ϊ����*********************************************/
/*
return������
@parameter1���Ƕ�
*/
double tran2rad(double degree)
{
	double Rad_result;
	Rad_result=degree*3.1415926/180;
	return Rad_result;
}



/**************************************************�������************************************************/
/*
return��������������
@parameter1���м�����ṹ�壨��ַ��
@parameter2���߰�����ṹ�壨��ַ��
*/
double distance_cal(Relay_t*prelay,Relay_t*ppresent)
{
	double x1,x2,y1,y2,Distance_result;
	x1=tran2rad(ppresent->longtitude);
	y1=tran2rad(ppresent->latitude);
	x2=tran2rad(prelay->longtitude);
	y2=tran2rad(prelay->latitude);
	Distance_result=Earth_R*acos(cos(y1)*cos(y2)*cos(x1-x2)+sin(y1)*sin(y2));//������㹫ʽ
	return Distance_result;
}

/**********************************************����LoRa�������ϴ�*******************************************/
void LoRa_Operation(void)
{
	for(int i=0;i<MAX_Num_of_Relay;i++)
	{
		distances=distance_cal(&Relay_Buff[i],&Present_posi);//�߰����м̵ľ���
		if((distances>0)&&(distances<50))
		{
			HAL_GPIO_WritePin(LoRa_Config_GPIO_Port,LoRa_Config_Pin,GPIO_PIN_RESET);//LoRa��������ģʽ
			osDelay(100);
			lora_id=i+2;
			LoRa_ID_Config[5]=lora_id;
			HAL_UART_Transmit_IT(&huart4,LoRa_ID_Config,8);//����LORa
		}
		else
		{		
			Data_Upstream();//���ݴ���ϴ�			
		}
	}
	
}



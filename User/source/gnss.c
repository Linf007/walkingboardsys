#include "Gnss.h"
#include "Lora.h"
#include "math.h"

/************************************GNSSģ�����ݴ���************************************/
/************************************GNSSģ�����ݴ���************************************/

//Gnssģ�鷢��GNRMC���ģ���ͷ��0x24,0x47,0x4E,0x52,0x4D,0x43
//Gnss���Ͳ��������ݣ���Ҫ���ź�ǿ�Ⱦ����������ÿ����жϽ��ղ���������һ֡һ֡��������

uint8_t GnssBuffer_old[100]={0};//Gnss�������ݻ���Ĵ���//��������100��
uint8_t GnssBuffer[100]={0};//�м�Ĵ���
uint8_t GnssData[30]={0};
uint8_t latdata[10]={0};
uint8_t longdata[11]={0};

extern Relay_t Present_posi;

void Gnss_Data_Handle(void)//GNSS���ݴ�����
{
  uint8_t GnssClear[100]={0};
	uint8_t j=0,i=0;
	uint8_t Loca[8];
	uint8_t gnss_temp[21]={0};
	double long_dec=0.0;
	double lat_dec=0.0;
	memcpy(&GnssBuffer,&GnssBuffer_old,100);
	if(GnssBuffer[0]!=0x24&&GnssBuffer[1]!=0x47&&GnssBuffer[2]!=0x4E&&GnssBuffer[3]!=0x52&&GnssBuffer[4]!=0x4D&&GnssBuffer[5]!=0x43)//�������֡ͷ��������
	{
	memcpy(&GnssBuffer,&GnssClear,100);
	}
	else//��ʼ����������
	{
		 if(GnssBuffer[0]==0x24&&GnssBuffer[1]==0x47&&GnssBuffer[2]==0x4E&&GnssBuffer[3]==0x52&&GnssBuffer[4]==0x4D&&GnssBuffer[5]==0x43)//���֡ͷ
	  {
			for(j=0;j<100;j++)//Ѱ�����ݵĽڵ�
		 {
		   if(GnssBuffer[j]==0x2C)//Ѱ�ҽڵ�λ��
		  {
			  i++;//�궨2C�Ĺ̶�λ��,���궨���ŵ�λ��
				switch (i)
				{  
					case 3:Loca[0]=j;break;//3-4����֮��Ϊγ������
				 	case 4:Loca[1]=j;break;
 					case 5:Loca[2]=j;break;//5-6����֮��Ϊ��������
				 	case 6:Loca[3]=j;break;
					case 7:Loca[4]=j;break;//7-8����֮��Ϊ�ٶ�����
	 				case 8:Loca[5]=j;break;		
					default: break; 
				}
		  }
		  else{}
			  
		 }
		 	memcpy(&GnssData[0],&GnssBuffer[Loca[0]],sizeof(uint8_t)*(Loca[1]-Loca[0]+1));//γ��
			memcpy(&GnssData[(Loca[1]-Loca[0]+1)],&GnssBuffer[Loca[2]+1],sizeof(uint8_t)*(Loca[3]-Loca[2]));//����
			memcpy(&GnssData[(Loca[3]-Loca[2]+Loca[1]-Loca[0]+1)],&GnssBuffer[Loca[4]+1],sizeof(uint8_t)*(Loca[5]-Loca[4]));//�����ٶ�
	  
			if((Loca[1]-Loca[0]-1==0)&&(Loca[3]-Loca[2]-1==0))
			{
				memcpy(&latdata,&GnssClear,10);
				memcpy(&longdata,&GnssClear,11);			
			}	
			
			else
			{
				memcpy(&gnss_temp,&GnssData[1],10);
				memcpy(&gnss_temp[10],&GnssData[12],11);					
			
			
				for(int k=0;k<21;k++)
				{
					gnss_temp[k]=gnss_temp[k]-48;
				}
					memcpy(&latdata,&gnss_temp,10);
					memcpy(&longdata,&gnss_temp[10],11);
			}
			lat_dec=(double)((latdata[2]*10+latdata[3]+latdata[5]*0.1+latdata[6]*0.01+latdata[7]*0.001+latdata[8]*0.0001+latdata[9]*0.00001)/60);
			long_dec=(double)((longdata[3]*10+longdata[4]+longdata[6]*0.1+longdata[7]*0.01+longdata[8]*0.001+longdata[9]*0.0001+longdata[10]*0.00001)/60);
			Present_posi.longtitude=longdata[0]*100+longdata[1]*10+longdata[2]+long_dec;
			Present_posi.latitude=((double)(latdata[0]*10+latdata[1])+lat_dec);
		
		}
	}
	
}




#include "angle.h"

/************************************��Ǵ��������ݴ���************************************/
/************************************��Ǵ��������ݴ���************************************/

//��ͷ0x55,��ʶλ���Ǽ��ٶ�0x51�����ٶ�0x52���Ƕ�0x53
//��Ǵ��������Ͷ�������33�ֽ�/֡������11�ֽ�Ϊ�Ǽ��ٶ����ݡ�11�ֽ�Ϊ���ٶ����ݡ�11�ֽ�Ϊ�Ƕ�����
//ֻ��Ҫ�õ��Ƕ�����

float X=0,Y=0,Z=0;//��Ǵ��������ݣ��ֱ��нǶȹ�ת�ǣ�x �ᣩ,�����ǣ�y �ᣩ,ƫ���ǣ�z �ᣩ
uint8_t AngleReal[11]={0};//��ǽǶ�
uint8_t AngleBuffer_old[66]={0};//�������ݽ��ռĴ���(��֡����)
uint8_t AngleBuffer[33]={0};//������ݼĴ�����һ֡���������ݣ�
uint8_t AngleData[angle_data_len]={0};


void Angle_Data_Handle(void)//������ݴ�����
{
		uint8_t AngleTemp[66]={0};
		uint8_t AngleClear[66]={0};//����Ĵ���
		uint8_t  i=0;
		//�Ȱѽ��յ����ݷ����м�Ĵ���
		memcpy(&AngleTemp[0],&AngleBuffer_old[0],sizeof(uint8_t)*66);
		//��Ǵ�����ԭʼ���ݣ���ʱ���ܴ������Ĳ��ǰ��ֲ��ϵ�����˳��
		//���Խ���66�ֽڣ���֡�����ݣ��ҵ�֡ͷ0x55��Ű�������һ�����ݷ���AngleBuffer��		
		for(i=0;i<11;i++)
		{
			if(AngleTemp[i]==0x55)//�ҳ���ͷ����ȡһ֡�������ݵ�AngleBuffer
			{
				memcpy(&AngleBuffer[0],&AngleTemp[i],sizeof(uint8_t)*33);
			}
		}
    if(AngleBuffer[0]!=0x55)//������ǵ�һ֡���ݣ�������
		{
		memcpy(&AngleBuffer[0],&AngleClear,sizeof(uint8_t)*33);
		}
	  else//������������ʱ����ʼ����
	  {
			if(AngleBuffer[1]==0x51)//�жϱ�ʶλ������ֻ��Ҫ�Ƕ����ݣ���0x53֮���11λ���ݣ�
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

		//�Ƕȼ��㣬���幫ʽ�ο�ģ���ֲ�ά������JY62
		X=(float)(((short)AngleReal[3]<<8)|AngleReal[2])/32768*180*100;//��100����ȥ��1000ʹ��
		Y=(float)(((short)AngleReal[5]<<8)|AngleReal[4])/32768*180*100;
		Z=(float)(((short)AngleReal[7]<<8)|AngleReal[6])/32768*180*100;
		AngleData[0]=(uint8_t)X;//�Ͱ�λ
		AngleData[1]=X/256;//�߰�λ
		AngleData[2]=(uint8_t)Y;
		AngleData[3]=Y/256;
		AngleData[4]=(uint8_t)Z;
		AngleData[5]=Z/256;

}


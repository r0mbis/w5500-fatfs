#include "SPI_SDcard.h"
#include "spi.h"
#include "sysVar.h"

/*�������ԣ�https://blog.csdn.net/qq_37182134/article/details/83859618*/

/*

	���԰����˵����
		DAT3��CS�ߣ�		PC11
		CLK��SCK�ߣ�		PC12
		CMD��MOSI�ߣ�		PD2
		DAT0��MISO��		PC8
		���ź�����Ҫ10~100K�������裬���԰���Ӳ���Ѿ�������
		
		
	��Ŀ�����˵����
		DAT3��CS�ߣ�		PC5
		CLK��SCK�ߣ�		PB14
		CMD��MOSI�ߣ�		PB15
		DAT0��MISO��		PC13
*/

SPI_SD_CardInfo SPI_SDcardInfo;	//Информация о SD-карте (получается автоматически во время инициализации)


/*ѡ����ģ��SPI����Ӳ��SPI��0��ģ��SPI��1��Ӳ��SPI
	�����ģ��SPI���������涨���IO�ӿںͳ�ʼ��
	�����Ӳ��SPI����ȥ��һ��Ӳ��SPI���ٵ�PeriphConfig.c���涨��ʹ��CS���ŵĽӿںͳ�ʼ��
*/

#define SPI_SD_Use_simuSPI_or_hardwareSPI	1	


#if (SPI_SD_Use_simuSPI_or_hardwareSPI == 0)
	//�ӿڶ��壬�û��޸�
	
	#define SDcard_simuSPI_CS 	PBout(14) 		//simuSPI��CS��
	#define SDcard_simuSPI_SCK 	simuSPI_SCK		//simuSPI��SCK��
	#define SDcard_simuSPI_MOSI simuSPI_MOSI 	//simuSPI��MOSI��
	#define SDcard_simuSPI_MISO simuSPI_MISO 	//simuSPI��MISO��
	
	#define CS_H SDcard_simuSPI_CS = IO_High
	#define CS_L SDcard_simuSPI_CS = IO_Low
	
	#define SPI_ReadWrite_Byte(x) SimuSPI_ReadWriteByte(&simuSPI_Handle,x) //SPI���������ӿڶ���
	
	void SDcard_simuSPI_CS_IO_init(void)
	{
		//simuSPI��CS���ų�ʼ�����û��޸�
		GPIO_InitTypeDef GPIO_Initure;
		
		__HAL_RCC_GPIOB_CLK_ENABLE();	
		
		GPIO_Initure.Pin = GPIO_PIN_14;
		GPIO_Initure.Mode = GPIO_MODE_OUTPUT_PP;  	//�������
		GPIO_Initure.Pull=GPIO_PULLUP;          	//����
		GPIO_Initure.Speed=GPIO_SPEED_HIGH;     	//����
		HAL_GPIO_Init(GPIOB,&GPIO_Initure);
	}

#endif

#if (SPI_SD_Use_simuSPI_or_hardwareSPI == 1)
	//�ӿڶ��壬�û��޸�
	#define CS_H SPI2_CS_H
	#define CS_L SPI2_CS_L
	
	uint8_t SPI_ReadWrite_Byte(uint8_t TxData)
	{
		uint8_t Rxdata;
		HAL_SPI_TransmitReceive(&hspi2,&TxData,&Rxdata,1, 100);
		return Rxdata;
	}

#endif

/*�����ǹ̶�����Ĳ���*/
//IO��ʼ��
void SDcard_SPI_Init(void)
{
	#if (SPI_SD_Use_simuSPI_or_hardwareSPI == 0)
		
		simuSPI_IO_init();
		SDcard_simuSPI_CS_IO_init();

		HAL_Delay(10);
	#endif
	
	#if (SPI_SD_Use_simuSPI_or_hardwareSPI == 1)
		//Ӳ��SPI��ʼ�����û��޸�
		//sys_SPI1_ENABLE();
		void MX_SPI2_Init(void);
		HAL_Delay(10);
	#endif
	
	SPI_ReadWrite_Byte(0xff);
}

//ȡ��Ƭѡ �ͷ�SPI����
void SD_DisSelect(void)
{
  CS_H;
  SPI_ReadWrite_Byte(0xff);
}
//�ȴ���׼����
//׼�����˷���ֵ0�������������
uint8_t SD_WaitReady(void)
{
  uint32_t t=0;
  do
  {
	if(SPI_ReadWrite_Byte(0xff)==0XFF)return 0;//OK
	t++;		  	
  }while(t<0X000FFF);
  return 1;
}
//ѡ��SD�����ȴ���׼����
//����ֵ 0�ɹ���1ʧ��
uint8_t SD_Select(void)
{
  CS_L;
  if(SD_WaitReady()==0)return 0;
  SD_DisSelect();
  return 1;
}
//�����Ӧ
//Response Ҫ�õ��Ļ�Ӧ?
//������Ӧ����
uint8_t SD_GetResponse(uint8_t Response)
{
  uint16_t Count=0xFFFF;   		//�ȴ�����				  
  while ((SPI_ReadWrite_Byte(0xff)!=Response)&&Count)Count--;	  
  if (Count==0)return MSD_RESPONSE_FAILURE;  
  else return MSD_RESPONSE_NO_ERROR;
}
//Читаем содержимое пакета данных с SD-карты
// область буффера данных Buf
//Длина читаемых данных len
uint8_t SD_RecvData(uint8_t* buf,uint16_t len)
{			  	  
  if(SD_GetResponse(0xFE))return 1;
  while(len--)
  {
    *buf=SPI_ReadWrite_Byte(0xff);
     buf++;
  }
  SPI_ReadWrite_Byte(0xff);
  SPI_ReadWrite_Byte(0xff);									  					    
  return 0;
}
//��SD��д��һ�����ݰ�������512�ֽ�
//buf���ݻ���
//����
//����ֵ0�ɹ� ����ʧ��
uint8_t SD_SendBlock(uint8_t* buf,uint8_t cmd)
{	
  uint16_t t;		  	  
  if(SD_WaitReady())return 1;
  SPI_ReadWrite_Byte(cmd);
  if(cmd!=0XFD)
  {
	for(t=0;t<512;t++)SPI_ReadWrite_Byte(buf[t]);
	SPI_ReadWrite_Byte(0xFF);
	SPI_ReadWrite_Byte(0xFF);
	t=SPI_ReadWrite_Byte(0xFF);
	if((t&0x1F)!=0x05)return 2;								  					    
   }						 									  					    
  return 0;
}
//Отправляем команду на SD-карту
//uint8_t cmd  команда
//u32 arg параметр команды аргумент
uint8_t SD_SendCmd(uint8_t cmd, uint32_t arg, uint8_t crc)
{
  uint8_t r1;	
  uint8_t Retry=0; 
  SD_DisSelect();
  if(SD_Select())return 0XFF;
    SPI_ReadWrite_Byte(cmd | 0x40);
    SPI_ReadWrite_Byte(arg >> 24);
    SPI_ReadWrite_Byte(arg >> 16);
    SPI_ReadWrite_Byte(arg >> 8);
    SPI_ReadWrite_Byte(arg);	  
    SPI_ReadWrite_Byte(crc); 
  if(cmd==CMD12) SPI_ReadWrite_Byte(0xff);//Skip a stuff byte when stop reading
	Retry=0X1F;
  do
   {
	 r1=SPI_ReadWrite_Byte(0xFF);
   }while((r1&0X80) && Retry--);	 
  return r1;
}
//��ȡSD����CID��Ϣ���� ������������Ϣ
//u8 *cid_data	���CID���ڴ�����16���ֽ�
//���� 0:NO_ERR
//		 1:����														   
uint8_t SD_GetCID(uint8_t *cid_data)
{
  uint8_t r1;	   
  r1=SD_SendCmd(CMD10,0,0x01);
  if(r1==0x00)
  {
	r1=SD_RecvData(cid_data,16);	//����16���ֽڵ����� 
  }
  SD_DisSelect();
  if(r1)return 1;
  else return 0;
}																				  
//��ȡSD����CSD��Ϣ���� �����������ٶ���Ϣ
//u8 *cid_data	���CsD���ڴ�����16���ֽ�
//���� 0:NO_ERR
//		 1:����														   
uint8_t SD_GetCSD(uint8_t *csd_data)
{
  uint8_t r1;	 
  r1=SD_SendCmd(CMD9,0,0x01);
  if(r1==0)
  {
     r1=SD_RecvData(csd_data, 16); 
   }
  SD_DisSelect();
  if(r1)return 1;
  else return 0;
}  
//��ȡSD����������
//����ֵ:0: ȡ����������
//      ����:SD����(������/512�ֽ�)														  
uint32_t SD_GetSectorCount(void)
{
  uint8_t csd[16];
  uint32_t Capacity;  
  uint8_t n;
  uint16_t csize;  					    
  //ȡCSD��Ϣ
  if(SD_GetCSD(csd)!=0) return 0;	    
    
  if((csd[0]&0xC0)==0x40)	 //V2.00��
  {	
	 csize = csd[9] + ((uint16_t)csd[8] << 8) + 1;
	 Capacity = (uint32_t)csize << 10;//�õ�������		   
  }else//V1.XX��
   {	
	  n = (csd[5] & 15) + ((csd[10] & 128) >> 7) + ((csd[9] & 3) << 1) + 2;
	  csize = (csd[8] >> 6) + ((uint16_t)csd[7] << 2) + ((uint16_t)(csd[6] & 3) << 10) + 1;
	  Capacity= (uint32_t)csize << (n - 9);//�õ������� 
   }
   return Capacity;
}

uint8_t SD_Type;

//��ʼ��SD
//����0�ɹ�������ʧ��
uint8_t SPI_SD_Init(void)
{
  uint8_t r1;      // 
  uint16_t retry;  // 
  uint8_t buf[4];  
  uint16_t i;
 
  SDcard_SPI_Init();	//��ʼ��IO
	
  for(i=0;i<10;i++)SPI_ReadWrite_Byte(0XFF);//��������74������
  retry=20;
  do
  {
	 r1=SD_SendCmd(CMD0,0,0x95);//����IDLE״̬
   }while((r1!=0X01) && retry--);
   SD_Type=0;//Ĭ���޿�
  if(r1==0X01)
   {
	  if(SD_SendCmd(CMD8,0x1AA,0x87)==1)//SD V2.0
	  {
		for(i=0;i<4;i++)buf[i]=SPI_ReadWrite_Byte(0XFF);	//Get trailing return value of R7 resp
		if(buf[2]==0X01&&buf[3]==0XAA)//���Ƿ�֧��2.7~3.6V
		{
		  retry=0XFFFE;
		  do
		  {
			 SD_SendCmd(CMD55,0,0X01);	//����CMD55
			 r1=SD_SendCmd(CMD41,0x40000000,0X01);//����CMD41
		    }while(r1&&retry--);
		  if(retry&&SD_SendCmd(CMD58,0,0X01)==0)//����SD2.0��ʼ
		  {
			 for(i=0;i<4;i++)buf[i]=SPI_ReadWrite_Byte(0XFF);//�õ�OCRֵ
			 if(buf[0]&0x40)SD_Type=SD_TYPE_V2HC;    //���CCS
			  else SD_Type=SD_TYPE_V2;   
		    }
		 }
		}else//SD V1.x/ MMC	V3
		{
		  SD_SendCmd(CMD55,0,0X01);		//����CMD55
		  r1=SD_SendCmd(CMD41,0,0X01);	//����CMD41
		  if(r1<=1)
		  {		
			SD_Type=SD_TYPE_V1;
			retry=0XFFFE;
			do 
			{
			  SD_SendCmd(CMD55,0,0X01);	//����CMD55
			  r1=SD_SendCmd(CMD41,0,0X01);//����CMD41
			}while(r1&&retry--);
		  }else
		     {
			   SD_Type=SD_TYPE_MMC;//MMC V3
			   retry=0XFFFE;
			   do 
			    {											    
				  r1=SD_SendCmd(CMD1,0,0X01);
			     }while(r1&&retry--);  
		   }
		  if(retry==0||SD_SendCmd(CMD16,512,0X01)!=0)SD_Type=SD_TYPE_ERR;
		}
   }
  SD_DisSelect();
   
//  SPI2_SetSpeed(SPI_BaudRatePrescaler_2);
	
   HAL_Delay(10);
   
   SPI_SD_ReadInfo();	//��ȡ����Ϣ������ SPI_SDcardInfo ����ṹ��
   
  if(SD_Type)return 0;
  else if(r1)return r1; 	   
  return 0xaa;
}
uint8_t SPI_SD_ReadDisk2(uint8_t*buf,uint32_t sector,uint8_t cnt)
{
  uint8_t r1;
  if(SD_Type!=SD_TYPE_V2HC)sector <<= 9;
  if(cnt==1)
  {
	r1=SD_SendCmd(CMD17,sector,0X01);
	if(r1==0)
	{
	  r1=SD_RecvData(buf,512);	   
	}
  }else{
	   r1=SD_SendCmd(CMD18,sector,0X01);
	   do
	   {
		  r1=SD_RecvData(buf,512);	 
		  buf+=512;  
		}while(--cnt && r1==0); 	
	   SD_SendCmd(CMD12,0,0X01);	
  }   
  SD_DisSelect();
  return r1;
}
//u8*buf���ݻ���
//sector��ʼ����
//������
uint8_t SPI_SD_WriteDisk2(uint8_t*buf,uint32_t sector,uint8_t cnt)
{
  uint8_t r1;
  if(SD_Type!=SD_TYPE_V2HC)sector *= 512;
  if(cnt==1)
  {
	 r1=SD_SendCmd(CMD24,sector,0X01);
	 if(r1==0)
	 {
		r1=SD_SendBlock(buf,0xFE);  
	  }
   }else
    {
	  if(SD_Type!=SD_TYPE_MMC)
	   {
		  SD_SendCmd(CMD55,0,0X01);	
		  SD_SendCmd(CMD23,cnt,0X01);
		}
 	  r1=SD_SendCmd(CMD25,sector,0X01);
	  if(r1==0)
	  {
		do
		{
		  r1=SD_SendBlock(buf,0xFC);	 
		  buf+=512;  
		 }while(--cnt && r1==0);
		 r1=SD_SendBlock(0,0xFD);
		}
	}   
  SD_DisSelect();
  return r1;
}

//��ȡSPI������SD������Ϣ
//����0�ɹ�
uint8_t SPI_SD_ReadInfo(void)
{
	uint8_t cid_buf[16],csd_buf[16],sta = 1;
	
	SPI_SDcardInfo.CardType = SD_Type;
	
	sta = SD_GetCID(cid_buf);
	if(sta == 0)
	{
		SPI_SDcardInfo.SD_cid.ManufacturerID = cid_buf[0];
		
		SPI_SDcardInfo.SD_cid.OEM_AppliID = cid_buf[1]<<8;
		SPI_SDcardInfo.SD_cid.OEM_AppliID |= cid_buf[2];
		
		SPI_SDcardInfo.SD_cid.ProdName1 = cid_buf[3]<<24;
		SPI_SDcardInfo.SD_cid.ProdName1 |= cid_buf[4]<<16;
		SPI_SDcardInfo.SD_cid.ProdName1 |= cid_buf[5]<<8;
		SPI_SDcardInfo.SD_cid.ProdName1 |= cid_buf[6];
		
		SPI_SDcardInfo.SD_cid.ProdName2 = cid_buf[7];
		
		SPI_SDcardInfo.SD_cid.ProdRev = cid_buf[8];
		
		SPI_SDcardInfo.SD_cid.ProdSN = cid_buf[9]<<24;
		SPI_SDcardInfo.SD_cid.ProdSN |= cid_buf[10]<<16;
		SPI_SDcardInfo.SD_cid.ProdSN |= cid_buf[11]<<8;
		SPI_SDcardInfo.SD_cid.ProdSN |= cid_buf[12];
		
		SPI_SDcardInfo.SD_cid.Reserved1 |= (cid_buf[13] & 0xf0)>>4;
		
		SPI_SDcardInfo.SD_cid.ManufactDate = (cid_buf[13] &0x0f)<<8;
		SPI_SDcardInfo.SD_cid.ManufactDate |= cid_buf[14];
		
		SPI_SDcardInfo.SD_cid.CID_CRC =  (cid_buf[15] & 0xfe) >> 1;
		
		SPI_SDcardInfo.SD_cid.Reserved2 = 1;
	}else{return 1;}
	
	sta = SD_GetCSD(csd_buf);
	if(sta == 0)
	{
		SPI_SDcardInfo.SD_csd.CSDStruct = (csd_buf[0] & 0xC0) >> 6;
		SPI_SDcardInfo.SD_csd.SysSpecVersion = (csd_buf[0] & 0x3C) >> 2;
		SPI_SDcardInfo.SD_csd.Reserved1 = csd_buf[0] & 0x03;
		
		SPI_SDcardInfo.SD_csd.TAAC = csd_buf[1];
		
		SPI_SDcardInfo.SD_csd.NSAC = csd_buf[2];
		
		SPI_SDcardInfo.SD_csd.MaxBusClkFrec = csd_buf[3];
		
		SPI_SDcardInfo.SD_csd.CardComdClasses = csd_buf[4] << 4;
		SPI_SDcardInfo.SD_csd.CardComdClasses |= (csd_buf[5] & 0xF0) >> 4;
		
		SPI_SDcardInfo.SD_csd.RdBlockLen = csd_buf[5] & 0x0F;
		
		SPI_SDcardInfo.SD_csd.PartBlockRead = (csd_buf[6] & 0x80) >> 7;
		SPI_SDcardInfo.SD_csd.WrBlockMisalign = (csd_buf[6] & 0x40) >> 6;
		SPI_SDcardInfo.SD_csd.RdBlockMisalign = (csd_buf[6] & 0x20) >> 5;
		SPI_SDcardInfo.SD_csd.DSRImpl = (csd_buf[6] & 0x10) >> 4;
		SPI_SDcardInfo.SD_csd.Reserved2 = 0;
		
		SPI_SDcardInfo.SD_csd.DeviceSize = (csd_buf[6] & 0x03) << 10;
		
		if ((SD_Type == SD_TYPE_V1) || (SD_Type == SD_TYPE_V2))
		{
			/*!< Byte 7 */
			SPI_SDcardInfo.SD_csd.DeviceSize |= (csd_buf[7]) << 2;
	 
			/*!< Byte 8 */
			SPI_SDcardInfo.SD_csd.DeviceSize |= (csd_buf[8] & 0xC0) >> 6;
	 
			SPI_SDcardInfo.SD_csd.MaxRdCurrentVDDMin = (csd_buf[8] & 0x38) >> 3;
			SPI_SDcardInfo.SD_csd.MaxRdCurrentVDDMax = (csd_buf[8] & 0x07);
	 
			/*!< Byte 9 */
			SPI_SDcardInfo.SD_csd.MaxWrCurrentVDDMin = (csd_buf[9] & 0xE0) >> 5;
			SPI_SDcardInfo.SD_csd.MaxWrCurrentVDDMax = (csd_buf[9] & 0x1C) >> 2;
			SPI_SDcardInfo.SD_csd.DeviceSizeMul = (csd_buf[9] & 0x03) << 1;
			/*!< Byte 10 */
			SPI_SDcardInfo.SD_csd.DeviceSizeMul |= (csd_buf[10] & 0x80) >> 7;
		}else if (SD_Type == SD_TYPE_V2HC)	//V2.0 SDHC card info.
		{
			SPI_SDcardInfo.SD_csd.DeviceSize = (csd_buf[7] & 0x3F) << 16;
	 
			SPI_SDcardInfo.SD_csd.DeviceSize |= (csd_buf[8] << 8);
		
			SPI_SDcardInfo.SD_csd.DeviceSize |= (csd_buf[9]);		
		}
		
		SPI_SDcardInfo.SD_csd.EraseGrSize = (csd_buf[10] & 0x40) >> 6;
		SPI_SDcardInfo.SD_csd.EraseGrMul = (csd_buf[10] & 0x3F) << 1;

		/*!< Byte 11 */
		SPI_SDcardInfo.SD_csd.EraseGrMul |= (csd_buf[11] & 0x80) >> 7;
		SPI_SDcardInfo.SD_csd.WrProtectGrSize = (csd_buf[11] & 0x7F);

		/*!< Byte 12 */
		SPI_SDcardInfo.SD_csd.WrProtectGrEnable = (csd_buf[12] & 0x80) >> 7;
		SPI_SDcardInfo.SD_csd.ManDeflECC = (csd_buf[12] & 0x60) >> 5;
		SPI_SDcardInfo.SD_csd.WrSpeedFact = (csd_buf[12] & 0x1C) >> 2;
		SPI_SDcardInfo.SD_csd.MaxWrBlockLen = (csd_buf[12] & 0x03) << 2;

		/*!< Byte 13 */
		SPI_SDcardInfo.SD_csd.MaxWrBlockLen |= (csd_buf[13] & 0xC0) >> 6;
		SPI_SDcardInfo.SD_csd.WriteBlockPaPartial = (csd_buf[13] & 0x20) >> 5;
		SPI_SDcardInfo.SD_csd.Reserved3 = 0;
		SPI_SDcardInfo.SD_csd.ContentProtectAppli = (csd_buf[13] & 0x01);

		/*!< Byte 14 */
		SPI_SDcardInfo.SD_csd.FileFormatGrouop = (csd_buf[14] & 0x80) >> 7;
		SPI_SDcardInfo.SD_csd.CopyFlag = (csd_buf[14] & 0x40) >> 6;
		SPI_SDcardInfo.SD_csd.PermWrProtect = (csd_buf[14] & 0x20) >> 5;
		SPI_SDcardInfo.SD_csd.TempWrProtect = (csd_buf[14] & 0x10) >> 4;
		SPI_SDcardInfo.SD_csd.FileFormat = (csd_buf[14] & 0x0C) >> 2;
		SPI_SDcardInfo.SD_csd.ECC = (csd_buf[14] & 0x03);

		/*!< Byte 15 */
		SPI_SDcardInfo.SD_csd.CSD_CRC = (csd_buf[15] & 0xFE) >> 1;
		SPI_SDcardInfo.SD_csd.Reserved4 = 1;
	}else{return 1;}
	
	if ((SD_Type == SD_TYPE_V1) || (SD_Type == SD_TYPE_V2))
	{
		SPI_SDcardInfo.CardCapacity=(SPI_SDcardInfo.SD_csd.DeviceSize+1);			//���㿨����
		SPI_SDcardInfo.CardCapacity*=(1<<(SPI_SDcardInfo.SD_csd.DeviceSizeMul+2));
		SPI_SDcardInfo.CardBlockSize=1<<(SPI_SDcardInfo.SD_csd.RdBlockLen);			//���С
		SPI_SDcardInfo.CardCapacity*=SPI_SDcardInfo.CardBlockSize;
	}else if(SD_Type == SD_TYPE_V2HC)
	{
 		SPI_SDcardInfo.CardCapacity=(long long)(SPI_SDcardInfo.SD_csd.DeviceSize+1)*512*1024;//���㿨����
		SPI_SDcardInfo.CardBlockSize=512; 			//���С�̶�Ϊ512�ֽ�
	}
	
	return 0;
}






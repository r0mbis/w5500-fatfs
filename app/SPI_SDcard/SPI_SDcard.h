#ifndef __SPI_SDCARD_H
#define __SPI_SDCARD_H
#include <stdint.h>
//#include "simuspi.h"		//ģ��SPIЭ��


//SD��CSD�Ĵ�������		  
typedef struct
{
	uint8_t  CSDStruct;            /*!< CSD structure */
	uint8_t  SysSpecVersion;       /*!< System specification version */
	uint8_t  Reserved1;            /*!< Reserved */
	uint8_t  TAAC;                 /*!< Data read access-time 1 */
	uint8_t  NSAC;                 /*!< Data read access-time 2 in CLK cycles */
	uint8_t  MaxBusClkFrec;        /*!< Max. bus clock frequency */
	uint16_t CardComdClasses;      /*!< Card command classes */
	uint8_t  RdBlockLen;           /*!< Max. read data block length */
	uint8_t  PartBlockRead;        /*!< Partial blocks for read allowed */
	uint8_t  WrBlockMisalign;      /*!< Write block misalignment */
	uint8_t  RdBlockMisalign;      /*!< Read block misalignment */
	uint8_t  DSRImpl;              /*!< DSR implemented */
	uint8_t  Reserved2;            /*!< Reserved */
	uint32_t DeviceSize;           /*!< Device Size */
	uint8_t  MaxRdCurrentVDDMin;   /*!< Max. read current @ VDD min */
	uint8_t  MaxRdCurrentVDDMax;   /*!< Max. read current @ VDD max */
	uint8_t  MaxWrCurrentVDDMin;   /*!< Max. write current @ VDD min */
	uint8_t  MaxWrCurrentVDDMax;   /*!< Max. write current @ VDD max */
	uint8_t  DeviceSizeMul;        /*!< Device size multiplier */
	uint8_t  EraseGrSize;          /*!< Erase group size */
	uint8_t  EraseGrMul;           /*!< Erase group size multiplier */
	uint8_t  WrProtectGrSize;      /*!< Write protect group size */
	uint8_t  WrProtectGrEnable;    /*!< Write protect group enable */
	uint8_t  ManDeflECC;           /*!< Manufacturer default ECC */
	uint8_t  WrSpeedFact;          /*!< Write speed factor */
	uint8_t  MaxWrBlockLen;        /*!< Max. write data block length */
	uint8_t  WriteBlockPaPartial;  /*!< Partial blocks for write allowed */
	uint8_t  Reserved3;            /*!< Reserded */
	uint8_t  ContentProtectAppli;  /*!< Content protection application */
	uint8_t  FileFormatGrouop;     /*!< File format group */
	uint8_t  CopyFlag;             /*!< Copy flag (OTP) */
	uint8_t  PermWrProtect;        /*!< Permanent write protection */
	uint8_t  TempWrProtect;        /*!< Temporary write protection */
	uint8_t  FileFormat;           /*!< File Format */
	uint8_t  ECC;                  /*!< ECC code */
	uint8_t  CSD_CRC;              /*!< CSD CRC */
	uint8_t  Reserved4;            /*!< always 1*/
}SPI_SD_CSD; 
//SD��CID�Ĵ�������
typedef struct
{
	uint8_t  ManufacturerID;       /*!< ManufacturerID */
	uint16_t OEM_AppliID;          /*!< OEM/Application ID */
	uint32_t ProdName1;            /*!< Product Name part1 */
	uint8_t  ProdName2;            /*!< Product Name part2*/
	uint8_t  ProdRev;              /*!< Product Revision */
	uint32_t ProdSN;               /*!< Product Serial Number */
	uint8_t  Reserved1;            /*!< Reserved1 */
	uint16_t ManufactDate;         /*!< Manufacturing Date */
	uint8_t  CID_CRC;              /*!< CID CRC */
	uint8_t  Reserved2;            /*!< always 1 */
}SPI_SD_CID;	

//Информация о SD-карте, включая CSD, CID и другие данные
//TODO: Если в будущем вы захотите использовать несколько SD-карт SPI, запишите вывод CS каждой карты в структуре дескриптора карты ниже.
//Затем вам нужно изменить базовые функции, функции инициализации, чтения, записи и чтения информации в файле .c для поддержки различения разных карточек. Входной параметр функции — это дескриптор разных карточек
typedef struct
{
  SPI_SD_CSD SD_csd;
  SPI_SD_CID SD_cid;
  long long CardCapacity;  	//Емкость SD-карты, единица измерения: байт, поддерживаются карты с максимальным размером 2^64 байта.
  uint32_t CardBlockSize; 		//Размер блока SD-карты
  uint16_t RCA;					//Относительный адрес карты
  uint8_t CardType;				//Тип карты
}SPI_SD_CardInfo;

extern SPI_SD_CardInfo SPI_SDcardInfo;				//SD����Ϣ����ʼ��ʱ�Զ���ȡ��
uint8_t SPI_SD_Init(void);								//�û��ã�һ��������ʼ��SPI������SD��
uint8_t SPI_SD_ReadDisk2(uint8_t* buf,uint32_t sector,uint8_t cnt);		//�û��ã���SD��,fatfs/usb����
uint8_t SPI_SD_WriteDisk2(uint8_t* buf,uint32_t sector,uint8_t cnt);	//�û��ã�дSD��,fatfs/usb����


uint8_t SPI_SD_ReadInfo(void);
uint32_t SD_GetSectorCount(void);

// SD�����Ͷ���
#define SD_TYPE_ERR     0X00
#define SD_TYPE_MMC     0X01
#define SD_TYPE_V1      0X02
#define SD_TYPE_V2      0X04
#define SD_TYPE_V2HC    0X06	   
// SD��ָ���	   
#define CMD0    0       //����λ
#define CMD1    1
#define CMD8    8       //Команда 8 ,SEND_IF_COND
#define CMD9    9       //Команда 9, чтение данных CSD
#define CMD10   10      //Команда 10, прочитать данные CID
#define CMD12   12      //Команда 12, остановить передачу данных
#define CMD16   16      //Команда 16, установите SectorSize и верните 0x00
#define CMD17   17      //Команда 17, прочитать сектор
#define CMD18   18      //Команда 18, прочитать несколько секторов
#define CMD23   23      //Команда 23: настроить несколько секторов для предварительного стирания N блоков перед записью.
#define CMD24   24      //Команда 24, запись сектора
#define CMD25   25      //Команда 25, запись нескольких секторов
#define CMD41   41      //Команда 41, возврат 0x00
#define CMD55   55      //Команда 55, возврат 0x01
#define CMD58   58      //Команда 58, прочитать OCR
#define CMD59   59      //Команда 59, включение/отключение CRC, должна возвращать 0x00.
//����д���Ӧ������
#define MSD_DATA_OK                0x05
#define MSD_DATA_CRC_ERROR         0x0B
#define MSD_DATA_WRITE_ERROR       0x0D
#define MSD_DATA_OTHER_ERROR       0xFF
//SD����Ӧ��־��
#define MSD_RESPONSE_NO_ERROR      0x00
#define MSD_IN_IDLE_STATE          0x01
#define MSD_ERASE_RESET            0x02
#define MSD_ILLEGAL_COMMAND        0x04
#define MSD_COM_CRC_ERROR          0x08
#define MSD_ERASE_SEQUENCE_ERROR   0x10
#define MSD_ADDRESS_ERROR          0x20
#define MSD_PARAMETER_ERROR        0x40
#define MSD_RESPONSE_FAILURE       0xFF


/*
�����ܽ᣺
		��SD����������Ҫ�г�ʼ��������д�������ȡ�
		1�� ��ʼ�� ���裺
		��1��      ��ʱ����74clock
		��2��      ����CMD0����Ҫ����0x01������Idle״̬
		��3��      ѭ������CMD55+ACMD41��ֱ������0x00������Ready״̬��
		�����MMC���˲�Ӧ����CMD1��
		2�� �� ���裺
		��1��      ����CMD17�����飩��CMD18����飩���������0x00
		��2��      �������ݿ�ʼ����0xfe����0xfc�� + ��ʽ����512Bytes + CRC У��2Bytes
		Ĭ����ʽ��������ݳ�����512Bytes������CMD16���á�
		3�� д ���裺
		��1��      ����CMD24�����飩��CMD25����飩д�������0x00
		��2��      �������ݿ�ʼ����0xfe����0xfc�� + ��ʽ����512Bytes + CRCУ��2Bytes
		4�� ���� ���裺
		��1��      ����CMD32����һ��������ָ���׸�Ҫ�����������ţ�SD�ֲ���˵�ǿ�ţ�
		��2��      ����CMD33,��ָ������������
		��3��      ����CMD38������ָ�����������
		��3��˳���ܵߵ���
		��Ҫע�ⷢ��CMD����ʱ������Ҫ��һ���ֽڵ�CRCУ�����ݣ���֮Ҫ��֤ÿ�η��͵����ݰ����ȷ���Э��Ҫ��������ݷ���ʱ��Ҫ��
		
		
		 ��sd����ʼ���������棬��������sd����ʼ�����̴�������һ���ģ�һЩ�ر�ĵط��У�
			1.�ڳ�ʼ�������ǰ����Ҫ����72��ʱ�����ڵ�ʱ���źŸ�sd������Ϊʹ���ź�
			2.����cmd0ʱ�����sd����⵽DAT3Ϊ����״̬���ͻ��Զ�����SPIģʽ�����ҷ���һ����Ϊ0xff��ֵ��Ϊ��Ӧ
			3.�ڷ���acmd41ʱ����������ֵ����0xff8000������0
			4.�ڶ�ȡsd��cid�Ĵ���ʱʹ�õ�������cmd10������cmd2
			5.spiģʽ��֧��ѡ������cmd3
*/



#endif


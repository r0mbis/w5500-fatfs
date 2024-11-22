#include "main.h"
#include "FreeRTOS.h"
#include "task.h"
#include "event_groups.h"
#include "wizchip_conf.h"
#include "dhcp.h"
#include "spi.h"
#include "sys_log.h"
#include "w5500_driver.h"
#include "server_device.h"

#define W5500_RESET_0 HAL_GPIO_WritePin(W5500_RESET_GPIO_Port,W5500_RESET_Pin,GPIO_PIN_RESET)
#define W5500_RESET_1 HAL_GPIO_WritePin(W5500_RESET_GPIO_Port,W5500_RESET_Pin,GPIO_PIN_SET)

wiz_NetInfo network_info = {
	.mac = { 0xEA, 0x11, 0x22, 0x33, 0x44, 0xEA },
	.ip = {192, 168, 31 , 5},
	.gw = {192, 168, 31 , 1},
	.sn = {255, 255, 255, 0},
	.dns = {8, 8, 8, 8},
	.dhcp = NETINFO_DHCP
};
netmode_type g_net_mode;

// 1K should be enough, see https://forum.wiznet.io/t/topic/1612/2
static uint8_t dhcp_buffer[1024] = { 0, };
// 1K seems to be enough for this buffer as well
uint8_t dns_buffer[1024];

volatile bool dhcp_get_ip = false;

void dhcp_IPconflict_callback(void)
{
	log_i(ETH_TAG, "ip conflict from DHCP\r\n");
	// halt or reset or any...
	while(1)
	{
		vTaskDelay(1000 * 1000);
	}
}

void dhcp_IPassigned_callback(void)
{
	getIPfromDHCP(network_info.ip);
	getGWfromDHCP(network_info.gw);
	getSNfromDHCP(network_info.sn);
	getDNSfromDHCP(network_info.dns);
	network_info.dhcp = NETINFO_DHCP;
	network_initialize(network_info); // apply from DHCP
	print_network_information(network_info);

	log_i(ETH_TAG, "DHCP leased time : %ld seconds\r\n", getDHCPLeasetime());
}

void InitW5500(void)
{
	log_i(ETH_TAG, "\nStarted, waiting for phy ....\r\n");
	W5500_RESET_0;
	HAL_Delay(100);
	W5500_RESET_1;
	HAL_Delay(100);
	reg_wizchip_cris_cbfunc(SPI_CrisEnter, SPI_CrisExit);
	reg_wizchip_cs_cbfunc(SPI_CS_Select, SPI_CS_Deselect);
	reg_wizchip_spi_cbfunc(SPI_ReadByte, SPI_WriteByte);
	reg_wizchip_spiburst_cbfunc(SPI_Readbuff, SPI_Writebuff);

	uint8_t temp;
	uint8_t rx_tx_buff_sizes[] = {2, 2, 2, 2, 2, 2, 2, 2};
	wizchip_init(rx_tx_buff_sizes, rx_tx_buff_sizes);

	/* Check PHY link status */
	do
	{
		if (ctlwizchip(CW_GET_PHYLINK, (void *)&temp) == -1)
		{
			log_i(ETH_TAG, "Unknown PHY link status\r\n");
			
			return;
		}
	} while (temp == PHY_LINK_OFF);

	if (getVERSIONR() != 0x04)
	{
		log_i(ETH_TAG, " ACCESS ERR : VERSION != 0x04, read value = 0x%02x\r\n", getVERSIONR());
		while (1)
			;
	}
	// set MAC address before using DHCP
	setSHAR(network_info.mac);

	//network_init();
}


void dhcp_init(EventGroupHandle_t eventgroup)
{
	log_i(ETH_TAG, "DHCP client running\r\n");
	DHCP_init(DHCP_SOCKET, dhcp_buffer);
    reg_dhcp_cbfunc(dhcp_IPassigned_callback, dhcp_IPassigned_callback, dhcp_IPconflict_callback);
	xEventGroupClearBits(eventgroup, EVENT_DHCP_IP_RECEIVED);
	//dhcp_get_ip = false;
}

void network_initialize(wiz_NetInfo net_info)
{
	ctlnetwork(CN_SET_NETINFO, (void *)&net_info);
}

void print_network_information(wiz_NetInfo net_info)
{
    uint8_t tmp_str[8] = {
        0,
    };

    ctlnetwork(CN_GET_NETINFO, (void *)&net_info);
    ctlwizchip(CW_GET_ID, (void *)tmp_str);

    if (net_info.dhcp == NETINFO_DHCP)
    {
        log_i(ETH_TAG, "====================================================================================================\r\n");
        log_i(ETH_TAG, " %s network configuration : DHCP\r\n\n", (char *)tmp_str);
    }
    else
    {
        log_i(ETH_TAG, "====================================================================================================\r\n");
        log_i(ETH_TAG, " %s network configuration : static\r\n\n", (char *)tmp_str);
    }

    log_i(ETH_TAG, " MAC         : %02X:%02X:%02X:%02X:%02X:%02X\r\n", net_info.mac[0], net_info.mac[1], net_info.mac[2], net_info.mac[3], net_info.mac[4], net_info.mac[5]);
    log_i(ETH_TAG, " IP          : %d.%d.%d.%d\r\n", net_info.ip[0], net_info.ip[1], net_info.ip[2], net_info.ip[3]);
    log_i(ETH_TAG, " Subnet Mask : %d.%d.%d.%d\r\n", net_info.sn[0], net_info.sn[1], net_info.sn[2], net_info.sn[3]);
    log_i(ETH_TAG, " Gateway     : %d.%d.%d.%d\r\n", net_info.gw[0], net_info.gw[1], net_info.gw[2], net_info.gw[3]);
    log_i(ETH_TAG, " DNS         : %d.%d.%d.%d\r\n", net_info.dns[0], net_info.dns[1], net_info.dns[2], net_info.dns[3]);
    log_i(ETH_TAG, "====================================================================================================\r\n\n");
}

void SPI_WriteByte(unsigned char TxData)
{
	unsigned char sendbuf;
	sendbuf = TxData;
	HAL_StatusTypeDef res;
	res = HAL_SPI_Transmit(&hspi1, &sendbuf, 1, 255);
	switch(res)
	{
		case HAL_OK :
			//printf("send ok \r\n");
			break;
		case HAL_ERROR :
			//printf("send HAL_ERROR\r\n");
			break;
		case HAL_BUSY :
			//printf("send HAL_BUSY\r\n");
			break;
		case HAL_TIMEOUT :
			//printf("send HAL_TIMEOUT\r\n");
			break;
	}
}

unsigned char SPI_ReadByte(void)
{
	unsigned char recvbuf;
	HAL_StatusTypeDef res;	
	res = HAL_SPI_Receive(&hspi1, &recvbuf, 1, 255);
	switch(res)
	{
		case HAL_OK :
			//printf("recv ok \r\n");
			break;
		case HAL_ERROR :
			//printf("recv HAL_ERROR\r\n");
			break;
		case HAL_BUSY :
			//printf("recv HAL_BUSY\r\n");
			break;
		case HAL_TIMEOUT :
			//printf("recv HAL_TIMEOUT\r\n");
			break;
	}
	return recvbuf;
}

void SPI_Writebuff(uint8_t* buff, uint16_t len)
{
	HAL_SPI_Transmit(&hspi1, buff, len, HAL_MAX_DELAY);
}

void SPI_Readbuff(uint8_t* buff, uint16_t len)
{
	HAL_SPI_Receive(&hspi1, buff, len, HAL_MAX_DELAY);
}
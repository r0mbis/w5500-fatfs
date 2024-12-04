#include "main.h"
#include "wizchip_conf.h"
#include "server_device.h"
#include "sys_log.h"
#include "socket.h"
#include "ping.h"
#include <stdlib.h>
#include <string.h>

char *login_list[] = 
{
    "roman",
    "vasia",
    NULL
};

char *password_list[] =
{
    "12345",
    "54321",
    NULL
};

// ftp_data_t ftp_data = {
//     .login = login_list,
//     .password = password_list
// };
extern uint8_t ping_reply_received;
extern PINGMSGR PingReply;
extern volatile bool dhcp_get_ip;

void server_loop(socket_data_t *socket_info)
{
    int32_t ret = 0;
    uint16_t size = 0;
    switch (getSn_SR(socket_info->socket_id))
    {
        case SOCK_ESTABLISHED:
            if (!socket_info->socket_open)
            {
                socket_info->socket_open = true;
                socket_info->last_command_send = HAL_GetTick();
                socket_info->last_command_received = HAL_GetTick();
                log_i(ETH_TAG, "[%d]: SOCK_ESTABLISHED\r\n", socket_info->socket_id);
            } 
            if ((size = getSn_RX_RSR(socket_info->socket_id)) > 0)
            {
                memset(socket_info->receive_buffer + socket_info->receive_size, 0, size);
                ret = recv(socket_info->socket_id, socket_info->receive_buffer, size);
                if (ret != size)
                {
                    log_i(ETH_TAG, "[%d]: Received size is not equal to read size. Closing socket.\r\n", socket_info->socket_id);
                    if (ret == SOCK_BUSY)
                        return;
                    if (ret < 0)
                    {
                        socket_info->socket_open = false;
                        close(socket_info->socket_id);
                        return;
                    }
                }
                else
                {
                    socket_info->receive_size += size;
                    socket_info->last_command_received = HAL_GetTick();
                }
            }
            else
            {
                uint32_t time_now = HAL_GetTick();
                uint32_t diff_time_no_command = time_now - socket_info->last_command_received;
                if (diff_time_no_command > (TIMEOUT_SECONDS * 1000))
                {
                    //Close the connection when no data received in the last 15 seconds
                    log_i(ETH_TAG, "[%d]: Closing due to timeout without command.\r\n", socket_info->socket_id);
                    socket_info->socket_open = false;
                    close(socket_info->socket_id);
                    return;
                }
            }
            if (socket_info->send_size > 0)
            {
                ret = send(socket_info->socket_id, socket_info->send_buffer, socket_info->send_size);
                socket_info->send_size = 0;
                socket_info->last_command_send = HAL_GetTick();
                if (ret < 0)
                {
                    socket_info->socket_open = false;
                    close(socket_info->socket_id);
                    return;
                }
            }
            break;

        case SOCK_CLOSE_WAIT:
            log_i(ETH_TAG, "[%d]: SOCK_CLOSE_WAIT\r\n", socket_info->socket_id);
            ret = disconnect(socket_info->socket_id);
            if (ret !=SOCK_OK)
                return;
            socket_info->socket_open = false;
            break;

        case SOCK_CLOSED:
            log_i(ETH_TAG, "[%d]: SOCK_CLOSED\r\n", socket_info->socket_id);
            if (socket(socket_info->socket_id, Sn_MR_TCP, LISTENING_PORT, 0x0) != socket_info->socket_id)
            {
                socket_info->socket_open = false;
                close(socket_info->socket_id);
                return;
            }
            break;
            
        case SOCK_INIT:
            log_i(ETH_TAG, "[%d]: SOCK_INIT\r\n", socket_info->socket_id);
            socket_info->send_size = 0;
            socket_info->receive_size = 0;
            if(listen(socket_info->socket_id) != SOCK_OK)
            {
                return;
            }
            break;
        
        default:
            break;
    }
}

void udp_loop(socket_data_udp_t *socket_info)
{
    int32_t ret;
    uint16_t size;
    uint16_t sentsize;
    switch (getSn_SR(socket_info->socket_id))
    {
        case SOCK_UDP:
            if ((size = getSn_RX_RSR(socket_info->socket_id)) > 0)
            {
                if (size > BUFFER_SIZE)
                    size = BUFFER_SIZE;
                memset(socket_info->receive_buffer, 0, size);
                ret = recvfrom(socket_info->socket_id, socket_info->receive_buffer, size, socket_info->destip, &(socket_info->destport));
                if (ret < 0)
                {
                    log_e(ETH_TAG, "\t%d: recvfrom() SOCKET UDP ERROR: %d", socket_info->socket_id, ret);
                    return;
                }
                if (strcmp((void*)socket_info->receive_buffer, "Connect\n") == 0)
                {
                    size = strlen("Connect\n") + 1;
                    sentsize = 0;
                    do
                    {
                        ret = sendto(socket_info->socket_id, socket_info->receive_buffer + sentsize, size, socket_info->destip, socket_info->destport);
                        if (ret < 0)
                            return;
                        sentsize += ret;    // Don't care SOCKERR_BUSY, because it is zero.
                    } while (sentsize != size);
                }
                else if (strcmp((char*)socket_info->receive_buffer, "Firmware\n") == 0)
                {
                    log_i(ETH_TAG, "\tJump To Bootloader");
                    //Reset_MCU();
                    //JumpToAddrFlash(ADDR_FLASH_PAGE_BOOTLOADER);
                }
                else if (strcmp((char*)socket_info->receive_buffer, "Bootloader\n") == 0)
                {
                    log_i(ETH_TAG, "\tWrite Bootloader");
                    //Write_Bootloader_Flash();
                }

            }
            break;
        case SOCK_CLOSED:
            log_i(ETH_TAG, "\t[%d]: Closed, UDP, port [%d]", socket_info->socket_id, socket_info->sourceport);
            if (socket(socket_info->socket_id, Sn_MR_UDP, socket_info->sourceport, 0x0) != socket_info->socket_id)
            {
                close(socket_info->socket_id);
                return;
            }
            log_i(ETH_TAG, "\t[%d]: Opened, UDP, port [%d]", socket_info->socket_id, socket_info->sourceport);
            break;
        default:
            break;
    }
}

void control_task(void *params)
{
    device_data_t *device_data_ptr = (device_data_t *)params;
    log_i(ETH_TAG, "Device control task started.\r\n");
    uint32_t value = 0;

    while(1)
    {
        message_t message;
        if (xQueueReceive(device_data_ptr->receive_queue, (void*)&message, 2000) == pdTRUE)
        {
            log_i(ETH_TAG, "Processing message received from tcp client: %d, message_type: %d\r\n", message.client, message.message_type);
            if (message.message_type == MSG_SET)
            {
                value = message.value;
                if (value == 1)
                {
                    __NOP();
                }
            }
            else if (message.message_type == MSG_BLINK)
            {
                value = message.value;
                xQueueSend(device_data_ptr->blink_queue, (void*)&value, 10);
            }

        message_t reply_message;
        reply_message.client = message.client;
        reply_message.message_type = message.message_type;
        reply_message.value = value;
        xQueueSend(device_data_ptr->send_queue, (void*)&reply_message, 90);
        }
        // message_t reply_message;
        // reply_message.client = message.client;
        // reply_message.message_type = message.message_type;
        // reply_message.value = value;
        // xQueueSend(device_data_ptr->send_queue, (void*)&reply_message, 90);
    }
}

bool handle_receive_bufffer(socket_data_t* socket_info, message_t* message)
{
    char *end = strnstr((char*)socket_info->receive_buffer, "#", socket_info->receive_size);
    if (end)
    {
        *end = 0;
        if (!strcmp((char*)socket_info->receive_buffer, "GET"))
        {
            message->message_type = MSG_GET_STATUS;
            message->client = socket_info->socket_id;
            message->value = 0;

            socket_info->receive_size = 0;
            return true;
        }
        if (!strcmp((char*)socket_info->receive_buffer, "SET"))
        {
            message->message_type = MSG_SET;
            message->client = socket_info->socket_id;
            message->value = atoi((char*)&(socket_info->receive_buffer[3]));

            socket_info->receive_size = 0;
            return true;
        }
        if (!strcmp((char*)socket_info->receive_buffer, "HB"))
        {
            message->message_type = MSG_KEEPALIVE;
            message->client = socket_info->socket_id;
            message->value = 0;

            socket_info->receive_size = 0;
            return true;
        }
        if (!strncmp((char*)socket_info->receive_buffer, "BLINK", 5))
        {
            message->message_type = MSG_BLINK;
            message->client = socket_info->socket_id;
            message->value = atoi((char*)&(socket_info->receive_buffer[5]));

            socket_info->receive_size = 0;
            return true;
        }

        //We received a end character, but did not recognise the type; reset the buffer
        socket_info->receive_size = 0;
    }

    return false;
}

void blink_task (void *params)
{
    uint32_t blink_delay_ms = 1000;
    device_data_t *device_data_ptr = (device_data_t*)params;
    portTickType last_wake_time = xTaskGetTickCount();
    while (1)
    {
        vTaskDelayUntil(&last_wake_time, blink_delay_ms);
        HAL_GPIO_TogglePin(LED_PORT, LED_ORANGE);
        xQueueReceive(device_data_ptr->blink_queue, (void*)&blink_delay_ms, 0);
    }
}

void ping_task (void *params)
{   device_data_t *device_data_ptr = (device_data_t*) params;
    uint8_t ip_addr[4];
    while (1)
    {
        //xSemaphoreTake(device_data_ptr->ping_sem, portMAX_DELAY);
            while((xEventGroupWaitBits(device_data_ptr->events, EVENT_DHCP_IP_RECEIVED, pdFALSE, pdFALSE, pdMS_TO_TICKS(1000)) & EVENT_DHCP_IP_RECEIVED) == EVENT_DHCP_IP_RECEIVED)
            {
                ping_loop(ip_addr);
                vTaskDelay(pdMS_TO_TICKS(100));
            }
    }
}

// mcu software reset
// void reboot(void)
// {
//     pfunction jump_to_application;
//     uint32_t jump_address;
//     log_i (ETH_TAG, " system rebooting...");
//     jump_address = *((volatile uint32_t*)0x00000004U);
//     jump_to_application = (pfunction) jump_address;
//     jump_to_application();
// }

void ping_loop (uint8_t *ip_adr)
{
    int32_t ret = 0;
    uint16_t size = 0;
    switch (getSn_SR(ICMP_PING_SOCKET))
    {
        case SOCK_CLOSED:
            log_i(ETH_TAG, "[%d]: SOCK_CLOSED\r\n", ICMP_PING_SOCKET);
            close(ICMP_PING_SOCKET);
            //socket_info->socket_open = false;
            IINCHIP_WRITE(Sn_PROTO(ICMP_PING_SOCKET), IPPROTO_ICMP); // set ICMP Protocol
            if((ret = socket(ICMP_PING_SOCKET, Sn_MR_IPRAW, 3000, 0)) != ICMP_PING_SOCKET)  // open the SOCKET with IPRAW mode, if fail then Error
            {
                log_i(ETH_TAG, "  socket %d fail %d", ICMP_PING_SOCKET, ret);
                return;
            }
            while (getSn_SR(ICMP_PING_SOCKET) != SOCK_IPRAW)
                ;
            log_i(ETH_TAG, " [%d]: SOCK_IPRAW", ICMP_PING_SOCKET);
            // socket_info->send_size = 0;
            // socket_info->receive_size = 0;
            vTaskDelay(pdMS_TO_TICKS(1000));
            break;
        case SOCK_IPRAW:
            if ((size = getSn_RX_RSR(ICMP_PING_SOCKET)) > 0)
            {
                //memset(socket_info->receive_buffer + socket_info->receive_size, 0, size);
                ping_reply(ICMP_PING_SOCKET, ip_adr, size);
                if (ping_reply_received)
                {
                    ret = sendto(ICMP_PING_SOCKET, (uint8_t*)&PingReply, (size - 6), ip_adr, 3000);
                    ping_reply_received = 0;
                    if (ret < 0)
                    {
                        close(ICMP_PING_SOCKET);
                    }
                    vTaskDelay(500);
                }
            }
            break;
        
        default:
            break;
    }
}

#ifndef SERVER_DEVICE_H
#define SERVER_DEVICE_H

#include <stdint.h>
#include <stdbool.h>
#include "FreeRTOS.h"
#include "semphr.h"
#include "event_groups.h"
//#include "queue.h"

#define LISTENING_PORT  1234
#define LISTENING_SOCKETS 1
#define TIMEOUT_SECONDS 30
#define ICMP_PING_SOCKET 1

#define EVENT_SERVER_RUN    (1U << 0)
#define EVENT_DHCP_IP_RECEIVED  (1U << 1)

#define KEEP_ALIVE_SECONDS  10U 

#define BUFFER_SIZE 128

typedef void (*pfunction)(void);

typedef enum type_e
{
    NO_MESSAGE,
    MSG_GET_STATUS,
    MSG_SET,
    MSG_BLINK,
    MSG_REPLY,
    MSG_KEEPALIVE,
} type_t;

typedef struct device_data_s
{
    SemaphoreHandle_t ip_assigned_sem;
    //SemaphoreHandle_t ping_sem;
    QueueHandle_t send_queue;
    QueueHandle_t receive_queue;
    QueueHandle_t blink_queue;
    EventGroupHandle_t events;
    bool server_run;
} device_data_t;

typedef struct socket_data_s
{
    uint8_t socket_id;
    uint16_t listening_port;
    bool socket_open;
    uint8_t receive_buffer[BUFFER_SIZE];
    uint16_t receive_size;
    uint8_t send_buffer[BUFFER_SIZE];
    uint16_t send_size;
    uint32_t last_command_received;
    uint32_t last_command_send;
} socket_data_t;

typedef struct message_s
{
    int32_t client;
    int value;
    type_t message_type;
} message_t;

typedef struct ftp_data_s
{
    uint8_t login[10];
    uint8_t password[10];
} ftp_data_t;

void server_loop(socket_data_t *socket_info);
bool handle_receive_bufffer(socket_data_t* socket_info, message_t* message);
void control_task (void *params);
void blink_task (void *params);
void ping_task (void *params);
void ping_loop (uint8_t *ip_adr);


#endif
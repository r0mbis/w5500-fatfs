#pragma once

#include "stddef.h"
#include "pb_encode.h"
#include "pb_decode.h"
#include "messages.pb.h"

#define MESSAGE_MAX_LENGTH 128
#define MAGIC_NUMBER 0x01EFCDAB
#define MAGIC_NUMBER_SIZE 4
#define LENGTH_SIZE 4
#define PACKET_RX_MAX_LENGTH 128

typedef struct pb_api_t pb_api_t;
typedef enum packet_state_e packet_state_e;
typedef bool (*tx_callback)(char*, size_t);
typedef void (*rx_callback)(pb_api_t* pb, Msg* message);

enum packet_state_e {
    MAGIC_NUMBER_1_BYTE,
    MAGIC_NUMBER_2_BYTE,
    MAGIC_NUMBER_3_BYTE,
    MAGIC_NUMBER_4_BYTE,
    LENGTH,
    PAYLOAD
};

struct pb_api_t {
    size_t buffer_index;
    size_t packet_rx_index;
    size_t header_size;
    size_t message_length;
    size_t packet_rx_length;
    int has_magic_number;
    char packet_rx[PACKET_RX_MAX_LENGTH];
    char buffer[MESSAGE_MAX_LENGTH];
    char magic_and_length_buffer[8];
    tx_callback tx_callback;
    rx_callback rx_callback;
    packet_state_e packet_state;
};

pb_api_t* protobuf_api_init();


void protobuf_set_tx_callback(pb_api_t* pb, tx_callback cb);
void protobuf_set_rx_callback(pb_api_t* pb, rx_callback cb);

Msg protobuf_create_msg(uint8_t msg_tag);
void protobuf_tx(pb_api_t* pb, Msg* message);
bool protobuf_rx(pb_api_t* pb, char* data, size_t length);
void protobuf_rx_char(pb_api_t* pb, char c);
void protobuf_log( char* string);
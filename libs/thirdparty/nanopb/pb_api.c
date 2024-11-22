#include "pb_api.h"
#include "arm_math.h"
#include "sys_log.h"
#include "stdint.h"
#include "sys_descriptor.h"
#include "bsp_vcp.h"
#include <stdio.h>

#define TAG "proto"
static pb_api_t __pb_static = {0};
#define PB_HEADER_SIZE 8

Msg protobuf_create_msg(uint8_t msg_tag){
    Msg message = Msg_init_zero;
    message.which_msg = msg_tag;
    message.device_id = sys_descriptor_get()->moduleId;

    return message;
}


static void protobuf_update_header(char* buf, uint32_t len) {
    // Insert magic number
    buf[0] = 0x01; 
    buf[1] = 0xEF; 
    buf[2] = 0xCD; 
    buf[3] = 0xAB; 
    // Insert length
    buf[4] = (len >> 0) & 0xFF;
    buf[5] = (len >> 8) & 0xFF;
    buf[6] = (len >> 16) & 0xFF;
    buf[7] = (len >> 24) & 0xFF;

}


bool protobuf_encode_string(pb_ostream_t* stream, const pb_field_t* field, void* const* arg)
{
    const char* str = (const char*)(*arg);

    if (!pb_encode_tag_for_field(stream, field)){
        return false;
    }

    return pb_encode_string(stream, (uint8_t*)str, strlen(str));
}



void protobuf_set_tx_callback(pb_api_t* pb, tx_callback cb){
    pb->tx_callback = cb;
}

void protobuf_set_rx_callback(pb_api_t* pb, rx_callback cb){
    pb->rx_callback = cb;
}

pb_api_t* protobuf_api_init(){
    return &__pb_static;
}


void protobuf_tx(pb_api_t* pb, Msg* message) {

    size_t header_size = PB_HEADER_SIZE;
    size_t total_buffer_size = sizeof(pb->buffer);

    // Stream starts after the header space in the buffer
    pb_ostream_t stream = pb_ostream_from_buffer((void*) (pb->buffer + header_size), total_buffer_size - header_size);

    // Encode the message into the buffer (after the header space)
    if (!pb_encode(&stream, Msg_fields, message)) {
        log_e(TAG, "Encoding failed: %s\n", PB_GET_ERROR(&stream));
        return;
    }

    // Update the header with the correct message length and magic bytes
    protobuf_update_header(pb->buffer, stream.bytes_written);

    // Now send both the header and the message in one go
    size_t total_message_size = header_size + stream.bytes_written;
    if (!pb->tx_callback(pb->buffer, total_message_size)) {
        log_e(TAG, "Failed to send message via UART");
    }
}



static void process_protobuf_message(pb_api_t* pb, Msg* message) {
    // log_i(TAG, "Processing message of type: %d", message->which_msg);
    if(message->device_id != sys_descriptor_get()->moduleId){
        log_e(TAG, "Device ID mismatch.");
        return;
    }

    pb->rx_callback(pb, message);

}



// Decode and process the Protobuf message
bool protobuf_rx(pb_api_t* pb, char* data, size_t length) {
    pb_istream_t stream = pb_istream_from_buffer((uint8_t*)data, length);

    // Initialize an empty message
    Msg message = Msg_init_zero;

    // Decode the message
    if (!pb_decode(&stream, Msg_fields, &message)) {
        log_e(TAG, "Decoding failed: %s\n", PB_GET_ERROR(&stream));
        return false;
    }

    // Process the decoded message
    process_protobuf_message(pb,&message);
    return true;
}

// Function to print a byte array in hex format
void log_hex(const char* label, const uint8_t* data, size_t length) {
    char log_message[256]; // Adjust size as needed
    size_t pos = 0;
    
    pos += snprintf(log_message + pos, sizeof(log_message) - pos, "%s: ", label);
    for (size_t i = 0; i < length; ++i) {
        pos += snprintf(log_message + pos, sizeof(log_message) - pos, "%02X ", data[i]);
    }
    log_i(TAG, "%s", log_message);
}

void protobuf_rx_char(pb_api_t* pb, char c) {
    switch (pb->packet_state)
    {
    case MAGIC_NUMBER_1_BYTE:
        if (c == 0xAB)
        {
            pb->packet_rx[MAGIC_NUMBER_1_BYTE] = c;
            pb->packet_state = MAGIC_NUMBER_2_BYTE;
        }
        break;
    case MAGIC_NUMBER_2_BYTE:
        if (c == 0xCD)
        {   pb->packet_rx[MAGIC_NUMBER_2_BYTE] = c;
            pb->packet_state = MAGIC_NUMBER_3_BYTE;
        }
        else    pb->packet_state = MAGIC_NUMBER_1_BYTE;
        break;
    case MAGIC_NUMBER_3_BYTE:
        if (c == 0xEF)
        {
            pb->packet_rx[MAGIC_NUMBER_3_BYTE] = c;
            pb->packet_state = MAGIC_NUMBER_4_BYTE;
        }
        else    pb->packet_state = MAGIC_NUMBER_1_BYTE;
        break;
    case MAGIC_NUMBER_4_BYTE:
        if (c == 0x01)
        {
            pb->packet_rx[MAGIC_NUMBER_4_BYTE] = c;
            pb->packet_state = LENGTH;
            pb->packet_rx_index = MAGIC_NUMBER_SIZE;
        }
        else pb->packet_state = MAGIC_NUMBER_1_BYTE;
        break;
    case LENGTH:
        pb->packet_rx[pb->packet_rx_index++] = c;
        if (pb->packet_rx_index == 8)
        {
            pb->packet_rx_length = *(uint32_t*)(pb->packet_rx + MAGIC_NUMBER_SIZE);
            pb->packet_state = PAYLOAD;
        }
        break;
    case PAYLOAD:
        pb->packet_rx[pb->packet_rx_index++] = c;
        // Check if the full message has been received
        if (pb->packet_rx_index == (pb->packet_rx_length + 8))
        {
            // Extract the full message
            char* message_data = pb->packet_rx + MAGIC_NUMBER_SIZE + LENGTH_SIZE;
            // Process the message
            protobuf_rx(pb, message_data, pb->packet_rx_length);
            pb->packet_state = MAGIC_NUMBER_1_BYTE;
        }
        break;
    
    default:
        break;
    }
    // Append the received character to the buffer
    // if (pb->buffer_index < sizeof(pb->buffer)) {
    //     pb->buffer[pb->buffer_index++] = (uint8_t)c;
    // } else {
    //     // Buffer overflow
    //     pb->buffer_index = 0;
    //     pb->has_magic_number = 0;
    //     pb->message_length = 0;
    //     return;
    // }

    // if (pb->buffer_index >= (MAGIC_NUMBER_SIZE + LENGTH_SIZE)) {
    //     if (!pb->has_magic_number) {
    //         // Check for magic number
    //         uint32_t received_magic = *((uint32_t*)pb->buffer);
    //         if (received_magic != MAGIC_NUMBER) {
    //             // Magic number mismatch
    //             pb->buffer_index = 0;
    //             return;
    //         }
    //         // Magic number matches, set flag and read length
    //         pb->has_magic_number = 1;
    //         pb->message_length = *((uint32_t*)(pb->buffer + MAGIC_NUMBER_SIZE));
    //         // Adjust buffer index to account for the length field
    //         pb->buffer_index = MAGIC_NUMBER_SIZE + LENGTH_SIZE;
    //     }

    //     // Check if the full message has been received
    //     if (pb->buffer_index >= (MAGIC_NUMBER_SIZE + LENGTH_SIZE + pb->message_length)) {
    //         // Extract the full message
    //         char* message_data = pb->buffer + MAGIC_NUMBER_SIZE + LENGTH_SIZE;
    //         size_t message_data_length = pb->message_length;

    //         // Process the message
    //         protobuf_rx(pb, (char*) message_data, message_data_length);

    //         // Reset buffer for the next message
    //         pb->buffer_index = 0;
    //         pb->has_magic_number = 0;
    //         pb->message_length = 0;
    //     }
    // }
}


void protobuf_log( char* string){
    Msg message = protobuf_create_msg(Msg_log_tag);
    message.msg.log.text.arg = string; 
    message.msg.log.text.funcs.encode = &protobuf_encode_string;
    protobuf_tx(&__pb_static, &message);
}




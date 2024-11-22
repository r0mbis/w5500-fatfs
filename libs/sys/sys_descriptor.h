#pragma once

#include <stdint.h>
#include "fw_bundler.h"

#ifdef __cplusplus
extern "C" {
#endif

#define APP_DESCRIPTOR_MAGIC_WORD 0xABCD7654

#ifndef PROJECT_NAME
#define PROJECT_NAME PROJECT_NAME_STR
#endif

#define BOARD_ID "SOCRAT_TEST_BOARD"


const sys_descriptor_t* sys_descriptor_get();

void decriptor_log(const sys_descriptor_t* app_descr);


#ifdef __cplusplus
}
#endif
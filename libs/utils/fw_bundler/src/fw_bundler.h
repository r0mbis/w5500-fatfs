#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define DESCRIPTOR_MAGIC_WORD 0xABCD7654

/**
 * Application descriptor structure.
 *
 * Note: The memory layout of this structure is not allowed to change,
 * because it is also handled by the bootloader and the build scripts.
 */

typedef struct {
    const uint32_t magic_word; /*!< Magic word DESCRIPTOR_MAGIC_WORD */
    const uint32_t reserved1[3]; /*!< Reserved */

    const char project_name[32]; /*!< Project name */
    const char version[32]; /*!< Application version */
    const char build_date[20]; /*!< Build timestamp */
    const char build_describe[54]; /*!< Build Git description */
    const uint32_t vector_addr;
    //
    const int16_t id;
    const uint32_t moduleId;
    const uint8_t hwVersionMajor;
    const uint8_t hwVersionMinor;
    const uint16_t hwVersionPatch;
    const uint8_t fwVersionMajor;
    const uint8_t fwVersionMinor;
    const uint8_t fwVersionPatch;
    const uint8_t fwVersionBuild;
    const uint32_t serialNumber;
    const uint32_t reserved2;

    uint8_t ecdsa_sig[64];
    uint32_t size;
    uint32_t crc32; /*!< CRC-32 checksum using the STM32F4 hardware algorithm */
} sys_descriptor_t;


_Static_assert(sizeof(sys_descriptor_t) == 256, "sys_descriptor_t should be 256 bytes");



#ifdef __cplusplus
}
#endif
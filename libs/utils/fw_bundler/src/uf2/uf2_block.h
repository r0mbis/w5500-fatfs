#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// All entries are little endian.
#define UF2_MAGIC_START0 0x0A324655UL // "UF2\n"
#define UF2_MAGIC_START1 0x9E5D5157UL // Randomly selected
#define UF2_MAGIC_END 0x0AB16F30UL // Ditto

// If set, the block is "comment" and should not be flashed to the device
#define UF2_FLAG_NOFLASH 0x00000001
#define UF2_FLAG_FILE_CONTAINER 0x00001000
#define UF2_FLAG_FAMILY_ID_PRESENT 0x00002000
#define UF2_FLAG_MD5_CHECKSUM_PRESENT 0x00004000
#define Uf2_FLAG_PAYLOAD_ENCRYPTED 0x00008000
#define Uf2_FLAG_CRC 0x10000

#define UF2_IS_MY_FAMILY(bl) \
    (((bl)->flags & UF2_FLAG_FAMILYID_PRESENT) == 0 || (bl)->familyID == UF2_FAMILY)


typedef struct {
    // 32 byte header
    uint32_t magicStart0;
    uint32_t magicStart1;
    uint32_t flags;
    uint32_t targetAddr;
    uint32_t payloadSize;
    uint32_t blockNo;
    uint32_t numBlocks;

    union {
        uint32_t fileSize;
        uint32_t familyID;
        uint32_t reserved;
    };
    union {
        uint32_t crc[119];
        uint8_t data[476];
    };

    // store magic also at the end to limit damage from partial block reads
    uint32_t magicEnd;
} UF2_Block;


#ifdef __cplusplus
}
#endif
# Firmware bundler utility
converts binary firmware file into uf2 format with optional encryption and signing

## UF2: file format

A UF2 file consists of 512 byte blocks. Each block starts with a 32 byte
header, followed by data, and a final magic number.
All fields, except for data, are 32 bit unsigned little endian integers.

| Offset | Size | Value                                             |
|--------|------|---------------------------------------------------|
| 0      | 4    | First magic number, `0x0A324655` (`"UF2\n"`)      |
| 4      | 4    | Second magic number, `0x9E5D5157`                 |
| 8      | 4    | Flags                                             |
| 12     | 4    | Address in flash where the data should be written |
| 16     | 4    | Number of bytes used in data (often 256)          |
| 20     | 4    | Sequential block number; starts at 0              |
| 24     | 4    | Total number of blocks in file                    |
| 28     | 4    | File size or board family ID or zero              |
| 32     | 476  | Data, padded with zeros                           |
| 508    | 4    | Final magic number, `0x0AB16F30`                  |

## Family ID

Family ID is generated seperately for both bootloader and firmware binaries, with following commands respectevly:

targets.mk example: 
```
FAMILY_ID		= 0x$(shell echo "$(TARGET)boot" | sha1sum | cut -c 1-8 )
FAMILY_ID_APP	= 0x$(shell echo "$(TARGET)fw" | sha1sum | cut -c 1-8)
```

Bootloader verifies correct FamilyID when recieves uf2 block, in case of wrong Family ID, block is skipped.
This allows to concatenate binaries for different targets and use single update file for all boards.

**If bundling bootloader, use additional `-b` argument, for correct descriptor placement at the end.**

## CRC Checksum

With `-c` argument, CRC32 checksum is calculated for each block, using stm32 algorhitm. Used to compare it with flash crc for verification of correct flashing.


## Encryption

Tinyaes library is used for encryption/decryption (by both bundler and firmware).
With `-e` argument, data in each uf2 block is encrypted using AES256.

**Test key and initialization vector are located in `src/tinyaes/keys.h` header**

## Signing firmware (WIP)
- Generating private key:
  `openssl ecparam -name secp256k1 -genkey -noout -out private.pem`
- Generating public key:
  `openssl ec -in private.pem -pubout -out public.pem`
- To generate a signature:
  `openssl dgst -sha256 -sign private.pem -out build/fw.bin.sig build/fw.bin`
- Verification:
  `openssl dgst -sha256 -verify public.pem -signature build/fw.bin.sig build/fw.bin`

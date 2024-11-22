#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <argp.h>

#include "fw_bundler.h"
#include "uf2_block.h"
#include "tinyaes/aes.h"
#include "tinyaes/keys.h"
#include "ecdsautil/ecdsa.h"
#include "ecdsautil/sha256.h"

#define CBC 1

static unsigned long sw_crc32_by_byte_table[256] = {
    0x00000000, 0x04C11DB7, 0x09823B6E, 0x0D4326D9, // 0..3
    0x130476DC, 0x17C56B6B, 0x1A864DB2, 0x1E475005, // 4..7
    0x2608EDB8, 0x22C9F00F, 0x2F8AD6D6, 0x2B4BCB61, // 8..B
    0x350C9B64, 0x31CD86D3, 0x3C8EA00A, 0x384FBDBD, // C..F

    0x4C11DB70, 0x48D0C6C7, 0x4593E01E, 0x4152FDA9, 0x5F15ADAC, 0x5BD4B01B, 0x569796C2,
    0x52568B75, 0x6A1936C8, 0x6ED82B7F, 0x639B0DA6, 0x675A1011, 0x791D4014, 0x7DDC5DA3,
    0x709F7B7A, 0x745E66CD, 0x9823B6E0, 0x9CE2AB57, 0x91A18D8E, 0x95609039, 0x8B27C03C,
    0x8FE6DD8B, 0x82A5FB52, 0x8664E6E5, 0xBE2B5B58, 0xBAEA46EF, 0xB7A96036, 0xB3687D81,
    0xAD2F2D84, 0xA9EE3033, 0xA4AD16EA, 0xA06C0B5D, 0xD4326D90, 0xD0F37027, 0xDDB056FE,
    0xD9714B49, 0xC7361B4C, 0xC3F706FB, 0xCEB42022, 0xCA753D95, 0xF23A8028, 0xF6FB9D9F,
    0xFBB8BB46, 0xFF79A6F1, 0xE13EF6F4, 0xE5FFEB43, 0xE8BCCD9A, 0xEC7DD02D, 0x34867077,
    0x30476DC0, 0x3D044B19, 0x39C556AE, 0x278206AB, 0x23431B1C, 0x2E003DC5, 0x2AC12072,
    0x128E9DCF, 0x164F8078, 0x1B0CA6A1, 0x1FCDBB16, 0x018AEB13, 0x054BF6A4, 0x0808D07D,
    0x0CC9CDCA, 0x7897AB07, 0x7C56B6B0, 0x71159069, 0x75D48DDE, 0x6B93DDDB, 0x6F52C06C,
    0x6211E6B5, 0x66D0FB02, 0x5E9F46BF, 0x5A5E5B08, 0x571D7DD1, 0x53DC6066, 0x4D9B3063,
    0x495A2DD4, 0x44190B0D, 0x40D816BA, 0xACA5C697, 0xA864DB20, 0xA527FDF9, 0xA1E6E04E,
    0xBFA1B04B, 0xBB60ADFC, 0xB6238B25, 0xB2E29692, 0x8AAD2B2F, 0x8E6C3698, 0x832F1041,
    0x87EE0DF6, 0x99A95DF3, 0x9D684044, 0x902B669D, 0x94EA7B2A, 0xE0B41DE7, 0xE4750050,
    0xE9362689, 0xEDF73B3E, 0xF3B06B3B, 0xF771768C, 0xFA325055, 0xFEF34DE2, 0xC6BCF05F,
    0xC27DEDE8, 0xCF3ECB31, 0xCBFFD686, 0xD5B88683, 0xD1799B34, 0xDC3ABDED, 0xD8FBA05A,
    0x690CE0EE, 0x6DCDFD59, 0x608EDB80, 0x644FC637, 0x7A089632, 0x7EC98B85, 0x738AAD5C,
    0x774BB0EB, 0x4F040D56, 0x4BC510E1, 0x46863638, 0x42472B8F, 0x5C007B8A, 0x58C1663D,
    0x558240E4, 0x51435D53, 0x251D3B9E, 0x21DC2629, 0x2C9F00F0, 0x285E1D47, 0x36194D42,
    0x32D850F5, 0x3F9B762C, 0x3B5A6B9B, 0x0315D626, 0x07D4CB91, 0x0A97ED48, 0x0E56F0FF,
    0x1011A0FA, 0x14D0BD4D, 0x19939B94, 0x1D528623, 0xF12F560E, 0xF5EE4BB9, 0xF8AD6D60,
    0xFC6C70D7, 0xE22B20D2, 0xE6EA3D65, 0xEBA91BBC, 0xEF68060B, 0xD727BBB6, 0xD3E6A601,
    0xDEA580D8, 0xDA649D6F, 0xC423CD6A, 0xC0E2D0DD, 0xCDA1F604, 0xC960EBB3, 0xBD3E8D7E,
    0xB9FF90C9, 0xB4BCB610, 0xB07DABA7, 0xAE3AFBA2, 0xAAFBE615, 0xA7B8C0CC, 0xA379DD7B,
    0x9B3660C6, 0x9FF77D71, 0x92B45BA8, 0x9675461F, 0x8832161A, 0x8CF30BAD, 0x81B02D74,
    0x857130C3, 0x5D8A9099, 0x594B8D2E, 0x5408ABF7, 0x50C9B640, 0x4E8EE645, 0x4A4FFBF2,
    0x470CDD2B, 0x43CDC09C, 0x7B827D21, 0x7F436096, 0x7200464F, 0x76C15BF8, 0x68860BFD,
    0x6C47164A, 0x61043093, 0x65C52D24, 0x119B4BE9, 0x155A565E, 0x18197087, 0x1CD86D30,
    0x029F3D35, 0x065E2082, 0x0B1D065B, 0x0FDC1BEC, 0x3793A651, 0x3352BBE6, 0x3E119D3F,
    0x3AD08088, 0x2497D08D, 0x2056CD3A, 0x2D15EBE3, 0x29D4F654, 0xC5A92679, 0xC1683BCE,
    0xCC2B1D17, 0xC8EA00A0, 0xD6AD50A5, 0xD26C4D12, 0xDF2F6BCB, 0xDBEE767C, 0xE3A1CBC1,
    0xE760D676, 0xEA23F0AF, 0xEEE2ED18, 0xF0A5BD1D, 0xF464A0AA, 0xF9278673, 0xFDE69BC4,
    0x89B8FD09, 0x8D79E0BE, 0x803AC667, 0x84FBDBD0, 0x9ABC8BD5, 0x9E7D9662, 0x933EB0BB,
    0x97FFAD0C, 0xAFB010B1, 0xAB710D06, 0xA6322BDF, 0xA2F33668, 0xBCB4666D, 0xB8757BDA,
    0xB5365D03, 0xB1F740B4};

static uint32_t crc32 = 0xFFFFFFFF;
char filename[128];

static uint32_t get_fsize(char* filename) {
    uint32_t size = 0;

    FILE* fp;
    if((fp = fopen(filename, "rb")) == NULL) {
        printf("\tUF2\tError occured while opening '%s'\n", filename);
        return 0;
    }

    fseek(fp, 0, SEEK_END);
    size = ftello(fp);

    fclose(fp);

    printf("\tUF2\tFile '%s' size is %d bytes\n", filename, size);

    return size;
}

static uint32_t calc_crc(uint32_t crc32, const uint8_t* pbuffer, uint32_t num_of_bytes) {
    const uint32_t num_of_tail_byte = num_of_bytes & 3;
    uint32_t num_of_dword = num_of_bytes >> 2;
    uint32_t last_data;

    while(num_of_dword--) {
        crc32 = crc32 ^ *((unsigned long*)pbuffer);
        pbuffer += 4;

        for(uint32_t i = 0; i < 4; ++i) {
            crc32 = (crc32 << 8) ^ sw_crc32_by_byte_table[crc32 >> 24];
        }
    }

    switch(num_of_tail_byte) {
    case 0:
        return crc32;
    case 1:
        last_data = pbuffer[0] << 24;
        break;
    case 2:
        last_data = *((uint16_t*)(&pbuffer[0]));
        last_data <<= 16;
        break;
    case 3:
        last_data = *((uint16_t*)(&pbuffer[0]));
        last_data <<= 8;
        last_data += pbuffer[2] << 24;
        break;
    default:
        break;
    }

    crc32 = calc_crc(crc32, (uint8_t*)&last_data, 4);

    return crc32;
}

static uint32_t get_file_crc32(char* filename, bool bootloader) {
    uint32_t fw_size;
    uint32_t fw_size_in_dword;
    uint8_t fw_size_of_tail;
    uint8_t fw_buf[4];

    FILE* fp;
    if((fp = fopen(filename, "rb")) == NULL) {
        printf("\tUF2\tError occured while opening '%s'\n", filename);
        return 0;
    }

    if(bootloader) {
        fw_size = get_fsize(filename) - 4;
    } else {
        fseek(fp, 512, SEEK_SET);
        fw_size = get_fsize(filename) - 512;
    }

    fw_size_in_dword = fw_size >> 2;
    fw_size_of_tail = fw_size & 3;

    puts("\tUF2\tCalculating CRC32 checksum of the binary file ...");

    for(uint32_t i = 0; i < fw_size_in_dword; ++i) {
        size_t res = fread(fw_buf, sizeof(uint8_t), sizeof(fw_buf), fp);
        if(res != sizeof(fw_buf)) {
            puts("\tUF2\tError occured while reading binary file!");
            exit(EXIT_FAILURE);
        }

        crc32 = calc_crc(crc32, fw_buf, sizeof(fw_buf));
    }

    if(fw_size_of_tail != 0) {
        size_t res = fread(fw_buf, sizeof(uint8_t), fw_size_of_tail, fp);
        if(res != fw_size_of_tail) {
            puts("\tUF2\tError occured while reading binary file!");
            exit(EXIT_FAILURE);
        }

        crc32 = calc_crc(crc32, fw_buf, fw_size_of_tail);
    }

    printf("CRC32: %08X\n", crc32);

    fclose(fp);

    return crc32;
}

//

const char* argp_program_version = "fw bundler 1.2";

const char* argp_program_bug_address = "<baranov@socrat.pro>";

/* This structure is used by main to communicate with parse_opt. */
struct arguments {
    char* args[128]; /* ARG1 and ARG2 */
    uint32_t verbose, encrypt, calc_crc, bootloader; /* The -v -e -c flags */
    char *outfile, *infile, *signfile; /* Argument for -o and -i*/
    uint32_t family_id, bin_start; /* Arguments for -a and -b */
};

/*
   OPTIONS.  Field 1 in ARGP.
   Order of fields: {NAME, KEY, ARG, FLAGS, DOC}.
*/
static struct argp_option options[] = {
    {"verbose", 'v', 0, 0, "Produce verbose output"},
    {"encrypt", 'e', 0, 0, "Encrypt binary"},
    {"crc32", 'c', 0, 0, "Append crc32 checksum"},
    {"bootloader", 'b', 0, 0, "Prepare bootloader image (differs in descriptor pos)"},
    {"family", 'f', "0x78301e2d", 0, "Family ID"},
    {"address", 'a', "0x08000000", 0, "Binary address"},
    {"output", 'o', "OUTFILE", 0, "Name of output file"},
    {"input", 'i', "INFILE", 0, "Input file to proceed"},
    {"signature", 's', "INFILE", 0, "Private key for signing firmware"},
    {0}};

/*
   PARSER. Field 2 in ARGP.
   Order of parameters: KEY, ARG, STATE.
*/

static error_t parse_opt(int key, char* arg, struct argp_state* state) {
    struct arguments* arguments = state->input;

    switch(key) {
    case 'v':
        arguments->verbose = 1;
        break;
    case 'e':
        arguments->encrypt = 1;
        break;
    case 'b':
        arguments->bootloader = 1;
    case 'c':
        arguments->calc_crc = 1;
        break;
    case 'f':
        sscanf(arg, "%X", &arguments->family_id);
        break;
    case 'a':
        sscanf(arg, "%X", &arguments->bin_start);
        break;
    case 'o':
        arguments->outfile = arg;
        break;
    case 'i':
        arguments->infile = arg;
        break;
    case 's':
        arguments->signfile = arg;
        break;
    default:
        return ARGP_ERR_UNKNOWN;
    }
    return 0;
}

/*
   ARGS_DOC. Field 3 in ARGP.
   A description of the non-option command-line arguments
     that we accept.
*/
static char args_doc[] = " -f id -a addr -i input -o output -s private.pem";

/*
  DOC.  Field 4 in ARGP.
  Program documentation.
*/
static char doc[] = "fw_bundler -- Generate .uf2 firmware bundle with optional encryption, crc check and signing";

/*
   The ARGP structure itself.
*/
static struct argp argp = {options, parse_opt, args_doc, doc};

/*
   The main function.
   Notice how now the only function call needed to process
   all command-line options and arguments nicely
   is argp_parse.
*/
struct arguments arguments;
void log_out(char* str) {
    if(arguments.verbose) fprintf(stdout, "%s", str);
}



static void prv_sha256(FILE *fp, uint8_t *hash_out)
{
  #define READ_BUF_SZ 128
  static uint8_t read_buf[READ_BUF_SZ] = {0};

  ecdsa_sha256_context_t ctx;
  ecdsa_sha256_init(&ctx);

  fseek(fp, sizeof(sys_descriptor_t), SEEK_SET);
  size_t readsize = 0;
  while ((readsize = fread(read_buf, 1, READ_BUF_SZ, fp)) > 0) {
    ecdsa_sha256_update(&ctx, read_buf, readsize);
  }

  ecdsa_sha256_final(&ctx, hash_out);
}

void hexdump(uint8_t *buffer, size_t len) {
  while (len--){
    printf("%02hhx", *(buffer++));
  }
}


int main(int argc, char** argv) {

    /* Set argument defaults */
    arguments.outfile = NULL;
    arguments.infile = NULL;
    arguments.family_id = 0;
    arguments.bin_start = 0;
    arguments.verbose = 0;
    arguments.encrypt = 0;
    arguments.calc_crc = 0;
    arguments.bootloader = 0;

    /* Where the magic happens */
    argp_parse(&argp, argc, argv, 0, 0, &arguments);

    /* Print argument values */
    if(arguments.verbose) {

        if(arguments.bootloader) {
            printf("\tUF2\tBootloader processing..");
        }

        printf("\tUF2\tFAMILY = %X @addr %X  Enc:%d Crc:%d\n",
               arguments.family_id,
               arguments.bin_start,
               arguments.encrypt,
               arguments.calc_crc);
    }

    struct AES_ctx ctx;
    FILE* f = fopen(arguments.infile, "r+");
    FILE* fout = fopen(arguments.outfile, "wb");

    AES_init_ctx_iv(&ctx, local_key, local_iv);

    if(!f) {
        fprintf(stderr, "\tUF2\tNo such file: %s\n", arguments.infile);
        return 1;
    }

    uint32_t firmware_crc = get_file_crc32(arguments.infile, arguments.bootloader);

    fseek(f, 0UL, SEEK_END);

    uint32_t sz = ftell(f);

    ecdsa_signature_t sig = {0};
    sys_descriptor_t descriptor = {0};

    if(arguments.calc_crc) {
        if(arguments.bootloader) { // bootloader stores descriptor at the end of binary
            fseek(f, -4, SEEK_END);
            fwrite(&firmware_crc, 4, 1, f);
            // not writing size here as bootloader is fixed 128kb (1 sector)
            printf("\tUF2\tCRC Added %x to %s\n", firmware_crc, arguments.infile);

            // read descriptor anyway
            fseek(f, -(sizeof(sys_descriptor_t)), SEEK_END);
            fread(&descriptor, sizeof(sys_descriptor_t), 1, f);


        } else { // firmware stores descriptor at the very beginning

            fseek(f, 0, SEEK_SET);
            fread(&descriptor, sizeof(sys_descriptor_t), 1, f);

            printf("\tUF2\tDescriptor found: %s\n", descriptor.project_name);

            descriptor.size = get_fsize(arguments.infile) - 512;
            descriptor.crc32 = firmware_crc;

            fseek(f, 0, SEEK_SET);

            fwrite(&descriptor, sizeof(sys_descriptor_t), 1, f);
            printf("\tUF2\tDescriptor updated, size - %d \n", descriptor.size);
            printf("\tUF2\tCRC Added %x to %s\n", firmware_crc, arguments.infile);

        }
    }

#if 0 
    if(arguments.signfile) {
        ecc_int256_t secret, hash;
        ecdsa_sha256_context_t sha256_ctx; 
        ecdsa_verify_context_t ecdsa_verify_ctx;

        uint8_t signature[sizeof(ecdsa_signature_t)];
        uint8_t key_buff[sizeof(ecdsa_signature_t)];

        printf("\tUF2\tSigning fw with %s\n", arguments.signfile);

        uint8_t hash_out[64];
        prv_sha256( f, hash_out);

        ecdsa_verify_prepare_legacy(&ecdsa_verify_ctx, &hash_out, &sig);
        
        printf("\r\n\tUF2\tsha256: ");

        hexdump(hash_out, sizeof(ecdsa_signature_t));

        memcpy(&descriptor.ecdsa_sig, hash_out, sizeof(ecdsa_signature_t));
        printf("\r\n\tUF2\tsig: ");
        
        hexdump(&sig, sizeof(ecdsa_signature_t));

        // ecdsa_sign_legacy(signature, hash_out, key_buff);
        if(arguments.bootloader){
            fseek(fout, -(sizeof(sys_descriptor_t)), SEEK_END);
            fwrite(&descriptor, sizeof(sys_descriptor_t), 1, fout);
        } else {
            fseek(fout, 0, SEEK_SET);
            fwrite(&descriptor, sizeof(sys_descriptor_t), 1, fout);
        }
    }

#endif 
    fseek(f, 0UL, SEEK_SET);


    UF2_Block bl;

    if(arguments.calc_crc) {

        memset(&bl, 0, sizeof(bl));

        bl.magicStart0 = UF2_MAGIC_START0;
        bl.magicStart1 = UF2_MAGIC_START1;
        bl.magicEnd = UF2_MAGIC_END;
        bl.targetAddr = arguments.bin_start;
        bl.numBlocks = (sz + 255) / 256;
        bl.payloadSize = 4;
        bl.familyID = arguments.family_id;
        bl.flags = UF2_FLAG_FAMILY_ID_PRESENT | Uf2_FLAG_CRC;

        bl.crc[0] = firmware_crc;
        bl.crc[1] = sz;

        if(arguments.encrypt) {

            bl.crc[2] = 0xABCD7654;
            bl.crc[3] = 0xABCD7654;
            bl.crc[4] = 0xABCD7654;
            bl.crc[5] = 0xABCD7654;

            AES_CBC_encrypt_buffer(&ctx, (uint8_t*)&bl.crc[2], 4);
        }

        fwrite(&bl, 1, sizeof(bl), fout);
    }

    memset(&bl, 0, sizeof(UF2_Block));

    bl.magicStart0 = UF2_MAGIC_START0;
    bl.magicStart1 = UF2_MAGIC_START1;
    bl.magicEnd = UF2_MAGIC_END;
    bl.targetAddr = arguments.bin_start;
    bl.numBlocks = (sz + 255) / 256;
    bl.payloadSize = 256;
    bl.familyID = arguments.family_id;

    if(arguments.encrypt) {
        bl.flags = UF2_FLAG_FAMILY_ID_PRESENT | Uf2_FLAG_PAYLOAD_ENCRYPTED;
    } else {
        bl.flags = UF2_FLAG_FAMILY_ID_PRESENT;
    }

    int numbl = 0;
    while(fread(bl.data, 1, bl.payloadSize, f)) {
        bl.blockNo = numbl++;

        if(arguments.encrypt) {
            AES_init_ctx_iv(&ctx, local_key, local_iv);
            AES_CBC_encrypt_buffer(&ctx, bl.data, bl.payloadSize);
        }

        fwrite(&bl, 1, sizeof(bl), fout);
        bl.targetAddr += bl.payloadSize;
        // clear for next iteration, in case we get a short read
        memset(bl.data, 0, sizeof(bl.data));
    }

    fclose(fout);
    fclose(f);
    printf("\tUF2\tWrote %d blocks to %s\n", numbl, arguments.outfile);
    return 0;
}

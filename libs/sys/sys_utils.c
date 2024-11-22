#include "sys_utils.h"
#include "printf.h"

static float __lut_cos_buffer[TRIG_TABLE_SIZE];

static const uint8_t bit_rev_lut[16] = {
    0x0,
    0x8,
    0x4,
    0xc,
    0x2,
    0xa,
    0x6,
    0xe,
    0x1,
    0x9,
    0x5,
    0xd,
    0x3,
    0xb,
    0x7,
    0xf,
};

FORCE_INLINE uint8_t util_rev_byte(uint8_t n) {
    return (bit_rev_lut[n & 0b1111] << 4) | bit_rev_lut[n >> 4];
}

void util_prepare_lut() {
    for(size_t i = 0; i < TRIG_TABLE_SIZE; i++) {
        __lut_cos_buffer[i] = arm_cos_f32((((i + 0.5f) / TRIG_TABLE_SIZE * TWO_PI)));
    }
}

FORCE_INLINE float lutsin(float rad) {
    return __lut_cos_buffer[(int)(mabsf(HALF_PI - rad) * RADIANS_TO_TABLE_INDEX) &
                            TRIG_TABLE_MASK];
}
FORCE_INLINE float lutcos(float rad) {
    return __lut_cos_buffer[(int)(rad * RADIANS_TO_TABLE_INDEX) & TRIG_TABLE_MASK];
}

FORCE_INLINE float lerp(const float k0, const float k1, const float t) {
    return (k0 + t * (k1 - k0));
}

FORCE_INLINE float lerp_precise(const float k0, const float k1, const float t) {
    return (1 - t) * k0 + t * k1;
}

FORCE_INLINE float
map(const float x, const float imin, const float imax, const float omin, const float omax) {
    return (x - imin) * (omax - omin) / (imax - imin) + omin;
}

FORCE_INLINE void util_calc_radiant(float* xy, float value, uint8_t scale, uint8_t size) {
    xy[0] = lutsin(value);
    xy[1] = lutcos(value);
    arm_scale_f32(xy, scale, xy, 2);
}

/*******************************************************************************
 * sha1digest: https://github.com/CTrabant/teeny-sha1
 *
 * Calculate the SHA-1 value for supplied data buffer and generate a
 * text representation in hexadecimal.
 *
 * Based on https://github.com/jinqiangshou/EncryptionLibrary, credit
 * goes to @jinqiangshou, all new bugs are mine.
 *
 * @input:
 *    data      -- data to be hashed
 *    databytes -- bytes in data buffer to be hashed
 *
 * @output:
 *    digest    -- the result, MUST be at least 20 bytes
 *    hexdigest -- the result in hex, MUST be at least 41 bytes
 *
 * At least one of the output buffers must be supplied.  The other, if not 
 * desired, may be set to NULL.
 *
 * @return: 0 on success and non-zero on error.
 ******************************************************************************/
int sha1digest(uint8_t* digest, char* hexdigest, const uint8_t* data, size_t databytes) {
#define SHA1ROTATELEFT(value, bits) (((value) << (bits)) | ((value) >> (32 - (bits))))

    uint32_t W[80];
    uint32_t H[] = {0x67452301, 0xEFCDAB89, 0x98BADCFE, 0x10325476, 0xC3D2E1F0};
    uint32_t a;
    uint32_t b;
    uint32_t c;
    uint32_t d;
    uint32_t e;
    uint32_t f = 0;
    uint32_t k = 0;

    uint32_t idx;
    uint32_t lidx;
    uint32_t widx;
    uint32_t didx = 0;

    int32_t wcount;
    uint32_t temp;
    uint64_t databits = ((uint64_t)databytes) * 8;
    uint32_t loopcount = (databytes + 8) / 64 + 1;
    uint32_t tailbytes = 64 * loopcount - databytes;
    uint8_t datatail[128] = {0};

    if(!digest && !hexdigest) return -1;

    if(!data) return -1;

    /* Pre-processing of data tail (includes padding to fill out 512-bit chunk):
     Add bit '1' to end of message (big-endian)
     Add 64-bit message length in bits at very end (big-endian) */
    datatail[0] = 0x80;
    datatail[tailbytes - 8] = (uint8_t)(databits >> 56 & 0xFF);
    datatail[tailbytes - 7] = (uint8_t)(databits >> 48 & 0xFF);
    datatail[tailbytes - 6] = (uint8_t)(databits >> 40 & 0xFF);
    datatail[tailbytes - 5] = (uint8_t)(databits >> 32 & 0xFF);
    datatail[tailbytes - 4] = (uint8_t)(databits >> 24 & 0xFF);
    datatail[tailbytes - 3] = (uint8_t)(databits >> 16 & 0xFF);
    datatail[tailbytes - 2] = (uint8_t)(databits >> 8 & 0xFF);
    datatail[tailbytes - 1] = (uint8_t)(databits >> 0 & 0xFF);

    /* Process each 512-bit chunk */
    for(lidx = 0; lidx < loopcount; lidx++) {
        /* Compute all elements in W */
        memset(W, 0, 80 * sizeof(uint32_t));

        /* Break 512-bit chunk into sixteen 32-bit, big endian words */
        for(widx = 0; widx <= 15; widx++) {
            wcount = 24;

            /* Copy byte-per byte from specified buffer */
            while(didx < databytes && wcount >= 0) {
                W[widx] += (((uint32_t)data[didx]) << wcount);
                didx++;
                wcount -= 8;
            }
            /* Fill out W with padding as needed */
            while(wcount >= 0) {
                W[widx] += (((uint32_t)datatail[didx - databytes]) << wcount);
                didx++;
                wcount -= 8;
            }
        }

        /* Extend the sixteen 32-bit words into eighty 32-bit words, with potential optimization from:
       "Improving the Performance of the Secure Hash Algorithm (SHA-1)" by Max Locktyukhin */
        for(widx = 16; widx <= 31; widx++) {
            W[widx] = SHA1ROTATELEFT((W[widx - 3] ^ W[widx - 8] ^ W[widx - 14] ^ W[widx - 16]), 1);
        }
        for(widx = 32; widx <= 79; widx++) {
            W[widx] =
                SHA1ROTATELEFT((W[widx - 6] ^ W[widx - 16] ^ W[widx - 28] ^ W[widx - 32]), 2);
        }

        /* Main loop */
        a = H[0];
        b = H[1];
        c = H[2];
        d = H[3];
        e = H[4];

        for(idx = 0; idx <= 79; idx++) {
            if(idx <= 19) {
                f = (b & c) | ((~b) & d);
                k = 0x5A827999;
            } else if(idx >= 20 && idx <= 39) {
                f = b ^ c ^ d;
                k = 0x6ED9EBA1;
            } else if(idx >= 40 && idx <= 59) {
                f = (b & c) | (b & d) | (c & d);
                k = 0x8F1BBCDC;
            } else if(idx >= 60 && idx <= 79) {
                f = b ^ c ^ d;
                k = 0xCA62C1D6;
            }
            temp = SHA1ROTATELEFT(a, 5) + f + e + k + W[idx];
            e = d;
            d = c;
            c = SHA1ROTATELEFT(b, 30);
            b = a;
            a = temp;
        }

        H[0] += a;
        H[1] += b;
        H[2] += c;
        H[3] += d;
        H[4] += e;
    }

    /* Store binary digest in supplied buffer */
    if(digest) {
        for(idx = 0; idx < 5; idx++) {
            digest[idx * 4 + 0] = (uint8_t)(H[idx] >> 24);
            digest[idx * 4 + 1] = (uint8_t)(H[idx] >> 16);
            digest[idx * 4 + 2] = (uint8_t)(H[idx] >> 8);
            digest[idx * 4 + 3] = (uint8_t)(H[idx]);
        }
    }

    /* Store hex version of digest in supplied buffer */
    if(hexdigest) {
        snprintf(hexdigest, 41, "%08x%08x%08x%08x%08x", H[0], H[1], H[2], H[3], H[4]);
    }

    return 0;
} /* End of sha1digest() */


FORCE_INLINE float lim_f(float in, float min, float max) {
    // return fmin_fast(fmax_fast(in, min), max);
    return (in < min ? min : in > max ? max : in);
}

FORCE_INLINE float lim_f_0_1(float in) {
    // return fmin_fast(fmax_fast(in, 0.0f), 1.f);

    return (in < 0.0f ? 0.0f : in > 1.0f ? 1.0f : in);
}

FORCE_INLINE float lim_f_n1_1(float in) {
    // return fmin_fast(fmax_fast(in, -1.f), 1.f);

    return (in < -1.0f ? -1.0f : in > 1.0f ? 1.0f : in);
}


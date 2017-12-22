/*
 * Copyright (c) 2016 spreadtrum, Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files
 * (the "Software"), to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify, merge,
 * publish, distribute, sublicense, and/or sell copies of the Software,
 * and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#ifndef _HW_CRYPTO_H
#define _HW_CRYPTO_H
#include <sprd_crypto.h>
#include <sprd_ecc_types.h>
#include <sprd_ecc.h>
#include <sprd_rsa.h>
#include <sprd_rng.h>
#include <sprd_aes.h>

#include <io_device_def.h>

#define SPRD_ENGINE_ECC_GEN             0x01
#define SPRD_ENGINE_ECC_SIGN            0x02
#define SPRD_ENGINE_ECC_VERIFY          0x03

#define SPRD_ENGINE_RSA_GEN             0x21
#define SPRD_ENGINE_RSA_ENCRYPT         0x22
#define SPRD_ENGINE_RSA_DECRYPT         0x23
#define SPRD_ENGINE_RSA_SIGN_RAW        0x24
#define SPRD_ENGINE_RSA_VERIFY_RAW      0x25

#define SPRD_ENGINE_RNG_GEN             0x31
#define SPRD_ENGINE_HUK_DERIVE          0x41

typedef struct sprd_ecc_params{
    sprd_ecc_pubkey_t *pubkey;
    sprd_ecc_prikey_t *prikey;
    uint32_t pubkey_len;
    uint32_t prikey_len;
    const uint8_t *digest;
    uint32_t digest_len;
    uint32_t sig_len;
    const uint8_t * sig;
}sprd_ecc_params_t;

typedef struct sprd_rsa_params{
    sprd_rsa_keypair_t *priv_key;
    sprd_rsa_pubkey_t *pub_key;
    const uint8_t * dig;
    uint32_t dig_size;
    size_t max_out;
    const uint8_t *sig;
    uint32_t sig_size;
    sprd_rsa_padding_t padding;
    int32_t result;
    uint8_t * key_e;
    uint32_t key_e_len;
    int32_t key_len;
    uint8_t * key_n;
    uint8_t * key_d;
}sprd_rsa_params_t;

typedef struct sprd_rng_params{
	uint32_t len;
	uint8_t *out;
}sprd_rng_params_t;

typedef struct sprd_hwkey_params{
	uint32_t pt_len;
	uint32_t ct_len;
	uint8_t *pt;
	uint8_t *ct;
}sprd_hwkey_params_t;

#endif

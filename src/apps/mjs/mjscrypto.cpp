#include "mjscrypto.h"
#include <mbedtls/aes.h>
#include <mbedtls/md5.h>
#include <esp_crc.h>
#include <esp_random.h>
#include <string.h>
#include "mjs.h"

#define AES_BLOCK_SIZE 16

static void bytes_to_hex(const uint8_t* src, size_t len, char* dst) {
    static const char hex_chars[] = "0123456789abcdef";
    for (size_t i = 0; i < len; i++) {
        dst[i * 2] = hex_chars[(src[i] >> 4) & 0x0F];
        dst[i * 2 + 1] = hex_chars[src[i] & 0x0F];
    }
    dst[len * 2] = '\0';
}

static int hex_char_to_nibble(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    return -1;
}

static int hex_to_bytes(const char* src, size_t hex_len, uint8_t* dst) {
    if (hex_len % 2 != 0) return -1;
    for (size_t i = 0; i < hex_len / 2; i++) {
        int hi = hex_char_to_nibble(src[i * 2]);
        int lo = hex_char_to_nibble(src[i * 2 + 1]);
        if (hi < 0 || lo < 0) return -1;
        dst[i] = (uint8_t)((hi << 4) | lo);
    }
    return 0;
}

// crypto.encrypt(plaintext, key) -> hex string
static void mjs_crypto_encrypt(struct mjs* mjs) {
    mjs_val_t pt_val = mjs_arg(mjs, 0);
    mjs_val_t key_val = mjs_arg(mjs, 1);

    size_t plaintext_len;
    const char* plaintext = mjs_get_string(mjs, &pt_val, &plaintext_len);
    size_t key_len;
    const char* key = mjs_get_string(mjs, &key_val, &key_len);

    if (key_len != 16 && key_len != 24 && key_len != 32) {
        mjs_return(mjs, mjs_mk_undefined());
        return;
    }

    // PKCS7 padding
    uint8_t pad_value = AES_BLOCK_SIZE - (plaintext_len % AES_BLOCK_SIZE);
    size_t padded_len = plaintext_len + pad_value;

    uint8_t* padded = static_cast<uint8_t*>(malloc(padded_len));
    if (!padded) {
        mjs_return(mjs, mjs_mk_undefined());
        return;
    }
    memcpy(padded, plaintext, plaintext_len);
    memset(padded + plaintext_len, pad_value, pad_value);

    // Generate random IV
    uint8_t iv[AES_BLOCK_SIZE];
    esp_fill_random(iv, AES_BLOCK_SIZE);

    // Encrypt
    mbedtls_aes_context ctx;
    mbedtls_aes_init(&ctx);
    int ret = mbedtls_aes_setkey_enc(&ctx, reinterpret_cast<const uint8_t*>(key), key_len * 8);
    if (ret != 0) {
        mbedtls_aes_free(&ctx);
        free(padded);
        mjs_return(mjs, mjs_mk_undefined());
        return;
    }

    uint8_t* ciphertext = static_cast<uint8_t*>(malloc(padded_len));
    if (!ciphertext) {
        mbedtls_aes_free(&ctx);
        free(padded);
        mjs_return(mjs, mjs_mk_undefined());
        return;
    }

    uint8_t iv_copy[AES_BLOCK_SIZE];
    memcpy(iv_copy, iv, AES_BLOCK_SIZE);
    ret = mbedtls_aes_crypt_cbc(&ctx, MBEDTLS_AES_ENCRYPT, padded_len, iv_copy, padded, ciphertext);
    mbedtls_aes_free(&ctx);
    free(padded);

    if (ret != 0) {
        free(ciphertext);
        mjs_return(mjs, mjs_mk_undefined());
        return;
    }

    // Build output: IV + ciphertext, hex-encoded
    size_t output_bin_len = AES_BLOCK_SIZE + padded_len;
    char* hex_output = static_cast<char*>(malloc(output_bin_len * 2 + 1));
    if (!hex_output) {
        free(ciphertext);
        mjs_return(mjs, mjs_mk_undefined());
        return;
    }

    bytes_to_hex(iv, AES_BLOCK_SIZE, hex_output);
    bytes_to_hex(ciphertext, padded_len, hex_output + AES_BLOCK_SIZE * 2);
    free(ciphertext);

    mjs_return(mjs, mjs_mk_string(mjs, hex_output, ~0, 1));
    free(hex_output);
}

// crypto.decrypt(hex_string, key) -> plaintext string
static void mjs_crypto_decrypt(struct mjs* mjs) {
    mjs_val_t hex_val = mjs_arg(mjs, 0);
    mjs_val_t key_val = mjs_arg(mjs, 1);

    size_t hex_len;
    const char* hex_input = mjs_get_string(mjs, &hex_val, &hex_len);
    size_t key_len;
    const char* key = mjs_get_string(mjs, &key_val, &key_len);

    if (key_len != 16 && key_len != 24 && key_len != 32) {
        mjs_return(mjs, mjs_mk_undefined());
        return;
    }

    if (hex_len < 64 || hex_len % 2 != 0) {
        mjs_return(mjs, mjs_mk_undefined());
        return;
    }

    size_t bin_len = hex_len / 2;
    uint8_t* bin_data = static_cast<uint8_t*>(malloc(bin_len));
    if (!bin_data) {
        mjs_return(mjs, mjs_mk_undefined());
        return;
    }

    if (hex_to_bytes(hex_input, hex_len, bin_data) != 0) {
        free(bin_data);
        mjs_return(mjs, mjs_mk_undefined());
        return;
    }

    uint8_t iv[AES_BLOCK_SIZE];
    memcpy(iv, bin_data, AES_BLOCK_SIZE);
    size_t ciphertext_len = bin_len - AES_BLOCK_SIZE;

    if (ciphertext_len == 0 || ciphertext_len % AES_BLOCK_SIZE != 0) {
        free(bin_data);
        mjs_return(mjs, mjs_mk_undefined());
        return;
    }

    mbedtls_aes_context ctx;
    mbedtls_aes_init(&ctx);
    int ret = mbedtls_aes_setkey_dec(&ctx, reinterpret_cast<const uint8_t*>(key), key_len * 8);
    if (ret != 0) {
        mbedtls_aes_free(&ctx);
        free(bin_data);
        mjs_return(mjs, mjs_mk_undefined());
        return;
    }

    uint8_t* pt = static_cast<uint8_t*>(malloc(ciphertext_len));
    if (!pt) {
        mbedtls_aes_free(&ctx);
        free(bin_data);
        mjs_return(mjs, mjs_mk_undefined());
        return;
    }

    ret = mbedtls_aes_crypt_cbc(&ctx, MBEDTLS_AES_DECRYPT, ciphertext_len, iv, bin_data + AES_BLOCK_SIZE, pt);
    mbedtls_aes_free(&ctx);
    free(bin_data);

    if (ret != 0) {
        free(pt);
        mjs_return(mjs, mjs_mk_undefined());
        return;
    }

    // Validate PKCS7 padding
    uint8_t pad_value = pt[ciphertext_len - 1];
    if (pad_value == 0 || pad_value > AES_BLOCK_SIZE) {
        free(pt);
        mjs_return(mjs, mjs_mk_undefined());
        return;
    }
    for (size_t i = ciphertext_len - pad_value; i < ciphertext_len; i++) {
        if (pt[i] != pad_value) {
            free(pt);
            mjs_return(mjs, mjs_mk_undefined());
            return;
        }
    }

    size_t plaintext_len = ciphertext_len - pad_value;
    mjs_return(mjs, mjs_mk_string(mjs, reinterpret_cast<const char*>(pt), plaintext_len, 1));
    free(pt);
}

// crypto.md5(data) -> hex string
static void mjs_crypto_md5(struct mjs* mjs) {
    mjs_val_t data_val = mjs_arg(mjs, 0);
    size_t data_len;
    const char* data = mjs_get_string(mjs, &data_val, &data_len);

    uint8_t hash[16];
    mbedtls_md5_context ctx;
    mbedtls_md5_init(&ctx);
    mbedtls_md5_starts_ret(&ctx);
    mbedtls_md5_update_ret(&ctx, reinterpret_cast<const uint8_t*>(data), data_len);
    mbedtls_md5_finish_ret(&ctx, hash);
    mbedtls_md5_free(&ctx);

    char hex_output[33];
    bytes_to_hex(hash, 16, hex_output);

    mjs_return(mjs, mjs_mk_string(mjs, hex_output, ~0, 1));
}

// crypto.crc32(data) -> number
static void mjs_crypto_crc32(struct mjs* mjs) {
    mjs_val_t data_val = mjs_arg(mjs, 0);
    size_t data_len;
    const char* data = mjs_get_string(mjs, &data_val, &data_len);

    uint32_t crc = esp_crc32_le(0, reinterpret_cast<const uint8_t*>(data), data_len);
    mjs_return(mjs, mjs_mk_number(mjs, crc));
}

void mjs_crypto_register(struct mjs* mjs) {
    mjs_val_t crypto = mjs_mk_object(mjs);
    mjs_set(mjs, crypto, "encrypt", ~0, mjs_mk_foreign_func(mjs, (mjs_func_ptr_t)mjs_crypto_encrypt));
    mjs_set(mjs, crypto, "decrypt", ~0, mjs_mk_foreign_func(mjs, (mjs_func_ptr_t)mjs_crypto_decrypt));
    mjs_set(mjs, crypto, "md5", ~0, mjs_mk_foreign_func(mjs, (mjs_func_ptr_t)mjs_crypto_md5));
    mjs_set(mjs, crypto, "crc32", ~0, mjs_mk_foreign_func(mjs, (mjs_func_ptr_t)mjs_crypto_crc32));
    mjs_val_t global = mjs_get_global(mjs);
    mjs_set(mjs, global, "crypto", ~0, crypto);
}

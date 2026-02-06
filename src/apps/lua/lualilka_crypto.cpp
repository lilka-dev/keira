#include "lualilka_crypto.h"

#include <mbedtls/aes.h>
#include <mbedtls/md5.h>
#include <esp_crc.h>
#include <esp_random.h>
#include <string.h>

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
int lualilka_crypto_encrypt(lua_State* L) {
    int n = lua_gettop(L);
    if (n != 2) {
        return luaL_error(L, "Очікується 2 аргументи, отримано %d", n);
    }

    size_t plaintext_len;
    const char* plaintext = luaL_checklstring(L, 1, &plaintext_len);
    size_t key_len;
    const char* key = luaL_checklstring(L, 2, &key_len);

    if (key_len != 16 && key_len != 24 && key_len != 32) {
        return luaL_error(L, "Ключ має бути 16, 24 або 32 байти, отримано %d", (int)key_len);
    }

    // PKCS7 padding
    uint8_t pad_value = AES_BLOCK_SIZE - (plaintext_len % AES_BLOCK_SIZE);
    size_t padded_len = plaintext_len + pad_value;

    uint8_t* padded = (uint8_t*)malloc(padded_len);
    if (!padded) {
        return luaL_error(L, "Не вдалося виділити пам'ять");
    }
    memcpy(padded, plaintext, plaintext_len);
    memset(padded + plaintext_len, pad_value, pad_value);

    // Generate random IV
    uint8_t iv[AES_BLOCK_SIZE];
    esp_fill_random(iv, AES_BLOCK_SIZE);

    // Encrypt
    mbedtls_aes_context ctx;
    mbedtls_aes_init(&ctx);
    int ret = mbedtls_aes_setkey_enc(&ctx, (const uint8_t*)key, key_len * 8);
    if (ret != 0) {
        mbedtls_aes_free(&ctx);
        free(padded);
        return luaL_error(L, "Помилка ініціалізації AES: %d", ret);
    }

    uint8_t* ciphertext = (uint8_t*)malloc(padded_len);
    if (!ciphertext) {
        mbedtls_aes_free(&ctx);
        free(padded);
        return luaL_error(L, "Не вдалося виділити пам'ять");
    }

    // CBC encrypt (iv is modified in place, so we use a copy)
    uint8_t iv_copy[AES_BLOCK_SIZE];
    memcpy(iv_copy, iv, AES_BLOCK_SIZE);
    ret = mbedtls_aes_crypt_cbc(&ctx, MBEDTLS_AES_ENCRYPT, padded_len, iv_copy, padded, ciphertext);
    mbedtls_aes_free(&ctx);
    free(padded);

    if (ret != 0) {
        free(ciphertext);
        return luaL_error(L, "Помилка шифрування AES: %d", ret);
    }

    // Build output: IV + ciphertext, hex-encoded
    size_t output_bin_len = AES_BLOCK_SIZE + padded_len;
    char* hex_output = (char*)malloc(output_bin_len * 2 + 1);
    if (!hex_output) {
        free(ciphertext);
        return luaL_error(L, "Не вдалося виділити пам'ять");
    }

    // Hex-encode IV
    bytes_to_hex(iv, AES_BLOCK_SIZE, hex_output);
    // Hex-encode ciphertext
    bytes_to_hex(ciphertext, padded_len, hex_output + AES_BLOCK_SIZE * 2);
    free(ciphertext);

    lua_pushstring(L, hex_output);
    free(hex_output);
    return 1;
}

// crypto.decrypt(hex_string, key) -> plaintext string
int lualilka_crypto_decrypt(lua_State* L) {
    int n = lua_gettop(L);
    if (n != 2) {
        return luaL_error(L, "Очікується 2 аргументи, отримано %d", n);
    }

    size_t hex_len;
    const char* hex_input = luaL_checklstring(L, 1, &hex_len);
    size_t key_len;
    const char* key = luaL_checklstring(L, 2, &key_len);

    if (key_len != 16 && key_len != 24 && key_len != 32) {
        return luaL_error(L, "Ключ має бути 16, 24 або 32 байти, отримано %d", (int)key_len);
    }

    // Minimum: 16 bytes IV + 16 bytes ciphertext = 32 bytes = 64 hex chars
    if (hex_len < 64 || hex_len % 2 != 0) {
        return luaL_error(L, "Невірний формат зашифрованих даних");
    }

    size_t bin_len = hex_len / 2;
    uint8_t* bin_data = (uint8_t*)malloc(bin_len);
    if (!bin_data) {
        return luaL_error(L, "Не вдалося виділити пам'ять");
    }

    if (hex_to_bytes(hex_input, hex_len, bin_data) != 0) {
        free(bin_data);
        return luaL_error(L, "Невірний hex-рядок");
    }

    // Extract IV and ciphertext
    uint8_t iv[AES_BLOCK_SIZE];
    memcpy(iv, bin_data, AES_BLOCK_SIZE);
    size_t ciphertext_len = bin_len - AES_BLOCK_SIZE;

    if (ciphertext_len == 0 || ciphertext_len % AES_BLOCK_SIZE != 0) {
        free(bin_data);
        return luaL_error(L, "Невірний розмір зашифрованих даних");
    }

    // Decrypt
    mbedtls_aes_context ctx;
    mbedtls_aes_init(&ctx);
    int ret = mbedtls_aes_setkey_dec(&ctx, (const uint8_t*)key, key_len * 8);
    if (ret != 0) {
        mbedtls_aes_free(&ctx);
        free(bin_data);
        return luaL_error(L, "Помилка ініціалізації AES: %d", ret);
    }

    uint8_t* plaintext = (uint8_t*)malloc(ciphertext_len);
    if (!plaintext) {
        mbedtls_aes_free(&ctx);
        free(bin_data);
        return luaL_error(L, "Не вдалося виділити пам'ять");
    }

    ret = mbedtls_aes_crypt_cbc(&ctx, MBEDTLS_AES_DECRYPT, ciphertext_len, iv, bin_data + AES_BLOCK_SIZE, plaintext);
    mbedtls_aes_free(&ctx);
    free(bin_data);

    if (ret != 0) {
        free(plaintext);
        return luaL_error(L, "Помилка дешифрування AES: %d", ret);
    }

    // Validate and remove PKCS7 padding
    uint8_t pad_value = plaintext[ciphertext_len - 1];
    if (pad_value == 0 || pad_value > AES_BLOCK_SIZE) {
        free(plaintext);
        return luaL_error(L, "Невірний ключ або пошкоджені дані");
    }
    for (size_t i = ciphertext_len - pad_value; i < ciphertext_len; i++) {
        if (plaintext[i] != pad_value) {
            free(plaintext);
            return luaL_error(L, "Невірний ключ або пошкоджені дані");
        }
    }

    size_t plaintext_len = ciphertext_len - pad_value;
    lua_pushlstring(L, (const char*)plaintext, plaintext_len);
    free(plaintext);
    return 1;
}

// crypto.md5(data) -> hex string
int lualilka_crypto_md5(lua_State* L) {
    int n = lua_gettop(L);
    if (n != 1) {
        return luaL_error(L, "Очікується 1 аргумент, отримано %d", n);
    }

    size_t data_len;
    const char* data = luaL_checklstring(L, 1, &data_len);

    uint8_t hash[16];
    mbedtls_md5_context ctx;
    mbedtls_md5_init(&ctx);
    mbedtls_md5_starts_ret(&ctx);
    mbedtls_md5_update_ret(&ctx, (const uint8_t*)data, data_len);
    mbedtls_md5_finish_ret(&ctx, hash);
    mbedtls_md5_free(&ctx);

    char hex_output[33];
    bytes_to_hex(hash, 16, hex_output);

    lua_pushstring(L, hex_output);
    return 1;
}

// crypto.crc32(data) -> integer
int lualilka_crypto_crc32(lua_State* L) {
    int n = lua_gettop(L);
    if (n != 1) {
        return luaL_error(L, "Очікується 1 аргумент, отримано %d", n);
    }

    size_t data_len;
    const char* data = luaL_checklstring(L, 1, &data_len);

    // ESP ROM CRC32 (апаратна функція з ROM)
    uint32_t crc = esp_crc32_le(0, (const uint8_t*)data, data_len);

    lua_pushinteger(L, crc);
    return 1;
}

static const luaL_Reg lualilka_crypto[] = {
    {"encrypt", lualilka_crypto_encrypt},
    {"decrypt", lualilka_crypto_decrypt},
    {"md5", lualilka_crypto_md5},
    {"crc32", lualilka_crypto_crc32},
    {NULL, NULL},
};

int lualilka_crypto_register(lua_State* L) {
    luaL_newlib(L, lualilka_crypto);
    lua_setglobal(L, "crypto");
    return 0;
}

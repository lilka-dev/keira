#include "mjsspi.h"
#include <lilka.h>
#include "mjs.h"

// The user SPI bus (SPI2 / FSPI) is only available on Lilka v2 (ESP32-S3).
#ifdef SPI2_NUM
#    define MJS_SPI_BUS       lilka::SPI2
#    define MJS_SPI_AVAILABLE 1
#else
#    define MJS_SPI_AVAILABLE 0
#endif

// spi.begin([sck, miso, mosi])
static void mjs_spi_begin(struct mjs* mjs) {
#if MJS_SPI_AVAILABLE
    int n = mjs_nargs(mjs);
    if (n >= 3) {
        int sck = mjs_get_int(mjs, mjs_arg(mjs, 0));
        int miso = mjs_get_int(mjs, mjs_arg(mjs, 1));
        int mosi = mjs_get_int(mjs, mjs_arg(mjs, 2));
        MJS_SPI_BUS.begin(sck, miso, mosi);
    } else {
        MJS_SPI_BUS.begin();
    }
    mjs_return(mjs, mjs_mk_undefined());
#else
    mjs_set_errorf(mjs, MJS_INTERNAL_ERROR, "spi is not available on this board");
    mjs_return(mjs, mjs_mk_undefined());
#endif
}

// spi.transfer(data[, frequency[, mode]]) -> received byte(s)
static void mjs_spi_transfer(struct mjs* mjs) {
#if MJS_SPI_AVAILABLE
    mjs_val_t dataArg = mjs_arg(mjs, 0);
    mjs_val_t freqArg = mjs_arg(mjs, 1);
    mjs_val_t modeArg = mjs_arg(mjs, 2);
    uint32_t freq = mjs_is_number(freqArg) ? (uint32_t)mjs_get_int(mjs, freqArg) : 4000000;
    int mode = mjs_is_number(modeArg) ? mjs_get_int(mjs, modeArg) : SPI_MODE0;
    SPISettings settings(freq, MSBFIRST, mode);

    if (mjs_is_number(dataArg)) {
        uint8_t out = (uint8_t)(mjs_get_int(mjs, dataArg) & 0xFF);
        MJS_SPI_BUS.beginTransaction(settings);
        uint8_t in = MJS_SPI_BUS.transfer(out);
        MJS_SPI_BUS.endTransaction();
        mjs_return(mjs, mjs_mk_number(mjs, in));
        return;
    }

    int len = mjs_is_array(dataArg) ? mjs_array_length(mjs, dataArg) : 0;
    mjs_val_t arr = mjs_mk_array(mjs);
    MJS_SPI_BUS.beginTransaction(settings);
    for (int i = 0; i < len; i++) {
        uint8_t out = (uint8_t)(mjs_get_int(mjs, mjs_array_get(mjs, dataArg, i)) & 0xFF);
        uint8_t in = MJS_SPI_BUS.transfer(out);
        mjs_array_push(mjs, arr, mjs_mk_number(mjs, in));
    }
    MJS_SPI_BUS.endTransaction();
    mjs_return(mjs, arr);
#else
    mjs_set_errorf(mjs, MJS_INTERNAL_ERROR, "spi is not available on this board");
    mjs_return(mjs, mjs_mk_undefined());
#endif
}

// spi.close()
static void mjs_spi_close(struct mjs* mjs) {
#if MJS_SPI_AVAILABLE
    MJS_SPI_BUS.end();
#endif
    mjs_return(mjs, mjs_mk_undefined());
}

void mjs_spi_register(struct mjs* mjs) {
    mjs_val_t spi = mjs_mk_object(mjs);
    mjs_set(mjs, spi, "begin", ~0, mjs_mk_foreign_func(mjs, (mjs_func_ptr_t)mjs_spi_begin));
    mjs_set(mjs, spi, "transfer", ~0, mjs_mk_foreign_func(mjs, (mjs_func_ptr_t)mjs_spi_transfer));
    mjs_set(mjs, spi, "close", ~0, mjs_mk_foreign_func(mjs, (mjs_func_ptr_t)mjs_spi_close));
    mjs_set(mjs, spi, "MODE0", ~0, mjs_mk_number(mjs, SPI_MODE0));
    mjs_set(mjs, spi, "MODE1", ~0, mjs_mk_number(mjs, SPI_MODE1));
    mjs_set(mjs, spi, "MODE2", ~0, mjs_mk_number(mjs, SPI_MODE2));
    mjs_set(mjs, spi, "MODE3", ~0, mjs_mk_number(mjs, SPI_MODE3));
    mjs_val_t global = mjs_get_global(mjs);
    mjs_set(mjs, global, "spi", ~0, spi);
}

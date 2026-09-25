#include "wallpaper.h"

#include <stdio.h>
#include <string.h>

// Delay between redraws of a static (non-animated) wallpaper
#define WALLPAPER_STATIC_DELAY_MS 100

Wallpaper::~Wallpaper() {
    close();
}

bool Wallpaper::open(const char* path, int16_t bufferWidth, int16_t bufferHeight) {
    close();

    // Detect format by file signature
    uint8_t signature[4] = {0};
    FILE* file = fopen(path, "rb");
    if (file == nullptr) return false;
    size_t signatureSize = fread(signature, 1, sizeof(signature), file);
    fclose(file);
    if (signatureSize != sizeof(signature)) return false;

    buffer = static_cast<uint16_t*>(ps_malloc(bufferWidth * bufferHeight * sizeof(uint16_t)));
    if (buffer == nullptr) {
        lilka::serial.err("Wallpaper: failed to allocate buffer");
        return false;
    }
    memset(buffer, 0, bufferWidth * bufferHeight * sizeof(uint16_t));
    width = bufferWidth;
    height = bufferHeight;

    bool opened = false;
    if (memcmp(signature, "GIF8", 4) == 0) {
        opened = openGIF(path);
    } else if (signature[0] == 0xFF && signature[1] == 0xD8) {
        opened = openJPEG(path);
    } else if ((signature[0] == 0x89 && memcmp(&signature[1], "PNG", 3) == 0) || memcmp(signature, "BM", 2) == 0) {
        opened = openImage(path);
    } else {
        lilka::serial.err("Wallpaper: %s has unsupported format", path);
    }

    if (!opened) close();
    return opened;
}

bool Wallpaper::openGIF(const char* path) {
    gif.reset(new AnimatedGIF());
    gif->begin(GIF_PALETTE_RGB565_LE);
    if (!gif->open(
            path,
            fileOpen,
            fileClose,
            [](GIFFILE* f, uint8_t* buf, int32_t len) {
                return fileRead(static_cast<FILE*>(f->fHandle), f->iSize, &f->iPos, buf, len);
            },
            [](GIFFILE* f, int32_t position) {
                fseek(static_cast<FILE*>(f->fHandle), position, SEEK_SET);
                return f->iPos = position;
            },
            gifDraw
        )) {
        if (gif->getLastError() == GIF_TOO_WIDE) {
            lilka::serial.err("Wallpaper: %s is too wide, max GIF width is %d px", path, MAX_WIDTH);
        } else {
            lilka::serial.err("Wallpaper: failed to open GIF %s", path);
        }
        gif.reset();
        return false;
    }

    if (gif->allocFrameBuf(gifAlloc) != GIF_SUCCESS) {
        lilka::serial.err("Wallpaper: failed to allocate GIF frame buffer");
        gif->close();
        gif.reset();
        return false;
    }
    gif->setDrawType(GIF_DRAW_COOKED);

    offsetX = (width - gif->getCanvasWidth()) / 2;
    offsetY = (height - gif->getCanvasHeight()) / 2;
    return true;
}

bool Wallpaper::openJPEG(const char* path) {
    std::unique_ptr<JPEGDEC> jpeg(new JPEGDEC());
    if (!jpeg->open(
            path,
            fileOpen,
            fileClose,
            [](JPEGFILE* f, uint8_t* buf, int32_t len) {
                return fileRead(static_cast<FILE*>(f->fHandle), f->iSize, &f->iPos, buf, len);
            },
            [](JPEGFILE* f, int32_t position) {
                fseek(static_cast<FILE*>(f->fHandle), position, SEEK_SET);
                return f->iPos = position;
            },
            jpegDraw
        )) {
        lilka::serial.err("Wallpaper: failed to open JPEG %s", path);
        return false;
    }

    // Pick the smallest downscale that fits the image into buffer, crop if even 1/8 doesn't fit
    // JPEG_SCALE_HALF/QUARTER/EIGHTH option values are equal to their divisors
    int scale = 1;
    while (scale < JPEG_SCALE_EIGHTH && (jpeg->getWidth() / scale > width || jpeg->getHeight() / scale > height)) {
        scale *= 2;
    }
    offsetX = (width - jpeg->getWidth() / scale) / 2;
    offsetY = (height - jpeg->getHeight() / scale) / 2;

    jpeg->setPixelType(RGB565_LITTLE_ENDIAN);
    jpeg->setUserPointer(this);
    bool decoded = jpeg->decode(0, 0, scale > 1 ? scale : 0) == 1;
    if (!decoded) {
        // Progressive JPEGs are not supported by the decoder
        lilka::serial.err("Wallpaper: failed to decode JPEG %s (error %d)", path, jpeg->getLastError());
    }
    jpeg->close();
    return decoded;
}

bool Wallpaper::openImage(const char* path) {
    // Transparent PNG pixels are shown as black
    std::unique_ptr<lilka::Image> image(lilka::resources.loadImage(path, lilka::colors::Black));
    if (!image) {
        lilka::serial.err("Wallpaper: failed to load image %s", path);
        return false;
    }
    offsetX = (width - static_cast<int>(image->width)) / 2;
    offsetY = (height - static_cast<int>(image->height)) / 2;
    blit(image->pixels, 0, 0, image->width, image->height, image->width);
    return true;
}

void Wallpaper::close() {
    if (gif) {
        gif->freeFrameBuf(gifFree);
        gif->close();
        gif.reset();
    }
    if (buffer != nullptr) {
        free(buffer);
        buffer = nullptr;
    }
}

bool Wallpaper::isOpen() {
    return buffer != nullptr;
}

int Wallpaper::nextFrame() {
    if (!gif) return WALLPAPER_STATIC_DELAY_MS;

    int delayMs = 0;
    // Returns 0 on the last frame (and rewinds automatically), -1 on error
    if (gif->playFrame(false, &delayMs, this) < 0) {
        gif->reset();
    }
    return delayMs;
}

void Wallpaper::draw(lilka::Canvas* canvas) {
    canvas->draw16bitRGBBitmap(0, 0, buffer, width, height);
}

void Wallpaper::blit(const uint16_t* pixels, int x, int y, int w, int h, int stride) {
    x += offsetX;
    y += offsetY;
    int startX = x < 0 ? -x : 0;
    int endX = x + w > width ? width - x : w;
    if (startX >= endX) return;

    for (int row = 0; row < h; row++) {
        int bufferY = y + row;
        if (bufferY < 0) continue;
        if (bufferY >= height) break;
        memcpy(
            &buffer[bufferY * width + x + startX], &pixels[row * stride + startX], (endX - startX) * sizeof(uint16_t)
        );
    }
}

void* Wallpaper::fileOpen(const char* path, int32_t* size) {
    FILE* file = fopen(path, "rb");
    if (file == nullptr) return nullptr;
    fseek(file, 0, SEEK_END);
    *size = ftell(file);
    fseek(file, 0, SEEK_SET);
    return file;
}

void Wallpaper::fileClose(void* handle) {
    fclose(static_cast<FILE*>(handle));
}

int32_t Wallpaper::fileRead(FILE* file, int32_t size, int32_t* pos, uint8_t* buf, int32_t len) {
    int32_t bytesLeft = size - *pos;
    if (len > bytesLeft) len = bytesLeft;
    if (len <= 0) return 0;
    int32_t bytesRead = fread(buf, 1, len, file);
    *pos += bytesRead;
    return bytesRead;
}

void Wallpaper::gifDraw(GIFDRAW* pDraw) {
    // Called once per line of the frame, pixels are already merged with previous frames
    auto wallpaper = static_cast<Wallpaper*>(pDraw->pUser);
    wallpaper->blit(
        reinterpret_cast<const uint16_t*>(pDraw->pPixels), pDraw->iX, pDraw->iY + pDraw->y, pDraw->iWidth, 1, 0
    );
}

int Wallpaper::jpegDraw(JPEGDRAW* pDraw) {
    // Called once per block of MCUs
    auto wallpaper = static_cast<Wallpaper*>(pDraw->pUser);
    wallpaper->blit(pDraw->pPixels, pDraw->x, pDraw->y, pDraw->iWidthUsed, pDraw->iHeight, pDraw->iWidth);
    return 1; // Continue decoding
}

void* Wallpaper::gifAlloc(uint32_t size) {
    return ps_malloc(size);
}

void Wallpaper::gifFree(void* p) {
    free(p);
}

#pragma once

#include <memory>
#include <AnimatedGIF.h>
#include <JPEGDEC.h>
#include <lilka.h>

// Home screen wallpaper, read from a file and decoded into an RGB565 buffer
// of a given size. The image is centered within the buffer.
// Supported formats: animated GIF, PNG, JPEG (baseline) and BMP.
class Wallpaper {
public:
    ~Wallpaper();
    // Opens image file. Returns false if file is missing or can't be decoded
    bool open(const char* path, int16_t bufferWidth, int16_t bufferHeight);
    void close();
    bool isOpen();
    // Decodes next frame into buffer. Returns delay in ms before the next frame should be shown
    int nextFrame();
    void draw(lilka::Canvas* canvas);

private:
    bool openGIF(const char* path);
    bool openJPEG(const char* path);
    bool openImage(const char* path);
    // Copies a block of pixels into buffer at image position (x, y), clipping it to buffer bounds
    void blit(const uint16_t* pixels, int x, int y, int w, int h, int stride);

    static void* fileOpen(const char* path, int32_t* size);
    static void fileClose(void* handle);
    static int32_t fileRead(FILE* file, int32_t size, int32_t* pos, uint8_t* buf, int32_t len);
    static void gifDraw(GIFDRAW* pDraw);
    static int jpegDraw(JPEGDRAW* pDraw);
    static void* gifAlloc(uint32_t size);
    static void gifFree(void* p);

    std::unique_ptr<AnimatedGIF> gif;
    uint16_t* buffer = nullptr;
    int16_t width = 0;
    int16_t height = 0;
    // Position of the image top-left corner within buffer, negative if image is bigger than buffer
    int16_t offsetX = 0;
    int16_t offsetY = 0;
};

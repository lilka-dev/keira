#pragma once

#include <memory>
#include <AnimatedGIF.h>

#include "keira/app.h"

// Fullscreen image viewer. Supported formats: animated GIF, PNG, JPEG (baseline) and BMP.
// Image is centered on screen, images bigger than screen can be panned with D-pad.
class ImageViewerApp : public App {
public:
    explicit ImageViewerApp(String path);
    ~ImageViewerApp() override;
    void run() override;

private:
    bool open();
    bool openGIF();
    bool openJPEG();
    bool openImage();
    void close();
    // Moves the viewport by (dx, dy) keeping it within image bounds. Returns true if viewport has moved
    bool pan(int dx, int dy);
    void draw();
    // Copies a block of pixels into image at (x, y), clipping it to image bounds
    void blit(const uint16_t* pixels, int x, int y, int w, int h, int stride);

    String path;
    std::unique_ptr<lilka::Image> image;
    std::unique_ptr<AnimatedGIF> gif;
    // Top-left corner of the visible part of image, used only if image is bigger than screen
    int viewX = 0;
    int viewY = 0;
};

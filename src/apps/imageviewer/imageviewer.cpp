#include <JPEGDEC.h>
#include <stdio.h>
#include <string.h>

#include "imageviewer.h"
#include "keira/keira.h"

// Lower bound for frame time, GIF frames may have zero delay
#define IMAGEVIEWER_MIN_FRAME_MS 30
// Delay between redraws of a static (non-animated) image
#define IMAGEVIEWER_STATIC_FRAME_MS 100
// Pan speed while D-pad is held
#define IMAGEVIEWER_PAN_STEP 8

// File callbacks shared by GIF and JPEG decoders
static void* fileOpen(const char* path, int32_t* size) {
    FILE* file = fopen(path, "rb");
    if (file == nullptr) return nullptr;
    fseek(file, 0, SEEK_END);
    *size = ftell(file);
    fseek(file, 0, SEEK_SET);
    return file;
}

static void fileClose(void* handle) {
    fclose(static_cast<FILE*>(handle));
}

static int32_t fileRead(void* handle, int32_t size, int32_t* pos, uint8_t* buf, int32_t len) {
    int32_t bytesLeft = size - *pos;
    if (len > bytesLeft) len = bytesLeft;
    if (len <= 0) return 0;
    int32_t bytesRead = fread(buf, 1, len, static_cast<FILE*>(handle));
    *pos += bytesRead;
    return bytesRead;
}

static int32_t fileSeek(void* handle, int32_t* pos, int32_t position) {
    fseek(static_cast<FILE*>(handle), position, SEEK_SET);
    return *pos = position;
}

static void* gifAlloc(uint32_t size) {
    return ps_malloc(size);
}

static void gifFree(void* p) {
    free(p);
}

ImageViewerApp::ImageViewerApp(String path) : App("Image Viewer"), path(path) {
    setktStackSize(8192); // Image decoders are stack-hungry
    setFlags(APP_FLAG_FULLSCREEN);
}

ImageViewerApp::~ImageViewerApp() {
    close();
}

void ImageViewerApp::run() {
    if (!open()) {
        alert(K_S_ERROR, K_S_IMAGEVIEWER_CANT_OPEN);
        return;
    }
    // Start from the center of images bigger than screen
    int centerX = (static_cast<int>(image->width) - canvas->width()) / 2;
    int centerY = (static_cast<int>(image->height) - canvas->height()) / 2;
    pan(centerX, centerY);

    while (1) {
        int delayMs = IMAGEVIEWER_STATIC_FRAME_MS;
        // Returns 0 on the last frame, -1 on error
        if (gif && gif->playFrame(false, &delayMs, this) < 0) {
            gif->reset();
        }
        draw();

        // Wait for the next frame while staying responsive to buttons
        TickType_t frameEnd = xTaskGetTickCount() + pdMS_TO_TICKS(max(delayMs, IMAGEVIEWER_MIN_FRAME_MS));
        do {
            lilka::State state = lilka::controller.getState();
            if (state.b.justPressed || state.a.justPressed) return;

            int dx = (state.right.pressed - state.left.pressed) * IMAGEVIEWER_PAN_STEP;
            int dy = (state.down.pressed - state.up.pressed) * IMAGEVIEWER_PAN_STEP;
            if (pan(dx, dy)) draw();

            vTaskDelay(pdMS_TO_TICKS(10));
        } while (xTaskGetTickCount() < frameEnd);
    }
}

bool ImageViewerApp::open() {
    // Detect format by file signature
    uint8_t signature[4] = {0};
    FILE* file = fopen(path.c_str(), "rb");
    if (file == nullptr) {
        lilka::serial.err("ImageViewer: can't open %s", path.c_str());
        return false;
    }
    size_t signatureSize = fread(signature, 1, sizeof(signature), file);
    fclose(file);
    if (signatureSize != sizeof(signature)) return false;

    bool opened = false;
    if (memcmp(signature, "GIF8", 4) == 0) {
        opened = openGIF();
    } else if (signature[0] == 0xFF && signature[1] == 0xD8) {
        opened = openJPEG();
    } else if ((signature[0] == 0x89 && memcmp(&signature[1], "PNG", 3) == 0) || memcmp(signature, "BM", 2) == 0) {
        opened = openImage();
    } else {
        lilka::serial.err("ImageViewer: %s has unsupported format", path.c_str());
    }

    if (!opened) close();
    return opened;
}

bool ImageViewerApp::openGIF() {
    gif.reset(new AnimatedGIF());
    gif->begin(GIF_PALETTE_RGB565_LE);
    if (!gif->open(
            path.c_str(),
            fileOpen,
            fileClose,
            [](GIFFILE* f, uint8_t* buf, int32_t len) { return fileRead(f->fHandle, f->iSize, &f->iPos, buf, len); },
            [](GIFFILE* f, int32_t position) { return fileSeek(f->fHandle, &f->iPos, position); },
            [](GIFDRAW* pDraw) {
                // Called once per line of the frame, pixels are already merged with previous frames
                auto app = static_cast<ImageViewerApp*>(pDraw->pUser);
                app->blit(
                    reinterpret_cast<const uint16_t*>(pDraw->pPixels),
                    pDraw->iX,
                    pDraw->iY + pDraw->y,
                    pDraw->iWidth,
                    1,
                    pDraw->iWidth
                );
            }
        )) {
        if (gif->getLastError() == GIF_TOO_WIDE) {
            lilka::serial.err("ImageViewer: %s is too wide, max GIF width is %d px", path.c_str(), MAX_WIDTH);
        } else {
            lilka::serial.err("ImageViewer: failed to open GIF %s", path.c_str());
        }
        gif.reset();
        return false;
    }

    image.reset(new lilka::Image(gif->getCanvasWidth(), gif->getCanvasHeight()));
    if (image->pixels == nullptr) {
        lilka::serial.err("ImageViewer: failed to allocate image buffer");
        return false;
    }
    memset(image->pixels, 0, image->width * image->height * sizeof(uint16_t));

    if (gif->allocFrameBuf(gifAlloc) != GIF_SUCCESS) {
        lilka::serial.err("ImageViewer: failed to allocate GIF frame buffer");
        return false;
    }
    gif->setDrawType(GIF_DRAW_COOKED);
    return true;
}

bool ImageViewerApp::openJPEG() {
    std::unique_ptr<JPEGDEC> jpeg(new JPEGDEC());
    if (!jpeg->open(
            path.c_str(),
            fileOpen,
            fileClose,
            [](JPEGFILE* f, uint8_t* buf, int32_t len) { return fileRead(f->fHandle, f->iSize, &f->iPos, buf, len); },
            [](JPEGFILE* f, int32_t position) { return fileSeek(f->fHandle, &f->iPos, position); },
            [](JPEGDRAW* pDraw) {
                // Called once per block of MCUs
                auto app = static_cast<ImageViewerApp*>(pDraw->pUser);
                app->blit(pDraw->pPixels, pDraw->x, pDraw->y, pDraw->iWidthUsed, pDraw->iHeight, pDraw->iWidth);
                return 1; // Continue decoding
            }
        )) {
        lilka::serial.err("ImageViewer: failed to open JPEG %s", path.c_str());
        return false;
    }

    // Pick the smallest downscale that fits the image on screen, the rest can be panned
    // JPEG_SCALE_HALF/QUARTER/EIGHTH option values are equal to their divisors
    int scale = 1;
    while (scale < JPEG_SCALE_EIGHTH &&
           (jpeg->getWidth() / scale > canvas->width() || jpeg->getHeight() / scale > canvas->height())) {
        scale *= 2;
    }
    image.reset(new lilka::Image((jpeg->getWidth() + scale - 1) / scale, (jpeg->getHeight() + scale - 1) / scale));
    if (image->pixels == nullptr) {
        lilka::serial.err("ImageViewer: failed to allocate image buffer");
        jpeg->close();
        return false;
    }
    memset(image->pixels, 0, image->width * image->height * sizeof(uint16_t));

    jpeg->setPixelType(RGB565_LITTLE_ENDIAN);
    jpeg->setUserPointer(this);
    bool decoded = jpeg->decode(0, 0, scale > 1 ? scale : 0) == 1;
    if (!decoded) {
        // Progressive JPEGs are not supported by the decoder
        lilka::serial.err("ImageViewer: failed to decode JPEG %s (error %d)", path.c_str(), jpeg->getLastError());
    }
    jpeg->close();
    return decoded;
}

bool ImageViewerApp::openImage() {
    // Transparent PNG pixels are shown as black
    image.reset(lilka::resources.loadImage(path, lilka::colors::Black));
    if (!image) {
        lilka::serial.err("ImageViewer: failed to load image %s", path.c_str());
        return false;
    }
    return true;
}

void ImageViewerApp::close() {
    if (gif) {
        gif->freeFrameBuf(gifFree);
        gif->close();
        gif.reset();
    }
    image.reset();
}

bool ImageViewerApp::pan(int dx, int dy) {
    int maxX = max(static_cast<int>(image->width) - canvas->width(), 0);
    int maxY = max(static_cast<int>(image->height) - canvas->height(), 0);
    int newX = constrain(viewX + dx, 0, maxX);
    int newY = constrain(viewY + dy, 0, maxY);
    if (newX == viewX && newY == viewY) return false;
    viewX = newX;
    viewY = newY;
    return true;
}

void ImageViewerApp::draw() {
    int w = image->width;
    int h = image->height;
    // Center images smaller than screen, show the viewport of bigger ones
    int x = w < canvas->width() ? (canvas->width() - w) / 2 : -viewX;
    int y = h < canvas->height() ? (canvas->height() - h) / 2 : -viewY;
    canvas->fillScreen(lilka::colors::Black);
    canvas->draw16bitRGBBitmap(x, y, image->pixels, w, h);
    queueDraw();
}

void ImageViewerApp::blit(const uint16_t* pixels, int x, int y, int w, int h, int stride) {
    int width = image->width;
    int height = image->height;
    int startX = x < 0 ? -x : 0;
    int endX = x + w > width ? width - x : w;
    if (startX >= endX) return;

    for (int row = 0; row < h; row++) {
        int imageY = y + row;
        if (imageY < 0) continue;
        if (imageY >= height) break;
        memcpy(
            &image->pixels[imageY * width + x + startX],
            &pixels[row * stride + startX],
            (endX - startX) * sizeof(uint16_t)
        );
    }
}

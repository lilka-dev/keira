#pragma once
#include "keira/app.h"

#include <esp_partition.h>
#include <vector>

// Installs the apps of a full flash image (e.g. retro-go_*_lilka.img) into the partitions with the
// same labels. The image's bootloader and partition table are ignored, Keira itself is never touched.
class FlashImageApp : public App {
public:
    explicit FlashImageApp(const String& path);

    // True when the file looks like a full flash image (partition table at 0x8000)
    static bool isFlashImage(const String& path);

    void run() override;

private:
    struct ImagePart {
        const esp_partition_t* target;
        uint32_t offset; // in the image file
        uint32_t size;
    };

    bool collectParts(FILE* file, std::vector<ImagePart>& parts);
    bool writePart(FILE* file, const ImagePart& part, uint32_t& written, uint32_t total);

    String path;
    String error;
};

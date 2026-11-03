#include "flashimage.h"
#include "keira/keira.h"
#include "keira/ksystem.h"
#include "keira/utils/defer.h"
#include "keira/utils/string.h"

#include <esp_ota_ops.h>

// Layout of a full ESP32 flash image: partition table entries at 0x8000
static constexpr uint32_t IMAGE_PARTITION_TABLE_OFFSET = 0x8000;
static constexpr uint16_t IMAGE_PARTITION_MAGIC = 0x50AA;
static constexpr size_t IMAGE_PARTITION_MAX_ENTRIES = 95;
static constexpr uint8_t IMAGE_PARTITION_TYPE_APP = 0x00;
static constexpr size_t FLASH_SECTOR_SIZE = 4096;

struct __attribute__((packed)) ImagePartitionEntry {
    uint16_t magic;
    uint8_t type;
    uint8_t subtype;
    uint32_t offset;
    uint32_t size;
    char label[16];
    uint32_t flags;
};

static_assert(sizeof(ImagePartitionEntry) == 32, "partition table entry is 32 bytes");

FlashImageApp::FlashImageApp(const String& path) : App("FlashImage"), path(path) {
    setktStackSize(8192);
}

bool FlashImageApp::isFlashImage(const String& path) {
    FILE* file = fopen(path.c_str(), "rb");
    if (!file) return false;
    Defer closeFile([file]() { fclose(file); });

    ImagePartitionEntry entry;
    return fseek(file, IMAGE_PARTITION_TABLE_OFFSET, SEEK_SET) == 0 && fread(&entry, sizeof(entry), 1, file) == 1 &&
           entry.magic == IMAGE_PARTITION_MAGIC;
}

bool FlashImageApp::collectParts(FILE* file, std::vector<ImagePart>& parts) {
    if (fseek(file, 0, SEEK_END) != 0) return false;
    uint32_t fileSize = ftell(file);
    if (fseek(file, IMAGE_PARTITION_TABLE_OFFSET, SEEK_SET) != 0) return false;

    std::vector<ImagePartitionEntry> entries;
    ImagePartitionEntry entry;
    while (entries.size() < IMAGE_PARTITION_MAX_ENTRIES && fread(&entry, sizeof(entry), 1, file) == 1 &&
           entry.magic == IMAGE_PARTITION_MAGIC) {
        entries.push_back(entry);
    }

    const esp_partition_t* running = esp_ota_get_running_partition();
    for (const auto& e : entries) {
        if (e.type != IMAGE_PARTITION_TYPE_APP || e.offset >= fileSize) continue;
        char label[sizeof(e.label) + 1] = {0};
        memcpy(label, e.label, sizeof(e.label));

        // Only apps that have a home in Keira's partition table, and never Keira itself
        const esp_partition_t* target =
            esp_partition_find_first(ESP_PARTITION_TYPE_APP, ESP_PARTITION_SUBTYPE_ANY, label);
        if (!target || target == running) continue;

        uint32_t size = std::min(e.size, fileSize - e.offset);
        if (size > target->size) {
            error = StringFormat(K_S_FLASHIMAGE_TOO_BIG_FMT, label);
            return false;
        }
        parts.push_back({target, e.offset, size});
    }
    if (parts.empty()) {
        error = K_S_FLASHIMAGE_NO_PARTS;
        return false;
    }
    return true;
}

bool FlashImageApp::writePart(FILE* file, const ImagePart& part, uint32_t& written, uint32_t total) {
    uint32_t eraseSize = (part.size + FLASH_SECTOR_SIZE - 1) / FLASH_SECTOR_SIZE * FLASH_SECTOR_SIZE;
    esp_err_t err = esp_partition_erase_range(part.target, 0, eraseSize);
    if (err != ESP_OK || fseek(file, part.offset, SEEK_SET) != 0) {
        error = StringFormat(K_S_FLASHIMAGE_WRITE_ERROR_FMT, part.target->label, esp_err_to_name(err));
        return false;
    }

    uint8_t* buf = static_cast<uint8_t*>(calloc(1, FLASH_SECTOR_SIZE));
    if (!buf) {
        error = StringFormat(K_S_FLASHIMAGE_WRITE_ERROR_FMT, part.target->label, esp_err_to_name(ESP_ERR_NO_MEM));
        return false;
    }
    Defer freeBuf([buf]() { free(buf); });

    lilka::ProgressDialog dialog(K_S_FLASHIMAGE_TITLE, part.target->label);
    for (uint32_t off = 0; off < part.size; off += FLASH_SECTOR_SIZE) {
        size_t chunk = std::min<uint32_t>(FLASH_SECTOR_SIZE, part.size - off);
        if (fread(buf, 1, chunk, file) != chunk) {
            error = StringFormat(K_S_FLASHIMAGE_WRITE_ERROR_FMT, part.target->label, "read");
            return false;
        }
        err = esp_partition_write(part.target, off, buf, chunk);
        if (err != ESP_OK) {
            error = StringFormat(K_S_FLASHIMAGE_WRITE_ERROR_FMT, part.target->label, esp_err_to_name(err));
            return false;
        }
        written += chunk;
        dialog.setProgress(static_cast<int64_t>(written) * 100 / total);
        dialog.draw(canvas);
        queueDraw();
    }
    return true;
}

void FlashImageApp::run() {
    FILE* file = fopen(path.c_str(), "rb");
    if (!file) {
        alert(K_S_ERROR, StringFormat(K_S_FLASHIMAGE_WRITE_ERROR_FMT, path.c_str(), strerror(errno)));
        return;
    }
    Defer closeFile([file]() { fclose(file); });

    std::vector<ImagePart> parts;
    if (!collectParts(file, parts)) {
        alert(K_S_ERROR, error);
        return;
    }

    String labels;
    uint32_t total = 0;
    for (const auto& part : parts) {
        labels += (labels.isEmpty() ? "" : ", ") + String(part.target->label);
        total += part.size;
    }
    String about =
        StringFormat(K_S_FLASHIMAGE_CONFIRM_FMT, labels.c_str(), lilka::fileutils.getHumanFriendlySize(total).c_str());
    if (!confirm(K_S_FLASHIMAGE_TITLE, about)) return;

    uint32_t written = 0;
    for (const auto& part : parts) {
        if (!writePart(file, part, written, total)) {
            alert(K_S_ERROR, error);
            return;
        }
    }

    // Image's first app is its entry point (e.g. Retro-Go's launcher)
    const esp_partition_t* entry = parts.front().target;
    if (confirm(K_S_FLASHIMAGE_TITLE, StringFormat(K_S_FLASHIMAGE_DONE_FMT, entry->label))) {
        KeiraSystem::bootPartition(entry);
    }
}

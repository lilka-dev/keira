#include "partitions.h"
#include "serial.h"

#include <esp_ota_ops.h>

// TODO: sys.get_partition_* to be deprecated
// TODO: simplify multiboot

namespace lilka {
// Construction
Partition::Partition(const esp_partition_t* partition) {
    this->partition = partition;
}

PartitionList::PartitionList() {
    // Retrive partition iterator
    esp_partition_iterator_t cur = esp_partition_find(ESP_PARTITION_TYPE_ANY, ESP_PARTITION_SUBTYPE_ANY, NULL);
    // Fill parts vector
    while (cur != NULL) {
        const esp_partition_t* part = esp_partition_get(cur);
        Partition* pPart = new Partition(part);
        parts.push_back(pPart);

        // advance
        cur = esp_partition_next(cur);
    }

    // yeah, it's NULL but it's okay
    // Release partition iterator.
    // Parameters:
    //     iterator -- Iterator obtained using esp_partition_find. The iterator is allowed to be NULL, so it is not necessary to check its value before calling this function.
    esp_partition_iterator_release(cur);
}

PartitionList::~PartitionList() {
    for (const auto& part : this->parts) {
        delete part;
    }
}

// Example callbacks:
static bool part_on_partition_flash_chunk_default(
    void* ctx, Partition* part, const String& filename, size_t offset, long fSize
) {
    lilka::serial.log("Flashing %s to flash:%s [%d/%d]", filename.c_str(), part->getLabel(), offset, fSize);

    return true;
}

static bool part_on_partition_backup_chunk_default(
    void* ctx, Partition* part, const String& filename, size_t offset, long fSize
) {
    lilka::serial.log("Backuping flash:%s to %s [%d/%d]", part->getLabel(), filename.c_str(), offset, fSize);

    return true;
}

// Operations:
bool Partition::flash(const String& filename, onPartitionChunkClbk chunkClbk, void* clbkData) {
    // Open file
    FILE* f = fopen(filename.c_str(), "rb");
    if (!f) return false;

    // Determine file size
    if (fseek(f, 0, SEEK_END) != 0) {
        fclose(f);
        return false;
    }
    long fSize = ftell(f);
    if (fSize < 0) {
        fclose(f);
        return false;
    }
    if (fseek(f, 0, SEEK_SET) != 0) {
        fclose(f);
        return false;
    }

    // Determine if flashing possible
    if (fSize > partition->size) {
        fclose(f);
        return false;
    }

    // Erase partition
    if (ESP_OK != esp_partition_erase_range(partition, 0, partition->size)) {
        fclose(f);
        return false;
    };

    // Allocate chunk
    char* chunk = static_cast<char*>(malloc(sizeof(char) * LILKA_PART_CHUNK_SIZE));
    if (!chunk) {
        fclose(f);
        return false;
    }

    // Prepare callback
    onPartitionChunkClbk clbk = (chunkClbk) ? chunkClbk : part_on_partition_flash_chunk_default;

    // Do flash
    size_t offset = 0;
    size_t bRead = 0;
    while ((bRead = fread(chunk, 1, LILKA_PART_CHUNK_SIZE, f)) > 0) {
        // Write
        if (ESP_OK != esp_partition_write(partition, offset, chunk, bRead)) break;

        // Advance
        offset = offset + bRead;

        // Launch callback
        bool proceed = clbk(clbkData, this, filename, offset, fSize);

        // Handle interruption possibility
        if (!proceed) break;
    }
    // Mark done
    // TODO: optional verify, checksum
    // uint8_t checksum[32];
    // memset(checksum, NULL, 32);
    // esp_partition_get_sha256(partition, checksum);
    bool isFlashDone = (offset == fSize);

    // Close file
    fclose(f);

    free(chunk);

    return isFlashDone;
}

bool Partition::backup(const String& filename, onPartitionChunkClbk chunkClbk, void* clbkData) {
    // Open file
    FILE* f = fopen(filename.c_str(), "wb");
    if (!f) return false;

    // Allocate chunk
    char* chunk = static_cast<char*>(malloc(sizeof(char) * LILKA_PART_CHUNK_SIZE));
    if (!chunk) {
        fclose(f);
        return false;
    }

    // Prepare callback
    onPartitionChunkClbk clbk = (chunkClbk) ? chunkClbk : part_on_partition_backup_chunk_default;

    size_t remaining = partition->size;
    size_t offset = 0;

    while (remaining > 0) {
        size_t to_read = (remaining > LILKA_PART_CHUNK_SIZE) ? LILKA_PART_CHUNK_SIZE : remaining;
        if (ESP_OK != esp_partition_read(partition, offset, chunk, to_read)) break;

        size_t written = fwrite(chunk, 1, to_read, f);
        if (written != to_read) break;

        offset += to_read;
        remaining -= to_read;

        // Launch callback
        bool proceed = clbk(clbkData, this, filename, offset, partition->size);
        if (!proceed) break;
    }

    // Mark done
    // TODO: optional verify, checksum
    // uint8_t checksum[32];
    // memset(checksum, NULL, 32);
    // esp_partition_get_sha256(partition, checksum);
    bool isBackupDone = (remaining == 0);

    free(chunk);
    fclose(f);

    return isBackupDone;
}

bool Partition::isRunning() {
    return partition == esp_ota_get_running_partition();
}

// Accessors:
esp_flash_t* Partition::getFlashChip() {
    return (partition != NULL) ? partition->flash_chip : NULL;
}

esp_partition_type_t Partition::getType() {
    return (partition != NULL) ? partition->type : ESP_PARTITION_TYPE_ANY;
}

esp_partition_subtype_t Partition::getSubtype() {
    return (partition != NULL) ? partition->subtype : ESP_PARTITION_SUBTYPE_ANY;
}

uint32_t Partition::getAddress() {
    return (partition != NULL) ? partition->address : 0;
}

uint32_t Partition::getSize() {
    return (partition != NULL) ? partition->size : 0;
}
const char* Partition::getLabel() {
    return (partition != NULL) ? partition->label : NULL;
}

bool Partition::getEncrypted() {
    return (partition != NULL) ? partition->encrypted : false;
}

Partition* PartitionList::operator[](size_t index) {
    return parts[index];
}

Partition* PartitionList::operator[](const String& rval) {
    for (const auto& part : parts) {
        if (strcmp(rval.c_str(), part->getLabel()) == 0) return part;
    }

    // not found
    return NULL;
}
// Iterator and size
const std::vector<Partition*>::iterator PartitionList::begin() {
    return parts.begin();
};

const std::vector<Partition*>::iterator PartitionList::end() {
    return parts.end();
};

size_t PartitionList::size() {
    return parts.size();
}

PartitionList partitions;
} // namespace lilka

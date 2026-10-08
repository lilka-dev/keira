#pragma once
#include <Arduino.h>
#include <esp_partition.h>
#include <vector>
#ifndef LILKA_PART_CHUNK_SIZE
#    define LILKA_PART_CHUNK_SIZE 512
#endif
namespace lilka {

class Partition;
// Callback definition called on each chunk flash/backup done
typedef bool (*onPartitionChunkClbk)(void* ctx, Partition* part, const String& filename, size_t offset, long fSize);

// Callback and data casts
#define LILKA_PARTITIONS_ON_CHUNK_CLBK_CAST(CLBK)            reinterpret_cast<lilka::onPartitionChunkClbk>(CLBK)
#define LILKA_PARTITIONS_ON_CHUNK__CLBK_DATA_CAST(CLBK_DATA) reinterpret_cast<void*>(CLBK_DATA)

// TODO: Documentation on Partitions

class Partition {
public:
    explicit Partition(const esp_partition_t* partition);
    // Operations:
    bool flash(const String& filename, onPartitionChunkClbk chunkClbk, void* clbkData);
    bool backup(const String& filename, onPartitionChunkClbk chunkClbk, void* clbkData);
    // returns true if partition represents one which we just boot from
    bool isRunning();
    // Accessors:
    esp_flash_t* getFlashChip();
    esp_partition_type_t getType();
    esp_partition_subtype_t getSubtype();
    uint32_t getAddress();
    uint32_t getSize();
    const char* getLabel();
    bool getEncrypted();

private:
    // flash_chip
    // type
    // subtype
    // address
    // size
    // label
    // encrypted
    const esp_partition_t* partition;
};

class PartitionList {
public:
    PartitionList();
    ~PartitionList();

    // Acesses partition by it's index
    Partition* operator[](size_t index);

    // Accesses partition by it's label
    Partition* operator[](const String& rval);

    // Iterator and size
    const std::vector<Partition*>::iterator begin();
    const std::vector<Partition*>::iterator end();
    size_t size();

private:
    std::vector<Partition*> parts;
};

extern PartitionList partitions;
} // namespace lilka

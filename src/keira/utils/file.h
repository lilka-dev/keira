#pragma once
#include <lilka/fileutils.h>
#include <sys/types.h>
#include <dirent.h>
#include <stdio.h>
#include <esp_vfs.h>
#include <soc/soc_caps.h>
#define MKPATH_MODE 0777

// List of helpers to be used with files

// Returns true if file exists
bool fexist(const char* path);

// Returns file size in bytes
long fsize(FILE* fd);

// Returns length of directory
// Note, this function can set errno
long lendir(DIR* dirfd);

// Creates a path
int mkpath(const char* path, mode_t mode = MKPATH_MODE);

// Storage
char mkpath_buf[SOC_CPU_CORES_NUM][ESP_VFS_PATH_MAX + 1];

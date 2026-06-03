#pragma once
#include <lilka/fileutils.h>
#include <sys/types.h>
#include <dirent.h>
#include <stdio.h>

#define MKPATH_MODE 0777

// List of helpers to be used with files

// Returns true if file exists
bool fexist(const char* path);

// Returns file size in bytes
long fsize(FILE* fd);

// Returns count of entriens inside directory
// Note, this function can set errno
long lendir(DIR* dirfd);

// Creates a path
int mkpath(const char* path, mode_t mode = MKPATH_MODE);

// Drops everything inside path
// rm -rf
void rmpath(const char* path);

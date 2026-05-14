#pragma once
#include <dirent.h>
#include <stdio.h>
// List of helpers to be used with files

// Returns true if file exists
bool fexist(const char* path);

// Returns file size in bytes
long fsize(FILE* fd);

// Returns length of directory
// Note, this function can set errno
long lendir(DIR* dirfd);

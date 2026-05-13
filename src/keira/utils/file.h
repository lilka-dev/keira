#pragma once
// List of helpers to be used with files

// Returns true if file exists
bool fexist(const char* path);

// Returns file size in bytes, or -1 on error
ssize_t fsize(FILE *fd);

// Returns length of directory, -1 on error
// Note, this function can set errno
ssize_t lendir(DIR* dirfd);
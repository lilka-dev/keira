#pragma once
#include <Arduino.h>
// String Utils:  /////////////////////////////////////////////////////////////////////////////////////
// Packs data in a String in c-style format fmt
// Returns formated Arduino String
String StringFormat(const char* fmt, ...);
// GUIDELINE: Use StringFormat instead of sprintf()
///////////////////////////////////////////////////////////////////////////////////////////////////////

// Checks if c-string ends with another c-string
bool strstrends(const char* haystack, const char* needle);

// Checks if c-string ends with another c-string ignoring case
bool strcasestrends(const char* haystack, const char* needle);

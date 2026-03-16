#include <lilka/serial.h>
#include "debug.h"

#define LEP_FORMAT "==> %s:%d %s\n"
#define LXP_FORMAT "<== %s:%d %s\n"

void keira_log_entry_point(const char* file, uint32_t line, const char* func) {
    lilka::serial.log(LEP_FORMAT, file, line, func);
}

void keira_log_exit_point(const char* file, uint32_t line, const char* func) {
    lilka::serial.log(LXP_FORMAT, file, line, func);
}

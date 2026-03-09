# Makefile for building Keira DynApp .so files
#
# Prerequisites:
#   - xtensa-esp32s3-elf-gcc toolchain in PATH, or set TOOLCHAIN_PATH below.
#   - PlatformIO users: the toolchain is at
#       ~/.platformio/packages/toolchain-xtensa-esp32s3/bin/
#
# Usage:
#   make                  # Build demo.so
#   make clean            # Remove build artifacts
#   make TOOLCHAIN_PATH=~/.platformio/packages/toolchain-xtensa-esp32s3/bin  # Custom toolchain path

# ── Toolchain ────────────────────────────────────────────────────────────────
# Set TOOLCHAIN_PATH to the directory containing xtensa-esp32s3-elf-gcc,
# or leave empty if it's already in PATH.
TOOLCHAIN_PATH ?= $(HOME)/.platformio/packages/toolchain-xtensa-esp32s3/bin

ifneq ($(TOOLCHAIN_PATH),)
  CROSS := $(TOOLCHAIN_PATH)/xtensa-esp32s3-elf-
else
  CROSS := xtensa-esp32s3-elf-
endif

CC      := $(CROSS)gcc
LD      := $(CROSS)gcc
OBJDUMP := $(CROSS)objdump
SIZE    := $(CROSS)size

# ── Project ──────────────────────────────────────────────────────────────────
TARGET  := demo.so
SRCS    := main.c
OBJS    := $(SRCS:.c=.o)

# ── Flags ────────────────────────────────────────────────────────────────────
CFLAGS := \
	-c \
	-fPIC \
	-Os \
	-Wall \
	-Wextra \
	-nostdlib \
	-fdata-sections \
	-ffunction-sections \
	-fvisibility=hidden \
	-I.

LDFLAGS := \
	-shared \
	-fPIC \
	-nostdlib \
	-nostartfiles \
	-static-libgcc \
	-e app_main \
	-fdata-sections \
	-ffunction-sections \
	-Wl,--gc-sections \
	-fvisibility=hidden \
	-Wl,--strip-all \
	-Wl,--strip-debug \
	-Wl,--allow-shlib-undefined

# ── Targets ──────────────────────────────────────────────────────────────────
.PHONY: all clean info

all: $(TARGET)
	@echo "✓ Built $(TARGET) ($$($(SIZE) -A $(TARGET) | grep Total | awk '{print $$2}') bytes)"

$(TARGET): $(OBJS)
	$(LD) $(LDFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm -f $(OBJS) $(TARGET)

info: $(TARGET)
	$(OBJDUMP) -h $(TARGET)
	@echo "---"
	$(OBJDUMP) -t $(TARGET) | grep -E "app_main|keira_"
	@echo "---"
	$(SIZE) -A $(TARGET)

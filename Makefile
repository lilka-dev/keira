BOARD ?= lilka_v2
# Default board builds in build/, same as plain idf.py; other boards get build-<board>/
BUILD_DIR ?= $(if $(filter lilka_v2,$(BOARD)),build,build-$(BOARD))
# Host and container paths differ, so Docker builds can't share a build dir with local ones
DOCKER_BUILD_DIR ?= $(BUILD_DIR)-docker
PORT ?=
# Extra compile flags for one build, e.g. make FLAGS="-DLANG_EN -DFMANAGER_DEBUG"
FLAGS ?=

# ESP-IDF version pinned by the board (or the board it includes), also used as the Docker image tag
BOARD_BASE := $(shell sed -n 's|.*include.*/\.\./\([^/]*\)/board\.cmake.*|\1|p' boards/$(BOARD)/board.cmake)
IDF_VERSION := $(shell sed -n 's/.*KEIRA_IDF_VERSION "\(.*\)".*/\1/p' \
	boards/$(BOARD)/board.cmake $(if $(BOARD_BASE),boards/$(BOARD_BASE)/board.cmake) | head -1)
IDF_IMAGE ?= espressif/idf:v$(IDF_VERSION)
DOCKER ?= docker

IDF_PY = idf.py -B $(1) -DKEIRA_BOARD=$(BOARD) -DKEIRA_BUILD_FLAGS="$(subst $() ,;,$(strip $(FLAGS)))" $(if $(PORT),-p $(PORT))

CPPCHECK ?= cppcheck
CLANG_FORMAT ?= $(shell command -v clang-format-20 2>/dev/null || echo clang-format)

# Not our code, or generated
LINT_EXCLUDE = .ccls-cache build components doomgeneric bak LodePNG

help: ## Show this help
	@grep -E '^[a-zA-Z_-]+:.*?## .*$$' $(MAKEFILE_LIST) | awk 'BEGIN {FS = ":.*?## "}; {printf "\033[36m%-16s\033[0m %s\n", $$1, $$2}'

.PHONY: submodules
submodules: ## Fetch pinned library submodules
	git submodule update --init --depth 1

.PHONY: all build
all: build
build: ## Build firmware (needs ESP-IDF in the environment)
	$(call IDF_PY,$(BUILD_DIR)) build

.PHONY: docker-build
docker-build: ## Build firmware in the pinned ESP-IDF Docker image
	$(DOCKER) run --rm -u $$(id -u):$$(id -g) -e HOME=/tmp -v "$(CURDIR)":/project -w /project $(IDF_IMAGE) \
		$(call IDF_PY,$(DOCKER_BUILD_DIR)) build

.PHONY: docker-flash
docker-flash: ## Flash the docker-build output (needs only esptool.py locally, one that has --after watchdog_reset)
	@# While USB Drive is open, the port is Keira's TinyUSB CDC: the reset drops the board into download mode
	@# on USB-Serial/JTAG and the port re-enumerates under the same name, which esptool can't follow.
	@# So kick it first, wait for the port, then flash. Harmless otherwise
	-@esptool.py --chip esp32s3 $(if $(PORT),-p $(PORT)) --before default_reset --after no_reset \
		--connect-attempts 1 read_mac >/dev/null 2>&1; \
		sleep 1; for i in $$(seq 20); do [ -z "$(PORT)" ] || [ -e "$(PORT)" ] && break; sleep 0.25; done
	@# A hard reset (RTS) through USB-Serial/JTAG leaves the chip in download mode, the watchdog reset boots the app
	cd $(DOCKER_BUILD_DIR) && esptool.py --chip esp32s3 $(if $(PORT),-p $(PORT)) -b 460800 \
		--before default_reset --after watchdog_reset write_flash @flash_args

.PHONY: docker-monitor
docker-monitor: ## Serial monitor for the docker-build output (no local CMake/build needed), PORT is required
	python $(IDF_PATH)/tools/idf_monitor.py --port $(PORT) --baud 115200 \
		--toolchain-prefix xtensa-esp32s3-elf- --target esp32s3 $(DOCKER_BUILD_DIR)/keira.elf

.PHONY: flash
flash: ## Flash bootloader, partition table and firmware
	$(call IDF_PY,$(BUILD_DIR)) flash

.PHONY: flash-fs
flash-fs: ## Flash the SPIFFS image built from data/spiffs/ (overwrites files on the device)
	$(call IDF_PY,$(BUILD_DIR)) spiffs-flash

.PHONY: monitor
monitor: ## Serial monitor with backtrace decoding
	$(call IDF_PY,$(BUILD_DIR)) monitor

.PHONY: menuconfig
menuconfig: ## Edit sdkconfig for this build dir (persist changes in boards/$(BOARD)/sdkconfig.defaults)
	$(call IDF_PY,$(BUILD_DIR)) menuconfig

.PHONY: clean fullclean
clean: ## Remove build outputs
	$(call IDF_PY,$(BUILD_DIR)) clean
fullclean: ## Remove the whole build dir, including sdkconfig
	rm -rf $(BUILD_DIR)

.PHONY: compile_commands
compile_commands: ## Link compile_commands.json from the build dir
	ln -sf $(BUILD_DIR)/compile_commands.json compile_commands.json

.PHONY: decode_backtrace
decode_backtrace: ## Decode a backtrace: make decode_backtrace BT="0x4200...:0x3fc9... ..."
	xtensa-esp32s3-elf-addr2line -pfiaC -e $(BUILD_DIR)/keira.elf $(BT)

# QEMU (Espressif fork with ESP32-S3 support): https://github.com/espressif/qemu/releases
QEMU_RELEASE ?= esp-develop-9.2.2-20260417
QEMU_DIR ?= build-qemu
QEMU_HOST := $(subst arm64,aarch64,$(shell uname -m))-$(if $(filter Darwin,$(shell uname -s)),apple-darwin,linux-gnu)
QEMU_ARCHIVE := qemu-xtensa-softmmu-$(subst -,_,$(QEMU_RELEASE))-$(QEMU_HOST).tar.xz
# The one from make qemu-install, if there is one, else from PATH
QEMU ?= $(if $(wildcard $(QEMU_DIR)/qemu/bin/qemu-system-xtensa),$(QEMU_DIR)/qemu/bin/qemu-system-xtensa,qemu-system-xtensa)
QEMU_BOARD ?= lilka_v2_qemu
# Set NO_DOCKER=1 when already inside an ESP-IDF environment (e.g. the espressif/idf CI container)
NO_DOCKER ?=
QEMU_BUILD_DIR ?= build-$(QEMU_BOARD)$(if $(NO_DOCKER),,-docker)
SPIFFS_OFFSET := $(shell sed -n 's/^spiffs, *data, *spiffs, *\([^,]*\),.*/\1/p' boards/lilka_v2/partitions.csv)
# Runs a command in the QEMU build dir, inside the pinned ESP-IDF image unless NO_DOCKER is set
IN_QEMU_BUILD_DIR = $(if $(NO_DOCKER),cd $(QEMU_BUILD_DIR) &&,$(DOCKER) run --rm -u $$(id -u):$$(id -g) -e HOME=/tmp \
	-v "$(CURDIR)":/project -w /project/$(QEMU_BUILD_DIR) $(IDF_IMAGE))

.PHONY: qemu-install qemu-build qemu-image qemu-run
qemu-install: ## Download Espressif QEMU to build-qemu/ (macOS: brew install libgcrypt glib pixman sdl2 libslirp)
	rm -rf $(QEMU_DIR) && mkdir -p $(QEMU_DIR)
	curl -fsSL "https://github.com/espressif/qemu/releases/download/$(QEMU_RELEASE)/$(QEMU_ARCHIVE)" | tar -xJ -C $(QEMU_DIR)
	$(QEMU_DIR)/qemu/bin/qemu-system-xtensa --version

qemu-build: ## Build the QEMU firmware (board lilka_v2_qemu), in Docker unless NO_DOCKER=1
	$(MAKE) $(if $(NO_DOCKER),build,docker-build) BOARD=$(QEMU_BOARD)

qemu-image: ## Merge the QEMU firmware and data/spiffs/ into one 16 MB flash image
	$(IN_QEMU_BUILD_DIR) esptool.py --chip esp32s3 merge_bin --fill-flash-size 16MB -o flash_image.bin \
		@flash_args $(SPIFFS_OFFSET) spiffs.bin
	@echo "Flash image: $(QEMU_BUILD_DIR)/flash_image.bin"

qemu-run: ## Run Keira in QEMU, commands from stdin or SCRIPT=file
	python3 tools/emu/keira_emu.py --qemu $(QEMU) --image $(QEMU_BUILD_DIR)/flash_image.bin $(SCRIPT)

.PHONY: qemu-test
qemu-test: ## Run the emulator tests in tests/emu (needs pytest)
	KEIRA_QEMU_IMAGE=$(QEMU_BUILD_DIR)/flash_image.bin QEMU=$(QEMU) python3 -m pytest -v tests/emu

.PHONY: clang-format
clang-format: ## Run clang-format check
	find . \
		\( $(foreach d,$(LINT_EXCLUDE),-name $(notdir $(d)) -o) -false \) -prune \
		-o \( -iname '*.h' -o -iname '*.hpp' -o -iname '*.c' -o -iname '*.cpp' \) -print \
		| xargs $(CLANG_FORMAT) --dry-run --Werror

.PHONY: cppcheck
cppcheck: ## Run cppcheck check
	$(CPPCHECK) . $(foreach d,$(LINT_EXCLUDE),-i$(d)) \
		--enable=performance,style \
		--suppress=knownPointerToBool \
		--suppress=noCopyConstructor \
		--suppress=noOperatorEq \
		--suppress=useStlAlgorithm \
		--inline-suppr \
		--error-exitcode=1

.PHONY: checklang
checklang: ## Run localization files check
	python tools/checklang.py

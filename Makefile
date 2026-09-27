BOARD ?= lilka_v2
# Default board builds in build/, same as plain idf.py; other boards get build-<board>/
BUILD_DIR ?= $(if $(filter lilka_v2,$(BOARD)),build,build-$(BOARD))
# Host and container paths differ, so Docker builds can't share a build dir with local ones
DOCKER_BUILD_DIR ?= $(BUILD_DIR)-docker
PORT ?=
# Extra compile flags for one build, e.g. make FLAGS="-DLANG_EN -DFMANAGER_DEBUG"
FLAGS ?=

# ESP-IDF version pinned by the board, also used as the Docker image tag
IDF_VERSION := $(shell sed -n 's/.*KEIRA_IDF_VERSION "\(.*\)".*/\1/p' boards/$(BOARD)/board.cmake)
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
docker-flash: ## Flash the docker-build output (needs only esptool.py locally)
	cd $(DOCKER_BUILD_DIR) && esptool.py --chip esp32s3 $(if $(PORT),-p $(PORT)) -b 460800 \
		--before default_reset --after hard_reset write_flash @flash_args

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

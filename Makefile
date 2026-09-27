BOARD ?= lilka_v2
BUILD_DIR ?= build/$(BOARD)
PORT ?=
# Extra compile flags for one build, e.g. make FLAGS="-DLANG_EN -DFMANAGER_DEBUG"
FLAGS ?=

# ESP-IDF version pinned by the board, also used as the Docker image tag
IDF_VERSION := $(shell sed -n 's/.*KEIRA_IDF_VERSION "\(.*\)".*/\1/p' boards/$(BOARD)/board.cmake)
IDF_IMAGE ?= espressif/idf:v$(IDF_VERSION)
DOCKER ?= docker

IDF_PY = idf.py -B $(BUILD_DIR) -DKEIRA_BOARD=$(BOARD) -DKEIRA_BUILD_FLAGS="$(subst $() ,;,$(strip $(FLAGS)))" $(if $(PORT),-p $(PORT))

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
	$(IDF_PY) build

.PHONY: docker-build
docker-build: ## Build firmware in the pinned ESP-IDF Docker image
	$(DOCKER) run --rm -u $$(id -u):$$(id -g) -e HOME=/tmp -v "$(CURDIR)":/project -w /project $(IDF_IMAGE) \
		$(IDF_PY) build

.PHONY: flash
flash: ## Flash bootloader, partition table and firmware
	$(IDF_PY) flash

.PHONY: flash-fs
flash-fs: ## Flash the SPIFFS image built from data/spiffs/ (overwrites files on the device)
	$(IDF_PY) spiffs-flash

.PHONY: monitor
monitor: ## Serial monitor with backtrace decoding
	$(IDF_PY) monitor

.PHONY: menuconfig
menuconfig: ## Edit sdkconfig for this build dir (persist changes in boards/$(BOARD)/sdkconfig.defaults)
	$(IDF_PY) menuconfig

.PHONY: clean fullclean
clean: ## Remove build outputs
	$(IDF_PY) clean
fullclean: ## Remove the whole build dir, including sdkconfig
	$(IDF_PY) fullclean

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

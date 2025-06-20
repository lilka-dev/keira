ENV ?= v2

CPPCHECK ?= cppcheck
CLANG_FORMAT ?= $(shell command -v clang-format-17 2>/dev/null || echo clang-format)

help: ## Show this help
	@grep -E '^[a-zA-Z_-]+:.*?## .*$$' $(MAKEFILE_LIST) | awk 'BEGIN {FS = ":.*?## "}; {printf "\033[36m%-16s\033[0m %s\n", $$1, $$2}'

all:
	pio -f -c vim run -e $(ENV)

upload-usb:
	pio -f -c vim run --target upload -e $(ENV)

clean:
	pio -f -c vim run --target clean -e $(ENV)

program:
	pio -f -c vim run --target program -e $(ENV)

update:
	pio -f -c vim update

compile_commands:
	pio run -t compiledb -e $(ENV)

uploadfs:
	pio run -t uploadfs -e $(ENV)

.PHONY: clang-format
clang-format: ## Run clang-format check
	find \
		. \
		-not \( -name .ccls-cache -prune \) \
		-not \( -name .pio -prune \) \
		-not \( -name doomgeneric -prune \) \
		-not \( -name bak -prune \) \
		-not \( -name mJS -prune \) \
		-not \( -name SimpleFTPServer -prune \) \
		-not \( -name LodePNG -prune \) \
		-iname *.h \
		-o -iname *.cpp \
		-o -iname *.c \
		-o -iname *.hpp \
		-o -iname *.h \
		| xargs $(CLANG_FORMAT) --dry-run --Werror

.PHONY: cppcheck
cppcheck: ## Run cppcheck check
	$(CPPCHECK) . -i.ccls-cache -ipio -idoomgeneric -ibak -imJS -iSimpleFTPServer -iLodePNG \
		--enable=performance,style \
		--suppress=knownPointerToBool \
		--suppress=noCopyConstructor \
		--suppress=noOperatorEq \
		--inline-suppr \
		--error-exitcode=1
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
	$(CPPCHECK) . -i.ccls-cache -i.pio -idoomgeneric -ibak -imJS -iSimpleFTPServer -iLodePNG \
		--enable=performance,style \
		--suppress=knownPointerToBool \
		--suppress=noCopyConstructor \
		--suppress=noOperatorEq \
		--inline-suppr \
		--error-exitcode=1

.PHONY: unused-macros
unused-macros: ## Find unused #define macros
	@echo "Collecting source files..."
	@files=$$(find . \
		-not \( -name .ccls-cache -prune \) \
		-not \( -name .pio -prune \) \
		-not \( -name doomgeneric -prune \) \
		-not \( -name bak -prune \) \
		-not \( -name mJS -prune \) \
		-not \( -name SimpleFTPServer -prune \) \
		-not \( -name LodePNG -prune \) \
		\( -iname "*.h" -o -iname "*.cpp" -o -iname "*.c" -o -iname "*.hpp" \) 2>/dev/null); \
	echo "Extracting macros..."; \
	macros=$$(echo "$$files" | xargs grep -h "^[[:space:]]*#define[[:space:]]\+[A-Za-z_][A-Za-z0-9_]*" 2>/dev/null \
		| sed -n 's/^[[:space:]]*#define[[:space:]]\+\([A-Za-z_][A-Za-z0-9_]*\).*/\1/p' \
		| sort -u); \
	total=$$(echo "$$macros" | wc -l); \
	echo "Found $$total macros. Building search pattern..."; \
	pattern=$$(echo "$$macros" | tr '\n' '|' | sed 's/|$$//'); \
	echo "Counting all occurrences (single pass)..."; \
	counts=$$(echo "$$files" | xargs grep -ohE "\b($$pattern)\b" 2>/dev/null | sort | uniq -c | sort -rn); \
	echo "Analyzing results..."; \
	echo "$$macros" | while read macro; do \
		cnt=$$(echo "$$counts" | grep -E "^[[:space:]]*[0-9]+[[:space:]]+$$macro$$" | awk '{print $$1}'); \
		if [ -z "$$cnt" ] || [ "$$cnt" -le 1 ]; then \
			printf "\033[33mPotentially unused: %s\033[0m\n" "$$macro"; \
		fi; \
	done; \
	echo "Done!"
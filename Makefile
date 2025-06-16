ENV ?= v2

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

KICAD_CLI = kicad-cli
BUILDER = dirhtml

.PHONY: all
all: clean doxygen
	sphinx-build -M $(BUILDER) . _build

.PHONY: doxygen
doxygen:
	cd _doxygen && doxygen Doxyfile

.PHONY: autobuild
autobuild: clean doxygen
	@echo "Live-reload the documentation"
ifeq ($(OS),Windows_NT)
	sphinx-autobuild -b $(BUILDER) . _build\html --ignore "*\\_doxygen\\*"
else
	sphinx-autobuild -b $(BUILDER) . _build/html --ignore '**/_doxygen/**'
endif

.PHONY: clean
clean:
ifeq ($(OS),Windows_NT)
	@if exist "_build" rd /s /q "_build"
	@if exist "_doxygen\\xml" rd /s /q "_doxygen\\xml"
else
	rm -rf _build _doxygen/xml
endif

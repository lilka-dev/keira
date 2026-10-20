/**
 * @file dynloader.h
 * @brief Dynamic ELF Loader for Lilka (ESP32-S3)
 *
 * Loads .so (ELF shared object) files from the SD card into PSRAM
 * and executes them as dynamically linked apps within KeiraOS.
 *
 * Based on espressif/elf_loader (Apache-2.0)
 * https://components.espressif.com/components/espressif/elf_loader
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef LILKA_DYNLOADER_H
#define LILKA_DYNLOADER_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ── ELF types (32-bit Xtensa) ──────────────────────────────────────────── */

#define EI_NIDENT 16

typedef unsigned int Elf32_Addr;
typedef unsigned int Elf32_Off;
typedef unsigned int Elf32_Word;
typedef unsigned short Elf32_Half;
typedef int Elf32_Sword;

typedef struct {
    unsigned char ident[EI_NIDENT];
    Elf32_Half type;
    Elf32_Half machine;
    Elf32_Word version;
    Elf32_Addr entry;
    Elf32_Off phoff;
    Elf32_Off shoff;
    Elf32_Word flags;
    Elf32_Half ehsize;
    Elf32_Half phentsize;
    Elf32_Half phnum;
    Elf32_Half shentsize;
    Elf32_Half shnum;
    Elf32_Half shstrndx;
} lilka_elf32_hdr_t;

typedef struct {
    Elf32_Word name;
    Elf32_Word type;
    Elf32_Word flags;
    Elf32_Addr addr;
    Elf32_Off offset;
    Elf32_Word size;
    Elf32_Word link;
    Elf32_Word info;
    Elf32_Word addralign;
    Elf32_Word entsize;
} lilka_elf32_shdr_t;

typedef struct {
    Elf32_Word name;
    Elf32_Addr value;
    Elf32_Word size;
    unsigned char info;
    unsigned char other;
    Elf32_Half shndx;
} lilka_elf32_sym_t;

typedef struct {
    Elf32_Addr offset;
    Elf32_Word info;
    Elf32_Sword addend;
} lilka_elf32_rela_t;

/* ── Section indices ─────────────────────────────────────────────────────── */

#define LILKA_ELF_SEC_TEXT   0
#define LILKA_ELF_SEC_BSS    1
#define LILKA_ELF_SEC_DATA   2
#define LILKA_ELF_SEC_RODATA 3
#define LILKA_ELF_SEC_DRLRO  4
#define LILKA_ELF_SECS       5

/* ── ELF constants ───────────────────────────────────────────────────────── */

/* Section types */
#define LILKA_SHT_NULL     0
#define LILKA_SHT_PROGBITS 1
#define LILKA_SHT_SYMTAB   2
#define LILKA_SHT_STRTAB   3
#define LILKA_SHT_RELA     4
#define LILKA_SHT_NOBITS   8
#define LILKA_SHT_DYNSYM   11

/* Section flags */
#define LILKA_SHF_WRITE     1
#define LILKA_SHF_ALLOC     2
#define LILKA_SHF_EXECINSTR 4

/* Symbol binding / type */
#define LILKA_STB_GLOBAL     1
#define LILKA_STT_NOTYPE     0
#define LILKA_STT_OBJECT     1
#define LILKA_STT_FUNC       2
#define LILKA_STT_SECTION    3
#define LILKA_STT_FILE       4
#define LILKA_STT_COMMON     5

#define LILKA_SHN_UNDEF      0

#define LILKA_ELF_ST_BIND(i) ((i) >> 4)
#define LILKA_ELF_ST_TYPE(i) ((i) & 0xf)
#define LILKA_ELF_R_SYM(i)   ((i) >> 8)
#define LILKA_ELF_R_TYPE(i)  ((unsigned char)(i))

/* Xtensa relocation types */
#define R_XTENSA_NONE     0
#define R_XTENSA_32       1
#define R_XTENSA_RTLD     2
#define R_XTENSA_GLOB_DAT 3
#define R_XTENSA_JMP_SLOT 4
#define R_XTENSA_RELATIVE 5

/* Alignment macro */
#define LILKA_ELF_ALIGN(a, s) (((a) + ((s)-1)) & ~((s)-1))

/* ── ELF section descriptor ─────────────────────────────────────────────── */

typedef struct {
    uintptr_t v_addr; /* virtual address in the ELF */
    uint32_t offset; /* offset in the ELF file */
    uintptr_t addr; /* physical address in memory */
    uint32_t size; /* section size */
} lilka_elf_sec_t;

/* ── Symbol export entry ─────────────────────────────────────────────────── */

typedef struct {
    const char* name;
    void* addr;
} lilka_dynsym_t;

/* Helper macros */
#define LILKA_DYNSYM_EXPORT(sym) \
    { #sym, (void*)&sym }
#define LILKA_DYNSYM_END \
    { NULL, NULL }

/* ── DynLoader state ─────────────────────────────────────────────────────── */

typedef struct {
    /* Memory pointers */
    unsigned char* ptext; /* text (code) buffer in PSRAM (data-bus addr) */
    unsigned char* pdata; /* data+rodata+bss buffer */

    /* Section descriptors */
    lilka_elf_sec_t sec[LILKA_ELF_SECS];

    /* Entry point (instruction-bus address) */
    int (*entry)(int argc, char* argv[]);

    /* Exported symbols from the loaded .so */
    uint16_t sym_count;
    lilka_dynsym_t* symtab;
} lilka_elf_t;

/* ── Public API ──────────────────────────────────────────────────────────── */

/**
 * @brief Maximum number of registered symbol tables.
 */
#define LILKA_DYNSYM_MAX_TABLES 32

/**
 * @brief Register a symbol table array for resolution during ELF loading.
 *
 * @param table  Array of lilka_dynsym_t terminated with LILKA_DYNSYM_END.
 * @return 0 on success, negative errno on failure.
 */
int lilka_dynloader_register_symbols(const lilka_dynsym_t* table);

/**
 * @brief Unregister a previously registered symbol table.
 *
 * @param table  Pointer passed to lilka_dynloader_register_symbols().
 * @return 0 on success, negative errno on failure.
 */
int lilka_dynloader_unregister_symbols(const lilka_dynsym_t* table);

/**
 * @brief Resolve a symbol name in all registered tables.
 *
 * @param name  Symbol name to look up.
 * @return Address of the symbol, or 0 if not found.
 */
uintptr_t lilka_dynloader_find_symbol(const char* name);

/**
 * @brief Initialize an ELF loader context.
 *
 * @param elf  Pointer to lilka_elf_t structure.
 * @return 0 on success, negative errno on failure.
 */
int lilka_elf_init(lilka_elf_t* elf);

/**
 * @brief Load and relocate an ELF from a memory buffer.
 *
 * @param elf   Initialized lilka_elf_t structure.
 * @param pbuf  Pointer to the raw ELF file data.
 * @param size  Size of the ELF data.
 * @return 0 on success, negative errno on failure.
 */
int lilka_elf_relocate(lilka_elf_t* elf, const uint8_t* pbuf, size_t size);

/**
 * @brief Execute the loaded ELF's entry point.
 *
 * @param elf   Relocated ELF context.
 * @param argc  Argument count.
 * @param argv  Argument vector.
 * @return Return value from the ELF's main/app_main.
 */
int lilka_elf_run(lilka_elf_t* elf, int argc, char* argv[]);

/**
 * @brief Deinitialize the ELF loader and free all resources.
 *
 * @param elf  ELF context to clean up.
 */
void lilka_elf_deinit(lilka_elf_t* elf);

/**
 * @brief High-level: load a .so file from a filesystem path, relocate,
 *        execute with given arguments, and clean up.
 *
 * @param path  Full filesystem path (e.g., "/sd/apps/demo.so").
 * @param argc  Argument count for the app.
 * @param argv  Argument values for the app.
 * @return 0 on success, negative errno on failure.
 */
int lilka_dynloader_run(const char* path, int argc, char* argv[]);

#ifdef __cplusplus
} /* extern "C" */
#endif

/* ── C++ wrapper ─────────────────────────────────────────────────────────── */

#ifdef __cplusplus
namespace lilka {

/// High-level C++ wrapper around the ELF dynamic loader.
class DynLoader {
public:
    DynLoader() : loaded(false), errorMsg(nullptr) {
        lilka_elf_init(&elf);
    }

    ~DynLoader() {
        unload();
    }

    /// Register a symbol table for resolution.
    static int registerSymbols(const lilka_dynsym_t* table) {
        return lilka_dynloader_register_symbols(table);
    }

    /// Unregister a symbol table.
    static int unregisterSymbols(const lilka_dynsym_t* table) {
        return lilka_dynloader_unregister_symbols(table);
    }

    /// Load a .so file from the given filesystem path.
    /// @return 0 on success, negative on error.
    int load(const char* path);

    /// Execute the loaded app's entry point.
    /// @return Return value from the app.
    int execute(int argc, char* argv[]);

    /// Unload the app and free all resources.
    void unload();

    /// Get the last error message (or nullptr).
    const char* getError() const {
        return errorMsg;
    }

    /// Check if a .so is currently loaded.
    bool isLoaded() const {
        return loaded;
    }

private:
    lilka_elf_t elf;
    uint8_t* fileData = nullptr;
    size_t fileSize = 0;
    bool loaded;
    const char* errorMsg;
};

} // namespace lilka
#endif

#endif // LILKA_DYNLOADER_H

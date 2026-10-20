/**
 * @file dynloader.cpp
 * @brief Dynamic ELF Loader implementation for Lilka (ESP32-S3)
 *
 * Adapted from espressif/elf_loader v1.3.1 (Apache-2.0)
 * https://components.espressif.com/components/espressif/elf_loader
 *
 * This implementation is specifically tailored for ESP32-S3 with PSRAM,
 * using bus-address mirroring to execute code from PSRAM via the
 * instruction bus.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "dynloader.h"
#include "serial.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/errno.h>
#include <fcntl.h>
#include <unistd.h>

#include "esp_heap_caps.h"
#include "esp_idf_version.h"
#include "soc/soc.h"

/* ── ESP32-S3 PSRAM bus-address offset ──────────────────────────────────── */

/*
 * On ESP32-S3, PSRAM is mapped to both the data bus and instruction bus.
 * Data bus:        SOC_DROM_LOW  (0x3C000000)
 * Instruction bus: SOC_IROM_LOW  (0x42000000)
 *
 * To execute code allocated via the data bus, we add this offset to
 * remap the address to the instruction bus.
 */
#if defined(CONFIG_IDF_TARGET_ESP32S3) || defined(ESP32S3) || (LILKA_VERSION == 2)
#    define LILKA_TEXT_OFFSET (SOC_IROM_LOW - SOC_DROM_LOW)
#else
#    define LILKA_TEXT_OFFSET 0
#endif

/* ── Forward declarations ───────────────────────────────────────────────── */

extern "C" {

/* Cache writeback - needed after loading code into PSRAM */
#if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(5, 0, 0)
extern void Cache_WriteBack_All(void);
#else
extern void esp_spiram_writeback_cache(void);
#endif
extern void spi_flash_disable_interrupts_caches_and_other_cpu(void);
extern void spi_flash_enable_interrupts_caches_and_other_cpu(void);

} // extern "C"

/* ── Global symbol tables ───────────────────────────────────────────────── */

static const lilka_dynsym_t* g_sym_tables[LILKA_DYNSYM_MAX_TABLES] = {};

int lilka_dynloader_register_symbols(const lilka_dynsym_t* table) {
    if (!table) return -EINVAL;
    for (int i = 0; i < LILKA_DYNSYM_MAX_TABLES; i++) {
        if (g_sym_tables[i] == table) return -EEXIST;
        if (g_sym_tables[i] == nullptr) {
            g_sym_tables[i] = table;
            return 0;
        }
    }
    return -ENOMEM;
}

int lilka_dynloader_unregister_symbols(const lilka_dynsym_t* table) {
    if (!table) return -EINVAL;
    for (int i = 0; i < LILKA_DYNSYM_MAX_TABLES; i++) {
        if (g_sym_tables[i] == table) {
            g_sym_tables[i] = nullptr;
            return 0;
        }
    }
    return -EINVAL;
}

uintptr_t lilka_dynloader_find_symbol(const char* name) {
    if (!name) return 0;
    for (int i = 0; i < LILKA_DYNSYM_MAX_TABLES; i++) {
        if (!g_sym_tables[i]) continue;
        const lilka_dynsym_t* sym = g_sym_tables[i];
        while (sym->name) {
            if (strcmp(sym->name, name) == 0) {
                return (uintptr_t)sym->addr;
            }
            sym++;
        }
    }
    return 0;
}

/* ── Memory allocation ──────────────────────────────────────────────────── */

static void* elf_malloc(uint32_t n, bool exec) {
    if (exec) {
        /*
         * Allocate executable code in D/IRAM (internal RAM).
         * D/IRAM supports both instruction fetch AND data load/store
         * at the same address, which is required for Xtensa L32R
         * instructions that load constants from literal pools
         * embedded in .text.
         *
         * PSRAM I-bus (0x42000000+) only supports instruction fetch,
         * so L32R from PSRAM causes LoadStoreError.
         */
        void* p = heap_caps_malloc(n, MALLOC_CAP_EXEC);
        if (p) return p;
        lilka::serial.log("dynloader [WARN] IRAM alloc failed (%u bytes), falling back to PSRAM", n);
    }
    /* Data sections go to PSRAM */
    return heap_caps_malloc(n, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
}

static void elf_free(void* ptr) {
    heap_caps_free(ptr);
}

/*
 * IRAM-safe memory operations.
 *
 * On ESP32-S3, the IRAM instruction bus (0x4037xxxx) only supports
 * 32-bit word-aligned access. Standard memcpy/memset use byte
 * load/store internally and will cause LoadStoreError on IRAM.
 *
 * These helpers read bytes from src (which may be unaligned) and
 * write 32-bit words to the IRAM destination.
 */

/// Word-aligned copy to IRAM. dst must be 4-byte aligned. src may be unaligned.
static void iram_cpy(void* dst, const void* src, size_t n) {
    volatile uint32_t* d = reinterpret_cast<volatile uint32_t*>(dst);
    const uint8_t* s = reinterpret_cast<const uint8_t*>(src);
    size_t words = n / 4;
    size_t tail = n % 4;

    for (size_t i = 0; i < words; i++) {
        uint32_t w = (uint32_t)s[0] | ((uint32_t)s[1] << 8) | ((uint32_t)s[2] << 16) | ((uint32_t)s[3] << 24);
        *d++ = w;
        s += 4;
    }
    if (tail) {
        uint32_t w = 0;
        for (size_t i = 0; i < tail; i++)
            w |= ((uint32_t)s[i]) << (i * 8);
        *d = w;
    }
}

/// Word-aligned fill for IRAM. ptr must be 4-byte aligned.
static void iram_set(void* ptr, uint8_t val, size_t n) {
    volatile uint32_t* d = reinterpret_cast<volatile uint32_t*>(ptr);
    uint32_t fill = (uint32_t)val | ((uint32_t)val << 8) | ((uint32_t)val << 16) | ((uint32_t)val << 24);
    size_t words = (n + 3) / 4;
    for (size_t i = 0; i < words; i++)
        d[i] = fill;
}

/// Check if address is in IRAM (instruction-bus only, no byte access)
static inline bool is_iram_addr(uintptr_t addr) {
    return addr >= SOC_IRAM_LOW && addr < SOC_IRAM_HIGH;
}

/* ── Address remapping ──────────────────────────────────────────────────── */

/// Remap data-bus address to instruction-bus address if it falls within .text.
/// Only applies when .text is in PSRAM data-bus range (< 0x40000000).
/// IRAM addresses (>= 0x40000000) are already on the instruction bus — no remapping needed.
static inline uintptr_t remap_text(lilka_elf_t* elf, uintptr_t sym) {
    const lilka_elf_sec_t* sec = &elf->sec[LILKA_ELF_SEC_TEXT];
    if (sym >= sec->addr && sym < (sec->addr + sec->size)) {
#if LILKA_TEXT_OFFSET != 0
        /*
         * ESP32-S3 address map:
         *   PSRAM data bus:  0x3C000000 - 0x3DFFFFFF  (needs +TEXT_OFFSET to reach I-bus)
         *   IRAM I-bus:      0x40370000 - 0x403E0000  (already executable, no remap)
         *
         * Only apply the PSRAM text offset if .text is below 0x40000000
         * (i.e. in the data-bus address space).
         */
        if (sec->addr < 0x40000000) {
            return sym + LILKA_TEXT_OFFSET;
        }
#endif
    }
    return sym;
}

/* ── Symbol mapping ─────────────────────────────────────────────────────── */

/// Map a virtual address from the ELF to the physical address in memory
static uintptr_t elf_map_sym(lilka_elf_t* elf, uintptr_t sym) {
    for (int i = 0; i < LILKA_ELF_SECS; i++) {
        if (elf->sec[i].size && sym >= elf->sec[i].v_addr && sym < (elf->sec[i].v_addr + elf->sec[i].size)) {
            return sym - elf->sec[i].v_addr + elf->sec[i].addr;
        }
    }
    return 0;
}

/* ── Xtensa relocations ────────────────────────────────────────────────── */

static int elf_arch_relocate(
    lilka_elf_t* elf, const lilka_elf32_rela_t* rela, const lilka_elf32_sym_t* sym, uint32_t addr
) {
    uint32_t* where = reinterpret_cast<uint32_t*>(elf_map_sym(elf, rela->offset));
    if (!where) {
        lilka::serial.log("dynloader: reloc where=NULL offset=0x%x", rela->offset);
        return -EINVAL;
    }

    switch (LILKA_ELF_R_TYPE(rela->info)) {
        case R_XTENSA_RELATIVE:
            *where = remap_text(elf, elf_map_sym(elf, *where));
            break;
        case R_XTENSA_RTLD:
            /* Runtime linker marker - nothing to do */
            break;
        case R_XTENSA_GLOB_DAT:
        case R_XTENSA_JMP_SLOT:
            *where = remap_text(elf, addr);
            break;
        case R_XTENSA_32:
            *where = remap_text(elf, addr + rela->addend);
            break;
        case R_XTENSA_NONE:
            break;
        default:
            lilka::serial.log("dynloader: unsupported reloc type %d", LILKA_ELF_R_TYPE(rela->info));
            return -EINVAL;
    }

    return 0;
}

/* ── Cache flush ────────────────────────────────────────────────────────── */

static void elf_flush_cache(void) {
#if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(5, 0, 0)
    Cache_WriteBack_All();
#else
    esp_spiram_writeback_cache();
#endif
    /*
     * NOTE: spi_flash_disable/enable_interrupts_caches_and_other_cpu()
     * was removed here. Those calls are intended for SPI flash write
     * protection and disable caches + interrupts on BOTH CPUs, which
     * causes a TG1WDT_SYS_RST (watchdog reset on the other core).
     *
     * Cache_WriteBack_All() alone is sufficient: it flushes dirty
     * data-cache lines to PSRAM so the instruction bus can fetch
     * the freshly loaded code. Since the .text region was just
     * allocated, there are no stale instruction-cache entries.
     */
}

/* ── Section loading (bus-address mirror mode for ESP32-S3) ─────────────── */

static int elf_load_sections(lilka_elf_t* elf, const uint8_t* pbuf) {
    const lilka_elf32_hdr_t* ehdr = reinterpret_cast<const lilka_elf32_hdr_t*>(pbuf);
    const lilka_elf32_shdr_t* shdr = reinterpret_cast<const lilka_elf32_shdr_t*>(pbuf + ehdr->shoff);
    const char* shstrtab = reinterpret_cast<const char*>(pbuf) + shdr[ehdr->shstrndx].offset;

    /* Find relevant sections */
    for (uint32_t i = 0; i < ehdr->shnum; i++) {
        const char* name = shstrtab + shdr[i].name;

        if (shdr[i].type == LILKA_SHT_PROGBITS && (shdr[i].flags & LILKA_SHF_ALLOC)) {
            if ((shdr[i].flags & LILKA_SHF_EXECINSTR) && strcmp(".text", name) == 0) {
                elf->sec[LILKA_ELF_SEC_TEXT].v_addr = shdr[i].addr;
                /*
                 * Use actual section size for VMA range checking (not aligned).
                 * Aligning to 4 bytes can extend the .text VMA range to overlap
                 * with adjacent sections (e.g. .rodata), causing elf_map_sym()
                 * to resolve .rodata addresses through .text — wrong memory type.
                 * Memory allocation is aligned separately below.
                 */
                elf->sec[LILKA_ELF_SEC_TEXT].size = shdr[i].size;
                elf->sec[LILKA_ELF_SEC_TEXT].offset = shdr[i].offset;
            } else if ((shdr[i].flags & LILKA_SHF_WRITE) && strcmp(".data", name) == 0) {
                elf->sec[LILKA_ELF_SEC_DATA].v_addr = shdr[i].addr;
                elf->sec[LILKA_ELF_SEC_DATA].size = shdr[i].size;
                elf->sec[LILKA_ELF_SEC_DATA].offset = shdr[i].offset;
            } else if (strcmp(".rodata", name) == 0) {
                elf->sec[LILKA_ELF_SEC_RODATA].v_addr = shdr[i].addr;
                elf->sec[LILKA_ELF_SEC_RODATA].size = shdr[i].size;
                elf->sec[LILKA_ELF_SEC_RODATA].offset = shdr[i].offset;
            } else if (strcmp(".data.rel.ro", name) == 0) {
                elf->sec[LILKA_ELF_SEC_DRLRO].v_addr = shdr[i].addr;
                elf->sec[LILKA_ELF_SEC_DRLRO].size = shdr[i].size;
                elf->sec[LILKA_ELF_SEC_DRLRO].offset = shdr[i].offset;
            }
        } else if (shdr[i].type == LILKA_SHT_NOBITS && (shdr[i].flags & (LILKA_SHF_ALLOC | LILKA_SHF_WRITE)) == (LILKA_SHF_ALLOC | LILKA_SHF_WRITE) && strcmp(".bss", name) == 0) {
            elf->sec[LILKA_ELF_SEC_BSS].v_addr = shdr[i].addr;
            elf->sec[LILKA_ELF_SEC_BSS].size = shdr[i].size;
            elf->sec[LILKA_ELF_SEC_BSS].offset = shdr[i].offset;
        }
    }

    /* Validate: we need at least a .text section */
    if (!elf->sec[LILKA_ELF_SEC_TEXT].size) {
        lilka::serial.log("dynloader: no .text section found");
        return -EINVAL;
    }

    /* Allocate text section — align allocation size to 4 bytes */
    uint32_t text_alloc_size = LILKA_ELF_ALIGN(elf->sec[LILKA_ELF_SEC_TEXT].size, 4);
    elf->ptext = static_cast<unsigned char*>(elf_malloc(text_alloc_size, true));
    if (!elf->ptext) {
        lilka::serial.log("dynloader: failed to alloc %u bytes for .text", text_alloc_size);
        return -ENOMEM;
    }
    bool text_in_iram = is_iram_addr((uintptr_t)elf->ptext);

    /* Zero padding bytes — use IRAM-safe write if needed */
    if (text_in_iram) {
        iram_set(elf->ptext, 0, text_alloc_size);
    } else {
        memset(elf->ptext, 0, text_alloc_size);
    }

    /* Allocate data sections */
    uint32_t data_size = elf->sec[LILKA_ELF_SEC_DATA].size + elf->sec[LILKA_ELF_SEC_RODATA].size +
                         elf->sec[LILKA_ELF_SEC_BSS].size + elf->sec[LILKA_ELF_SEC_DRLRO].size;
    if (data_size) {
        elf->pdata = static_cast<unsigned char*>(elf_malloc(data_size, false));
        if (!elf->pdata) {
            lilka::serial.log("dynloader: failed to alloc %u bytes for data", data_size);
            elf_free(elf->ptext);
            elf->ptext = nullptr;
            return -ENOMEM;
        }
    }

    /* Copy .text into executable memory — use IRAM-safe copy if needed */
    elf->sec[LILKA_ELF_SEC_TEXT].addr = (uintptr_t)elf->ptext;
    if (text_in_iram) {
        iram_cpy(elf->ptext, pbuf + elf->sec[LILKA_ELF_SEC_TEXT].offset, elf->sec[LILKA_ELF_SEC_TEXT].size);
    } else {
        memcpy(elf->ptext, pbuf + elf->sec[LILKA_ELF_SEC_TEXT].offset, elf->sec[LILKA_ELF_SEC_TEXT].size);
    }

    /* Copy data sections */
    if (data_size) {
        uint8_t* p = elf->pdata;

        if (elf->sec[LILKA_ELF_SEC_DATA].size) {
            elf->sec[LILKA_ELF_SEC_DATA].addr = (uintptr_t)p;
            memcpy(p, pbuf + elf->sec[LILKA_ELF_SEC_DATA].offset, elf->sec[LILKA_ELF_SEC_DATA].size);
            p += elf->sec[LILKA_ELF_SEC_DATA].size;
        }
        if (elf->sec[LILKA_ELF_SEC_RODATA].size) {
            elf->sec[LILKA_ELF_SEC_RODATA].addr = (uintptr_t)p;
            memcpy(p, pbuf + elf->sec[LILKA_ELF_SEC_RODATA].offset, elf->sec[LILKA_ELF_SEC_RODATA].size);
            p += elf->sec[LILKA_ELF_SEC_RODATA].size;
        }
        if (elf->sec[LILKA_ELF_SEC_DRLRO].size) {
            elf->sec[LILKA_ELF_SEC_DRLRO].addr = (uintptr_t)p;
            memcpy(p, pbuf + elf->sec[LILKA_ELF_SEC_DRLRO].offset, elf->sec[LILKA_ELF_SEC_DRLRO].size);
            p += elf->sec[LILKA_ELF_SEC_DRLRO].size;
        }
        if (elf->sec[LILKA_ELF_SEC_BSS].size) {
            elf->sec[LILKA_ELF_SEC_BSS].addr = (uintptr_t)p;
            memset(p, 0, elf->sec[LILKA_ELF_SEC_BSS].size);
        }
    }

    /* Set entry point (remap to instruction bus for execution) */
    uintptr_t entry_addr = ehdr->entry + elf->sec[LILKA_ELF_SEC_TEXT].addr - elf->sec[LILKA_ELF_SEC_TEXT].v_addr;
    elf->entry = reinterpret_cast<int (*)(int, char*[])>(remap_text(elf, entry_addr));

    return 0;
}

/* ── Core API implementation ────────────────────────────────────────────── */

int lilka_elf_init(lilka_elf_t* elf) {
    if (!elf) return -EINVAL;
    memset(elf, 0, sizeof(lilka_elf_t));
    return 0;
}

int lilka_elf_relocate(lilka_elf_t* elf, const uint8_t* pbuf, size_t size) {
    if (!elf || !pbuf || size < sizeof(lilka_elf32_hdr_t)) return -EINVAL;

    const lilka_elf32_hdr_t* ehdr = reinterpret_cast<const lilka_elf32_hdr_t*>(pbuf);

    /* Validate ELF magic */
    if (ehdr->ident[0] != 0x7f || ehdr->ident[1] != 'E' || ehdr->ident[2] != 'L' || ehdr->ident[3] != 'F') {
        lilka::serial.log("dynloader: invalid ELF magic");
        return -EINVAL;
    }

    /* Validate 32-bit, little-endian */
    if (ehdr->ident[4] != 1 || ehdr->ident[5] != 1) {
        lilka::serial.log("dynloader: expected 32-bit little-endian ELF");
        return -EINVAL;
    }

    /* Load sections into memory */
    int ret = elf_load_sections(elf, pbuf);
    if (ret) return ret;

    lilka::serial.log("dynloader: entry=%p", elf->entry);

    /* Process relocations */
    const lilka_elf32_shdr_t* shdr = reinterpret_cast<const lilka_elf32_shdr_t*>(pbuf + ehdr->shoff);

    for (uint32_t i = 0; i < ehdr->shnum; i++) {
        if (shdr[i].type == LILKA_SHT_RELA) {
            uint32_t nr_reloc = shdr[i].size / sizeof(lilka_elf32_rela_t);
            const lilka_elf32_rela_t* rela = reinterpret_cast<const lilka_elf32_rela_t*>(pbuf + shdr[i].offset);
            const lilka_elf32_sym_t* symtab =
                reinterpret_cast<const lilka_elf32_sym_t*>(pbuf + shdr[shdr[i].link].offset);
            const char* strtab = reinterpret_cast<const char*>(pbuf + shdr[shdr[shdr[i].link].link].offset);

            for (uint32_t j = 0; j < nr_reloc; j++) {
                lilka_elf32_rela_t rela_buf;
                memcpy(&rela_buf, &rela[j], sizeof(lilka_elf32_rela_t));

                const lilka_elf32_sym_t* sym = &symtab[LILKA_ELF_R_SYM(rela_buf.info)];
                int type = LILKA_ELF_R_TYPE(rela_buf.info);
                uintptr_t addr = 0;

                if (type == LILKA_STT_COMMON || type == LILKA_STT_OBJECT || type == LILKA_STT_SECTION) {
                    const char* sym_name = strtab + sym->name;
                    if (sym_name[0]) {
                        addr = lilka_dynloader_find_symbol(sym_name);
                        /* If not found externally, check local .data */
                        if (!addr && sym->shndx != LILKA_SHN_UNDEF) {
                            addr = (uintptr_t)(elf->sec[LILKA_ELF_SEC_DATA].addr + sym->value -
                                               elf->sec[LILKA_ELF_SEC_DATA].v_addr);
                        }
                        if (!addr) {
                            lilka::serial.log("dynloader: unresolved symbol: %s", sym_name);
                            return -ENOSYS;
                        }
                    }
                } else if (type == LILKA_STT_FILE) {
                    const char* func_name = strtab + sym->name;
                    if (sym->value) {
                        addr = elf_map_sym(elf, sym->value);
                    } else {
                        addr = lilka_dynloader_find_symbol(func_name);
                    }
                    /* Check local text */
                    if (!addr && sym->shndx != LILKA_SHN_UNDEF) {
                        addr = (uintptr_t)(elf->sec[LILKA_ELF_SEC_TEXT].addr + sym->value -
                                           elf->sec[LILKA_ELF_SEC_TEXT].v_addr);
                    }
                    if (!addr) {
                        lilka::serial.log("dynloader: unresolved function: %s", func_name);
                        return -ENOSYS;
                    }
                }

                ret = elf_arch_relocate(elf, &rela_buf, sym, addr);
                if (ret) return ret;
            }
        }
        /* Parse .dynsym for exported symbols */
        else if (shdr[i].type == LILKA_SHT_DYNSYM) {
            const lilka_elf32_sym_t* dsymtab = reinterpret_cast<const lilka_elf32_sym_t*>(pbuf + shdr[i].offset);
            const char* dstrtab = reinterpret_cast<const char*>(pbuf + shdr[shdr[i].link].offset);
            uint32_t nsyms = shdr[i].size / sizeof(lilka_elf32_sym_t);

            /* Count global functions */
            uint16_t count = 0;
            for (uint32_t j = 0; j < nsyms; j++) {
                if (LILKA_ELF_ST_BIND(dsymtab[j].info) == LILKA_STB_GLOBAL &&
                    LILKA_ELF_ST_TYPE(dsymtab[j].info) == LILKA_STT_FUNC) {
                    count++;
                }
            }

            if (count) {
                elf->symtab = static_cast<lilka_dynsym_t*>(elf_malloc(count * sizeof(lilka_dynsym_t), false));
                if (!elf->symtab) return -ENOMEM;
                memset(elf->symtab, 0, count * sizeof(lilka_dynsym_t));

                uint16_t idx = 0;
                for (uint32_t j = 0; j < nsyms && idx < count; j++) {
                    if (LILKA_ELF_ST_BIND(dsymtab[j].info) == LILKA_STB_GLOBAL &&
                        LILKA_ELF_ST_TYPE(dsymtab[j].info) == LILKA_STT_FUNC) {
                        /* Resolve address within loaded sections */
                        elf->symtab[idx].addr =
                            static_cast<void*>(elf->ptext + dsymtab[j].value - elf->sec[LILKA_ELF_SEC_TEXT].v_addr);
                        /* Copy name */
                        size_t len = strlen(dstrtab + dsymtab[j].name) + 1;
                        char* nm = static_cast<char*>(elf_malloc(len, false));
                        if (!nm) {
                            elf->sym_count = idx;
                            return -ENOMEM;
                        }
                        memcpy(nm, dstrtab + dsymtab[j].name, len);
                        elf->symtab[idx].name = nm;
                        lilka::serial.log("dynloader: export[%d] %s", idx, nm);
                        idx++;
                    }
                }
                elf->sym_count = count;
            }
        }
    }

    /* Flush PSRAM cache to ensure code is visible to instruction bus */
    elf_flush_cache();

    return 0;
}

int lilka_elf_run(lilka_elf_t* elf, int argc, char* argv[]) {
    if (!elf || !elf->entry) return -EINVAL;
    return elf->entry(argc, argv);
}

void lilka_elf_deinit(lilka_elf_t* elf) {
    if (!elf) return;

    if (elf->ptext) {
        elf_free(elf->ptext);
        elf->ptext = nullptr;
    }
    if (elf->pdata) {
        elf_free(elf->pdata);
        elf->pdata = nullptr;
    }
    if (elf->sym_count && elf->symtab) {
        for (int i = 0; i < elf->sym_count; i++) {
            if (elf->symtab[i].name) {
                // cppcheck-suppress cstyleCast
                elf_free(const_cast<char*>(elf->symtab[i].name));
            }
        }
        elf_free(elf->symtab);
        elf->symtab = nullptr;
    }
    elf->sym_count = 0;
    memset(elf, 0, sizeof(lilka_elf_t));
}

/* ── High-level helper ──────────────────────────────────────────────────── */

int lilka_dynloader_run(const char* path, int argc, char* argv[]) {
    if (!path) return -EINVAL;

    /* Open and read the .so file */
    FILE* f = fopen(path, "rb");
    if (!f) {
        lilka::serial.log("dynloader: failed to open %s", path);
        return -EIO;
    }

    fseek(f, 0, SEEK_END);
    long fsize = ftell(f);
    fseek(f, 0, SEEK_SET);

    if (fsize <= 0) {
        fclose(f);
        return -EIO;
    }

    uint8_t* buf = static_cast<uint8_t*>(elf_malloc(fsize, false));
    if (!buf) {
        fclose(f);
        lilka::serial.log("dynloader: failed to alloc %ld bytes for file", fsize);
        return -ENOMEM;
    }

    size_t rd = fread(buf, 1, fsize, f);
    fclose(f);

    if ((long)rd != fsize) {
        elf_free(buf);
        return -EIO;
    }

    /* Load, relocate, run */
    lilka_elf_t elf;
    lilka_elf_init(&elf);

    int ret = lilka_elf_relocate(&elf, buf, fsize);
    if (ret == 0) {
        ret = lilka_elf_run(&elf, argc, argv);
    }

    lilka_elf_deinit(&elf);
    elf_free(buf);
    return ret;
}

/* ── Default libc symbol table ──────────────────────────────────────────── */

/* Standard C functions exported for use by dynamically loaded apps */
extern "C" {
extern int __ltdf2(double, double);
extern unsigned int __fixunsdfsi(double);
extern int __gtdf2(double, double);
extern double __floatunsidf(unsigned int);
extern double __divdf3(double, double);
}

static const lilka_dynsym_t g_libc_symbols[] = {
    /* string.h */
    LILKA_DYNSYM_EXPORT(memset),
    LILKA_DYNSYM_EXPORT(memcpy),
    LILKA_DYNSYM_EXPORT(memmove),
    LILKA_DYNSYM_EXPORT(memcmp),
    LILKA_DYNSYM_EXPORT(strlen),
    LILKA_DYNSYM_EXPORT(strcmp),
    LILKA_DYNSYM_EXPORT(strncmp),
    LILKA_DYNSYM_EXPORT(strcpy),
    LILKA_DYNSYM_EXPORT(strncpy),
    LILKA_DYNSYM_EXPORT(strcat),
    LILKA_DYNSYM_EXPORT(strchr),
    LILKA_DYNSYM_EXPORT(strrchr),
    LILKA_DYNSYM_EXPORT(strtol),
    LILKA_DYNSYM_EXPORT(strtod),
    LILKA_DYNSYM_EXPORT(strerror),

    /* stdio.h */
    LILKA_DYNSYM_EXPORT(printf),
    LILKA_DYNSYM_EXPORT(snprintf),
    LILKA_DYNSYM_EXPORT(vsnprintf),
    LILKA_DYNSYM_EXPORT(puts),
    LILKA_DYNSYM_EXPORT(putchar),

    /* stdlib.h */
    LILKA_DYNSYM_EXPORT(malloc),
    LILKA_DYNSYM_EXPORT(calloc),
    LILKA_DYNSYM_EXPORT(realloc),
    LILKA_DYNSYM_EXPORT(free),
    LILKA_DYNSYM_EXPORT(rand),
    LILKA_DYNSYM_EXPORT(srand),
    {"abs", (void*)(int (*)(int))abs},

    /* math (compiler builtins) */
    LILKA_DYNSYM_EXPORT(__ltdf2),
    LILKA_DYNSYM_EXPORT(__fixunsdfsi),
    LILKA_DYNSYM_EXPORT(__gtdf2),
    LILKA_DYNSYM_EXPORT(__floatunsidf),
    LILKA_DYNSYM_EXPORT(__divdf3),

    /* FreeRTOS / system */
    LILKA_DYNSYM_EXPORT(usleep),
    LILKA_DYNSYM_EXPORT(vTaskDelay),

    LILKA_DYNSYM_END
};

/* Auto-register libc symbols at startup */
__attribute__((constructor)) static void _register_libc_symbols() {
    lilka_dynloader_register_symbols(g_libc_symbols);
}

/* ── C++ DynLoader class implementation ─────────────────────────────────── */

namespace lilka {

int DynLoader::load(const char* path) {
    if (loaded) unload();

    errorMsg = nullptr;

    FILE* f = fopen(path, "rb");
    if (!f) {
        errorMsg = "Failed to open .so file";
        serial.log("dynloader: %s: %s", errorMsg, path);
        return -EIO;
    }

    fseek(f, 0, SEEK_END);
    long fsize = ftell(f);
    fseek(f, 0, SEEK_SET);

    if (fsize <= 0) {
        fclose(f);
        errorMsg = "Invalid file size";
        return -EIO;
    }

    fileData = static_cast<uint8_t*>(elf_malloc(fsize, false));
    if (!fileData) {
        fclose(f);
        errorMsg = "Not enough memory for file";
        return -ENOMEM;
    }
    fileSize = fsize;

    size_t rd = fread(fileData, 1, fsize, f);
    fclose(f);

    if ((long)rd != fsize) {
        elf_free(fileData);
        fileData = nullptr;
        errorMsg = "Failed to read file";
        return -EIO;
    }

    int ret = lilka_elf_relocate(&elf, fileData, fileSize);
    if (ret != 0) {
        errorMsg = "ELF relocation failed";
        serial.log("dynloader: relocation failed: %d", ret);
        lilka_elf_deinit(&elf);
        elf_free(fileData);
        fileData = nullptr;
        return ret;
    }

    loaded = true;
    serial.log("dynloader: loaded successfully, entry=%p", elf.entry);
    return 0;
}

int DynLoader::execute(int argc, char* argv[]) {
    if (!loaded) {
        errorMsg = "No .so loaded";
        return -EINVAL;
    }
    return lilka_elf_run(&elf, argc, argv);
}

void DynLoader::unload() {
    if (!loaded) return;
    lilka_elf_deinit(&elf);
    if (fileData) {
        elf_free(fileData);
        fileData = nullptr;
    }
    fileSize = 0;
    loaded = false;
    errorMsg = nullptr;
    lilka_elf_init(&elf);
}

} // namespace lilka

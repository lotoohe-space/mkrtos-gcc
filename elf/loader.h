/****************************************************************************
 * ARMv7M ELF loader
 * Copyright (c) 2013-2015 Martin Ribelotta
 * Copyright (c) 2019 Johannes Taelman
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of copyright holders nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * ''AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL COPYRIGHT HOLDERS OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *****************************************************************************/

#ifndef LOADER_H_
#define LOADER_H_

#include "loader_userdata.h"
#include "loader_config.h"
#ifdef __cplusplus__
extern "C" {
#endif

#ifndef DOX

typedef struct {
    void *data;
    int secIdx;
    off_t relSecIdx;
    int sh_size;
} ELFSection_t;

typedef struct ELFExec {

    LOADER_USERDATA_T user_data;

    size_t sections;
    off_t sectionTable;
    off_t sectionTableStrings;

    size_t symbolCount;
    off_t symbolTable;
    off_t symbolTableStrings;
    off_t entry;

    ELFSection_t text;
    ELFSection_t rodata;
    ELFSection_t data;//fork时需要被复制
    ELFSection_t bss;//fork时需要被复制
    ELFSection_t init_array;
    ELFSection_t fini_array;
    ELFSection_t sdram_rodata;
    ELFSection_t sdram_data;//fork时需要被复制
    ELFSection_t sdram_bss;//fork时需要被复制

    unsigned int fini_array_size;
    uint32_t* used_count;//使用计数
//    uint8_t clone_vm;//子进程与父进程是否运行于相同的内存空间

} ELFExec_t;


#endif
/**
 * @defgroup elf_loader ELF Loader
 * @{
 */





typedef struct ELFExec ELFExec_t;

/**
 * Load ELF file from "path" with environment "env"
 * @param path Path to file to load
 * @param user_data Pointer to user data
 * @param exec returns pointer to ELFExec_t struct
 * @retval 0 On successful
 * @todo Error information
 */
extern int load_elf(const char *path, LOADER_USERDATA_T user_data, ELFExec_t **exec);

/**
 * Unload ELF
 * @param exec Pointer to ELFExec_t struct
 * @retval 0 On successful
 * @todo Error information
 */
extern int unload_elf(ELFExec_t *exec);

/**
 * Jump to entrypoint
 * @param exec Pointer to ELFExec_t struct
 * @retval 0 On successful
 * @todo Error information
 */
extern int jumpTo(ELFExec_t *e,char *const argv[ ], char *const envp[ ]) ;

/**
 * Get function pointer
 * @param exec Pointer to ELFExec_t struct
 * @param func_name function symbol name (mangled if C++)
 * @retval function pointer, 0 On failure
 * @todo Error information
 */
extern void * get_func(ELFExec_t *exec, const char *func_name);

/**
 * Get object pointer
 * @param exec Pointer to ELFExec_t struct
 * @param func_name function symbol name (mangled if C++)
 * @retval object pointer, 0 On failure
 * @todo Error information
 */
extern void * get_obj(ELFExec_t *exec, const char *obj_name);

/**
 * Get symbol
 * @param exec Pointer to ELFExec_t struct
 * @param symbol name (mangled if C++)
 * @retval value, 0 On failure
 * @todo Error information
 */
extern void * get_sym(ELFExec_t *exec, const char *sym_name, int symbol_type);


/** @} */

#ifdef __cplusplus__
}
#endif

#endif /* LOADER_H_ */

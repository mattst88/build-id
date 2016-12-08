/* vim: set expandtab tabstop=4 softtabstop=4 shiftwidth=4: */
/*
 * Copyright © 2016 Intel Corporation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#include <elf.h>
#include <stdio.h>
#include <string.h>

#include "build-id.h"

extern const ElfW(Nhdr) __note_gnu_build_id_start[] __attribute__((weak));
extern const ElfW(Nhdr) __note_gnu_build_id_end[] __attribute__((weak));

void
so_print(void)
{
    extern char etext, edata, end;
    printf("Shared object:\n");
    printf("    program text (etext)      %10p\n", &etext);
    printf("    initialized data (edata)  %10p\n", &edata);
    printf("    uninitialized data (end)  %10p\n", &end);
    printf("    note section start        %10p\n", __note_gnu_build_id_start);
    printf("    note section end          %10p\n", __note_gnu_build_id_end);
}

const ElfW(Nhdr) *
build_id_find_nhdr(void)
{
    const ElfW(Nhdr) *nhdr = (void *)__note_gnu_build_id_start;

    if (__note_gnu_build_id_start >= __note_gnu_build_id_end) {
        fprintf(stderr, "No .note section\n");
        return NULL;
    }

    while (nhdr->n_type != NT_GNU_BUILD_ID)
    {
        nhdr = (ElfW(Nhdr) *)
            ((intptr_t)nhdr + sizeof(ElfW(Nhdr)) + nhdr->n_namesz + nhdr->n_descsz);

        if (nhdr >= __note_gnu_build_id_end) {
            fprintf(stderr, "No .note.gnu.build-id section found\n");
            return NULL;
        }
    }

    return nhdr;
}

ElfW(Word)
build_id_length(const ElfW(Nhdr) *nhdr)
{
    return nhdr->n_descsz;
}

void
build_id_read(const ElfW(Nhdr) *nhdr, unsigned char *build_id)
{
    memcpy(build_id,
           (unsigned char *)nhdr + sizeof(ElfW(Nhdr)) + nhdr->n_namesz,
           nhdr->n_descsz);
}

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

#include <stdio.h>
#include <stdlib.h>

#include "build-id.h"

int
main(int argc, char *argv[])
{
    extern char etext, edata, end;
    extern const char __note_gnu_build_id_end[] __attribute__((weak));
    extern const char __note_gnu_build_id_start[] __attribute__((weak));
    printf("Executable:\n");
    printf("    program text (etext)      %10p\n", &etext);
    printf("    initialized data (edata)  %10p\n", &edata);
    printf("    uninitialized data (end)  %10p\n", &end);
    printf("    note section start        %10p\n", __note_gnu_build_id_start);
    printf("    note section end          %10p\n", __note_gnu_build_id_end);

    so_print();

    const ElfW(Nhdr) *nhdr = build_id_find_nhdr();
    if (!nhdr)
        return -1;

    ElfW(Word) len = build_id_length(nhdr);

    unsigned char *build_id = malloc(len * sizeof(char));
    if (!build_id)
        return -1;

    build_id_read(nhdr, build_id);

    printf("Build ID: ");
    for (ElfW(Word) i = 0; i < len; i++) {
        printf("%02x", build_id[i]);
    }
    printf("\n");
    return 0;
}

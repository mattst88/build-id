# build-id

Read your own `.note.gnu.build-id`

## Background

The `.note.gnu.build-id` section in an [ELF](https://en.wikipedia.org/wiki/Executable_and_Linkable_Format) binary contains a "strongly unique embedded identifier".

[binutils](https://www.gnu.org/software/binutils/)' `ld` has supported the `--build-id=...` option since version 2.18 (released 2007). When used, with a `sha1` or `md5` argument it directs `ld` to insert an ELF section `.note.gnu.build-id` into the binary containing a hash of the normative parts of the output&mdash;that is, an identifier that uniquely identifies the output file.

Its originally intended purpose (described [here](https://fedoraproject.org/wiki/Releases/FeatureBuildId)) is to simplify and improve debugging tools, but it is occasionally useful for a program to be able to read its own build-id. [Mesa](https://www.mesa3d.org/) uses the build-id of the running OpenGL or Vulkan driver as a way of identifying its on-disk cache of pre-compiled shader programs.

I spent a good amount of time researching possible ways of uniquely identifying the running OpenGL or Vulkan driver, and I saw that [others had similar questions](https://stackoverflow.com/questions/17637745/can-a-program-read-its-own-elf-section).

## The Problem

[Mesa](https://mesa3d.org/), the software project providing OpenGL and Vulkan on Linux, needs to identify its on-disk cache of compiled shader programs. Shader programs compiled by one version of Mesa may not work (or worse: cause GPU hangs) with another version, so how can we know whether the running version of Mesa generated those cached files?

I found the `--build-id=...` flag but struggled to find a way to access the identifier it generates from within a running process.

## The solution
The `dl_iterate_phdr` function is the critical piece of the puzzle, allowing the application to inspect the shared objects it has loaded. A callback function searches the program headers of each object loaded and finds the appropriate `.note.gnu.build-id` section. Mesa includes its own build-id into a collection of other data it hashes to produce the key to look up a shader in the on-disk cache, thereby ensuring that it will only load shader programs that were produced by the same build of Mesa.

With the problem now solved and the code in successful use in Mesa since 2017, my hope is to make the technique more widely known and in doing so to save others time. The code very small and MIT licensed, so feel free to include the two source files into your project.

## API
### Usage
Retrieve an opaque pointer to the `.note.gnu.build-id` ELF segment in the process's address space using either the filename of a loaded ELF binary or a symbol address. From the returned pointer, access the build-id and its length in bytes.

The API consists of only four functions and an opaque struct data type.
```c
struct build_id_note;
```

Find the `.note.gnu.build-id` section given the filename of the ELF binary. Returns `NULL` on failure.
```c
const struct build_id_note *
build_id_find_nhdr_by_name(const char *name);
```

Find the `.note.gnu.build-id` section given a symbol in the ELF binary. Returns `NULL` on failure.
```c
const struct build_id_note *
build_id_find_nhdr_by_symbol(const void *symbol);
```

Return the length (in bytes) of the build-id.
```c
ElfW(Word)
build_id_length(const struct build_id_note *note);
```

Return a pointer to the build-id.
```c
const uint8_t *
build_id_data(const struct build_id_note *note);
```

## Examples
Some demonstrations of the API are provided:
  * [test.c](test.c) - Retrieves its own build-id
  * [so-test.c](so-test.c) - Retrieves the build-id of a linked shared object
  * [dlopen-test.c](dlopen-test.c) - Retrieves the build-id of a `dlopen`'d shared object

```sh
$ ./build-id
Build ID: 5a9f352b656d36bd95b0cec8a31679dac872f5be
$ LD_LIBRARY_PATH=. ./so-build-id
Build ID: 79588ab64fe9fe95bce4243e26aee4449517434e
$ ./dlopen-build-id
Build ID: 79588ab64fe9fe95bce4243e26aee4449517434e
```

Separately, the `file` command can retrieve the build-ids:
```sh
$ file build-id
build-id: ELF 64-bit LSB pie executable, x86-64, version 1 (SYSV), dynamically linked, interpreter /lib64/ld-linux-x86-64.so.2, BuildID[sha1]=5a9f352b656d36bd95b0cec8a31679dac872f5be, for GNU/Linux 3.2.0, with debug_info, not stripped
$ file libbuild-id.so 
libbuild-id.so: ELF 64-bit LSB shared object, x86-64, version 1 (SYSV), dynamically linked, BuildID[sha1]=79588ab64fe9fe95bce4243e26aee4449517434e, not stripped
```

### Building
A simple `Makefile` builds the example programs with `-Wl,--build-id=sha1` (and `-fPIC`; see [Caveats](#caveats))
```sh
$ make
```

### Testing
`make check`  runs the example programs and verifies that they output the same build-id as reported by `file`.
```sh
$ make check
```

## Caveats
### -fPIC
Looking up a build-id given a symbol name (with `build_id_find_nhdr_by_symbol`) requires a call to the `dladdr` function. Quoting from the `dladdr(3)` man page:

> Sometimes, the function pointers you pass to dladdr() may surprise you.  On some architectures (notably i386 and x86-64), dli_fname and dli_fbase may end up pointing back at the  object  from  which you called dladdr(), even if the function used as an argument should come from a dynamically linked library.
>
> The problem is that the function pointer will still be resolved at compile time, but merely point to the plt (Procedure Linkage Table) section of the original object (which dispatches the call after asking the dynamic linker to resolve the symbol).  To work around this, you can try to compile the code to be position-independent: then, the compiler cannot prepare the pointer at compile time  any more and gcc(1) will generate code that just loads the final symbol address from the got (Global Offset Table) at run time before passing it to dladdr().

As a result, build code with `-fPIC` to ensure that `build_id_find_nhdr_by_symbol` works as expected. In practice, I found that compiling the `so-build-id` program without `-fPIC` with clang caused the program to fail.

### build-id dependent on compiler flags
Any change that affects the code or data of the ELF binary will also result in a different build-id. It's obvious but important to note that debug and release builds will have different build-ids.

For Mesa's usage this is entirely acceptable because we expect that the vast majority of users are using distribution-provided builds of Mesa.

## Other proposed (and failed) solutions

Other proposed solutions I tried failed for a variety of reasons:

  * "Just hash all of the source code"
  * Use a linker script to insert `start`/`end` symbols around the `.note.gnu.build-id`. Failed when reading the build-id of a shared object for unknown reasons. Incompatible with [gold](https://en.wikipedia.org/wiki/Gold_(linker)), which does not use linker scripts.
  * Use `dladdr()` to find the path and name of the binary. Open and read ELF sections. Effectively the same as what the code in this repository does, but without the guarantee that the binary you read from the disk is the same one that is executing in the current process.

# brainfuck

This is an experimental implementation of a brainfuck compiler targeting the C language.

Tested on Linux and macOS.

```bash
$ cat > A.bf <<EOF
>++++++++[<++++++++>-]<+.
EOF
$ bf A.bf
A
```

## `bf`, the interpreter

This is the brainfuck interpreter. It only takes the input file.

```bash
$ bf demo/hello_world.bf
Hello, World!
```

## `bfc`, the transpiler

This is the brainfuck transpiler. It produces C code compatible with the `libbf_rt`.

```bash
$ bfc demo/hello_world.bf demo/hello_world.c
$ cc -c -Iinclude demo/hello_world.c
$ cc -o hello_world hello_world.o libbf_rt_static.a
$ ./hello_world
Hello, World!
```

## `libbf_rt`, the brainfuck runtime

This is a lightweight layer that translates compiled brainfuck instructions to real C code. This is not intended to be
used directly.

It makes the strip dynamic memory and manages runtime errors.

```c++
// Example equivalent of ",." in C using libbf_rt

#include <librt.h>

#define TRY(i) do { error = i; if (error) return error; } while (0)

int main(void)
{
    int error;
    struct strip_t strip;
    
    TRY(brainfuck_rt_init(&strip));
    TRY(brainfuck_rt_inp(&strip, 1)); // ','
    TRY(brainfuck_rt_out(&strip, 1)); // '.'
    brainfuck_rt_deinit(&strip);
    return 0;
}
```

## Building from sources

To build from sources, you will need a recent version of CMake and a C compiler (MSVC not supported).

```bash
$ mkdir build
$ cmake ..
$ make

# Or
$ cmake -G Ninja ..
$ ninja
```

This will produce `bf`, `bfc`, `libbf_rt_static.a` and `libbf_rt_dynamic.so/dylib`.

# brainfuck

This is an experimental implementation of a brainfuck compiler targeting the C language.

Tested on Linux and macOS. JIT only works on macOS, it segfaults on Linux.

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

It's also, only on macOS for now, a JIT interpreter:

```bash
$ bf -jit demo/hello_world.bf
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

#include <libbf/rt.h>

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

## `libbf_int`, the brainfuck interpreter library

This is an abstraction layer over `libbf_rt`, it allows you to run brainfuck code directly into your binary.

It exposes: normal interpreter and JIT (works on macOS for now).

```c++
#include <stdio.h>
#include <string.h>
#include <libbf/int.h>

int main(void)
{
    int error;
    bf_state_t state;

    if ((error = bf_new(&state)))
    {
        fprintf(stderr, "cannot load libbf_int: %s\n", strerror(error));
        return 1;
    }

    const char *code = ">++++++++[<++++++++>-]<+.";

#if defined(__APPLE__)
    if ((error = bf_eval_jit(state, code)))
#else
    if ((error = bf_eval(state, code)))
#endif
    {
        fprintf(stderr, "runtime error: %s\n", strerror(error));
        bf_deinit(state);
        return 1;
    }
    
    printf("\n");
    bf_deinit(state);
    return 0;
}
```

```bash
$ cc -rdynamic -o test test.c libbf_int_static.a
$ ./test
A
```

## Building from sources

To build from sources, you will need a recent version of CMake and a C compiler (MSVC not supported).

```bash
$ mkdir build
$ cd build
$ cmake ..
$ make

# Or
$ cmake -G Ninja ..
$ ninja
```

This will produce `bf`, `bfc`, `libbf_rt_static.a` and `libbf_rt_dynamic.so/dylib`.

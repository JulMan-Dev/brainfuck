#if defined(__APPLE__)
#define SYM(name) _##name
#else
#define SYM(name) name
#endif

    .globl SYM(__jit_builtin_error)
SYM(__jit_builtin_error):
    ret

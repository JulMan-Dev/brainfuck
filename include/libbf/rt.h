#pragma once

#include <stddef.h>

#include "../common.h"

struct strip_t
{
    unsigned char* buf;
    size_t cap, index;
    int err;
    void *ext_out, *ext_inp;
};

typedef int (*bf_block_t)(struct strip_t*);

BF_API_PUBLIC int brainfuck_rt_init(struct strip_t*);
BF_API_PUBLIC int brainfuck_rt_deinit(struct strip_t*);
BF_API_PUBLIC int brainfuck_rt_lef(struct strip_t*, size_t);
BF_API_PUBLIC int brainfuck_rt_rig(struct strip_t*, size_t);
BF_API_PUBLIC int brainfuck_rt_inc(struct strip_t*, size_t);
BF_API_PUBLIC int brainfuck_rt_dec(struct strip_t*, size_t);
BF_API_PUBLIC int brainfuck_rt_out(struct strip_t*, size_t);
BF_API_PUBLIC int brainfuck_rt_inp(struct strip_t*, size_t);
BF_API_PUBLIC int brainfuck_rt_loop(struct strip_t*, bf_block_t);
BF_API_PUBLIC int brainfuck_rt_error(struct strip_t*, int);
BF_API_PUBLIC int brainfuck_rt_ext_sout(struct strip_t*, void*);
BF_API_PUBLIC int brainfuck_rt_ext_sinp(struct strip_t*, void*);

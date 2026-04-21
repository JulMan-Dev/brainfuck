#pragma once

#include <stdio.h>

#include "libbf/rt.h"
#include "common.h"

typedef void *bf_state_t; // opaque

API_PUBLIC int bf_new(bf_state_t *);
API_PUBLIC int bf_deinit(bf_state_t);
API_PUBLIC int bf_set_input(bf_state_t, FILE *);
API_PUBLIC int bf_set_output(bf_state_t, FILE *);
API_PUBLIC int bf_eval(bf_state_t, char *);
API_PUBLIC int bf_evals(bf_state_t, char *, size_t);
API_PUBLIC int bf_eval_jit(bf_state_t, char *);
API_PUBLIC int bf_evals_jit(bf_state_t, char *, size_t);
API_PUBLIC int bf_evalc(bf_state_t, bf_block_t);
API_PUBLIC int bf_get(bf_state_t, unsigned char *, size_t);
API_PUBLIC int bf_read(bf_state_t, unsigned char *, size_t , size_t);

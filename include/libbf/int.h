#pragma once

#include <stdio.h>

#include "libbf/rt.h"
#include "common.h"

typedef void *bf_state_t; // opaque
typedef void *bf_code_t; // opaque

API_PUBLIC int bf_new(bf_state_t *);
API_PUBLIC int bf_deinit(bf_state_t);
API_PUBLIC int bf_set_input(bf_state_t, FILE *);
API_PUBLIC int bf_set_output(bf_state_t, FILE *);
API_PUBLIC int bf_eval(bf_state_t, const char *);
API_PUBLIC int bf_evals(bf_state_t, const char *, size_t);
API_PUBLIC int bf_eval_jit(bf_state_t, const char *);
API_PUBLIC int bf_evals_jit(bf_state_t, const char *, size_t);
API_PUBLIC int bf_evalc(bf_state_t, bf_block_t);
API_PUBLIC int bf_get(bf_state_t, unsigned char *, size_t);
API_PUBLIC int bf_read(bf_state_t, unsigned char *, size_t , size_t);

// Prepared code

API_PUBLIC int bf_code_free(bf_state_t, bf_code_t);
API_PUBLIC int bf_prepare_bf(bf_state_t, const char *, bf_code_t *);
API_PUBLIC int bf_prepares_bf(bf_state_t, const char *, size_t, bf_code_t *);
API_PUBLIC int bf_evalp(bf_state_t, bf_code_t);
API_PUBLIC int bf_evalp_jit(bf_state_t, bf_code_t);
API_PUBLIC int bf_code_error(bf_state_t, bf_code_t, const char **, size_t *);

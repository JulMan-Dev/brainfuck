#pragma once

#include <stdio.h>

#include "libbf/rt.h"
#include "common.h"

typedef void *bf_state_t; // opaque
typedef void *bf_code_t; // opaque

/// Initializes a new brainfuck state. Check returned error code, it may fail.
BF_API_PUBLIC int bf_new(bf_state_t *);

/// Deinitializes a brainfuck state, freeing all acquired memory. This cannot fail.
BF_API_PUBLIC int bf_deinit(bf_state_t);

/// Changes the input file stream for input instructions. Default is `stdin` if not used. This cannot fail.
BF_API_PUBLIC int bf_set_input(bf_state_t, FILE *);

/// Changes the output file stream for output instructions. Default is `stdout` if not used. This cannot fail.
BF_API_PUBLIC int bf_set_output(bf_state_t, FILE *);

/// Evaluates a brainfuck C string. This is deprecated, you should use `bf_prepare_bf` instead and take care of errors
/// yourself, this just prints to `stderr`. This can fail.
///
/// @deprecated
__attribute__((__deprecated__))
BF_API_PUBLIC int bf_eval(bf_state_t, const char *);

/// Evaluates a brainfuck sized string. This is deprecated, you should use `bf_prepares_bf` instead and take care of
/// errors yourself, this just prints to `stderr`. This can fail.
///
/// @deprecated
__attribute__((__deprecated__))
BF_API_PUBLIC int bf_evals(bf_state_t, const char *, size_t);

/// Compiles to JIT a brainfuck C string and executes it. This is deprecated, you should use `bf_prepare_bf` instead and
/// take care of errors yourself, this just prints to `stderr`. This can fail.
///
/// For now, JIT compilation only works on macOS AArch64.
///
/// @deprecated
__attribute__((__deprecated__))
BF_API_PUBLIC int bf_eval_jit(bf_state_t, const char *);

/// Compiles to JIT a brainfuck sized string and executes it. This is deprecated, you should use `bf_prepares_bf`
/// instead and take care of errors yourself, this just prints to `stderr`. This can fail.
///
/// For now, JIT compilation only works on macOS AArch64.
///
/// @deprecated
 __attribute__((__deprecated__))
BF_API_PUBLIC int bf_evals_jit(bf_state_t, const char *, size_t);

/// Executes a compiled brainfuck block. Avoid using this to acquire the internal strip, for read-only access, you
/// could use `bf_get` or `bf_read`. This can fail, usually when the block fails.
BF_API_PUBLIC int bf_evalc(bf_state_t, bf_block_t);

/// Gets a single cell of the strip. Pass `NULL` to just check if the cell is allocated. This can fail.
///
/// Returns `0` on success, else a errno code.
///
/// This is equivalent of `bf_read(state, ptr, addr, 1)`.
BF_API_PUBLIC int bf_get(bf_state_t, unsigned char *, size_t);

/// Gets a bunch of bytes of the strip, starting from first and reads n bytes. Pass `NULL` to just check if whole
/// location is allocated. This can fail.
///
/// Returns `0` on success, else a errno code.
BF_API_PUBLIC int bf_read(bf_state_t, unsigned char *, size_t, size_t);

// Prepared code

/// Frees a prepared code structure. This checks for double free and fails. You cannot free bf_code_t not owned by the
/// passed state, fails. This can fail.
BF_API_PUBLIC int bf_code_free(bf_state_t, bf_code_t);

/// Prepares a brainfuck C string for future evaluation or JIT compilation. It can fail.
///
/// You can read the failure error details after it fails. Notice that if the output is written, you must free the
/// result.
BF_API_PUBLIC int bf_prepare_bf(bf_state_t, const char *, bf_code_t *);

/// Prepares a brainfuck sized string for future evaluation or JIT compilation. It can fail.
///
/// You can read the failure error details after it fails. Notice that if the output is written, you must free the
/// result.
BF_API_PUBLIC int bf_prepares_bf(bf_state_t, const char *, size_t, bf_code_t *);

/// Evaluates a prepared code structure. This can fail. The error is not marked into the code structure, it's just
/// returned.
BF_API_PUBLIC int bf_evalp(bf_state_t, bf_code_t);

/// Compiles to JIT a prepared code structure. This can fail. The error is not marked into the code structure, it's just
/// returned.
BF_API_PUBLIC int bf_evalp_jit(bf_state_t, bf_code_t);

/// Gets the internal error details from the code structure. This fails if the code structure is not errored.
BF_API_PUBLIC int bf_code_error(bf_state_t, bf_code_t, const char **, size_t *);

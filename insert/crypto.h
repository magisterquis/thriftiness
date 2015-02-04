/*
 * crypto.h
 * Functions definitions related to encryption/decryption
 * by J. Stuart McMurray
 * created 20150122
 * last modified 20150204
 *
 * Copyright (c) 2015 J. Stuart McMurray <kd5pbo@gmail.com>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include "chacha20_simple.h"

#ifndef HAVE_CRYPTO_H
#define HAVE_CRYPTO_H

/* Seeded the random number generator */
extern int random_seeded; /* Nonzero after seed_random() */

/* The context for the stream used to send nonces */
chacha20_ctx noncectx;
extern int noncestream_init_done; /* Nonzero after noncestream_init() */

/* Contexts for sending and receiving */
chacha20_ctx txctx;
chacha20_ctx rxctx;

/* Initialize the stream used to send nonces */
void noncestream_init();

/* Get the next nonce */
void make_nonce(uint8_t nonce[8]);
extern uint64_t nonce_ctr; /* Number of nonces sent */

#endif /* HAVE_CRYPTO_H */
/*
 * insert.c
 * The remote half of thriftiness
 * by J. Stuart McMurray
 * created 20150117
 * last modified 20150213
 *
 * Copyright (c) 2014 J. Stuart McMurray <kd5pbo@gmail.com>
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

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "comm.h"
#include "insert.h"
#include "net.h"
#include "retvals.h"

/* Install name buffer, for comparisons */
uint8_t installname[INSTALLNAMELEN];
uint8_t key[KEYLEN];

/* Error "returned" from read/write threads */
int reterr;             /* Error itself */
pthread_mutex_t retmtx; /* Mutex to lock reterr */

/* Wait for a connection or make a connection to a remote host, proxy comms
 * between us (pcap) and them */
int main(void) {
        int sleepsec;
        int remfd;
        char *endptr;
        int ret;
        int i;
        if (-1 == setenv("FOO", "bar", 1)) { /* DEBUG */
                printf("Setenv fail\n"); /* DEBUG */
        } /* DEBUG */

        /* Work how long to sleep between connections */
        sleepsec = strtol(SLEEPSEC, &endptr, 0);
        if ((0 == sleepsec) && ('\0' != *endptr)) {
                return RET_INV_SLEEP;
        }

        /* Make sure the installname isn't too long, and copy it to a buffer */
        if ('\0' == INSTALLNAME[0]) {
                exit(EX_INV_INL);
        } else if (INSTALLNAMELEN < strnlen(INSTALLNAME, INSTALLNAMELEN + 1)) {
                exit(EX_INV_INL);
        }
        memset(installname, 0, INSTALLNAMELEN);
        for (i = 0; i < strnlen(INSTALLNAME, INSTALLNAMELEN); ++i) {
                installname[i] = INSTALLNAME[i];
        }

        /* Copy the key to a buffer */
        memset(key, 0, sizeof(key));
        memcpy(key, KEY, sizeof(key));

        /* Initialize thread return integer and mutex */
        reterr = 0;
        pthread_mutex_init(retmtx, NULL);

        /* Set up the stream to make randomish nonces */
        for (;;) {
                /* Clear the error variable */
                unsetenv(ERRVAR);

                /* Get a file descriptor representing the remote end */
                if ('l' == ADDR[0]) {
                        remfd = peer_wait();
                } else if ('c' == ADDR[0]) {
                        remfd = peer_call();
                } else {
                        /* If the binary is misconfigured, exit with the
                         * appropriate code */
                        return RET_UNK_ACT;
                }
                /* Note any errors connecting or waiting, try again or die. */
                if (0 > remfd) {
                        seterr(remfd);
                        goto TRYAGAIN;
                }
                /* Attemp to handshake */
                if (0 > (ret = handshake(remfd))) {
                        /* If it fails, close the connection, try again */
                        close(remfd);
                        seterr(ret);
                        goto TRYAGAIN;
                }

                /* TODO: Start pcap going */
                /* TODO: Start pthread to receive data from pcap and send to shift */
                /* TODO: Call function to receive data from shift and put it on the network */
                shift_to_insert(remfd, NULL);

                /* TODO: Wait for insert->shift thread to end */
                /* TODO: seterr(reterr) */
                /* TODO: Implement set_reterr */

TRYAGAIN:
                close(remfd);
                sleep(sleepsec);
                sleep(4); /* DEBUG */
        }
}

/* Set the environment variable specifed by ERRVAR to the absolute value of the
 * argument */
void seterr(int code) {
        char codes[ERRLEN+1]; /* Error code as string */
        char *evar;           /* Environment variable to set */
        char *errloc;         /* Location of start of error */
        int ret;
        int i;
        const char * vars[] = {ERRVAR, "PATH", NULL};
        evar = NULL;
        errloc = NULL;

        /* First try ERRVAR, then PATH */
        for (i = 0; NULL != vars[i]; ++i) {
                /* Get a pointer to the variable */
                if (NULL == (evar = getenv(vars[i]))) {
                        continue;
                }
                /* Make sure there's enough space */
                if (ERRLEN > strnlen(evar, ERRLEN)) {
                        continue;
                }
                break;
        }
        
        /* If we couldn't find anything, unset PATH and give up */
        if (NULL == vars[i]) {
                goto giveup;
        }

        /* Print the error to a string */
        if (-1 == (ret = snprintf(codes, ERRLEN+1, "%0" STR(ERRLEN) "i",
                                        0 > code ? -1 * code : code))) {
                /* If we're here, something is wronger than before */
                goto giveup;
        }

        /* Put it in the environment variable */
        memcpy(evar, codes, ret);
        return;

giveup:
        unsetenv("PATH");
        return;
}

/* Safely reterr to r if it's not already set */
void set_reterr(int r) {
        /* If it's already set, give up */
        if (0 != reterr) {
                return;
        }
        /* Lock reterr */
        pthread_mutex_lock(&retmtx);
        /* Change it if it's still 0 */
        if (0 == reterr) {
                reterr = r;
        }
        /* Unlock reterr */
        pthread_mutex_unlock(&retmtx);
}

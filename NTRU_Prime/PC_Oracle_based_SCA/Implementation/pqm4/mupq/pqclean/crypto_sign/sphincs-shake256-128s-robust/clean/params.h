#ifndef PQCLEAN_SPHINCSSHAKE256128SROBUST_CLEAN_PARAMS_H
#define PQCLEAN_SPHINCSSHAKE256128SROBUST_CLEAN_PARAMS_H

/* Hash output length in bytes. */
#define PQCLEAN_SPHINCSSHAKE256128SROBUST_CLEAN_N 16
/* Height of the hypertree. */
#define PQCLEAN_SPHINCSSHAKE256128SROBUST_CLEAN_FULL_HEIGHT 64
/* Number of subtree layer. */
#define PQCLEAN_SPHINCSSHAKE256128SROBUST_CLEAN_D 8
/* FORS tree dimensions. */
#define PQCLEAN_SPHINCSSHAKE256128SROBUST_CLEAN_FORS_HEIGHT 15
#define PQCLEAN_SPHINCSSHAKE256128SROBUST_CLEAN_FORS_TREES 10
/* Winternitz parameter, */
#define PQCLEAN_SPHINCSSHAKE256128SROBUST_CLEAN_WOTS_W 16

/* The hash function is defined by linking a different hash.c file, as opposed
   to setting a #define constant. */

/* For clarity */
#define PQCLEAN_SPHINCSSHAKE256128SROBUST_CLEAN_ADDR_BYTES 32

/* WOTS parameters. */
#define PQCLEAN_SPHINCSSHAKE256128SROBUST_CLEAN_WOTS_LOGW 4

#define PQCLEAN_SPHINCSSHAKE256128SROBUST_CLEAN_WOTS_LEN1 (8 * PQCLEAN_SPHINCSSHAKE256128SROBUST_CLEAN_N / PQCLEAN_SPHINCSSHAKE256128SROBUST_CLEAN_WOTS_LOGW)

/* PQCLEAN_SPHINCSSHAKE256128SROBUST_CLEAN_WOTS_LEN2 is floor(log(len_1 * (w - 1)) / log(w)) + 1; we precompute */
#define PQCLEAN_SPHINCSSHAKE256128SROBUST_CLEAN_WOTS_LEN2 3

#define PQCLEAN_SPHINCSSHAKE256128SROBUST_CLEAN_WOTS_LEN (PQCLEAN_SPHINCSSHAKE256128SROBUST_CLEAN_WOTS_LEN1 + PQCLEAN_SPHINCSSHAKE256128SROBUST_CLEAN_WOTS_LEN2)
#define PQCLEAN_SPHINCSSHAKE256128SROBUST_CLEAN_WOTS_BYTES (PQCLEAN_SPHINCSSHAKE256128SROBUST_CLEAN_WOTS_LEN * PQCLEAN_SPHINCSSHAKE256128SROBUST_CLEAN_N)
#define PQCLEAN_SPHINCSSHAKE256128SROBUST_CLEAN_WOTS_PK_BYTES PQCLEAN_SPHINCSSHAKE256128SROBUST_CLEAN_WOTS_BYTES

/* Subtree size. */
#define PQCLEAN_SPHINCSSHAKE256128SROBUST_CLEAN_TREE_HEIGHT (PQCLEAN_SPHINCSSHAKE256128SROBUST_CLEAN_FULL_HEIGHT / PQCLEAN_SPHINCSSHAKE256128SROBUST_CLEAN_D)

/* FORS parameters. */
#define PQCLEAN_SPHINCSSHAKE256128SROBUST_CLEAN_FORS_MSG_BYTES ((PQCLEAN_SPHINCSSHAKE256128SROBUST_CLEAN_FORS_HEIGHT * PQCLEAN_SPHINCSSHAKE256128SROBUST_CLEAN_FORS_TREES + 7) / 8)
#define PQCLEAN_SPHINCSSHAKE256128SROBUST_CLEAN_FORS_BYTES ((PQCLEAN_SPHINCSSHAKE256128SROBUST_CLEAN_FORS_HEIGHT + 1) * PQCLEAN_SPHINCSSHAKE256128SROBUST_CLEAN_FORS_TREES * PQCLEAN_SPHINCSSHAKE256128SROBUST_CLEAN_N)
#define PQCLEAN_SPHINCSSHAKE256128SROBUST_CLEAN_FORS_PK_BYTES PQCLEAN_SPHINCSSHAKE256128SROBUST_CLEAN_N

/* Resulting SPX sizes. */
#define PQCLEAN_SPHINCSSHAKE256128SROBUST_CLEAN_BYTES (PQCLEAN_SPHINCSSHAKE256128SROBUST_CLEAN_N + PQCLEAN_SPHINCSSHAKE256128SROBUST_CLEAN_FORS_BYTES + PQCLEAN_SPHINCSSHAKE256128SROBUST_CLEAN_D * PQCLEAN_SPHINCSSHAKE256128SROBUST_CLEAN_WOTS_BYTES +\
        PQCLEAN_SPHINCSSHAKE256128SROBUST_CLEAN_FULL_HEIGHT * PQCLEAN_SPHINCSSHAKE256128SROBUST_CLEAN_N)
#define PQCLEAN_SPHINCSSHAKE256128SROBUST_CLEAN_PK_BYTES (2 * PQCLEAN_SPHINCSSHAKE256128SROBUST_CLEAN_N)
#define PQCLEAN_SPHINCSSHAKE256128SROBUST_CLEAN_SK_BYTES (2 * PQCLEAN_SPHINCSSHAKE256128SROBUST_CLEAN_N + PQCLEAN_SPHINCSSHAKE256128SROBUST_CLEAN_PK_BYTES)

/* Optionally, signing can be made non-deterministic using optrand.
   This can help counter side-channel attacks that would benefit from
   getting a large number of traces when the signer uses the same nodes. */
#define PQCLEAN_SPHINCSSHAKE256128SROBUST_CLEAN_OPTRAND_BYTES 32

#endif

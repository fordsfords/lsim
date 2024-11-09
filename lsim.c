/* lsim.c - logic simulator. */
/*
# This code and its documentation is Copyright 2024-2024 Steven Ford, http://geeky-boy.com
# and licensed "public domain" style under Creative Commons "CC0": http://creativecommons.org/publicdomain/zero/1.0/
# To the extent possible under law, the contributors to this project have
# waived all copyright and related or neighboring rights to this work.
# In other words, you can use this code for any purpose without any
# restrictions.  This work is published from: United States.  The project home
# is https://github.com/fordsfords/lsim
*/

#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "err.h"
#include "hmap.h"
#include "cfg.h"
#include "lsim.h"


#define E(e_test) do { \
  if ((e_test) == -1) { \
    fprintf(stderr, "ERROR [%s:%d]: '%s' returned -1\n", __FILE__, __LINE__, #e_test); \
    exit(1); \
  } \
} while (0)

#define ASSRT(assrt_cond) do { \
  if (! (assrt_cond)) { \
    fprintf(stderr, "ERROR [%s:%d]: assert '%s' failed\n", __FILE__, __LINE__, #assrt_cond); \
    exit(1); \
  } \
} while (0)


ERR_F lsim_create(lsim_t **rtn_lsim) {
  err_t *err;

  lsim_t *lsim;
  ERR_ASSRT(lsim = calloc(1, sizeof(lsim_t)), LSIM_ERR_NOMEM);

  *rtn_lsim = lsim;
  return ERR_OK;
}  /* lsim_create */


ERR_F lsim_delete(lsim_t *lsim) {
  free(lsim);

  return ERR_OK;
}  /* lsim_delete */

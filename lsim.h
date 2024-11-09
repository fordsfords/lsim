/* lsim.h - simple haslsim. */
/*
# This code and its documentation is Copyright 2024-2024 Steven Ford, http://geeky-boy.com
# and licensed "public domain" style under Creative Commons "CC0": http://creativecommons.org/publicdomain/zero/1.0/
# To the extent possible under law, the contributors to this project have
# waived all copyright and related or neighboring rights to this work.
# In other words, you can use this code for any purpose without any
# restrictions.  This work is published from: United States.  The project home
# is https://github.com/fordsfords/lsim
*/

#ifndef LSIM_H
#define LSIM_H

#include <stdint.h>
#include "err.h"
#include "hmap.h"

#ifdef __cplusplus
extern "C" {
#endif

#define LSIM_ERR_INTERNAL 1
#define LSIM_ERR_PARAM 2
#define LSIM_ERR_NOMEM 3

struct lsim_s {
  hmap_t *node_map;
};
typedef struct lsim_s lsim_t;

ERR_F lsim_create(lsim_t **rtn_lsim);
ERR_F lsim_delete(lsim_t *lsim);

#ifdef __cplusplus
}
#endif

#endif // LSIM_H

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
#include "hmap.h"

#ifdef __cplusplus
extern "C" {
#endif

struct lsim_s {
  hmap_t *node_map;
};
typedef struct lsim_s lsim_t;

int lsim_create(lsim_t **rtn_lsim, size_t table_size);

#ifdef __cplusplus
}
#endif

#endif // LSIM_H

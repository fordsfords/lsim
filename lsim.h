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

#ifdef LSIM_C
#  define ERR_CODE(err__code) ERR_API char *err__code = #err__code
#else
#  define ERR_CODE(err__code) ERR_API extern char *err__code
#endif
ERR_CODE(LSIM_ERR_INTERNAL);
ERR_CODE(LSIM_ERR_PARAM);
ERR_CODE(LSIM_ERR_NOMEM);
ERR_CODE(LSIM_ERR_COMMAND);
ERR_CODE(LSIM_ERR_NAME);
ERR_CODE(LSIM_ERR_EXIST);
ERR_CODE(LSIM_ERR_BADFILE);
ERR_CODE(LSIM_ERR_LINETOOLONG);
ERR_CODE(LSIM_ERR_MAXLOOPS);
#undef ERR_CODE


typedef struct lsim_s lsim_t;
typedef struct lsim_dev_s lsim_dev_t;  /* Generic device. */

/* Full definitions. */

struct lsim_s {
  cfg_t *cfg;
  hmap_t *devs;
  lsim_dev_t *out_changed_list;
  lsim_dev_t *in_changed_list;
  int stable;
  int quit;
};

ERR_F lsim_create(lsim_t **rtn_lsim, char *config_file_name);
ERR_F lsim_delete(lsim_t *lsim);

#ifdef __cplusplus
}
#endif

#endif // LSIM_H

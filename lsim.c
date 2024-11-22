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
#define LSIM_C
#include "lsim.h"
#include "lsim_cmd.h"


/* Config file definition and defaults. */
char *lsim_cfg_defaults[] = {
  "device_hash_buckets=10007",
  "max_propagate_cycles=5000",  /* For loop detection. */
  NULL
};


ERR_F lsim_create(lsim_t **rtn_lsim, char *config_file_name) {
  lsim_t *lsim;
  ERR_ASSRT(lsim = calloc(1, sizeof(lsim_t)), LSIM_ERR_NOMEM);

  ERR(cfg_create(&(lsim->cfg)));
  ERR(cfg_parse_string_list(lsim->cfg, CFG_MODE_ADD, lsim_cfg_defaults));
  if (config_file_name) {
    ERR(cfg_parse_file(lsim->cfg, CFG_MODE_UPDATE, config_file_name));
  }

  long device_hash_buckets;
  ERR(cfg_get_long_val(lsim->cfg, "device_hash_buckets", &device_hash_buckets));
  ERR(hmap_create(&(lsim->devs), device_hash_buckets));

  lsim->stable = 1;
  lsim->quit = 0;

  *rtn_lsim = lsim;
  return ERR_OK;
}  /* lsim_create */


ERR_F lsim_delete(lsim_t *lsim) {
printf("???delete cfg and devs\n");
  free(lsim);

  return ERR_OK;
}  /* lsim_delete */

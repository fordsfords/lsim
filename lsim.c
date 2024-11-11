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


char *lsim_cfg_defaults[] = {
  "device_hash_buckets=10007",
  NULL
};


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


ERR_F lsim_create(lsim_t **rtn_lsim, char *config_filename) {
  err_t *err;

  lsim_t *lsim;
  ERR_ASSRT(lsim = calloc(1, sizeof(lsim_t)), LSIM_ERR_NOMEM);

  ERR(cfg_create(&(lsim->cfg)));
  ERR(cfg_parse_string_list(lsim->cfg, CFG_MODE_ADD, lsim_cfg_defaults));
  if (config_filename) {
    ERR(cfg_parse_file(lsim->cfg, CFG_MODE_UPDATE, config_filename));
  }

  long device_hash_buckets;
  ERR(cfg_get_long_val(lsim->cfg, "device_hash_buckets", &device_hash_buckets));
  ERR(hmap_create(&(lsim->devs), device_hash_buckets));

  *rtn_lsim = lsim;
  return ERR_OK;
}  /* lsim_create */


ERR_F lsim_device_nand_create(lsim_t *lsim, char *name, int num_inputs) {
  device_t *dev;
  ERR_ASSRT(dev = calloc(1, sizeof(device_t)), LSIM_ERR_NOMEM);
  ERR_ASSRT(dev->name = strdup(name));
  dev->type = LSIM_DEV_TYPE_NAND;
  dev->nand.num_inputs = num_inputs;
  ERR_ASSRT(dev->nand.in_state = calloc(num_inputs, sizeof(int));

  ERR(hmap_write(lsim->devs, name, strlen(name), dev));

  return ERR_OK;
}  /* lsim_device_nand_create */


ERR_F lsim_delete(lsim_t *lsim) {
printf("???delete cfg and devs\n");
  free(lsim);

  return ERR_OK;
}  /* lsim_delete */

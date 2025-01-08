/* lsim_vcc.c */
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
#include <ctype.h>
#include "../err.h"
#include "../hmap.h"
#include "../cfg.h"
#include "../lsim.h"
#include "../lsim_dev.h"
#include "../lsim_devices.h"


ERR_F lsim_dev_vcc_get_out_terminal(lsim_t *lsim, lsim_dev_t *dev, const char *out_id, lsim_dev_out_terminal_t **out_terminal, int bit_offset) {
  (void)lsim;
  ERR_ASSRT(dev->type == LSIM_DEV_TYPE_VCC, LSIM_ERR_INTERNAL);

  ERR_ASSRT(bit_offset == 0, LSIM_ERR_COMMAND);  /* Only one output. */
  ERR_ASSRT(strcmp(out_id, "o0") == 0, LSIM_ERR_COMMAND);  /* Only one output. */
  *out_terminal = dev->vcc.o_terminal;

  return ERR_OK;
}  /* lsim_dev_vcc_get_out_terminal */


ERR_F lsim_dev_vcc_get_in_terminal(lsim_t *lsim, lsim_dev_t *dev, const char *in_id, lsim_dev_in_terminal_t **in_terminal, int bit_offset) {
  (void)lsim;  (void)in_id;  (void)in_terminal;  (void)bit_offset;
  ERR_ASSRT(dev->type == LSIM_DEV_TYPE_VCC, LSIM_ERR_INTERNAL);

  ERR_THROW(LSIM_ERR_COMMAND, "Attempt to get input state for vcc, which has no inputs");

  return ERR_OK;
}  /* lsim_dev_vcc_get_in_terminal */


ERR_F lsim_dev_vcc_power(lsim_t *lsim, lsim_dev_t *dev) {
  ERR_ASSRT(dev->type == LSIM_DEV_TYPE_VCC, LSIM_ERR_INTERNAL);

  dev->vcc.o_terminal->state = 0;  /* Starts out 0, "run_logic" sets it to 1. */
  ERR(lsim_dev_in_changed(lsim, dev));  /* Trigger to run the logic. */

  return ERR_OK;
}  /* lsim_dev_vcc_power */


ERR_F lsim_dev_vcc_run_logic(lsim_t *lsim, lsim_dev_t *dev) {
  ERR_ASSRT(dev->type == LSIM_DEV_TYPE_VCC, LSIM_ERR_INTERNAL);

  int out_changed = 0;
  if (dev->vcc.o_terminal->state == 0) {
    dev->vcc.o_terminal->state = 1;
    out_changed = 1;
  }
  if (out_changed) {
    ERR(lsim_dev_out_changed(lsim, dev));
  }

  int this_verbosity_level = dev->watch_level;
  if (lsim->verbosity_level > this_verbosity_level) {  /* Global verbosity level. */
    this_verbosity_level = lsim->verbosity_level;
  }
  if (this_verbosity_level == 2 || (this_verbosity_level == 1 && out_changed)) {
    printf("  vcc %s: o0=%d\n", dev->name, dev->vcc.o_terminal->state);
  }

  return ERR_OK;
}  /* lsim_dev_vcc_run_logic */


ERR_F lsim_dev_vcc_propagate_outputs(lsim_t *lsim, lsim_dev_t *dev) {
  ERR_ASSRT(dev->type == LSIM_DEV_TYPE_VCC, LSIM_ERR_INTERNAL);

  int out_state = dev->vcc.o_terminal->state;
  lsim_dev_in_terminal_t *dst_in_terminal = dev->vcc.o_terminal->in_terminal_list;

  while (dst_in_terminal) {
    if (dst_in_terminal->state != out_state) {
      dst_in_terminal->state = out_state;
      lsim_dev_t *dst_dev = dst_in_terminal->dev;
      ERR(lsim_dev_in_changed(lsim, dst_dev));
    }

    /* Propagate output to next connected device. */
    dst_in_terminal = dst_in_terminal->next_in_terminal;
  }

  return ERR_OK;
}  /* lsim_dev_vcc_propagate_outputs */


ERR_F lsim_dev_vcc_delete(lsim_t *lsim, lsim_dev_t *dev) {
  (void)lsim;
  ERR_ASSRT(dev->type == LSIM_DEV_TYPE_VCC, LSIM_ERR_INTERNAL);

  free(dev->vcc.o_terminal);

  free(dev->name);
  free(dev);

  return ERR_OK;
}  /* lsim_dev_vcc_delete */


ERR_F lsim_dev_vcc_create(lsim_t *lsim, char *dev_name) {
  /* Make sure name doesn't already exist. */
  err_t *err;
  err = hmap_slookup(lsim->devs, dev_name, NULL);
  ERR_ASSRT(err && err->code == HMAP_ERR_NOTFOUND, LSIM_ERR_EXIST);

  lsim_dev_t *dev;
  ERR(err_calloc((void **)&dev, 1, sizeof(lsim_dev_t)));
  ERR(err_strdup(&(dev->name), dev_name));
  dev->type = LSIM_DEV_TYPE_VCC;
  ERR(err_calloc((void **)&(dev->vcc.o_terminal), 1, sizeof(lsim_dev_out_terminal_t)));
  dev->vcc.o_terminal->dev = dev;

  /* Type-specific methods (inheritance). */
  dev->get_out_terminal = lsim_dev_vcc_get_out_terminal;
  dev->get_in_terminal = lsim_dev_vcc_get_in_terminal;
  dev->power = lsim_dev_vcc_power;
  dev->run_logic = lsim_dev_vcc_run_logic;
  dev->propagate_outputs = lsim_dev_vcc_propagate_outputs;
  dev->delete = lsim_dev_vcc_delete;

  ERR(hmap_swrite(lsim->devs, dev_name, dev));

  return ERR_OK;
}  /* lsim_dev_vcc_create */
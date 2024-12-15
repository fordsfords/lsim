/* lsim_nand.c */
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


ERR_F lsim_dev_nand_get_out_terminal(lsim_t *lsim, lsim_dev_t *dev, const char *out_id, lsim_dev_out_terminal_t **out_terminal) {
  (void)lsim;
  ERR_ASSRT(dev->type == LSIM_DEV_TYPE_NAND, LSIM_ERR_INTERNAL);

  ERR_ASSRT(strcmp(out_id, "o0") == 0, LSIM_ERR_COMMAND);  /* Only one output. */
  *out_terminal = dev->nand.out_terminal;

  return ERR_OK;
}  /* lsim_dev_nand_get_out_terminal */


ERR_F lsim_dev_nand_get_in_terminal(lsim_t *lsim, lsim_dev_t *dev, const char *in_id, lsim_dev_in_terminal_t **in_terminal) {
  (void)lsim;
  ERR_ASSRT(dev->type == LSIM_DEV_TYPE_NAND, LSIM_ERR_INTERNAL);

  ERR_ASSRT(in_id[0] == 'i', LSIM_ERR_COMMAND);
  long input_num;
  ERR(err_atol(in_id+1, &input_num));
  if (input_num >= dev->nand.num_inputs) {  /* Use throw instead of assert for more useful error message. */
    ERR_THROW(LSIM_ERR_COMMAND, "input number %d larger than number of inputs %d", (int)input_num, dev->nand.num_inputs);
  }

  *in_terminal = &dev->nand.in_terminals[input_num];

  return ERR_OK;
}  /* lsim_dev_nand_get_in_terminal */


ERR_F lsim_dev_nand_power(lsim_t *lsim, lsim_dev_t *dev) {
  ERR_ASSRT(dev->type == LSIM_DEV_TYPE_NAND, LSIM_ERR_INTERNAL);

  dev->nand.out_terminal->state = 0;

  int in_index;
  for (in_index = 0; in_index < dev->nand.num_inputs; in_index++) {
    dev->nand.in_terminals[in_index].state = 0;
  }
  ERR(lsim_dev_in_changed(lsim, dev));  /* Trigger to run the logic. */

  return ERR_OK;
}  /* lsim_dev_nand_power */


ERR_F lsim_dev_nand_run_logic(lsim_t *lsim, lsim_dev_t *dev) {
  ERR_ASSRT(dev->type == LSIM_DEV_TYPE_NAND, LSIM_ERR_INTERNAL);

  int new_output = 0;  /* Assume all inputs are 1. */
  int input_index;
  for (input_index = 0; input_index < dev->nand.num_inputs; input_index++) {
    /* Check for floating inputs. */
    if (dev->nand.in_terminals[input_index].driving_out_terminal == NULL) {
      ERR_THROW(LSIM_ERR_COMMAND, "Nand %s: input i%d is floating", dev->name, input_index);
    }
    if (dev->nand.in_terminals[input_index].state == 0) {
      /* At least one input is 0; output is 1. */
      new_output = 1;
      break;
    }
  }

  /* See if output changed. */
  int out_changed = 0;
  if (dev->nand.out_terminal->state != new_output) {
    dev->nand.out_terminal->state = new_output;
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
    printf("  nand %s: o0=%d\n", dev->name, dev->nand.out_terminal->state);
  }

  return ERR_OK;
}  /* lsim_dev_nand_run_logic */


ERR_F lsim_dev_nand_propagate_outputs(lsim_t *lsim, lsim_dev_t *dev) {
  ERR_ASSRT(dev->type == LSIM_DEV_TYPE_NAND, LSIM_ERR_INTERNAL);

  int out_state = dev->nand.out_terminal->state;
  lsim_dev_in_terminal_t *dst_in_terminal = dev->nand.out_terminal->in_terminal_list;

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
}  /* lsim_dev_nand_propagate_outputs */


ERR_F lsim_dev_nand_delete(lsim_t *lsim, lsim_dev_t *dev) {
  (void)lsim;
  ERR_ASSRT(dev->type == LSIM_DEV_TYPE_NAND, LSIM_ERR_INTERNAL);

  free(dev->nand.out_terminal);
  free(dev->nand.in_terminals);

  return ERR_OK;
}  /* lsim_dev_nand_delete */


ERR_F lsim_dev_nand_create(lsim_t *lsim, char *dev_name, long num_inputs) {
  ERR_ASSRT(num_inputs >= 1, LSIM_ERR_PARAM);

  /* Make sure name doesn't already exist. */
  err_t *err;
  err = hmap_slookup(lsim->devs, dev_name, NULL);
  ERR_ASSRT(err && err->code == HMAP_ERR_NOTFOUND, LSIM_ERR_EXIST);

  lsim_dev_t *dev;
  ERR(err_calloc((void **)&dev, 1, sizeof(lsim_dev_t)));
  ERR(err_strdup(&(dev->name), dev_name));
  dev->type = LSIM_DEV_TYPE_NAND;

  ERR(err_calloc((void **)&(dev->nand.out_terminal), 1, sizeof(lsim_dev_out_terminal_t)));
  dev->nand.out_terminal->dev = dev;
  dev->nand.num_inputs = num_inputs;
  ERR(err_calloc((void **)&(dev->nand.in_terminals), num_inputs, sizeof(lsim_dev_in_terminal_t)));

  int in_index;
  for (in_index = 0; in_index < dev->nand.num_inputs; in_index++) {
    dev->nand.in_terminals[in_index].dev = dev;
  }

  /* Type-specific methods (inheritance). */
  dev->get_out_terminal = lsim_dev_nand_get_out_terminal;
  dev->get_in_terminal = lsim_dev_nand_get_in_terminal;
  dev->power = lsim_dev_nand_power;
  dev->run_logic = lsim_dev_nand_run_logic;
  dev->propagate_outputs = lsim_dev_nand_propagate_outputs;
  dev->delete = lsim_dev_nand_delete;

  ERR(hmap_swrite(lsim->devs, dev_name, dev));

  return ERR_OK;
}  /* lsim_dev_nand_create */

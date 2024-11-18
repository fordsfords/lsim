/* lsim_cmd.c - logic simulator command processing. */
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
#include "err.h"
#include "hmap.h"
#include "cfg.h"
#include "lsim.h"
#include "lsim_dev.h"


ERR_F lsim_dev_vcc_get_input_state(lsim_t *lsim, lsim_dev_t *dev, const char *input_id, int **rtn_state_p) {
  ERR_ASSRT(dev->type == LSIM_DEV_TYPE_VCC, LSIM_ERR_INTERNAL);

  ERR_THROW(LSIM_ERR_COMMAND, "Attempt to get input state for vcc, which has no inputs");

  return ERR_OK;
}  /* lsim_dev_vcc_get_input_state */


ERR_F lsim_dev_vcc_get_output_wire(lsim_t *lsim, lsim_dev_t *dev, const char *output_id, lsim_wire_t **rtn_wire_p) {
  ERR_ASSRT(dev->type == LSIM_DEV_TYPE_VCC, LSIM_ERR_INTERNAL);

  /* There's only one output; ignore output_id. */
  *rtn_wire_p = &dev->vcc.out_wire;

  return ERR_OK;
}  /* lsim_dev_vcc_get_output_wire */


ERR_F lsim_dev_vcc_reset(lsim_t *lsim, lsim_dev_t *dev) {
  ERR_ASSRT(dev->type == LSIM_DEV_TYPE_VCC, LSIM_ERR_INTERNAL);

  dev->vcc.out_state = 1;
  dev->next_out_changed = lsim->out_changed_list;
  lsim->out_changed_list = dev;

  return ERR_OK;
}  /* lsim_dev_vcc_reset */


ERR_F lsim_dev_vcc_create(lsim_t *lsim, char *name) {
  /* Make sure name doesn't already exist. */
  err_t *err;
  err = hmap_lookup(lsim->devs, name, strlen(name), NULL);
  ERR_ASSRT(err && err->code == HMAP_ERR_NOTFOUND, LSIM_ERR_EXIST);

  lsim_dev_t *dev;
  ERR_ASSRT(dev = calloc(1, sizeof(lsim_dev_t)), LSIM_ERR_NOMEM);
  ERR_ASSRT(dev->name = strdup(name), LSIM_ERR_NOMEM);
  dev->type = LSIM_DEV_TYPE_VCC;
  dev->vcc.out_wire.src_device = dev;
  dev->get_input_state_p = lsim_dev_vcc_get_input_state;
  dev->get_output_wire_p = lsim_dev_vcc_get_output_wire;
  dev->reset = lsim_dev_vcc_reset;

  ERR(hmap_write(lsim->devs, name, strlen(name), dev));

  return ERR_OK;
}  /* lsim_dev_vcc_create */


/*******************************************************************************/


ERR_F lsim_dev_nand_get_input_state(lsim_t *lsim, lsim_dev_t *dev, const char *input_id, int **rtn_state_p) {
  ERR_ASSRT(dev->type == LSIM_DEV_TYPE_NAND, LSIM_ERR_INTERNAL);
  long input_num;
  ERR(cfg_atol(input_id, &input_num));
  if (input_num > dev->nand.num_inputs) {
    ERR_THROW(LSIM_ERR_COMMAND, "input number %d larger than number of inputs %d", (int)input_num, dev->nand.num_inputs);
  }

  *rtn_state_p = &dev->nand.in_states[input_num];

  return ERR_OK;
}  /* lsim_dev_nand_get_input_state */


ERR_F lsim_dev_nand_get_output_wire(lsim_t *lsim, lsim_dev_t *dev, const char *output_id, lsim_wire_t **rtn_wire_p) {
  ERR_ASSRT(dev->type == LSIM_DEV_TYPE_NAND, LSIM_ERR_INTERNAL);
  /* There's only one output; ignore output_id. */
  *rtn_wire_p = &dev->nand.out_wire;

  return ERR_OK;
}  /* lsim_dev_nand_get_output_wire */


ERR_F lsim_dev_nand_reset(lsim_t *lsim, lsim_dev_t *dev) {
  ERR_ASSRT(dev->type == LSIM_DEV_TYPE_NAND, LSIM_ERR_INTERNAL);

  int input_index;
  dev->nand.out_state = 0;
  for (input_index = 0; input_index < dev->nand.num_inputs; input_index++) {
    dev->nand.in_states[input_index] = 0;
  }
  dev->next_in_changed = lsim->in_changed_list;
  lsim->in_changed_list = dev;
  dev->next_out_changed = lsim->out_changed_list;
  lsim->out_changed_list = dev;

  return ERR_OK;
}  /* lsim_dev_nand_reset */


ERR_F lsim_dev_nand_create(lsim_t *lsim, char *name, long num_inputs) {
  ERR_ASSRT(num_inputs >= 1, LSIM_ERR_PARAM);

  /* Make sure name doesn't already exist. */
  err_t *err;
  err = hmap_lookup(lsim->devs, name, strlen(name), NULL);
  ERR_ASSRT(err && err->code == HMAP_ERR_NOTFOUND, LSIM_ERR_EXIST);

  lsim_dev_t *dev;
  ERR_ASSRT(dev = calloc(1, sizeof(lsim_dev_t)), LSIM_ERR_NOMEM);
  ERR_ASSRT(dev->name = strdup(name), LSIM_ERR_NOMEM);
  dev->type = LSIM_DEV_TYPE_NAND;
  dev->nand.out_wire.src_device = dev;
  dev->nand.num_inputs = num_inputs;
  ERR_ASSRT(dev->nand.in_states = calloc(num_inputs, sizeof(int)), LSIM_ERR_NOMEM);
  dev->get_input_state_p = lsim_dev_nand_get_input_state;
  dev->get_output_wire_p = lsim_dev_nand_get_output_wire;
  dev->reset = lsim_dev_nand_reset;

  ERR(hmap_write(lsim->devs, name, strlen(name), dev));

  return ERR_OK;
}  /* lsim_dev_nand_create */


/*******************************************************************************/


ERR_F lsim_dev_reset(lsim_t *lsim) {
  hmap_entry_t *dev_entry = NULL;
  do {
    ERR(hmap_next(lsim->devs, &dev_entry));
    if (dev_entry) {
      lsim_dev_t *cur_device = dev_entry->value;
      ERR(cur_device->reset(lsim, cur_device));
    }
  } while (dev_entry);

  return ERR_OK;
}  /* lsim_dev_reset */


ERR_F lsim_dev_run_logic(lsim_t *lsim) {
  ERR_ASSRT(lsim->out_changed_list == NULL, LSIM_ERR_INTERNAL);

  while (lsim->in_changed_list) {
    lsim_device_t *cur_device = lsim->in_changed_list;
    lsim->in_changed_list = cur_device->next_in_changed;
    cur_device->next_in_changed = NULL;

    int outputs_changed;
    ERR(cur_device->run_logic(lsim, cur_device, &outputs_changed));

    if (outputs_changed) {
      cur_device->next_out_changed = lsim->out_changed_list;
      lsim->out_changed_list = cur_device;
    }
  }  /* while in_changed_list */

  return ERR_OK;
}  /* lsim_dev_run_logic */


ERR_F lsim_dev_propogate_outputs(lsim_t *lsim) {
  ERR_ASSRT(lsim->in_changed_list == NULL, LSIM_ERR_INTERNAL);

  while (lsim->out_changed_list) {
    lsim_device_t *cur_device = lsim->out_changed_list;
    lsim->out_changed_list = cur_device->next_out_changed;
    cur_device->next_out_changed = NULL;

    ERR(cur_device->propogate_outputs(lsim, cur_device));
printf("???How do I follow each output's wire?\n");
  }  /* while out_changed_list */

  return ERR_OK;
}  /* lsim_dev_propogate_outputs */

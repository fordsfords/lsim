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


ERR_F lsim_dev_gnd_get_out_terminal(lsim_t *lsim, lsim_dev_t *dev, const char *out_id, lsim_dev_out_terminal_t **out_terminal) {
  ERR_ASSRT(dev->type == LSIM_DEV_TYPE_VCC, LSIM_ERR_INTERNAL);

  ERR_ASSRT(strcmp(out_id, "o0") == 0);  /* Only one output. */
  *out_terminal = &dev->gnd.out_terminal;

  return ERR_OK;
}  /* lsim_dev_gnd_get_out_terminal */


ERR_F lsim_dev_gnd_get_in_terminal(lsim_t *lsim, lsim_dev_t *dev, const char *in_id, lsim_dev_out_terminal_t **out_terminal) {
  ERR_ASSRT(dev->type == LSIM_DEV_TYPE_VCC, LSIM_ERR_INTERNAL);

  ERR_THROW(LSIM_ERR_COMMAND, "Attempt to get input state for gnd, which has no inputs");

  return ERR_OK;
}  /* lsim_dev_gnd_get_in_terminal */


ERR_F lsim_dev_gnd_reset(lsim_t *lsim, lsim_dev_t *dev) {
  ERR_ASSRT(dev->type == LSIM_DEV_TYPE_VCC, LSIM_ERR_INTERNAL);

  dev->gnd.out_state = 0;  /* Starts out 0, "run_logic" sets it to 1. */

  return ERR_OK;
}  /* lsim_dev_gnd_reset */


ERR_F lsim_dev_gnd_run_logic(lsim_t *lsim, lsim_dev_t *dev) {
  ERR_ASSRT(dev->type == LSIM_DEV_TYPE_VCC, LSIM_ERR_INTERNAL);

printf("???lsim_dev_gnd_run_logic: TBD\n");

  return ERR_OK;
}  /* lsim_dev_gnd_run_logic */


ERR_F lsim_dev_gnd_propogate_outputs(lsim_t *lsim, lsim_dev_t *dev) {
  ERR_ASSRT(dev->type == LSIM_DEV_TYPE_VCC, LSIM_ERR_INTERNAL);

printf("???lsim_dev_gnd_propogate_outputs: TBD\n");

  return ERR_OK;
}  /* lsim_dev_gnd_propogate_outputs */


ERR_F lsim_dev_gnd_create(lsim_t *lsim, char *dev_name) {
  /* Make sure name doesn't already exist. */
  err_t *err;
  err = hmap_lookup(lsim->devs, dev_name, strlen(dev_name), NULL);
  ERR_ASSRT(err && err->code == HMAP_ERR_NOTFOUND, LSIM_ERR_EXIST);

  lsim_dev_t *dev;
  ERR_ASSRT(dev = calloc(1, sizeof(lsim_dev_t)), LSIM_ERR_NOMEM);
  ERR_ASSRT(dev->name = strdup(dev_name), LSIM_ERR_NOMEM);
  dev->type = LSIM_DEV_TYPE_VCC;
  ERR_ASSRT(dev->gnd.out_terminal = calloc(1, sizeof(lsim_dev_out_terminal_t)), LSIM_ERR_NOMEM);
  dev->gnd.out_terminal->dev = dev

  /* Type-specific methods (inheritence). */
  dev->get_out_terminal = lsim_dev_gnd_get_out_terminal;
  dev->get_in_terminal = lsim_dev_gnd_get_in_terminal;
  dev->reset = lsim_dev_gnd_reset;
  dev->run_logic = lsim_dev_gnd_run_logic;
  dev->propogate_outputs = lsim_dev_gnd_propogate_outputs;

  ERR(hmap_write(lsim->devs, dev_name, strlen(dev_name), dev));

  return ERR_OK;
}  /* lsim_dev_gnd_create */


/*******************************************************************************/


ERR_F lsim_dev_vcc_get_out_terminal(lsim_t *lsim, lsim_dev_t *dev, const char *out_id, lsim_dev_out_terminal_t **out_terminal) {
  ERR_ASSRT(dev->type == LSIM_DEV_TYPE_VCC, LSIM_ERR_INTERNAL);

  ERR_ASSRT(strcmp(out_id, "o0") == 0);  /* Only one output. */
  *out_terminal = &dev->vcc.out_terminal;

  return ERR_OK;
}  /* lsim_dev_vcc_get_out_terminal */


ERR_F lsim_dev_vcc_get_in_terminal(lsim_t *lsim, lsim_dev_t *dev, const char *in_id, lsim_dev_out_terminal_t **out_terminal) {
  ERR_ASSRT(dev->type == LSIM_DEV_TYPE_VCC, LSIM_ERR_INTERNAL);

  ERR_THROW(LSIM_ERR_COMMAND, "Attempt to get input state for vcc, which has no inputs");

  return ERR_OK;
}  /* lsim_dev_vcc_get_in_terminal */


ERR_F lsim_dev_vcc_reset(lsim_t *lsim, lsim_dev_t *dev) {
  ERR_ASSRT(dev->type == LSIM_DEV_TYPE_VCC, LSIM_ERR_INTERNAL);

  dev->vcc.out_state = 0;  /* Starts out 0, "run_logic" sets it to 1. */

  return ERR_OK;
}  /* lsim_dev_vcc_reset */


ERR_F lsim_dev_vcc_run_logic(lsim_t *lsim, lsim_dev_t *dev) {
  ERR_ASSRT(dev->type == LSIM_DEV_TYPE_VCC, LSIM_ERR_INTERNAL);

printf("???lsim_dev_vcc_run_logic: TBD\n");

  return ERR_OK;
}  /* lsim_dev_vcc_run_logic */


ERR_F lsim_dev_vcc_propogate_outputs(lsim_t *lsim, lsim_dev_t *dev) {
  ERR_ASSRT(dev->type == LSIM_DEV_TYPE_VCC, LSIM_ERR_INTERNAL);

printf("???lsim_dev_vcc_propogate_outputs: TBD\n");

  return ERR_OK;
}  /* lsim_dev_vcc_propogate_outputs */


ERR_F lsim_dev_vcc_create(lsim_t *lsim, char *dev_name) {
  /* Make sure name doesn't already exist. */
  err_t *err;
  err = hmap_lookup(lsim->devs, dev_name, strlen(dev_name), NULL);
  ERR_ASSRT(err && err->code == HMAP_ERR_NOTFOUND, LSIM_ERR_EXIST);

  lsim_dev_t *dev;
  ERR_ASSRT(dev = calloc(1, sizeof(lsim_dev_t)), LSIM_ERR_NOMEM);
  ERR_ASSRT(dev->name = strdup(dev_name), LSIM_ERR_NOMEM);
  dev->type = LSIM_DEV_TYPE_VCC;
  ERR_ASSRT(dev->vcc.out_terminal = calloc(1, sizeof(lsim_dev_out_terminal_t)), LSIM_ERR_NOMEM);
  dev->vcc.out_terminal->dev = dev

  /* Type-specific methods (inheritence). */
  dev->get_out_terminal = lsim_dev_vcc_get_out_terminal;
  dev->get_in_terminal = lsim_dev_vcc_get_in_terminal;
  dev->reset = lsim_dev_vcc_reset;
  dev->run_logic = lsim_dev_vcc_run_logic;
  dev->propogate_outputs = lsim_dev_vcc_propogate_outputs;

  ERR(hmap_write(lsim->devs, dev_name, strlen(dev_name), dev));

  return ERR_OK;
}  /* lsim_dev_vcc_create */


/*******************************************************************************/


ERR_F lsim_dev_led_get_out_terminal(lsim_t *lsim, lsim_dev_t *dev, const char *out_id, lsim_dev_out_terminal_t **out_terminal) {
  ERR_ASSRT(dev->type == LSIM_DEV_TYPE_LED, LSIM_ERR_INTERNAL);

  ERR_THROW(LSIM_ERR_COMMAND, "Attempt to get output state for led, which has no outputs");

  return ERR_OK;
}  /* lsim_dev_led_get_out_terminal */


ERR_F lsim_dev_led_get_in_terminal(lsim_t *lsim, lsim_dev_t *dev, const char *in_id, lsim_dev_out_terminal_t **in_terminal) {
  ERR_ASSRT(dev->type == LSIM_DEV_TYPE_LED, LSIM_ERR_INTERNAL);

  ERR_ASSRT(strcmp(in_id, "i0") == 0);  /* Only one input. */
  *in_terminal = &dev->led.in_terminal;

  return ERR_OK;
}  /* lsim_dev_led_get_in_terminal */


ERR_F lsim_dev_led_reset(lsim_t *lsim, lsim_dev_t *dev) {
  ERR_ASSRT(dev->type == LSIM_DEV_TYPE_LED, LSIM_ERR_INTERNAL);

  dev->led.in_terminal->state = 0;

  return ERR_OK;
}  /* lsim_dev_led_reset */


ERR_F lsim_dev_led_run_logic(lsim_t *lsim, lsim_dev_t *dev) {
  ERR_ASSRT(dev->type == LSIM_DEV_TYPE_LED, LSIM_ERR_INTERNAL);

printf("???lsim_dev_led_run_logic: TBD\n");

  return ERR_OK;
}  /* lsim_dev_led_run_logic */


ERR_F lsim_dev_led_propogate_outputs(lsim_t *lsim, lsim_dev_t *dev) {
  ERR_ASSRT(dev->type == LSIM_DEV_TYPE_LED, LSIM_ERR_INTERNAL);

printf("???lsim_dev_led_propogate_outputs: TBD\n");

  return ERR_OK;
}  /* lsim_dev_led_propogate_outputs */


ERR_F lsim_dev_led_create(lsim_t *lsim, char *dev_name) {
  /* Make sure name doesn't already exist. */
  err_t *err;
  err = hmap_lookup(lsim->devs, dev_name, strlen(dev_name), NULL);
  ERR_ASSRT(err && err->code == HMAP_ERR_NOTFOUND, LSIM_ERR_EXIST);

  lsim_dev_t *dev;
  ERR_ASSRT(dev = calloc(1, sizeof(lsim_dev_t)), LSIM_ERR_NOMEM);
  ERR_ASSRT(dev->name = strdup(dev_name), LSIM_ERR_NOMEM);
  dev->type = LSIM_DEV_TYPE_LED;
  ERR_ASSRT(dev->led.in_terminal = calloc(1, sizeof(lsim_dev_in_terminal_t)), LSIM_ERR_NOMEM);
  dev->led.in_terminal->dev = dev

  /* Type-specific methods (inheritence). */
  dev->get_out_terminal = lsim_dev_led_get_out_terminal;
  dev->get_in_terminal = lsim_dev_led_get_in_terminal;
  dev->reset = lsim_dev_led_reset;
  dev->run_logic = lsim_dev_led_run_logic;
  dev->propogate_outputs = lsim_dev_led_propogate_outputs;

  ERR(hmap_write(lsim->devs, dev_name, strlen(dev_name), dev));

  return ERR_OK;
}  /* lsim_dev_led_create */


/*******************************************************************************/


ERR_F lsim_dev_nand_get_out_terminal(lsim_t *lsim, lsim_dev_t *dev, const char *out_id, lsim_dev_out_terminal_t **out_terminal) {
  ERR_ASSRT(dev->type == LSIM_DEV_TYPE_NAND, LSIM_ERR_INTERNAL);

  ERR_ASSRT(strcmp(out_id, "o0") == 0);  /* Only one output. */
  *out_terminal = &dev->nand.out_terminal;

  return ERR_OK;
}  /* lsim_dev_nand_get_out_terminal */


ERR_F lsim_dev_nand_get_in_terminal(lsim_t *lsim, lsim_dev_t *dev, const char *in_id, lsim_dev_in_terminal_t **in_terminal) {
  ERR_ASSRT(dev->type == LSIM_DEV_TYPE_NAND, LSIM_ERR_INTERNAL);

  ERR_ASSRT(in_id[0] == 'i');
  long input_num;
  ERR(cfg_atol(in_id+1, &input_num));
  if (input_num > dev->nand.num_inputs) {  /* Use throw instead of assert for more useful error message. */
    ERR_THROW(LSIM_ERR_COMMAND, "input number %d larger than number of inputs %d", (int)input_num, dev->nand.num_inputs);
  }

  *in_terminal = &dev->nand.in_terminals[input_num];

  return ERR_OK;
}  /* lsim_dev_nand_get_in_terminal */


ERR_F lsim_dev_nand_reset(lsim_t *lsim, lsim_dev_t *dev) {
  ERR_ASSRT(dev->type == LSIM_DEV_TYPE_NAND, LSIM_ERR_INTERNAL);

  dev->nand.out_terminal.state = 0;

  int input_index;
  for (input_index = 0; input_index < dev->nand.num_inputs; input_index++) {
    dev->nand.in_terminals[input_index] = 0;
  }

  return ERR_OK;
}  /* lsim_dev_nand_reset */


ERR_F lsim_dev_nand_run_logic(lsim_t *lsim, lsim_dev_t *dev) {
  ERR_ASSRT(dev->type == LSIM_DEV_TYPE_NAND, LSIM_ERR_INTERNAL);

printf("???lsim_dev_nand_run_logic: TBD\n");

  return ERR_OK;
}  /* lsim_dev_nand_run_logic */


ERR_F lsim_dev_nand_propogate_outputs(lsim_t *lsim, lsim_dev_t *dev) {
  ERR_ASSRT(dev->type == LSIM_DEV_TYPE_NAND, LSIM_ERR_INTERNAL);

printf("???lsim_dev_nand_propogate_outputs: TBD\n");

  return ERR_OK;
}  /* lsim_dev_nand_propogate_outputs */


ERR_F lsim_dev_nand_create(lsim_t *lsim, char *dev_name, long num_inputs) {
  ERR_ASSRT(num_inputs >= 1, LSIM_ERR_PARAM);

  /* Make sure name doesn't already exist. */
  err_t *err;
  err = hmap_lookup(lsim->devs, dev_name, strlen(dev_name), NULL);
  ERR_ASSRT(err && err->code == HMAP_ERR_NOTFOUND, LSIM_ERR_EXIST);

  lsim_dev_t *dev;
  ERR_ASSRT(dev = calloc(1, sizeof(lsim_dev_t)), LSIM_ERR_NOMEM);
  ERR_ASSRT(dev->name = strdup(dev_name), LSIM_ERR_NOMEM);
  dev->type = LSIM_DEV_TYPE_NAND;
  dev->nand.out_wire.src_device = dev;
  ERR_ASSRT(dev->nand.out_terminal = calloc(1, sizeof(lsim_dev_out_terminal_t)), LSIM_ERR_NOMEM);
  dev->nand.num_inputs = num_inputs;
  ERR_ASSRT(dev->nand.in_terminals = calloc(num_inputs, sizeof(lsim_dev_in_terminal_t)), LSIM_ERR_NOMEM);

  /* Type-specific methods (inheritence). */
  dev->get_out_terminal = lsim_dev_nand_get_out_terminal;
  dev->get_in_terminal = lsim_dev_nand_get_in_terminal;
  dev->reset = lsim_dev_nand_reset;
  dev->run_logic = lsim_dev_nand_run_logic;
  dev->propogate_outputs = lsim_dev_nand_propogate_outputs;

  ERR(hmap_write(lsim->devs, dev_name, strlen(dev_name), dev));

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


ERR_F lsim_dev_connect(lsim_t *lsim, const char *src_dev_name, const char *src_out_id, const char *dst_dev_name, const char *dst_in_id) {
  lsim_dev_t *src_dev;
  ERR(hmap_lookup(lsim->devs, src_dev_name, strlen(src_dev_name), &src_dev);
  lsim_dev_t *dst_dev;
  ERR(hmap_lookup(lsim->devs, dst_dev_name, strlen(dst_dev_name), &dst_dev);

  lsim_dev_out_terminal_t *src_out_terminal;
  ERR(src_dev->get_out_terminal(lsim, dev, src_out_id, &src_out_terminal));
  lsim_dev_in_terminal_t *dst_in_terminal;
  ERR(dst_dev->get_in_terminal(lsim, dev, dst_in_id, &dst_in_terminal));

  ERR_ASSRT(dst_in_terminal->driving_out_terminal == NULL);  /* Can't drive it twice. */

  dst_in_terminal->driving_out_terminal = src_out_terminal;
  dst_in_terminal->next_in_terminal = src_out_terminal->in_terminal_list;
  src_out_terminal->in_terminal_list = dst_in_terminal;

  return ERR_OK;
}  /* lsim_dev_connect */

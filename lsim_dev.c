/* lsim_dev.c - logic simulator command processing. */
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


ERR_F lsim_dev_out_changed(lsim_t *lsim, lsim_dev_t *dev) {
  /* If not already on the output changed list, add it. */
  if (! dev->out_changed) {
    dev->out_changed = 1;
    dev->next_out_changed = lsim->out_changed_list;
    lsim->out_changed_list = dev;
  }

  return ERR_OK;
}  /* lsim_dev_out_changed */


ERR_F lsim_dev_in_changed(lsim_t *lsim, lsim_dev_t *dev) {
  /* If not already on the input changed list, add it. */
  if (! dev->in_changed) {
    dev->in_changed = 1;
    dev->next_in_changed = lsim->in_changed_list;
    lsim->in_changed_list = dev;
  }

  return ERR_OK;
}  /* lsim_dev_in_changed */


/*******************************************************************************/


ERR_F lsim_dev_gnd_get_out_terminal(lsim_t *lsim, lsim_dev_t *dev, const char *out_id, lsim_dev_out_terminal_t **out_terminal) {
  ERR_ASSRT(dev->type == LSIM_DEV_TYPE_GND, LSIM_ERR_INTERNAL);

  ERR_ASSRT(strcmp(out_id, "o0") == 0, LSIM_ERR_COMMAND);  /* Only one output. */
  *out_terminal = dev->gnd.out_terminal;

  return ERR_OK;
}  /* lsim_dev_gnd_get_out_terminal */


ERR_F lsim_dev_gnd_get_in_terminal(lsim_t *lsim, lsim_dev_t *dev, const char *in_id, lsim_dev_in_terminal_t **in_terminal) {
  ERR_ASSRT(dev->type == LSIM_DEV_TYPE_GND, LSIM_ERR_INTERNAL);

  ERR_THROW(LSIM_ERR_COMMAND, "Attempt to get input state for gnd, which has no inputs");

  return ERR_OK;
}  /* lsim_dev_gnd_get_in_terminal */


ERR_F lsim_dev_gnd_reset(lsim_t *lsim, lsim_dev_t *dev) {
  ERR_ASSRT(dev->type == LSIM_DEV_TYPE_GND, LSIM_ERR_INTERNAL);

  dev->gnd.out_terminal->state = 0;
  ERR(lsim_dev_in_changed(lsim, dev));  /* Trigger to run the logic. */

  return ERR_OK;
}  /* lsim_dev_gnd_reset */


ERR_F lsim_dev_gnd_run_logic(lsim_t *lsim, lsim_dev_t *dev) {
  ERR_ASSRT(dev->type == LSIM_DEV_TYPE_GND, LSIM_ERR_INTERNAL);

  if (dev->gnd.out_terminal->state == 1) {
    dev->gnd.out_terminal->state = 0;
    ERR(lsim_dev_out_changed(lsim, dev));
    if (lsim->trace_level > 0 || dev->watch_level == 1) {
      printf("  gnd %s: o0=%d\n", dev->name, dev->gnd.out_terminal->state);
    }
  }
  if (dev->watch_level == 2) {
    printf("  gnd %s: o0=%d\n", dev->name, dev->gnd.out_terminal->state);
  }

  return ERR_OK;
}  /* lsim_dev_gnd_run_logic */


ERR_F lsim_dev_gnd_propagate_outputs(lsim_t *lsim, lsim_dev_t *dev) {
  ERR_ASSRT(dev->type == LSIM_DEV_TYPE_GND, LSIM_ERR_INTERNAL);

  int out_state = dev->gnd.out_terminal->state;
  lsim_dev_in_terminal_t *dst_in_terminal = dev->gnd.out_terminal->in_terminal_list;

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
}  /* lsim_dev_gnd_propagate_outputs */


ERR_F lsim_dev_gnd_create(lsim_t *lsim, char *dev_name) {
  /* Make sure name doesn't already exist. */
  err_t *err;
  err = hmap_lookup(lsim->devs, dev_name, strlen(dev_name), NULL);
  ERR_ASSRT(err && err->code == HMAP_ERR_NOTFOUND, LSIM_ERR_EXIST);

  lsim_dev_t *dev;
  ERR_ASSRT(dev = calloc(1, sizeof(lsim_dev_t)), LSIM_ERR_NOMEM);
  ERR_ASSRT(dev->name = strdup(dev_name), LSIM_ERR_NOMEM);
  dev->type = LSIM_DEV_TYPE_GND;
  ERR_ASSRT(dev->gnd.out_terminal = calloc(1, sizeof(lsim_dev_out_terminal_t)), LSIM_ERR_NOMEM);
  dev->gnd.out_terminal->dev = dev;

  /* Type-specific methods (inheritence). */
  dev->get_out_terminal = lsim_dev_gnd_get_out_terminal;
  dev->get_in_terminal = lsim_dev_gnd_get_in_terminal;
  dev->reset = lsim_dev_gnd_reset;
  dev->run_logic = lsim_dev_gnd_run_logic;
  dev->propagate_outputs = lsim_dev_gnd_propagate_outputs;

  ERR(hmap_write(lsim->devs, dev_name, strlen(dev_name), dev));

  return ERR_OK;
}  /* lsim_dev_gnd_create */


/*******************************************************************************/


ERR_F lsim_dev_vcc_get_out_terminal(lsim_t *lsim, lsim_dev_t *dev, const char *out_id, lsim_dev_out_terminal_t **out_terminal) {
  ERR_ASSRT(dev->type == LSIM_DEV_TYPE_VCC, LSIM_ERR_INTERNAL);

  ERR_ASSRT(strcmp(out_id, "o0") == 0, LSIM_ERR_COMMAND);  /* Only one output. */
  *out_terminal = dev->vcc.out_terminal;

  return ERR_OK;
}  /* lsim_dev_vcc_get_out_terminal */


ERR_F lsim_dev_vcc_get_in_terminal(lsim_t *lsim, lsim_dev_t *dev, const char *in_id, lsim_dev_in_terminal_t **in_terminal) {
  ERR_ASSRT(dev->type == LSIM_DEV_TYPE_VCC, LSIM_ERR_INTERNAL);

  ERR_THROW(LSIM_ERR_COMMAND, "Attempt to get input state for vcc, which has no inputs");

  return ERR_OK;
}  /* lsim_dev_vcc_get_in_terminal */


ERR_F lsim_dev_vcc_reset(lsim_t *lsim, lsim_dev_t *dev) {
  ERR_ASSRT(dev->type == LSIM_DEV_TYPE_VCC, LSIM_ERR_INTERNAL);

  dev->vcc.out_terminal->state = 0;  /* Starts out 0, "run_logic" sets it to 1. */
  ERR(lsim_dev_in_changed(lsim, dev));  /* Trigger to run the logic. */

  return ERR_OK;
}  /* lsim_dev_vcc_reset */


ERR_F lsim_dev_vcc_run_logic(lsim_t *lsim, lsim_dev_t *dev) {
  ERR_ASSRT(dev->type == LSIM_DEV_TYPE_VCC, LSIM_ERR_INTERNAL);

  if (dev->vcc.out_terminal->state == 0) {
    dev->vcc.out_terminal->state = 1;
    ERR(lsim_dev_out_changed(lsim, dev));
    if (lsim->trace_level > 0 || dev->watch_level == 1) {
      printf("  vcc %s: o0=%d\n", dev->name, dev->vcc.out_terminal->state);
    }
  }
  if (dev->watch_level == 2) {
    printf("  vcc %s: o0=%d\n", dev->name, dev->vcc.out_terminal->state);
  }

  return ERR_OK;
}  /* lsim_dev_vcc_run_logic */


ERR_F lsim_dev_vcc_propagate_outputs(lsim_t *lsim, lsim_dev_t *dev) {
  ERR_ASSRT(dev->type == LSIM_DEV_TYPE_VCC, LSIM_ERR_INTERNAL);

  int out_state = dev->vcc.out_terminal->state;
  lsim_dev_in_terminal_t *dst_in_terminal = dev->vcc.out_terminal->in_terminal_list;

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
  dev->vcc.out_terminal->dev = dev;

  /* Type-specific methods (inheritence). */
  dev->get_out_terminal = lsim_dev_vcc_get_out_terminal;
  dev->get_in_terminal = lsim_dev_vcc_get_in_terminal;
  dev->reset = lsim_dev_vcc_reset;
  dev->run_logic = lsim_dev_vcc_run_logic;
  dev->propagate_outputs = lsim_dev_vcc_propagate_outputs;

  ERR(hmap_write(lsim->devs, dev_name, strlen(dev_name), dev));

  return ERR_OK;
}  /* lsim_dev_vcc_create */


/*******************************************************************************/


ERR_F lsim_dev_swtch_get_out_terminal(lsim_t *lsim, lsim_dev_t *dev, const char *out_id, lsim_dev_out_terminal_t **out_terminal) {
  ERR_ASSRT(dev->type == LSIM_DEV_TYPE_SWTCH, LSIM_ERR_INTERNAL);

  ERR_ASSRT(strcmp(out_id, "o0") == 0, LSIM_ERR_COMMAND);  /* Only one output. */
  *out_terminal = dev->swtch.out_terminal;

  return ERR_OK;
}  /* lsim_dev_swtch_get_out_terminal */


ERR_F lsim_dev_swtch_get_in_terminal(lsim_t *lsim, lsim_dev_t *dev, const char *in_id, lsim_dev_in_terminal_t **in_terminal) {
  ERR_ASSRT(dev->type == LSIM_DEV_TYPE_SWTCH, LSIM_ERR_INTERNAL);

  ERR_THROW(LSIM_ERR_COMMAND, "Attempt to get input state for swtch, which has no inputs");

  return ERR_OK;
}  /* lsim_dev_swtch_get_in_terminal */


ERR_F lsim_dev_swtch_reset(lsim_t *lsim, lsim_dev_t *dev) {
  ERR_ASSRT(dev->type == LSIM_DEV_TYPE_SWTCH, LSIM_ERR_INTERNAL);

  dev->swtch.out_terminal->state = 0;
  ERR(lsim_dev_in_changed(lsim, dev));  /* Trigger to run the logic. */

  return ERR_OK;
}  /* lsim_dev_swtch_reset */


ERR_F lsim_dev_swtch_run_logic(lsim_t *lsim, lsim_dev_t *dev) {
  ERR_ASSRT(dev->type == LSIM_DEV_TYPE_SWTCH, LSIM_ERR_INTERNAL);

  if (dev->swtch.out_terminal->state != dev->swtch.swtch_state) {
    dev->swtch.out_terminal->state = dev->swtch.swtch_state;
    ERR(lsim_dev_out_changed(lsim, dev));
    if (lsim->trace_level > 0 || dev->watch_level == 1) {
      printf("  swtch %s: o0=%d\n", dev->name, dev->swtch.out_terminal->state);
    }
  }
  if (dev->watch_level == 2) {
    printf("  swtch %s: o0=%d\n", dev->name, dev->swtch.out_terminal->state);
  }

  return ERR_OK;
}  /* lsim_dev_swtch_run_logic */


ERR_F lsim_dev_swtch_propagate_outputs(lsim_t *lsim, lsim_dev_t *dev) {
  ERR_ASSRT(dev->type == LSIM_DEV_TYPE_SWTCH, LSIM_ERR_INTERNAL);

  int out_state = dev->swtch.out_terminal->state;
  lsim_dev_in_terminal_t *dst_in_terminal = dev->swtch.out_terminal->in_terminal_list;

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
}  /* lsim_dev_swtch_propagate_outputs */


ERR_F lsim_dev_swtch_create(lsim_t *lsim, char *dev_name, int init_state) {
  /* Make sure name doesn't already exist. */
  err_t *err;
  err = hmap_lookup(lsim->devs, dev_name, strlen(dev_name), NULL);
  ERR_ASSRT(err && err->code == HMAP_ERR_NOTFOUND, LSIM_ERR_EXIST);

  lsim_dev_t *dev;
  ERR_ASSRT(dev = calloc(1, sizeof(lsim_dev_t)), LSIM_ERR_NOMEM);
  ERR_ASSRT(dev->name = strdup(dev_name), LSIM_ERR_NOMEM);
  dev->type = LSIM_DEV_TYPE_SWTCH;
  ERR_ASSRT(dev->swtch.out_terminal = calloc(1, sizeof(lsim_dev_out_terminal_t)), LSIM_ERR_NOMEM);
  dev->swtch.out_terminal->dev = dev;
  dev->swtch.swtch_state = init_state;

  /* Type-specific methods (inheritence). */
  dev->get_out_terminal = lsim_dev_swtch_get_out_terminal;
  dev->get_in_terminal = lsim_dev_swtch_get_in_terminal;
  dev->reset = lsim_dev_swtch_reset;
  dev->run_logic = lsim_dev_swtch_run_logic;
  dev->propagate_outputs = lsim_dev_swtch_propagate_outputs;

  ERR(hmap_write(lsim->devs, dev_name, strlen(dev_name), dev));

  return ERR_OK;
}  /* lsim_dev_swtch_create */


/*******************************************************************************/


ERR_F lsim_dev_led_get_out_terminal(lsim_t *lsim, lsim_dev_t *dev, const char *out_id, lsim_dev_out_terminal_t **out_terminal) {
  ERR_ASSRT(dev->type == LSIM_DEV_TYPE_LED, LSIM_ERR_INTERNAL);

  ERR_THROW(LSIM_ERR_COMMAND, "Attempt to get output state for led, which has no outputs");

  return ERR_OK;
}  /* lsim_dev_led_get_out_terminal */


ERR_F lsim_dev_led_get_in_terminal(lsim_t *lsim, lsim_dev_t *dev, const char *in_id, lsim_dev_in_terminal_t **in_terminal) {
  ERR_ASSRT(dev->type == LSIM_DEV_TYPE_LED, LSIM_ERR_INTERNAL);

  ERR_ASSRT(strcmp(in_id, "i0") == 0, LSIM_ERR_COMMAND);  /* Only one input. */
  *in_terminal = dev->led.in_terminal;

  return ERR_OK;
}  /* lsim_dev_led_get_in_terminal */


ERR_F lsim_dev_led_reset(lsim_t *lsim, lsim_dev_t *dev) {
  ERR_ASSRT(dev->type == LSIM_DEV_TYPE_LED, LSIM_ERR_INTERNAL);

  dev->led.illuminated = 0;
  dev->led.in_terminal->state = 0;
  ERR(lsim_dev_in_changed(lsim, dev));  /* Trigger to run the logic. */

  return ERR_OK;
}  /* lsim_dev_led_reset */


ERR_F lsim_dev_led_run_logic(lsim_t *lsim, lsim_dev_t *dev) {
  ERR_ASSRT(dev->type == LSIM_DEV_TYPE_LED, LSIM_ERR_INTERNAL);

  if (dev->led.in_terminal->state != dev->led.illuminated) {
    dev->led.illuminated = dev->led.in_terminal->state;
    printf("Led %s: %s\n", dev->name, dev->led.illuminated ? "on" : "off");
  }

  return ERR_OK;
}  /* lsim_dev_led_run_logic */


ERR_F lsim_dev_led_propagate_outputs(lsim_t *lsim, lsim_dev_t *dev) {
  ERR_ASSRT(dev->type == LSIM_DEV_TYPE_LED, LSIM_ERR_INTERNAL);

  /* No output to propagate. */

  return ERR_OK;
}  /* lsim_dev_led_propagate_outputs */


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
  dev->led.in_terminal->dev = dev;

  /* Type-specific methods (inheritence). */
  dev->get_out_terminal = lsim_dev_led_get_out_terminal;
  dev->get_in_terminal = lsim_dev_led_get_in_terminal;
  dev->reset = lsim_dev_led_reset;
  dev->run_logic = lsim_dev_led_run_logic;
  dev->propagate_outputs = lsim_dev_led_propagate_outputs;

  ERR(hmap_write(lsim->devs, dev_name, strlen(dev_name), dev));

  return ERR_OK;
}  /* lsim_dev_led_create */


/*******************************************************************************/


ERR_F lsim_dev_nand_get_out_terminal(lsim_t *lsim, lsim_dev_t *dev, const char *out_id, lsim_dev_out_terminal_t **out_terminal) {
  ERR_ASSRT(dev->type == LSIM_DEV_TYPE_NAND, LSIM_ERR_INTERNAL);

  ERR_ASSRT(strcmp(out_id, "o0") == 0, LSIM_ERR_COMMAND);  /* Only one output. */
  *out_terminal = dev->nand.out_terminal;

  return ERR_OK;
}  /* lsim_dev_nand_get_out_terminal */


ERR_F lsim_dev_nand_get_in_terminal(lsim_t *lsim, lsim_dev_t *dev, const char *in_id, lsim_dev_in_terminal_t **in_terminal) {
  ERR_ASSRT(dev->type == LSIM_DEV_TYPE_NAND, LSIM_ERR_INTERNAL);

  ERR_ASSRT(in_id[0] == 'i', LSIM_ERR_COMMAND);
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

  dev->nand.out_terminal->state = 0;

  int in_index;
  for (in_index = 0; in_index < dev->nand.num_inputs; in_index++) {
    dev->nand.in_terminals[in_index].state = 0;
  }
  ERR(lsim_dev_in_changed(lsim, dev));  /* Trigger to run the logic. */

  return ERR_OK;
}  /* lsim_dev_nand_reset */


ERR_F lsim_dev_nand_run_logic(lsim_t *lsim, lsim_dev_t *dev) {
  ERR_ASSRT(dev->type == LSIM_DEV_TYPE_NAND, LSIM_ERR_INTERNAL);

  int new_output = 0;  /* Assume all inputs are 1. */
  int input_index;
  for (input_index = 0; input_index < dev->nand.num_inputs; input_index++) {
    if (dev->nand.in_terminals[input_index].state == 0) {
      /* At least one input is 0; output is 1. */
      new_output = 1;
      break;
    }
  }

  /* See if output changed. */
  if (dev->nand.out_terminal->state != new_output) {
    dev->nand.out_terminal->state = new_output;
    ERR(lsim_dev_out_changed(lsim, dev));  /* Trigger to run the logic. */
    if (lsim->trace_level > 0 || dev->watch_level == 1) {
      printf("  nand %s: o0=%d\n", dev->name, dev->nand.out_terminal->state);
    }
  }
  if (dev->watch_level == 2) {
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

  ERR_ASSRT(dev->nand.out_terminal = calloc(1, sizeof(lsim_dev_out_terminal_t)), LSIM_ERR_NOMEM);
  dev->nand.out_terminal->dev = dev;
  dev->nand.num_inputs = num_inputs;
  ERR_ASSRT(dev->nand.in_terminals = calloc(num_inputs, sizeof(lsim_dev_in_terminal_t)), LSIM_ERR_NOMEM);

  int in_index;
  for (in_index = 0; in_index < dev->nand.num_inputs; in_index++) {
    dev->nand.in_terminals[in_index].dev = dev;
  }

  /* Type-specific methods (inheritence). */
  dev->get_out_terminal = lsim_dev_nand_get_out_terminal;
  dev->get_in_terminal = lsim_dev_nand_get_in_terminal;
  dev->reset = lsim_dev_nand_reset;
  dev->run_logic = lsim_dev_nand_run_logic;
  dev->propagate_outputs = lsim_dev_nand_propagate_outputs;

  ERR(hmap_write(lsim->devs, dev_name, strlen(dev_name), dev));

  return ERR_OK;
}  /* lsim_dev_nand_create */


/*******************************************************************************/


ERR_F lsim_dev_connect(lsim_t *lsim, const char *src_dev_name, const char *src_out_id, const char *dst_dev_name, const char *dst_in_id) {
  lsim_dev_t *src_dev;
  ERR(hmap_lookup(lsim->devs, src_dev_name, strlen(src_dev_name), (void**)&src_dev));
  lsim_dev_t *dst_dev;
  ERR(hmap_lookup(lsim->devs, dst_dev_name, strlen(dst_dev_name), (void**)&dst_dev));

  lsim_dev_out_terminal_t *src_out_terminal;
  ERR(src_dev->get_out_terminal(lsim, src_dev, src_out_id, &src_out_terminal));
  lsim_dev_in_terminal_t *dst_in_terminal;
  ERR(dst_dev->get_in_terminal(lsim, dst_dev, dst_in_id, &dst_in_terminal));

  ERR_ASSRT(dst_in_terminal->driving_out_terminal == NULL, LSIM_ERR_COMMAND);  /* Can't drive it twice. */

  dst_in_terminal->driving_out_terminal = src_out_terminal;
  dst_in_terminal->next_in_terminal = src_out_terminal->in_terminal_list;
  src_out_terminal->in_terminal_list = dst_in_terminal;

  return ERR_OK;
}  /* lsim_dev_connect */


ERR_F lsim_dev_reset(lsim_t *lsim) {
  /* Step through entire hash map, starting with first entry. */
  hmap_entry_t *dev_entry = NULL;
  do {
    ERR(hmap_next(lsim->devs, &dev_entry));
    if (dev_entry) {
      lsim_dev_t *cur_dev = dev_entry->value;
      ERR_ASSRT(cur_dev->next_out_changed == NULL, LSIM_ERR_INTERNAL);
      ERR_ASSRT(cur_dev->next_in_changed == NULL, LSIM_ERR_INTERNAL);
      ERR(cur_dev->reset(lsim, cur_dev));
    }
  } while (dev_entry);

  return ERR_OK;
}  /* lsim_dev_reset */


ERR_F lsim_dev_move(lsim_t *lsim, char *name, long new_state) {
  lsim_dev_t *dev;
  ERR(hmap_lookup(lsim->devs, name, strlen(name), (void**)&dev));

  ERR_ASSRT(dev->type == LSIM_DEV_TYPE_SWTCH, LSIM_ERR_COMMAND);

  if (dev->swtch.swtch_state != new_state) {
    dev->swtch.swtch_state = new_state;
    ERR(lsim_dev_in_changed(lsim, dev));  /* Trigger to run the logic. */
  }

  return ERR_OK;
}  /* lsim_dev_move */


ERR_F lsim_dev_run_logic(lsim_t *lsim) {
  /* When starting to run the logic, output changed list should be empty. */
  ERR_ASSRT(lsim->out_changed_list == NULL, LSIM_ERR_INTERNAL);

  /* This loop visits every device on the "in_changed_list". Note that the
   * "run_logic" function does not add devices to that list (it adds
   * them to "out_changed_list"). So this can't loop infinitely. */
  while (lsim->in_changed_list) {
    /* Remove from input changed list. */
    lsim_dev_t *cur_dev = lsim->in_changed_list;
    lsim->in_changed_list = cur_dev->next_in_changed;
    cur_dev->next_in_changed = NULL;
    cur_dev->in_changed = 0;

    ERR(cur_dev->run_logic(lsim, cur_dev));
  }  /* while in_changed_list */

  return ERR_OK;
}  /* lsim_dev_run_logic */


ERR_F lsim_dev_propagate_outputs(lsim_t *lsim) {
  ERR_ASSRT(lsim->in_changed_list == NULL, LSIM_ERR_INTERNAL);

  /* This loop visits every device on the "out_changed_list". Note that the
   * "propogate_outputs" function does not add devices to that list (it adds
   * them to "in_changed_list"). So this can't loop infinitely. */
  while (lsim->out_changed_list) {
    /* Remove from output changed list. */
    lsim_dev_t *cur_dev = lsim->out_changed_list;
    lsim->out_changed_list = cur_dev->next_out_changed;
    cur_dev->next_out_changed = NULL;
    cur_dev->out_changed = 0;

    ERR(cur_dev->propagate_outputs(lsim, cur_dev));
  }  /* while out_changed_list */

  return ERR_OK;
}  /* lsim_dev_propagate_outputs */


ERR_F lsim_dev_watch(lsim_t *lsim, const char *dev_name, int watch_level) {
  lsim_dev_t *dev;
  ERR(hmap_lookup(lsim->devs, dev_name, strlen(dev_name), (void**)&dev));

  dev->watch_level = watch_level;

  return ERR_OK;
}  /* lsim_dev_watch */


ERR_F lsim_dev_step(lsim_t *lsim) {
  long max_propagate_cycles;
  ERR(cfg_get_long_val(lsim->cfg, "max_propagate_cycles", &max_propagate_cycles));

  long cur_cycle = 0;
  /* Loop while the logic states are still stabilizing. Note that this can
   * loop infinitely (e.g. a NAND oscillator), so the "max_propagate_cycles"
   * configuration parameter limits the loop count. */
  while (lsim->in_changed_list) {
    cur_cycle++;
    /* Prevent infinite loops. */
    ERR_ASSRT(cur_cycle <= max_propagate_cycles, LSIM_ERR_MAXLOOPS);

    ERR(lsim_dev_run_logic(lsim));
    ERR(lsim_dev_propagate_outputs(lsim));
  }

  return ERR_OK;
}  /* lsim_dev_step */

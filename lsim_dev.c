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
  (void)lsim;
  ERR_ASSRT(dev->type == LSIM_DEV_TYPE_GND, LSIM_ERR_INTERNAL);

  ERR_ASSRT(strcmp(out_id, "o0") == 0, LSIM_ERR_COMMAND);  /* Only one output. */
  *out_terminal = dev->gnd.out_terminal;

  return ERR_OK;
}  /* lsim_dev_gnd_get_out_terminal */


ERR_F lsim_dev_gnd_get_in_terminal(lsim_t *lsim, lsim_dev_t *dev, const char *in_id, lsim_dev_in_terminal_t **in_terminal) {
  (void)lsim;  (void)in_id;  (void)in_terminal;
  ERR_ASSRT(dev->type == LSIM_DEV_TYPE_GND, LSIM_ERR_INTERNAL);

  ERR_THROW(LSIM_ERR_COMMAND, "Attempt to get input state for gnd, which has no inputs");

  return ERR_OK;
}  /* lsim_dev_gnd_get_in_terminal */


ERR_F lsim_dev_gnd_power(lsim_t *lsim, lsim_dev_t *dev) {
  ERR_ASSRT(dev->type == LSIM_DEV_TYPE_GND, LSIM_ERR_INTERNAL);

  dev->gnd.out_terminal->state = 0;
  ERR(lsim_dev_in_changed(lsim, dev));  /* Trigger to run the logic. */

  return ERR_OK;
}  /* lsim_dev_gnd_power */


ERR_F lsim_dev_gnd_run_logic(lsim_t *lsim, lsim_dev_t *dev) {
  ERR_ASSRT(dev->type == LSIM_DEV_TYPE_GND, LSIM_ERR_INTERNAL);

  int out_changed = 0;
  if (dev->gnd.out_terminal->state == 1) {
    dev->gnd.out_terminal->state = 0;
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


ERR_F lsim_dev_gnd_delete(lsim_t *lsim, lsim_dev_t *dev) {
  (void)lsim;
  ERR_ASSRT(dev->type == LSIM_DEV_TYPE_GND, LSIM_ERR_INTERNAL);

  free(dev->gnd.out_terminal);

  return ERR_OK;
}  /* lsim_dev_gnd_delete */


ERR_F lsim_dev_gnd_create(lsim_t *lsim, char *dev_name) {
  /* Make sure name doesn't already exist. */
  err_t *err;
  err = hmap_lookup(lsim->devs, dev_name, strlen(dev_name), NULL);
  ERR_ASSRT(err && err->code == HMAP_ERR_NOTFOUND, LSIM_ERR_EXIST);

  lsim_dev_t *dev;
  ERR(err_calloc((void **)&dev, 1, sizeof(lsim_dev_t)));
  ERR(err_strdup(&(dev->name), dev_name));
  dev->type = LSIM_DEV_TYPE_GND;
  ERR(err_calloc((void **)&(dev->gnd.out_terminal), 1, sizeof(lsim_dev_out_terminal_t)));
  dev->gnd.out_terminal->dev = dev;

  /* Type-specific methods (inheritance). */
  dev->get_out_terminal = lsim_dev_gnd_get_out_terminal;
  dev->get_in_terminal = lsim_dev_gnd_get_in_terminal;
  dev->power = lsim_dev_gnd_power;
  dev->run_logic = lsim_dev_gnd_run_logic;
  dev->propagate_outputs = lsim_dev_gnd_propagate_outputs;
  dev->delete = lsim_dev_gnd_delete;

  ERR(hmap_write(lsim->devs, dev_name, strlen(dev_name), dev));

  return ERR_OK;
}  /* lsim_dev_gnd_create */


/*******************************************************************************/


ERR_F lsim_dev_vcc_get_out_terminal(lsim_t *lsim, lsim_dev_t *dev, const char *out_id, lsim_dev_out_terminal_t **out_terminal) {
  (void)lsim;
  ERR_ASSRT(dev->type == LSIM_DEV_TYPE_VCC, LSIM_ERR_INTERNAL);

  ERR_ASSRT(strcmp(out_id, "o0") == 0, LSIM_ERR_COMMAND);  /* Only one output. */
  *out_terminal = dev->vcc.out_terminal;

  return ERR_OK;
}  /* lsim_dev_vcc_get_out_terminal */


ERR_F lsim_dev_vcc_get_in_terminal(lsim_t *lsim, lsim_dev_t *dev, const char *in_id, lsim_dev_in_terminal_t **in_terminal) {
  (void)lsim;  (void)in_id;  (void)in_terminal;
  ERR_ASSRT(dev->type == LSIM_DEV_TYPE_VCC, LSIM_ERR_INTERNAL);

  ERR_THROW(LSIM_ERR_COMMAND, "Attempt to get input state for vcc, which has no inputs");

  return ERR_OK;
}  /* lsim_dev_vcc_get_in_terminal */


ERR_F lsim_dev_vcc_power(lsim_t *lsim, lsim_dev_t *dev) {
  ERR_ASSRT(dev->type == LSIM_DEV_TYPE_VCC, LSIM_ERR_INTERNAL);

  dev->vcc.out_terminal->state = 0;  /* Starts out 0, "run_logic" sets it to 1. */
  ERR(lsim_dev_in_changed(lsim, dev));  /* Trigger to run the logic. */

  return ERR_OK;
}  /* lsim_dev_vcc_power */


ERR_F lsim_dev_vcc_run_logic(lsim_t *lsim, lsim_dev_t *dev) {
  ERR_ASSRT(dev->type == LSIM_DEV_TYPE_VCC, LSIM_ERR_INTERNAL);

  int out_changed = 0;
  if (dev->vcc.out_terminal->state == 0) {
    dev->vcc.out_terminal->state = 1;
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


ERR_F lsim_dev_vcc_delete(lsim_t *lsim, lsim_dev_t *dev) {
  (void)lsim;
  ERR_ASSRT(dev->type == LSIM_DEV_TYPE_VCC, LSIM_ERR_INTERNAL);

  free(dev->vcc.out_terminal);

  return ERR_OK;
}  /* lsim_dev_vcc_delete */


ERR_F lsim_dev_vcc_create(lsim_t *lsim, char *dev_name) {
  /* Make sure name doesn't already exist. */
  err_t *err;
  err = hmap_lookup(lsim->devs, dev_name, strlen(dev_name), NULL);
  ERR_ASSRT(err && err->code == HMAP_ERR_NOTFOUND, LSIM_ERR_EXIST);

  lsim_dev_t *dev;
  ERR(err_calloc((void **)&dev, 1, sizeof(lsim_dev_t)));
  ERR(err_strdup(&(dev->name), dev_name));
  dev->type = LSIM_DEV_TYPE_VCC;
  ERR(err_calloc((void **)&(dev->vcc.out_terminal), 1, sizeof(lsim_dev_out_terminal_t)));
  dev->vcc.out_terminal->dev = dev;

  /* Type-specific methods (inheritance). */
  dev->get_out_terminal = lsim_dev_vcc_get_out_terminal;
  dev->get_in_terminal = lsim_dev_vcc_get_in_terminal;
  dev->power = lsim_dev_vcc_power;
  dev->run_logic = lsim_dev_vcc_run_logic;
  dev->propagate_outputs = lsim_dev_vcc_propagate_outputs;
  dev->delete = lsim_dev_vcc_delete;

  ERR(hmap_write(lsim->devs, dev_name, strlen(dev_name), dev));

  return ERR_OK;
}  /* lsim_dev_vcc_create */


/*******************************************************************************/


ERR_F lsim_dev_swtch_get_out_terminal(lsim_t *lsim, lsim_dev_t *dev, const char *out_id, lsim_dev_out_terminal_t **out_terminal) {
  (void)lsim;
  ERR_ASSRT(dev->type == LSIM_DEV_TYPE_SWTCH, LSIM_ERR_INTERNAL);

  ERR_ASSRT(strcmp(out_id, "o0") == 0, LSIM_ERR_COMMAND);  /* Only one output. */
  *out_terminal = dev->swtch.out_terminal;

  return ERR_OK;
}  /* lsim_dev_swtch_get_out_terminal */


ERR_F lsim_dev_swtch_get_in_terminal(lsim_t *lsim, lsim_dev_t *dev, const char *in_id, lsim_dev_in_terminal_t **in_terminal) {
  (void)lsim;  (void)in_id;  (void)in_terminal;
  ERR_ASSRT(dev->type == LSIM_DEV_TYPE_SWTCH, LSIM_ERR_INTERNAL);

  ERR_THROW(LSIM_ERR_COMMAND, "Attempt to get input state for swtch, which has no inputs");

  return ERR_OK;
}  /* lsim_dev_swtch_get_in_terminal */


ERR_F lsim_dev_swtch_power(lsim_t *lsim, lsim_dev_t *dev) {
  ERR_ASSRT(dev->type == LSIM_DEV_TYPE_SWTCH, LSIM_ERR_INTERNAL);

  dev->swtch.out_terminal->state = 0;
  ERR(lsim_dev_in_changed(lsim, dev));  /* Trigger to run the logic. */

  return ERR_OK;
}  /* lsim_dev_swtch_power */


ERR_F lsim_dev_swtch_run_logic(lsim_t *lsim, lsim_dev_t *dev) {
  ERR_ASSRT(dev->type == LSIM_DEV_TYPE_SWTCH, LSIM_ERR_INTERNAL);

  int out_changed = 0;
  if (dev->swtch.out_terminal->state != dev->swtch.swtch_state) {
    dev->swtch.out_terminal->state = dev->swtch.swtch_state;
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


ERR_F lsim_dev_swtch_delete(lsim_t *lsim, lsim_dev_t *dev) {
  (void)lsim;
  ERR_ASSRT(dev->type == LSIM_DEV_TYPE_SWTCH, LSIM_ERR_INTERNAL);

  free(dev->swtch.out_terminal);

  return ERR_OK;
}  /* lsim_dev_swtch_delete */


ERR_F lsim_dev_swtch_create(lsim_t *lsim, char *dev_name, int init_state) {
  /* Make sure name doesn't already exist. */
  err_t *err;
  err = hmap_lookup(lsim->devs, dev_name, strlen(dev_name), NULL);
  ERR_ASSRT(err && err->code == HMAP_ERR_NOTFOUND, LSIM_ERR_EXIST);

  lsim_dev_t *dev;
  ERR(err_calloc((void **)&dev, 1, sizeof(lsim_dev_t)));
  ERR(err_strdup(&(dev->name), dev_name));
  dev->type = LSIM_DEV_TYPE_SWTCH;
  ERR(err_calloc((void **)&(dev->swtch.out_terminal), 1, sizeof(lsim_dev_out_terminal_t)));
  dev->swtch.out_terminal->dev = dev;
  dev->swtch.swtch_state = init_state;

  /* Type-specific methods (inheritance). */
  dev->get_out_terminal = lsim_dev_swtch_get_out_terminal;
  dev->get_in_terminal = lsim_dev_swtch_get_in_terminal;
  dev->power = lsim_dev_swtch_power;
  dev->run_logic = lsim_dev_swtch_run_logic;
  dev->propagate_outputs = lsim_dev_swtch_propagate_outputs;
  dev->delete = lsim_dev_swtch_delete;

  ERR(hmap_write(lsim->devs, dev_name, strlen(dev_name), dev));

  return ERR_OK;
}  /* lsim_dev_swtch_create */


/*******************************************************************************/


ERR_F lsim_dev_clk1_get_out_terminal(lsim_t *lsim, lsim_dev_t *dev, const char *out_id, lsim_dev_out_terminal_t **out_terminal) {
  (void)lsim;
  ERR_ASSRT(dev->type == LSIM_DEV_TYPE_CLK1, LSIM_ERR_INTERNAL);

  if (strcmp(out_id, "q0") == 0) {
    *out_terminal = dev->clk1.q_terminal;
  }
  else if (strcmp(out_id, "Q0") == 0) {
    *out_terminal = dev->clk1.Q_terminal;
  }
  else { ERR_THROW(LSIM_ERR_COMMAND, "clk1 outputs are q0 and Q0, not '%s'", out_id); }

  return ERR_OK;
}  /* lsim_dev_clk1_get_out_terminal */


ERR_F lsim_dev_clk1_get_in_terminal(lsim_t *lsim, lsim_dev_t *dev, const char *in_id, lsim_dev_in_terminal_t **in_terminal) {
  (void)lsim;
  ERR_ASSRT(dev->type == LSIM_DEV_TYPE_CLK1, LSIM_ERR_INTERNAL);

  ERR_ASSRT(strcmp(in_id, "R0") == 0, LSIM_ERR_COMMAND);  /* Only one input. */

  *in_terminal = dev->clk1.Reset_terminal;

  return ERR_OK;
}  /* lsim_dev_clk1_get_in_terminal */


ERR_F lsim_dev_clk1_power(lsim_t *lsim, lsim_dev_t *dev) {
  (void)lsim;
  ERR_ASSRT(dev->type == LSIM_DEV_TYPE_CLK1, LSIM_ERR_INTERNAL);

  dev->clk1.Reset_terminal->state = 0;
  dev->clk1.q_terminal->state = 0;
  dev->clk1.Q_terminal->state = 0;
  /* Don't add clk1 to in_changed list because the logic is run explicitly. */

  return ERR_OK;
}  /* lsim_dev_clk1_power */


ERR_F lsim_dev_clk1_run_logic(lsim_t *lsim, lsim_dev_t *dev) {
  ERR_ASSRT(dev->type == LSIM_DEV_TYPE_CLK1, LSIM_ERR_INTERNAL);
  /* Check for floating inputs. */
  if (dev->clk1.Reset_terminal->driving_out_terminal == NULL) {
    ERR_THROW(LSIM_ERR_COMMAND, "Clock %s: input R0 is floating", dev->name);
  }

  int out_changed = 0;

  /* Process reset. */
  if (dev->clk1.Reset_terminal->state == 0) {
    if (dev->clk1.q_terminal->state != 0) {
      out_changed = 1;
    }
    dev->clk1.q_terminal->state = 0;
    dev->clk1.Q_terminal->state = 0;
  }
  else {  /* Not reset. */
    int new_state = lsim->total_ticks % 2;  /* Clock changes with each tick. */
    if (dev->clk1.q_terminal->state != new_state || dev->clk1.Q_terminal->state != (1 - new_state)) {
      out_changed = 1;
      dev->clk1.q_terminal->state = new_state;
      dev->clk1.Q_terminal->state = 1 - new_state;  /* Invert. */
    }
  } 
  if (out_changed) {
    ERR(lsim_dev_out_changed(lsim, dev));
  }

  int this_verbosity_level = dev->watch_level;
  if (lsim->verbosity_level > this_verbosity_level) {  /* Global verbosity level. */
    this_verbosity_level = lsim->verbosity_level;
  }
  if (this_verbosity_level == 2 || (this_verbosity_level == 1 && out_changed)) {
    printf("  clk1 %s: q0=%d, Q0=%d\n", dev->name, dev->clk1.q_terminal->state, dev->clk1.Q_terminal->state);
  }

  return ERR_OK;
}  /* lsim_dev_clk1_run_logic */


ERR_F lsim_dev_clk1_propagate_outputs(lsim_t *lsim, lsim_dev_t *dev) {
  ERR_ASSRT(dev->type == LSIM_DEV_TYPE_CLK1, LSIM_ERR_INTERNAL);

  int out_state = dev->clk1.q_terminal->state;
  lsim_dev_in_terminal_t *dst_in_terminal = dev->clk1.q_terminal->in_terminal_list;

  while (dst_in_terminal) {
    if (dst_in_terminal->state != out_state) {
      dst_in_terminal->state = out_state;
      lsim_dev_t *dst_dev = dst_in_terminal->dev;
      ERR(lsim_dev_in_changed(lsim, dst_dev));
    }

    /* Propagate output to next connected device. */
    dst_in_terminal = dst_in_terminal->next_in_terminal;
  }

  out_state = dev->clk1.Q_terminal->state;
  dst_in_terminal = dev->clk1.Q_terminal->in_terminal_list;

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
}  /* lsim_dev_clk1_propagate_outputs */


ERR_F lsim_dev_clk1_delete(lsim_t *lsim, lsim_dev_t *dev) {
  (void)lsim;
  ERR_ASSRT(dev->type == LSIM_DEV_TYPE_CLK1, LSIM_ERR_INTERNAL);

  free(dev->clk1.q_terminal);
  free(dev->clk1.Q_terminal);
  free(dev->clk1.Reset_terminal);

  return ERR_OK;
}  /* lsim_dev_clk1_delete */


ERR_F lsim_dev_clk1_create(lsim_t *lsim, char *dev_name) {
  /* Make sure name doesn't already exist. */
  err_t *err;
  err = hmap_lookup(lsim->devs, dev_name, strlen(dev_name), NULL);
  ERR_ASSRT(err && err->code == HMAP_ERR_NOTFOUND, LSIM_ERR_EXIST);

  ERR_ASSRT(lsim->active_clk_dev == NULL, LSIM_ERR_COMMAND);  /* Can't have multiple clocks. */

  lsim_dev_t *dev;
  ERR(err_calloc((void **)&dev, 1, sizeof(lsim_dev_t)));
  ERR(err_strdup(&(dev->name), dev_name));
  dev->type = LSIM_DEV_TYPE_CLK1;
  ERR(err_calloc((void **)&(dev->clk1.q_terminal), 1, sizeof(lsim_dev_out_terminal_t)));
  dev->clk1.q_terminal->dev = dev;
  ERR(err_calloc((void **)&(dev->clk1.Q_terminal), 1, sizeof(lsim_dev_out_terminal_t)));
  dev->clk1.Q_terminal->dev = dev;
  ERR(err_calloc((void **)&(dev->clk1.Reset_terminal), 1, sizeof(lsim_dev_in_terminal_t)));
  dev->clk1.Reset_terminal->dev = dev;

  /* Type-specific methods (inheritance). */
  dev->get_out_terminal = lsim_dev_clk1_get_out_terminal;
  dev->get_in_terminal = lsim_dev_clk1_get_in_terminal;
  dev->power = lsim_dev_clk1_power;
  dev->run_logic = lsim_dev_clk1_run_logic;
  dev->propagate_outputs = lsim_dev_clk1_propagate_outputs;
  dev->delete = lsim_dev_clk1_delete;

  ERR(hmap_write(lsim->devs, dev_name, strlen(dev_name), dev));

  lsim->active_clk_dev = dev;  /* Make clock visible to lsim_dev_tick. */

  return ERR_OK;
}  /* lsim_dev_clk1_create */


/*******************************************************************************/


ERR_F lsim_dev_led_get_out_terminal(lsim_t *lsim, lsim_dev_t *dev, const char *out_id, lsim_dev_out_terminal_t **out_terminal) {
  (void)lsim;  (void)out_id;  (void)out_terminal;
  ERR_ASSRT(dev->type == LSIM_DEV_TYPE_LED, LSIM_ERR_INTERNAL);

  ERR_THROW(LSIM_ERR_COMMAND, "Attempt to get output state for led, which has no outputs");

  return ERR_OK;
}  /* lsim_dev_led_get_out_terminal */


ERR_F lsim_dev_led_get_in_terminal(lsim_t *lsim, lsim_dev_t *dev, const char *in_id, lsim_dev_in_terminal_t **in_terminal) {
  (void)lsim;
  ERR_ASSRT(dev->type == LSIM_DEV_TYPE_LED, LSIM_ERR_INTERNAL);

  ERR_ASSRT(strcmp(in_id, "i0") == 0, LSIM_ERR_COMMAND);  /* Only one input. */
  *in_terminal = dev->led.in_terminal;

  return ERR_OK;
}  /* lsim_dev_led_get_in_terminal */


ERR_F lsim_dev_led_power(lsim_t *lsim, lsim_dev_t *dev) {
  ERR_ASSRT(dev->type == LSIM_DEV_TYPE_LED, LSIM_ERR_INTERNAL);

  dev->led.illuminated = 0;
  dev->led.in_terminal->state = 0;
  ERR(lsim_dev_in_changed(lsim, dev));  /* Trigger to run the logic. */

  return ERR_OK;
}  /* lsim_dev_led_power */


ERR_F lsim_dev_led_run_logic(lsim_t *lsim, lsim_dev_t *dev) {
  (void)lsim;
  ERR_ASSRT(dev->type == LSIM_DEV_TYPE_LED, LSIM_ERR_INTERNAL);
  /* Check for floating inputs. */
  if (dev->led.in_terminal->driving_out_terminal == NULL) {
    ERR_THROW(LSIM_ERR_COMMAND, "Led %s: input i0 is floating", dev->name);
  }

  if (dev->led.in_terminal->state != dev->led.illuminated) {
    dev->led.illuminated = dev->led.in_terminal->state;
    printf("Led %s: %s (tick %ld)\n", dev->name, dev->led.illuminated ? "on" : "off", lsim->total_ticks);
  }

  return ERR_OK;
}  /* lsim_dev_led_run_logic */


ERR_F lsim_dev_led_propagate_outputs(lsim_t *lsim, lsim_dev_t *dev) {
  (void)lsim;
  ERR_ASSRT(dev->type == LSIM_DEV_TYPE_LED, LSIM_ERR_INTERNAL);

  /* No output to propagate. */

  return ERR_OK;
}  /* lsim_dev_led_propagate_outputs */


ERR_F lsim_dev_led_delete(lsim_t *lsim, lsim_dev_t *dev) {
  (void)lsim;
  ERR_ASSRT(dev->type == LSIM_DEV_TYPE_LED, LSIM_ERR_INTERNAL);

  free(dev->led.in_terminal);

  return ERR_OK;
}  /* lsim_dev_led_delete */


ERR_F lsim_dev_led_create(lsim_t *lsim, char *dev_name) {
  /* Make sure name doesn't already exist. */
  err_t *err;
  err = hmap_lookup(lsim->devs, dev_name, strlen(dev_name), NULL);
  ERR_ASSRT(err && err->code == HMAP_ERR_NOTFOUND, LSIM_ERR_EXIST);

  lsim_dev_t *dev;
  ERR(err_calloc((void **)&dev, 1, sizeof(lsim_dev_t)));
  ERR(err_strdup(&(dev->name), dev_name));
  dev->type = LSIM_DEV_TYPE_LED;
  ERR(err_calloc((void **)&(dev->led.in_terminal), 1, sizeof(lsim_dev_in_terminal_t)));
  dev->led.in_terminal->dev = dev;

  /* Type-specific methods (inheritance). */
  dev->get_out_terminal = lsim_dev_led_get_out_terminal;
  dev->get_in_terminal = lsim_dev_led_get_in_terminal;
  dev->power = lsim_dev_led_power;
  dev->run_logic = lsim_dev_led_run_logic;
  dev->propagate_outputs = lsim_dev_led_propagate_outputs;
  dev->delete = lsim_dev_led_delete;

  ERR(hmap_write(lsim->devs, dev_name, strlen(dev_name), dev));

  return ERR_OK;
}  /* lsim_dev_led_create */


/*******************************************************************************/


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
  ERR(cfg_atol(in_id+1, &input_num));
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
  err = hmap_lookup(lsim->devs, dev_name, strlen(dev_name), NULL);
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

  ERR(hmap_write(lsim->devs, dev_name, strlen(dev_name), dev));

  return ERR_OK;
}  /* lsim_dev_nand_create */


/*******************************************************************************/


ERR_F lsim_dev_srlatch_get_out_terminal(lsim_t *lsim, lsim_dev_t *dev, const char *out_id, lsim_dev_out_terminal_t **out_terminal) {
  (void)lsim;
  ERR_ASSRT(dev->type == LSIM_DEV_TYPE_SRLATCH, LSIM_ERR_INTERNAL);

  if (strcmp(out_id, "q0") == 0) {
    *out_terminal = dev->srlatch.q_terminal;
  }
  else if (strcmp(out_id, "Q0") == 0) {
    *out_terminal = dev->srlatch.Q_terminal;
  }
  else ERR_THROW(LSIM_ERR_INTERNAL, "unrecognized out_id '%s'", out_id);

  return ERR_OK;
}  /* lsim_dev_srlatch_get_out_terminal */


ERR_F lsim_dev_srlatch_get_in_terminal(lsim_t *lsim, lsim_dev_t *dev, const char *in_id, lsim_dev_in_terminal_t **in_terminal) {
  (void)lsim;
  ERR_ASSRT(dev->type == LSIM_DEV_TYPE_SRLATCH, LSIM_ERR_INTERNAL);

  if (strcmp(in_id, "S0") == 0) {
    *in_terminal = dev->srlatch.S_terminal;
  }
  else if (strcmp(in_id, "R0") == 0) {
    *in_terminal = dev->srlatch.R_terminal;
  }
  else ERR_THROW(LSIM_ERR_INTERNAL, "unrecognized in_id '%s'", in_id);

  return ERR_OK;
}  /* lsim_dev_srlatch_get_in_terminal */


ERR_F lsim_dev_srlatch_power(lsim_t *lsim, lsim_dev_t *dev) {
  (void)lsim;
  ERR_ASSRT(dev->type == LSIM_DEV_TYPE_SRLATCH, LSIM_ERR_INTERNAL);

  return ERR_OK;
}  /* lsim_dev_srlatch_power */


ERR_F lsim_dev_srlatch_run_logic(lsim_t *lsim, lsim_dev_t *dev) {
  (void)lsim;
  ERR_ASSRT(dev->type == LSIM_DEV_TYPE_SRLATCH, LSIM_ERR_INTERNAL);

  ERR_THROW(LSIM_ERR_INTERNAL, "run logic should not be called for srlatch");

  return ERR_OK;
}  /* lsim_dev_srlatch_run_logic */


ERR_F lsim_dev_srlatch_propagate_outputs(lsim_t *lsim, lsim_dev_t *dev) {
  (void)lsim;
  ERR_ASSRT(dev->type == LSIM_DEV_TYPE_SRLATCH, LSIM_ERR_INTERNAL);

  ERR_THROW(LSIM_ERR_INTERNAL, "propagate outputs should not be called for srlatch");

  return ERR_OK;
}  /* lsim_dev_srlatch_propagate_outputs */


ERR_F lsim_dev_srlatch_delete(lsim_t *lsim, lsim_dev_t *dev) {
  (void)lsim;
  ERR_ASSRT(dev->type == LSIM_DEV_TYPE_SRLATCH, LSIM_ERR_INTERNAL);

  return ERR_OK;
}  /* lsim_dev_srlatch_delete */


ERR_F lsim_dev_srlatch_create(lsim_t *lsim, char *dev_name) {
  /* Make sure name doesn't already exist. */
  err_t *err;
  err = hmap_lookup(lsim->devs, dev_name, strlen(dev_name), NULL);
  ERR_ASSRT(err && err->code == HMAP_ERR_NOTFOUND, LSIM_ERR_EXIST);

  lsim_dev_t *dev;
  ERR(err_calloc((void **)&dev, 1, sizeof(lsim_dev_t)));
  ERR(err_strdup(&(dev->name), dev_name));
  dev->type = LSIM_DEV_TYPE_SRLATCH;

  char *nand_q_name;
  ERR(err_asprintf(&nand_q_name, "%s.nand_q", dev_name));
  ERR(lsim_dev_nand_create(lsim, nand_q_name, 2));
  lsim_dev_t *nand_q_dev;
  ERR(hmap_lookup(lsim->devs, nand_q_name, strlen(nand_q_name), (void**)&nand_q_dev));

  char *nand_Q_name;
  ERR(err_asprintf(&nand_Q_name, "%s.nand_Q", dev_name));
  ERR(lsim_dev_nand_create(lsim, nand_Q_name, 2));
  lsim_dev_t *nand_Q_dev;
  ERR(hmap_lookup(lsim->devs, nand_Q_name, strlen(nand_Q_name), (void**)&nand_Q_dev));

  /* Make connections. */
  ERR(lsim_dev_connect(lsim, nand_q_name, "o0", nand_Q_name, "i1"));
  ERR(lsim_dev_connect(lsim, nand_Q_name, "o0", nand_q_name, "i1"));

  /* Save the "external" terminals. */
  dev->srlatch.q_terminal = nand_q_dev->nand.out_terminal;
  dev->srlatch.Q_terminal = nand_Q_dev->nand.out_terminal;
  dev->srlatch.S_terminal = &nand_q_dev->nand.in_terminals[0];
  dev->srlatch.R_terminal = &nand_Q_dev->nand.in_terminals[0];

  /* Type-specific methods (inheritance). */
  dev->get_out_terminal = lsim_dev_srlatch_get_out_terminal;
  dev->get_in_terminal = lsim_dev_srlatch_get_in_terminal;
  dev->power = lsim_dev_srlatch_power;
  dev->run_logic = lsim_dev_srlatch_run_logic;
  dev->propagate_outputs = lsim_dev_srlatch_propagate_outputs;
  dev->delete = lsim_dev_srlatch_delete;

  ERR(hmap_write(lsim->devs, dev_name, strlen(dev_name), dev));

  free(nand_q_name);
  free(nand_Q_name);

  return ERR_OK;
}  /* lsim_dev_srlatch_create */


/*******************************************************************************/


ERR_F lsim_dev_dlatch_get_out_terminal(lsim_t *lsim, lsim_dev_t *dev, const char *out_id, lsim_dev_out_terminal_t **out_terminal) {
  (void)lsim;
  ERR_ASSRT(dev->type == LSIM_DEV_TYPE_DLATCH, LSIM_ERR_INTERNAL);

  if (strcmp(out_id, "q0") == 0) {
    *out_terminal = dev->dlatch.q_terminal;
  }
  else if (strcmp(out_id, "Q0") == 0) {
    *out_terminal = dev->dlatch.Q_terminal;
  }
  else ERR_THROW(LSIM_ERR_INTERNAL, "unrecognized out_id '%s'", out_id);

  return ERR_OK;
}  /* lsim_dev_dlatch_get_out_terminal */


ERR_F lsim_dev_dlatch_get_in_terminal(lsim_t *lsim, lsim_dev_t *dev, const char *in_id, lsim_dev_in_terminal_t **in_terminal) {
  (void)lsim;
  ERR_ASSRT(dev->type == LSIM_DEV_TYPE_DLATCH, LSIM_ERR_INTERNAL);

  if (strcmp(in_id, "S0") == 0) {
    *in_terminal = dev->dlatch.S_terminal;
  }
  else if (strcmp(in_id, "R0") == 0) {
    *in_terminal = dev->dlatch.R_terminal;
  }
  else if (strcmp(in_id, "d0") == 0) {
    *in_terminal = dev->dlatch.d_terminal;
  }
  else if (strcmp(in_id, "c0") == 0) {
    *in_terminal = dev->dlatch.c_terminal;
  }
  else ERR_THROW(LSIM_ERR_INTERNAL, "unrecognized in_id '%s'", in_id);

  return ERR_OK;
}  /* lsim_dev_dlatch_get_in_terminal */


ERR_F lsim_dev_dlatch_power(lsim_t *lsim, lsim_dev_t *dev) {
  (void)lsim;
  ERR_ASSRT(dev->type == LSIM_DEV_TYPE_DLATCH, LSIM_ERR_INTERNAL);

  return ERR_OK;
}  /* lsim_dev_dlatch_power */


ERR_F lsim_dev_dlatch_run_logic(lsim_t *lsim, lsim_dev_t *dev) {
  (void)lsim;
  ERR_ASSRT(dev->type == LSIM_DEV_TYPE_DLATCH, LSIM_ERR_INTERNAL);

  ERR_THROW(LSIM_ERR_INTERNAL, "run logic should not be called for dlatch");

  return ERR_OK;
}  /* lsim_dev_dlatch_run_logic */


ERR_F lsim_dev_dlatch_propagate_outputs(lsim_t *lsim, lsim_dev_t *dev) {
  (void)lsim;
  ERR_ASSRT(dev->type == LSIM_DEV_TYPE_DLATCH, LSIM_ERR_INTERNAL);

  ERR_THROW(LSIM_ERR_INTERNAL, "propagate outputs should not be called for dlatch");

  return ERR_OK;
}  /* lsim_dev_dlatch_propagate_outputs */


ERR_F lsim_dev_dlatch_delete(lsim_t *lsim, lsim_dev_t *dev) {
  (void)lsim;
  ERR_ASSRT(dev->type == LSIM_DEV_TYPE_DLATCH, LSIM_ERR_INTERNAL);

  return ERR_OK;
}  /* lsim_dev_dlatch_delete */


ERR_F lsim_dev_dlatch_create(lsim_t *lsim, char *dev_name) {
  /* Make sure name doesn't already exist. */
  err_t *err;
  err = hmap_lookup(lsim->devs, dev_name, strlen(dev_name), NULL);
  ERR_ASSRT(err && err->code == HMAP_ERR_NOTFOUND, LSIM_ERR_EXIST);

  lsim_dev_t *dev;
  ERR(err_calloc((void **)&dev, 1, sizeof(lsim_dev_t)));
  ERR(err_strdup(&(dev->name), dev_name));
  dev->type = LSIM_DEV_TYPE_DLATCH;

  char *nand_q_name;
  ERR(err_asprintf(&nand_q_name, "%s.nand_q", dev_name));
  ERR(lsim_dev_nand_create(lsim, nand_q_name, 3));
  lsim_dev_t *nand_q_dev;
  ERR(hmap_lookup(lsim->devs, nand_q_name, strlen(nand_q_name), (void**)&nand_q_dev));

  char *nand_Q_name;
  ERR(err_asprintf(&nand_Q_name, "%s.nand_Q", dev_name));
  ERR(lsim_dev_nand_create(lsim, nand_Q_name, 3));
  lsim_dev_t *nand_Q_dev;
  ERR(hmap_lookup(lsim->devs, nand_Q_name, strlen(nand_Q_name), (void**)&nand_Q_dev));

  char *nand_a_name;
  ERR(err_asprintf(&nand_a_name, "%s.nand_a", dev_name));
  ERR(lsim_dev_nand_create(lsim, nand_a_name, 3));
  lsim_dev_t *nand_a_dev;
  ERR(hmap_lookup(lsim->devs, nand_a_name, strlen(nand_a_name), (void**)&nand_a_dev));

  char *nand_b_name;
  ERR(err_asprintf(&nand_b_name, "%s.nand_b", dev_name));
  ERR(lsim_dev_nand_create(lsim, nand_b_name, 3));
  lsim_dev_t *nand_b_dev;
  ERR(hmap_lookup(lsim->devs, nand_b_name, strlen(nand_b_name), (void**)&nand_b_dev));

  char *nand_c_name;
  ERR(err_asprintf(&nand_c_name, "%s.nand_c", dev_name));
  ERR(lsim_dev_nand_create(lsim, nand_c_name, 3));
  lsim_dev_t *nand_c_dev;
  ERR(hmap_lookup(lsim->devs, nand_c_name, strlen(nand_c_name), (void**)&nand_c_dev));

  char *nand_d_name;
  ERR(err_asprintf(&nand_d_name, "%s.nand_d", dev_name));
  ERR(lsim_dev_nand_create(lsim, nand_d_name, 3));
  lsim_dev_t *nand_d_dev;
  ERR(hmap_lookup(lsim->devs, nand_d_name, strlen(nand_d_name), (void**)&nand_d_dev));

  /* Make connections. */
  ERR(lsim_dev_connect(lsim, nand_q_name, "o0", nand_Q_name, "i2"));
  ERR(lsim_dev_connect(lsim, nand_Q_name, "o0", nand_q_name, "i2"));

  ERR(lsim_dev_connect(lsim, nand_a_name, "o0", nand_b_name, "i2"));
  ERR(lsim_dev_connect(lsim, nand_b_name, "o0", nand_a_name, "i2"));
  ERR(lsim_dev_connect(lsim, nand_b_name, "o0", nand_c_name, "i1"));
  ERR(lsim_dev_connect(lsim, nand_b_name, "o0", nand_q_name, "i1"));

  ERR(lsim_dev_connect(lsim, nand_c_name, "o0", nand_d_name, "i2"));
  ERR(lsim_dev_connect(lsim, nand_c_name, "o0", nand_Q_name, "i1"));
  ERR(lsim_dev_connect(lsim, nand_d_name, "o0", nand_c_name, "i2"));
  ERR(lsim_dev_connect(lsim, nand_d_name, "o0", nand_a_name, "i1"));

  /* Chain "External" inputs for "set". */
  nand_q_dev->nand.in_terminals[0].next_in_terminal = &nand_a_dev->nand.in_terminals[0];
  /* Chain "External" inputs for "reset". */
  nand_Q_dev->nand.in_terminals[0].next_in_terminal = &nand_d_dev->nand.in_terminals[1];
  nand_d_dev->nand.in_terminals[1].next_in_terminal = &nand_b_dev->nand.in_terminals[1];
  /* Chain "External" inputs for "clock". */
  nand_c_dev->nand.in_terminals[0].next_in_terminal = &nand_b_dev->nand.in_terminals[0];

  /* Save the "external" terminals. */
  dev->dlatch.q_terminal = nand_q_dev->nand.out_terminal;
  dev->dlatch.Q_terminal = nand_Q_dev->nand.out_terminal;
  dev->dlatch.S_terminal = &nand_q_dev->nand.in_terminals[0];
  dev->dlatch.R_terminal = &nand_Q_dev->nand.in_terminals[0];
  dev->dlatch.d_terminal = &nand_d_dev->nand.in_terminals[0];
  dev->dlatch.c_terminal = &nand_c_dev->nand.in_terminals[0];

  /* Need to chain internal inputs onto the "external" inputs. */

  /* Type-specific methods (inheritance). */
  dev->get_out_terminal = lsim_dev_dlatch_get_out_terminal;
  dev->get_in_terminal = lsim_dev_dlatch_get_in_terminal;
  dev->power = lsim_dev_dlatch_power;
  dev->run_logic = lsim_dev_dlatch_run_logic;
  dev->propagate_outputs = lsim_dev_dlatch_propagate_outputs;
  dev->delete = lsim_dev_dlatch_delete;

  ERR(hmap_write(lsim->devs, dev_name, strlen(dev_name), dev));

  free(nand_q_name);  free(nand_Q_name);
  free(nand_a_name);  free(nand_b_name);
  free(nand_c_name);  free(nand_d_name);

  return ERR_OK;
}  /* lsim_dev_dlatch_create */


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

  /* Can't have two outputs driving the same input. */
  if (dst_in_terminal->driving_out_terminal != NULL) {
    ERR_THROW(LSIM_ERR_COMMAND, "Can't connect %s;%s to %s;%s, it's already connected to %s",
              src_dev->name, src_out_id, dst_dev->name, dst_in_id, dst_in_terminal->driving_out_terminal->dev->name);
  }
  ERR_ASSRT(dst_in_terminal->driving_out_terminal == NULL, LSIM_ERR_COMMAND);

  /* This input might be the head of a chain of inputs that need to be connected to this output. */
  lsim_dev_in_terminal_t *next_in_terminal;
  do {
    next_in_terminal = dst_in_terminal->next_in_terminal;
    dst_in_terminal->driving_out_terminal = src_out_terminal;
    /* Link this terminal into the output list. */
    dst_in_terminal->next_in_terminal = src_out_terminal->in_terminal_list;
    src_out_terminal->in_terminal_list = dst_in_terminal;
    /* Next input terminal in this chain. */
    dst_in_terminal = next_in_terminal;
  } while (dst_in_terminal);

  return ERR_OK;
}  /* lsim_dev_connect */


ERR_F lsim_dev_delete(lsim_t *lsim, lsim_dev_t *dev) {
  ERR(dev->delete(lsim, dev));
  free(dev->name);
  free(dev);

  return ERR_OK;
}  /* lsim_dev_delete */


ERR_F lsim_dev_delete_all(lsim_t *lsim) {
  /* Step through entire hash map, starting with first entry. */
  hmap_entry_t *dev_entry = NULL;
  do {
    ERR(hmap_next(lsim->devs, &dev_entry));
    if (dev_entry) {
      lsim_dev_t *dev = dev_entry->value;
      ERR(lsim_dev_delete(lsim, dev));
    }
  } while (dev_entry);

  return ERR_OK;
}  /* lsim_dev_delete_all */


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
   * "propagate_outputs" function does not add devices to that list (it adds
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


ERR_F lsim_dev_engine(lsim_t *lsim) {
  if (! lsim->power_on) {
    return ERR_OK;
  }
  long max_propagate_cycles;
  ERR(cfg_get_long_val(lsim->cfg, "max_propagate_cycles", &max_propagate_cycles));
  ERR_ASSRT(max_propagate_cycles > 0, LSIM_ERR_CONFIG);

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
}  /* lsim_dev_engine */


ERR_F lsim_dev_power(lsim_t *lsim) {
  lsim->power_on = 1;

  /* Step through entire hash map, starting with first entry. */
  hmap_entry_t *dev_entry = NULL;
  do {
    ERR(hmap_next(lsim->devs, &dev_entry));
    if (dev_entry) {
      lsim_dev_t *cur_dev = dev_entry->value;
      ERR_ASSRT(cur_dev->next_out_changed == NULL, LSIM_ERR_INTERNAL);
      ERR_ASSRT(cur_dev->next_in_changed == NULL, LSIM_ERR_INTERNAL);
      ERR(cur_dev->power(lsim, cur_dev));
    }
  } while (dev_entry);

  ERR(lsim_dev_engine(lsim));

  return ERR_OK;
}  /* lsim_dev_power */


ERR_F lsim_dev_move(lsim_t *lsim, const char *dev_name, long new_state) {
  lsim_dev_t *dev;
  ERR(hmap_lookup(lsim->devs, dev_name, strlen(dev_name), (void**)&dev));

  ERR_ASSRT(dev->type == LSIM_DEV_TYPE_SWTCH, LSIM_ERR_COMMAND);

  if (dev->swtch.swtch_state != new_state) {
    dev->swtch.swtch_state = new_state;
    ERR(lsim_dev_in_changed(lsim, dev));  /* Trigger to run the logic. */
  }

  ERR(lsim_dev_engine(lsim));

  return ERR_OK;
}  /* lsim_dev_move */


ERR_F lsim_dev_tick(lsim_t *lsim) {
  long max_propagate_cycles;
  ERR(cfg_get_long_val(lsim->cfg, "max_propagate_cycles", &max_propagate_cycles));
  ERR_ASSRT(max_propagate_cycles > 0, LSIM_ERR_CONFIG);

  lsim->total_ticks++;
  if (lsim->verbosity_level > 0) {
    printf(" Tick %ld:\n", lsim->total_ticks);
  }

  if (lsim->active_clk_dev) {
    ERR(lsim_dev_in_changed(lsim, lsim->active_clk_dev));
  }

  ERR(lsim_dev_engine(lsim));

  return ERR_OK;
}  /* lsim_dev_tick */


ERR_F lsim_dev_watch(lsim_t *lsim, const char *dev_name, int watch_level) {
  lsim_dev_t *dev;
  ERR(hmap_lookup(lsim->devs, dev_name, strlen(dev_name), (void**)&dev));

  dev->watch_level = watch_level;

  return ERR_OK;
}  /* lsim_dev_watch */
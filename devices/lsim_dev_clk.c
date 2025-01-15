/* lsim_dev_clk.c */
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


ERR_F lsim_dev_clk_get_out_terminal(lsim_t *lsim, lsim_dev_t *dev, const char *out_id, lsim_dev_out_terminal_t **out_terminal, int bit_offset) {
  (void)lsim;
  ERR_ASSRT(dev->type == LSIM_DEV_TYPE_CLK, LSIM_ERR_INTERNAL);

  if (strcmp(out_id, "q0") == 0) {
    ERR_ASSRT(bit_offset == 0, LSIM_ERR_COMMAND); /* No output array. */
    *out_terminal = dev->clk.q_terminal;
  }
  else if (strcmp(out_id, "Q0") == 0) {
    ERR_ASSRT(bit_offset == 0, LSIM_ERR_COMMAND); /* No output array. */
    *out_terminal = dev->clk.Q_terminal;
  }
  else ERR_THROW(LSIM_ERR_COMMAND, "Unrecognized out_id '%s'", out_id);

  return ERR_OK;
}  /* lsim_dev_clk_get_out_terminal */


ERR_F lsim_dev_clk_get_in_terminal(lsim_t *lsim, lsim_dev_t *dev, const char *in_id, lsim_dev_in_terminal_t **in_terminal, int bit_offset) {
  (void)lsim;
  ERR_ASSRT(dev->type == LSIM_DEV_TYPE_CLK, LSIM_ERR_INTERNAL);

  if (strcmp(in_id, "R0") == 0) {
    ERR_ASSRT(bit_offset == 0, LSIM_ERR_COMMAND); /* No input array. */
    *in_terminal = dev->clk.R_terminal;
  }
  else ERR_THROW(LSIM_ERR_COMMAND, "Unrecognized in_id '%s'", in_id);


  return ERR_OK;
}  /* lsim_dev_clk_get_in_terminal */


ERR_F lsim_dev_clk_power(lsim_t *lsim, lsim_dev_t *dev) {
  (void)lsim;
  ERR_ASSRT(dev->type == LSIM_DEV_TYPE_CLK, LSIM_ERR_INTERNAL);

  dev->clk.R_terminal->state = 0;
  dev->clk.q_terminal->state = 0;
  dev->clk.Q_terminal->state = 0;
  /* Don't add clk to in_changed list because the logic is run explicitly. */

  return ERR_OK;
}  /* lsim_dev_clk_power */


ERR_F lsim_dev_clk_run_logic(lsim_t *lsim, lsim_dev_t *dev) {
  ERR_ASSRT(dev->type == LSIM_DEV_TYPE_CLK, LSIM_ERR_INTERNAL);
  /* Check for floating inputs. */
  if (dev->clk.R_terminal->driving_out_terminal == NULL) {
    ERR_THROW(LSIM_ERR_COMMAND, "Clock %s: input R0 is floating", dev->name);
  }

  int out_changed = 0;

  /* Process reset. */
  if (dev->clk.R_terminal->state == 0) {
    if (dev->clk.q_terminal->state != 0) {
      out_changed = 1;
    }
    dev->clk.q_terminal->state = 0;
    dev->clk.Q_terminal->state = 0;
    lsim->cur_ticklet = -1;
  }
  else {  /* Not reset. */
    /* Before first "ticklet" command, cur_ticklet is -1. Clock output 0. */
    int new_state = (lsim->cur_ticklet + 1) & 1;  /* Clock changes with each ticklet. */
    if (dev->clk.q_terminal->state != new_state || dev->clk.Q_terminal->state != (1 - new_state)) {
      out_changed = 1;
      dev->clk.q_terminal->state = new_state;
      dev->clk.Q_terminal->state = 1 - new_state;  /* Invert. */
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
    printf("  clk %s: q0=%d, Q0=%d\n", dev->name, dev->clk.q_terminal->state, dev->clk.Q_terminal->state);
  }

  return ERR_OK;
}  /* lsim_dev_clk_run_logic */


ERR_F lsim_dev_clk_propagate_outputs(lsim_t *lsim, lsim_dev_t *dev) {
  ERR_ASSRT(dev->type == LSIM_DEV_TYPE_CLK, LSIM_ERR_INTERNAL);

  int out_state = dev->clk.q_terminal->state;
  lsim_dev_in_terminal_t *dst_in_terminal = dev->clk.q_terminal->in_terminal_list;

  while (dst_in_terminal) {
    if (dst_in_terminal->state != out_state) {
      dst_in_terminal->state = out_state;
      lsim_dev_t *dst_dev = dst_in_terminal->dev;
      ERR(lsim_dev_in_changed(lsim, dst_dev));
    }

    /* Propagate output to next connected device. */
    dst_in_terminal = dst_in_terminal->next_in_terminal;
  }

  out_state = dev->clk.Q_terminal->state;
  dst_in_terminal = dev->clk.Q_terminal->in_terminal_list;

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
}  /* lsim_dev_clk_propagate_outputs */


ERR_F lsim_dev_clk_delete(lsim_t *lsim, lsim_dev_t *dev) {
  (void)lsim;
  ERR_ASSRT(dev->type == LSIM_DEV_TYPE_CLK, LSIM_ERR_INTERNAL);

  free(dev->clk.q_terminal);
  free(dev->clk.Q_terminal);
  free(dev->clk.R_terminal);

  free(dev->name);
  free(dev);

  return ERR_OK;
}  /* lsim_dev_clk_delete */


ERR_F lsim_dev_clk_create(lsim_t *lsim, char *dev_name) {
  /* Make sure name doesn't already exist. */
  err_t *err;
  err = hmap_slookup(lsim->devs, dev_name, NULL);
  ERR_ASSRT(err && err->code == HMAP_ERR_NOTFOUND, LSIM_ERR_EXIST);

  ERR_ASSRT(lsim->active_clk_dev == NULL, LSIM_ERR_COMMAND);  /* Can't have multiple clocks. */

  lsim_dev_t *dev;
  ERR(err_calloc((void **)&dev, 1, sizeof(lsim_dev_t)));
  ERR(err_strdup(&(dev->name), dev_name));
  dev->type = LSIM_DEV_TYPE_CLK;
  ERR(err_calloc((void **)&(dev->clk.q_terminal), 1, sizeof(lsim_dev_out_terminal_t)));
  dev->clk.q_terminal->dev = dev;
  ERR(err_calloc((void **)&(dev->clk.Q_terminal), 1, sizeof(lsim_dev_out_terminal_t)));
  dev->clk.Q_terminal->dev = dev;
  ERR(err_calloc((void **)&(dev->clk.R_terminal), 1, sizeof(lsim_dev_in_terminal_t)));
  dev->clk.R_terminal->dev = dev;

  /* Type-specific methods (inheritance). */
  dev->get_out_terminal = lsim_dev_clk_get_out_terminal;
  dev->get_in_terminal = lsim_dev_clk_get_in_terminal;
  dev->power = lsim_dev_clk_power;
  dev->run_logic = lsim_dev_clk_run_logic;
  dev->propagate_outputs = lsim_dev_clk_propagate_outputs;
  dev->delete = lsim_dev_clk_delete;

  ERR(hmap_swrite(lsim->devs, dev_name, dev));

  lsim->active_clk_dev = dev;  /* Make clock visible to lsim_dev_ticklet(). */

  return ERR_OK;
}  /* lsim_dev_clk_create */

/* lsim_devices_probe.c */
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


ERR_F lsim_dev_probe_get_out_terminal(lsim_t *lsim, lsim_dev_t *dev, const char *out_id, lsim_dev_out_terminal_t **out_terminal, int bit_offset) {
  (void)lsim;  (void)out_id;  (void)out_terminal;  (void)bit_offset;
  ERR_ASSRT(dev->type == LSIM_DEV_TYPE_PROBE, LSIM_ERR_INTERNAL);

  ERR_THROW(LSIM_ERR_COMMAND, "Attempt to get output state for probe, which has no outputs");

  return ERR_OK;
}  /* lsim_dev_probe_get_out_terminal */


ERR_F lsim_dev_probe_get_in_terminal(lsim_t *lsim, lsim_dev_t *dev, const char *in_id, lsim_dev_in_terminal_t **in_terminal, int bit_offset) {
  (void)lsim;
  ERR_ASSRT(dev->type == LSIM_DEV_TYPE_PROBE, LSIM_ERR_INTERNAL);

  if (strcmp(in_id, "d0") == 0) {
    ERR_ASSRT(bit_offset == 0, LSIM_ERR_COMMAND);          /* Only one output. */
    *in_terminal = dev->probe.d_terminal;
  }
  else if (strcmp(in_id, "c0") == 0) {
    ERR_ASSRT(bit_offset == 0, LSIM_ERR_COMMAND);          /* Only one output. */
    *in_terminal = dev->probe.c_terminal;
  }
  else ERR_THROW(LSIM_ERR_COMMAND, "Unrecognized in_id '%s'", in_id);

  return ERR_OK;
}  /* lsim_dev_probe_get_in_terminal */


ERR_F lsim_dev_probe_power(lsim_t *lsim, lsim_dev_t *dev) {
  ERR_ASSRT(dev->type == LSIM_DEV_TYPE_PROBE, LSIM_ERR_INTERNAL);

  dev->probe.cur_step = 0;
  dev->probe.d_terminal->state = 0;
  dev->probe.prev_d_state = 0;
  dev->probe.d_changes_in_step = 0;
  dev->probe.c_terminal->state = 0;
  dev->probe.prev_c_state = 0;
  dev->probe.c_changes_in_step = 0;
  dev->probe.c_triggers_in_step = 0;
  ERR(lsim_dev_in_changed(lsim, dev));  /* Trigger to run the logic. */

  return ERR_OK;
}  /* lsim_dev_probe_power */


ERR_F lsim_dev_probe_run_logic(lsim_t *lsim, lsim_dev_t *dev) {
  ERR_ASSRT(dev->type == LSIM_DEV_TYPE_PROBE, LSIM_ERR_INTERNAL);
  /* Check for floating inputs. */
  if (dev->probe.d_terminal->driving_out_terminal == NULL) {
    ERR_THROW(LSIM_ERR_COMMAND, "Probe %s: input d0 is floating", dev->name);
  }
  if (dev->probe.c_terminal->driving_out_terminal == NULL) {
    ERR_THROW(LSIM_ERR_COMMAND, "Probe %s: input c0 is floating", dev->name);
  }

  if (dev->probe.cur_step != lsim->cur_step) {
    /* New step, reset counters. */
    dev->probe.d_changes_in_step = 0;
    dev->probe.c_changes_in_step = 0;
    dev->probe.c_triggers_in_step = 0;
    dev->probe.cur_step = lsim->cur_step;
  }

  if (dev->probe.d_terminal->state != dev->probe.prev_d_state) {
    dev->probe.d_changes_in_step++;
    if (dev->probe.c_triggers_in_step > 0) {
      printf("Warning: probe %s: data changed during control trigger, step %ld\n", dev->name, lsim->cur_step);
      lsim->total_warnings++;
    }
  }
  if (dev->probe.c_terminal->state != dev->probe.prev_c_state) {
    dev->probe.c_changes_in_step++;
  }
  if (dev->probe.flags & LSIM_DEV_PROBE_FLAGS_RISING_EDGE) {
    if (dev->probe.c_terminal->state && ! dev->probe.prev_d_state) {
      /* Control edge rising trigger. */
      dev->probe.c_triggers_in_step++;
      if (dev->probe.d_changes_in_step > 0) {
        printf("Warning: probe %s: data changed during control trigger, step %ld\n", dev->name, lsim->cur_step);
        lsim->total_warnings++;
      }
    }
  }
  else {  /* Not LSIM_DEV_PROBE_FLAGS_RISING_EDGE. */
    if (! dev->probe.c_terminal->state && dev->probe.prev_d_state) {
      /* Control edge falling trigger. */
      dev->probe.c_triggers_in_step++;
      if (dev->probe.d_changes_in_step > 0) {
        printf("Warning: probe %s: data changed during control trigger, step %ld\n", dev->name, lsim->cur_step);
        lsim->total_warnings++;
      }
    }
  }
  if (dev->probe.c_changes_in_step > 1) {
    printf("Warning: probe %s: control trigger glitch, step %ld\n", dev->name, lsim->cur_step);
    lsim->total_warnings++;
  }

  dev->probe.prev_d_state = dev->probe.d_terminal->state;
  dev->probe.prev_c_state = dev->probe.c_terminal->state;

  return ERR_OK;
}  /* lsim_dev_probe_run_logic */


ERR_F lsim_dev_probe_propagate_outputs(lsim_t *lsim, lsim_dev_t *dev) {
  (void)lsim;
  ERR_ASSRT(dev->type == LSIM_DEV_TYPE_PROBE, LSIM_ERR_INTERNAL);

  /* No output to propagate. */

  return ERR_OK;
}  /* lsim_dev_probe_propagate_outputs */


ERR_F lsim_dev_probe_delete(lsim_t *lsim, lsim_dev_t *dev) {
  (void)lsim;
  ERR_ASSRT(dev->type == LSIM_DEV_TYPE_PROBE, LSIM_ERR_INTERNAL);

  free(dev->probe.d_terminal);
  free(dev->probe.c_terminal);

  free(dev->name);
  free(dev);

  return ERR_OK;
}  /* lsim_dev_probe_delete */


ERR_F lsim_dev_probe_create(lsim_t *lsim, char *dev_name, long flags) {
  /* Make sure name doesn't already exist. */
  err_t *err;
  err = hmap_slookup(lsim->devs, dev_name, NULL);
  ERR_ASSRT(err && err->code == HMAP_ERR_NOTFOUND, LSIM_ERR_EXIST);

  lsim_dev_t *dev;
  ERR(err_calloc((void **)&dev, 1, sizeof(lsim_dev_t)));
  ERR(err_strdup(&(dev->name), dev_name));
  dev->type = LSIM_DEV_TYPE_PROBE;
  dev->probe.flags = flags;
  ERR(err_calloc((void **)&(dev->probe.d_terminal), 1, sizeof(lsim_dev_in_terminal_t)));
  dev->probe.d_terminal->dev = dev;
  ERR(err_calloc((void **)&(dev->probe.c_terminal), 1, sizeof(lsim_dev_in_terminal_t)));
  dev->probe.c_terminal->dev = dev;

  /* Type-specific methods (inheritance). */
  dev->get_out_terminal = lsim_dev_probe_get_out_terminal;
  dev->get_in_terminal = lsim_dev_probe_get_in_terminal;
  dev->power = lsim_dev_probe_power;
  dev->run_logic = lsim_dev_probe_run_logic;
  dev->propagate_outputs = lsim_dev_probe_propagate_outputs;
  dev->delete = lsim_dev_probe_delete;

  ERR(hmap_swrite(lsim->devs, dev_name, dev));

  return ERR_OK;
}  /* lsim_dev_probe_create */

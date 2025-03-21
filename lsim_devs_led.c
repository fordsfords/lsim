/* lsim_devs_led.c */
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
#include "lsim_devs.h"


ERR_F lsim_devs_led_get_out_terminal(lsim_t *lsim, lsim_dev_t *dev, const char *out_id, lsim_dev_out_terminal_t **out_terminal, int bit_offset) {
  (void)lsim;  (void)out_id;  (void)out_terminal;  (void)bit_offset;
  ERR_ASSRT(dev->type == LSIM_DEV_TYPE_LED, LSIM_ERR_INTERNAL);

  ERR_THROW(LSIM_ERR_COMMAND, "Attempt to get output for led, which has no outputs");

  return ERR_OK;
}  /* lsim_devs_led_get_out_terminal */


ERR_F lsim_devs_led_get_in_terminal(lsim_t *lsim, lsim_dev_t *dev, const char *in_id, lsim_dev_in_terminal_t **in_terminal, int bit_offset) {
  (void)lsim;
  ERR_ASSRT(dev->type == LSIM_DEV_TYPE_LED, LSIM_ERR_INTERNAL);

  ERR_ASSRT(bit_offset == 0, LSIM_ERR_COMMAND);  /* No input array. */
  if (strcmp(in_id, "i0") == 0) {
    *in_terminal = dev->led.i_terminal;
  }
  else ERR_THROW(LSIM_ERR_COMMAND, "Unrecognized in_id '%s'", in_id);

  return ERR_OK;
}  /* lsim_devs_led_get_in_terminal */


ERR_F lsim_devs_led_power(lsim_t *lsim, lsim_dev_t *dev) {
  ERR_ASSRT(dev->type == LSIM_DEV_TYPE_LED, LSIM_ERR_INTERNAL);

  dev->led.illuminated = 0;
  dev->led.cur_step = -1;
  dev->led.changes_in_step = 0;
  dev->led.i_terminal->state = 0;
  ERR(lsim_dev_in_changed(lsim, dev));  /* Trigger to run the logic. */

  return ERR_OK;
}  /* lsim_devs_led_power */


ERR_F lsim_devs_led_run_logic(lsim_t *lsim, lsim_dev_t *dev) {
  (void)lsim;
  ERR_ASSRT(dev->type == LSIM_DEV_TYPE_LED, LSIM_ERR_INTERNAL);
  /* Check for floating inputs. */
  if (dev->led.i_terminal->driving_out_terminal == NULL) {
    ERR_THROW(LSIM_ERR_COMMAND, "Led %s: input i0 is floating", dev->name);
  }

  /* Detect glitch-based flicker of LED (multiple transitions in same step).*/
  if (lsim->cur_step != dev->led.cur_step) {
    /* New step. */
    dev->led.cur_step = lsim->cur_step;
    dev->led.changes_in_step = 0;
  }
  if (dev->led.i_terminal->state != dev->led.illuminated) {
    dev->led.illuminated = dev->led.i_terminal->state;
    dev->led.changes_in_step++;
    printf("Led %s: %s (ticklet %ld)%s\n",
           dev->name, dev->led.illuminated ? "on" : "off", lsim->cur_ticklet,
           (dev->led.changes_in_step > 1) ? " glitch" : "");
  }

  return ERR_OK;
}  /* lsim_devs_led_run_logic */


ERR_F lsim_devs_led_propagate_outputs(lsim_t *lsim, lsim_dev_t *dev) {
  (void)lsim;
  ERR_ASSRT(dev->type == LSIM_DEV_TYPE_LED, LSIM_ERR_INTERNAL);

  /* No output to propagate. */

  return ERR_OK;
}  /* lsim_devs_led_propagate_outputs */


ERR_F lsim_devs_led_delete(lsim_t *lsim, lsim_dev_t *dev) {
  (void)lsim;
  ERR_ASSRT(dev->type == LSIM_DEV_TYPE_LED, LSIM_ERR_INTERNAL);

  free(dev->led.i_terminal);

  free(dev->name);
  free(dev);

  return ERR_OK;
}  /* lsim_devs_led_delete */


ERR_F lsim_devs_led_create(lsim_t *lsim, char *dev_name) {
  /* Make sure name doesn't already exist. */
  err_t *err;
  err = hmap_slookup(lsim->devs, dev_name, NULL);
  ERR_ASSRT(err && err->code == HMAP_ERR_NOTFOUND, LSIM_ERR_EXIST);

  lsim_dev_t *dev;
  ERR(err_calloc((void **)&dev, 1, sizeof(lsim_dev_t)));
  ERR(err_strdup(&(dev->name), dev_name));
  dev->type = LSIM_DEV_TYPE_LED;
  ERR(err_calloc((void **)&(dev->led.i_terminal), 1, sizeof(lsim_dev_in_terminal_t)));
  dev->led.i_terminal->dev = dev;

  /* Type-specific methods (inheritance). */
  dev->get_out_terminal = lsim_devs_led_get_out_terminal;
  dev->get_in_terminal = lsim_devs_led_get_in_terminal;
  dev->power = lsim_devs_led_power;
  dev->run_logic = lsim_devs_led_run_logic;
  dev->propagate_outputs = lsim_devs_led_propagate_outputs;
  dev->delete = lsim_devs_led_delete;

  ERR(hmap_swrite(lsim->devs, dev_name, dev));

  return ERR_OK;
}  /* lsim_devs_led_create */

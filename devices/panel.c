/* lsim_panel.c */
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


ERR_F lsim_dev_panel_get_out_terminal(lsim_t *lsim, lsim_dev_t *dev, const char *out_id, lsim_dev_out_terminal_t **out_terminal, int bit_offset) {
  (void)lsim;
  ERR_ASSRT(dev->type == LSIM_DEV_TYPE_PANEL, LSIM_ERR_INTERNAL);

  long bit_num;
  ERR(err_atol(out_id+1, &bit_num));
  bit_num += bit_offset;
  if (bit_num >= dev->panel.num_bits) {  /* Use throw instead of assert for more useful error message. */
    ERR_THROW(LSIM_ERR_COMMAND, "output bit number %ld larger than number of bits %d", bit_num, dev->panel.num_bits);
  }
  if (out_id[0] == 'o') {
    *out_terminal = dev->panel.o_terminals[bit_num];
  }
  else {
    ERR_THROW(LSIM_ERR_COMMAND, "Invalid panel output ID %s", out_id);
  }

  return ERR_OK;
}  /* lsim_dev_panel_get_out_terminal */


ERR_F lsim_dev_panel_get_in_terminal(lsim_t *lsim, lsim_dev_t *dev, const char *in_id, lsim_dev_in_terminal_t **in_terminal, int bit_offset) {
  (void)lsim;
  ERR_ASSRT(dev->type == LSIM_DEV_TYPE_PANEL, LSIM_ERR_INTERNAL);

  long bit_num;
  ERR(err_atol(in_id + 1, &bit_num));
  bit_num += bit_offset;
  if (bit_num >= dev->panel.num_bits) {
    /* Use throw instead of assert for more useful error message. */
    ERR_THROW(LSIM_ERR_COMMAND, "input bit number %ld larger than number of bits %d", bit_num, dev->panel.num_bits);
  }
  if (in_id[0] == 'i') {
    *in_terminal = dev->panel.i_terminals[bit_num];
  } else {
    ERR_THROW(LSIM_ERR_COMMAND, "Invalid panel input ID %s", in_id);
  }

  return ERR_OK;
}  /* lsim_dev_panel_get_in_terminal */


ERR_F lsim_dev_panel_power(lsim_t *lsim, lsim_dev_t *dev) {
  (void)lsim;
  ERR_ASSRT(dev->type == LSIM_DEV_TYPE_PANEL, LSIM_ERR_INTERNAL);

  /* This is a composite device. The underlying devices will be
   * processed on their own. Nothing to be done here. */

  return ERR_OK;
}  /* lsim_dev_panel_power */


ERR_F lsim_dev_panel_run_logic(lsim_t *lsim, lsim_dev_t *dev) {
  (void)lsim;
  ERR_ASSRT(dev->type == LSIM_DEV_TYPE_PANEL, LSIM_ERR_INTERNAL);

  ERR_THROW(LSIM_ERR_INTERNAL, "run logic should not be called for panel");

  return ERR_OK;
}  /* lsim_dev_panel_run_logic */


ERR_F lsim_dev_panel_propagate_outputs(lsim_t *lsim, lsim_dev_t *dev) {
  (void)lsim;
  ERR_ASSRT(dev->type == LSIM_DEV_TYPE_PANEL, LSIM_ERR_INTERNAL);

  ERR_THROW(LSIM_ERR_INTERNAL, "propagate outputs should not be called for panel");

  return ERR_OK;
}  /* lsim_dev_panel_propagate_outputs */


ERR_F lsim_dev_panel_delete(lsim_t *lsim, lsim_dev_t *dev) {
  (void)lsim;
  ERR_ASSRT(dev->type == LSIM_DEV_TYPE_PANEL, LSIM_ERR_INTERNAL);

  free(dev->name);
  free(dev);

  return ERR_OK;
}  /* lsim_dev_panel_delete */


ERR_F lsim_dev_panel_create(lsim_t *lsim, char *dev_name, long num_bits) {
  ERR_ASSRT(num_bits >= 1, LSIM_ERR_PARAM);

  /* Make sure name doesn't already exist. */
  err_t *err;
  err = hmap_slookup(lsim->devs, dev_name, NULL);
  ERR_ASSRT(err && err->code == HMAP_ERR_NOTFOUND, LSIM_ERR_EXIST);

  lsim_dev_t *dev;
  ERR(err_calloc((void **)&dev, 1, sizeof(lsim_dev_t)));
  ERR(err_strdup(&(dev->name), dev_name));
  dev->type = LSIM_DEV_TYPE_PANEL;
  dev->panel.num_bits = num_bits;

  ERR(err_calloc((void **)&dev->panel.o_terminals, num_bits, sizeof(lsim_dev_out_terminal_t *)));
  ERR(err_calloc((void **)&dev->panel.i_terminals, num_bits, sizeof(lsim_dev_in_terminal_t *)));

  /* Create N switches and LEDs. */
  int i;
  for (i = 0; i < num_bits; i++) {
    char *swtch_name;
    ERR(err_asprintf(&swtch_name, "%s.swtch.%d", dev_name, i));
    ERR(lsim_dev_swtch_create(lsim, swtch_name, 0));
    lsim_dev_t *swtch_dev;
    ERR(hmap_slookup(lsim->devs, swtch_name, (void **)&swtch_dev));

    char *led_name;
    ERR(err_asprintf(&led_name, "%s.led.%d", dev_name, i));
    ERR(lsim_dev_led_create(lsim, led_name));
    lsim_dev_t *led_dev;
    ERR(hmap_slookup(lsim->devs, led_name, (void **)&led_dev));

    /* Save the "external" terminals. */
    dev->panel.o_terminals[i] = swtch_dev->swtch.o_terminal;
    dev->panel.i_terminals[i] = led_dev->led.i_terminal;

    free(swtch_name);
    free(led_name);
  }

  /* Type-specific methods (inheritance). */
  dev->get_out_terminal = lsim_dev_panel_get_out_terminal;
  dev->get_in_terminal = lsim_dev_panel_get_in_terminal;
  dev->power = lsim_dev_panel_power;
  dev->run_logic = lsim_dev_panel_run_logic;
  dev->propagate_outputs = lsim_dev_panel_propagate_outputs;
  dev->delete = lsim_dev_panel_delete;

  /* Write the panel dev. */
  ERR(hmap_swrite(lsim->devs, dev_name, dev));

  return ERR_OK;
}  /* lsim_dev_panel_create */

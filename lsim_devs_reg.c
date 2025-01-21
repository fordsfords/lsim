/* lsim_devs_reg.c */
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


ERR_F lsim_devs_reg_get_out_terminal(lsim_t *lsim, lsim_dev_t *dev, const char *out_id, lsim_dev_out_terminal_t **out_terminal, int bit_offset) {
  (void)lsim;
  ERR_ASSRT(dev->type == LSIM_DEV_TYPE_REG, LSIM_ERR_INTERNAL);

  if (out_id[0] == 'q') {
    long bit_num;
    ERR(err_atol(out_id + 1, &bit_num));
    bit_num += bit_offset;
    if (bit_num >= dev->reg.num_bits) { /* Use throw instead of assert for more useful error message. */
      ERR_THROW(LSIM_ERR_COMMAND, "reg %s output %s plus offset %d larger than last bit %d",
                dev->name, out_id, bit_offset, dev->reg.num_bits - 1);
    }
    *out_terminal = dev->reg.q_terminals[bit_num];
  }
  else if (out_id[0] == 'Q') {
    long bit_num;
    ERR(err_atol(out_id + 1, &bit_num));
    bit_num += bit_offset;
    if (bit_num >= dev->reg.num_bits) { /* Use throw instead of assert for more useful error message. */
      ERR_THROW(LSIM_ERR_COMMAND, "reg %s output %s plus offset %d larger than last bit %d",
                dev->name, out_id, bit_offset, dev->reg.num_bits - 1);
    }
    *out_terminal = dev->reg.Q_terminals[bit_num];
  }
  else ERR_THROW(LSIM_ERR_COMMAND, "Unrecognized out_id '%s'", out_id);

  return ERR_OK;
}  /* lsim_devs_reg_get_out_terminal */


ERR_F lsim_devs_reg_get_in_terminal(lsim_t *lsim, lsim_dev_t *dev, const char *in_id, lsim_dev_in_terminal_t **in_terminal, int bit_offset) {
  (void)lsim;
  ERR_ASSRT(dev->type == LSIM_DEV_TYPE_REG, LSIM_ERR_INTERNAL);

  if (strcmp(in_id, "c0") == 0) {
    *in_terminal = dev->reg.c_terminal;
  }
  else if (strcmp(in_id, "R0") == 0) {
    *in_terminal = dev->reg.R_terminal;
  }
  else {
    long bit_num;
    ERR(err_atol(in_id + 1, &bit_num));
    bit_num += bit_offset;
    if (bit_num >= dev->reg.num_bits) {
      /* Use throw instead of assert for more useful error message. */
      ERR_THROW(LSIM_ERR_COMMAND, "reg %s input %s plus offset %d larger than last bit %d",
                dev->name, in_id, bit_offset, dev->reg.num_bits-1);
    }
    if (in_id[0] == 'd') {
      *in_terminal = dev->reg.d_terminals[bit_num];
    } else {
      ERR_THROW(LSIM_ERR_COMMAND, "Invalid reg input ID %s", in_id);
    }
  }

  return ERR_OK;
}  /* lsim_devs_reg_get_in_terminal */


ERR_F lsim_devs_reg_power(lsim_t *lsim, lsim_dev_t *dev) {
  (void)lsim;
  ERR_ASSRT(dev->type == LSIM_DEV_TYPE_REG, LSIM_ERR_INTERNAL);

  /* This is a composite device. The underlying devices will be
   * processed on their own. Nothing to be done here. */

  return ERR_OK;
}  /* lsim_devs_reg_power */


ERR_F lsim_devs_reg_run_logic(lsim_t *lsim, lsim_dev_t *dev) {
  (void)lsim;
  ERR_ASSRT(dev->type == LSIM_DEV_TYPE_REG, LSIM_ERR_INTERNAL);

  ERR_THROW(LSIM_ERR_INTERNAL, "run logic should not be called for reg");

  return ERR_OK;
}  /* lsim_devs_reg_run_logic */


ERR_F lsim_devs_reg_propagate_outputs(lsim_t *lsim, lsim_dev_t *dev) {
  (void)lsim;
  ERR_ASSRT(dev->type == LSIM_DEV_TYPE_REG, LSIM_ERR_INTERNAL);

  ERR_THROW(LSIM_ERR_INTERNAL, "propagate outputs should not be called for reg");

  return ERR_OK;
}  /* lsim_devs_reg_propagate_outputs */


ERR_F lsim_devs_reg_delete(lsim_t *lsim, lsim_dev_t *dev) {
  (void)lsim;
  ERR_ASSRT(dev->type == LSIM_DEV_TYPE_REG, LSIM_ERR_INTERNAL);

  free(dev->name);
  free(dev);

  return ERR_OK;
}  /* lsim_devs_reg_delete */


ERR_F lsim_devs_reg_create(lsim_t *lsim, char *dev_name, long num_bits) {
  ERR_ASSRT(num_bits >= 1, LSIM_ERR_PARAM);

  /* Make sure name doesn't already exist. */
  err_t *err;
  err = hmap_slookup(lsim->devs, dev_name, NULL);
  ERR_ASSRT(err && err->code == HMAP_ERR_NOTFOUND, LSIM_ERR_EXIST);

  lsim_dev_t *dev;
  ERR(err_calloc((void **)&dev, 1, sizeof(lsim_dev_t)));
  ERR(err_strdup(&(dev->name), dev_name));
  dev->type = LSIM_DEV_TYPE_REG;
  dev->reg.num_bits = num_bits;

  ERR(err_calloc((void **)&dev->reg.q_terminals, num_bits, sizeof(lsim_dev_out_terminal_t *)));
  ERR(err_calloc((void **)&dev->reg.Q_terminals, num_bits, sizeof(lsim_dev_out_terminal_t *)));
  ERR(err_calloc((void **)&dev->reg.d_terminals, num_bits, sizeof(lsim_dev_in_terminal_t *)));

  char *vcc_name;
  ERR(err_asprintf(&vcc_name, "%s.vcc", dev_name));
  ERR(lsim_devs_vcc_create(lsim, vcc_name));
  lsim_dev_t *vcc_dev;
  ERR(hmap_slookup(lsim->devs, vcc_name, (void **)&vcc_dev));

  /* Create N D-flipflops. */
  int i;
  for (i = 0; i < num_bits; i++) {
    char *dflipflop_name;
    ERR(err_asprintf(&dflipflop_name, "%s.dflipflop.%d", dev_name, i));
    ERR(lsim_devs_dflipflop_create(lsim, dflipflop_name));
    lsim_dev_t *dflipflop_dev;
    ERR(hmap_slookup(lsim->devs, dflipflop_name, (void **)&dflipflop_dev));

    /* Set input not needed; connect to VCC. */
    ERR(lsim_dev_connect(lsim, vcc_name, "o0", dflipflop_name, "S0", 0));

    /* Save the "external" terminals. */
    dev->reg.q_terminals[i] = dflipflop_dev->dflipflop.q_terminal;
    dev->reg.Q_terminals[i] = dflipflop_dev->dflipflop.Q_terminal;
    dev->reg.d_terminals[i] = dflipflop_dev->dflipflop.d_terminal;

    /* Chain "External" inputs for clock and reset. */
    ERR(lsim_dev_in_chain_add(&dev->reg.c_terminal, dflipflop_dev->dflipflop.c_terminal, NULL));
    ERR(lsim_dev_in_chain_add(&dev->reg.R_terminal, dflipflop_dev->dflipflop.R_terminal, NULL));

    free(dflipflop_name);
  }

  /* Type-specific methods (inheritance). */
  dev->get_out_terminal = lsim_devs_reg_get_out_terminal;
  dev->get_in_terminal = lsim_devs_reg_get_in_terminal;
  dev->power = lsim_devs_reg_power;
  dev->run_logic = lsim_devs_reg_run_logic;
  dev->propagate_outputs = lsim_devs_reg_propagate_outputs;
  dev->delete = lsim_devs_reg_delete;

  ERR(hmap_swrite(lsim->devs, dev_name, dev));

  free(vcc_name);

  return ERR_OK;
}  /* lsim_devs_reg_create */

/* lsim_dflipflop.c */
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


ERR_F lsim_dev_dflipflop_get_out_terminal(lsim_t *lsim, lsim_dev_t *dev, const char *out_id, lsim_dev_out_terminal_t **out_terminal, int bit_offset) {
  (void)lsim;
  ERR_ASSRT(dev->type == LSIM_DEV_TYPE_DFLIPFLOP, LSIM_ERR_INTERNAL);

  ERR_ASSRT(bit_offset == 0, LSIM_ERR_COMMAND);  /* Only one output. */
  if (strcmp(out_id, "q0") == 0) {
    *out_terminal = dev->dflipflop.q_terminal;
  }
  else if (strcmp(out_id, "Q0") == 0) {
    *out_terminal = dev->dflipflop.Q_terminal;
  }
  else ERR_THROW(LSIM_ERR_INTERNAL, "unrecognized out_id '%s'", out_id);

  return ERR_OK;
}  /* lsim_dev_dflipflop_get_out_terminal */


ERR_F lsim_dev_dflipflop_get_in_terminal(lsim_t *lsim, lsim_dev_t *dev, const char *in_id, lsim_dev_in_terminal_t **in_terminal, int bit_offset) {
  (void)lsim;
  ERR_ASSRT(dev->type == LSIM_DEV_TYPE_DFLIPFLOP, LSIM_ERR_INTERNAL);

  ERR_ASSRT(bit_offset == 0, LSIM_ERR_COMMAND);  /* Only one output. */
  if (strcmp(in_id, "S0") == 0) {
    *in_terminal = dev->dflipflop.S_terminal;
  }
  else if (strcmp(in_id, "R0") == 0) {
    *in_terminal = dev->dflipflop.R_terminal;
  }
  else if (strcmp(in_id, "d0") == 0) {
    *in_terminal = dev->dflipflop.d_terminal;
  }
  else if (strcmp(in_id, "c0") == 0) {
    *in_terminal = dev->dflipflop.c_terminal;
  }
  else ERR_THROW(LSIM_ERR_INTERNAL, "unrecognized in_id '%s'", in_id);

  return ERR_OK;
}  /* lsim_dev_dflipflop_get_in_terminal */


ERR_F lsim_dev_dflipflop_power(lsim_t *lsim, lsim_dev_t *dev) {
  (void)lsim;
  ERR_ASSRT(dev->type == LSIM_DEV_TYPE_DFLIPFLOP, LSIM_ERR_INTERNAL);

  /* This is a composite device. The underlying devices will be
   * processed on their own. Nothing to be done here. */

  return ERR_OK;
}  /* lsim_dev_dflipflop_power */


ERR_F lsim_dev_dflipflop_run_logic(lsim_t *lsim, lsim_dev_t *dev) {
  (void)lsim;
  ERR_ASSRT(dev->type == LSIM_DEV_TYPE_DFLIPFLOP, LSIM_ERR_INTERNAL);

  ERR_THROW(LSIM_ERR_INTERNAL, "run logic should not be called for dflipflop");

  return ERR_OK;
}  /* lsim_dev_dflipflop_run_logic */


ERR_F lsim_dev_dflipflop_propagate_outputs(lsim_t *lsim, lsim_dev_t *dev) {
  (void)lsim;
  ERR_ASSRT(dev->type == LSIM_DEV_TYPE_DFLIPFLOP, LSIM_ERR_INTERNAL);

  ERR_THROW(LSIM_ERR_INTERNAL, "propagate outputs should not be called for dflipflop");

  return ERR_OK;
}  /* lsim_dev_dflipflop_propagate_outputs */


ERR_F lsim_dev_dflipflop_delete(lsim_t *lsim, lsim_dev_t *dev) {
  (void)lsim;
  ERR_ASSRT(dev->type == LSIM_DEV_TYPE_DFLIPFLOP, LSIM_ERR_INTERNAL);

  free(dev->name);
  free(dev);

  return ERR_OK;
}  /* lsim_dev_dflipflop_delete */


/* The dflipflop is a bit complex. Refer to the schematic:
 * https://raw.githubusercontent.com/fordsfords/lsim/refs/heads/main/dflipflop.svg */
ERR_F lsim_dev_dflipflop_create(lsim_t *lsim, char *dev_name) {
  /* Make sure name doesn't already exist. */
  err_t *err;
  err = hmap_slookup(lsim->devs, dev_name, NULL);
  ERR_ASSRT(err && err->code == HMAP_ERR_NOTFOUND, LSIM_ERR_EXIST);

  lsim_dev_t *dev;
  ERR(err_calloc((void **)&dev, 1, sizeof(lsim_dev_t)));
  ERR(err_strdup(&(dev->name), dev_name));
  dev->type = LSIM_DEV_TYPE_DFLIPFLOP;

  char *nand_q_name;
  ERR(err_asprintf(&nand_q_name, "%s.nand_q", dev_name));
  ERR(lsim_dev_nand_create(lsim, nand_q_name, 3));
  lsim_dev_t *nand_q_dev;
  ERR(hmap_slookup(lsim->devs, nand_q_name, (void**)&nand_q_dev));

  char *nand_Q_name;
  ERR(err_asprintf(&nand_Q_name, "%s.nand_Q", dev_name));
  ERR(lsim_dev_nand_create(lsim, nand_Q_name, 3));
  lsim_dev_t *nand_Q_dev;
  ERR(hmap_slookup(lsim->devs, nand_Q_name, (void**)&nand_Q_dev));

  char *nand_a_name;
  ERR(err_asprintf(&nand_a_name, "%s.nand_a", dev_name));
  ERR(lsim_dev_nand_create(lsim, nand_a_name, 3));
  lsim_dev_t *nand_a_dev;
  ERR(hmap_slookup(lsim->devs, nand_a_name, (void**)&nand_a_dev));

  char *nand_b_name;
  ERR(err_asprintf(&nand_b_name, "%s.nand_b", dev_name));
  ERR(lsim_dev_nand_create(lsim, nand_b_name, 3));
  lsim_dev_t *nand_b_dev;
  ERR(hmap_slookup(lsim->devs, nand_b_name, (void**)&nand_b_dev));

  char *nand_c_name;
  ERR(err_asprintf(&nand_c_name, "%s.nand_c", dev_name));
  ERR(lsim_dev_nand_create(lsim, nand_c_name, 3));
  lsim_dev_t *nand_c_dev;
  ERR(hmap_slookup(lsim->devs, nand_c_name, (void**)&nand_c_dev));

  char *nand_d_name;
  ERR(err_asprintf(&nand_d_name, "%s.nand_d", dev_name));
  ERR(lsim_dev_nand_create(lsim, nand_d_name, 3));
  lsim_dev_t *nand_d_dev;
  ERR(hmap_slookup(lsim->devs, nand_d_name, (void**)&nand_d_dev));

  /* Make connections. */
  ERR(lsim_dev_connect(lsim, nand_q_name, "o0", nand_Q_name, "i0", 0));
  ERR(lsim_dev_connect(lsim, nand_Q_name, "o0", nand_q_name, "i2", 0));

  ERR(lsim_dev_connect(lsim, nand_a_name, "o0", nand_b_name, "i0", 0));
  ERR(lsim_dev_connect(lsim, nand_b_name, "o0", nand_a_name, "i2", 0));
  ERR(lsim_dev_connect(lsim, nand_b_name, "o0", nand_c_name, "i0", 0));
  ERR(lsim_dev_connect(lsim, nand_b_name, "o0", nand_q_name, "i1", 0));

  ERR(lsim_dev_connect(lsim, nand_c_name, "o0", nand_d_name, "i0", 0));
  ERR(lsim_dev_connect(lsim, nand_c_name, "o0", nand_Q_name, "i1", 0));
  ERR(lsim_dev_connect(lsim, nand_d_name, "o0", nand_c_name, "i2", 0));
  ERR(lsim_dev_connect(lsim, nand_d_name, "o0", nand_a_name, "i1", 0));

  /* Save references to the "external" output terminals. */
  dev->dflipflop.q_terminal = nand_q_dev->nand.o_terminal;
  dev->dflipflop.Q_terminal = nand_Q_dev->nand.o_terminal;
  /* Save references to the "external" input terminals. */
  ERR(lsim_dev_in_chain_add(&dev->dflipflop.S_terminal, nand_a_dev->nand.i_terminals[0], NULL));
  ERR(lsim_dev_in_chain_add(&dev->dflipflop.S_terminal, nand_q_dev->nand.i_terminals[0], NULL));
  ERR(lsim_dev_in_chain_add(&dev->dflipflop.c_terminal, nand_b_dev->nand.i_terminals[1], NULL));
  ERR(lsim_dev_in_chain_add(&dev->dflipflop.c_terminal, nand_c_dev->nand.i_terminals[1], NULL));
  ERR(lsim_dev_in_chain_add(&dev->dflipflop.d_terminal, nand_d_dev->nand.i_terminals[1], NULL));
  ERR(lsim_dev_in_chain_add(&dev->dflipflop.R_terminal, nand_b_dev->nand.i_terminals[2], NULL));
  ERR(lsim_dev_in_chain_add(&dev->dflipflop.R_terminal, nand_d_dev->nand.i_terminals[2], NULL));
  ERR(lsim_dev_in_chain_add(&dev->dflipflop.R_terminal, nand_Q_dev->nand.i_terminals[2], NULL));

  /* Type-specific methods (inheritance). */
  dev->get_out_terminal = lsim_dev_dflipflop_get_out_terminal;
  dev->get_in_terminal = lsim_dev_dflipflop_get_in_terminal;
  dev->power = lsim_dev_dflipflop_power;
  dev->run_logic = lsim_dev_dflipflop_run_logic;
  dev->propagate_outputs = lsim_dev_dflipflop_propagate_outputs;
  dev->delete = lsim_dev_dflipflop_delete;

  /* Write the dflipflop dev. */
  ERR(hmap_swrite(lsim->devs, dev_name, dev));

  free(nand_q_name);  free(nand_Q_name);
  free(nand_a_name);  free(nand_b_name);
  free(nand_c_name);  free(nand_d_name);

  return ERR_OK;
}  /* lsim_dev_dflipflop_create */

/* lsim_devices_srlatch.c */
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


ERR_F lsim_dev_srlatch_get_out_terminal(lsim_t *lsim, lsim_dev_t *dev, const char *out_id, lsim_dev_out_terminal_t **out_terminal, int bit_offset) {
  (void)lsim;
  ERR_ASSRT(dev->type == LSIM_DEV_TYPE_SRLATCH, LSIM_ERR_INTERNAL);

  ERR_ASSRT(bit_offset == 0, LSIM_ERR_COMMAND);  /* Only one output. */
  if (strcmp(out_id, "q0") == 0) {
    *out_terminal = dev->srlatch.q_terminal;
  }
  else if (strcmp(out_id, "Q0") == 0) {
    *out_terminal = dev->srlatch.Q_terminal;
  }
  else ERR_THROW(LSIM_ERR_COMMAND, "Unrecognized out_id '%s'", out_id);

  return ERR_OK;
}  /* lsim_dev_srlatch_get_out_terminal */


ERR_F lsim_dev_srlatch_get_in_terminal(lsim_t *lsim, lsim_dev_t *dev, const char *in_id, lsim_dev_in_terminal_t **in_terminal, int bit_offset) {
  (void)lsim;
  ERR_ASSRT(dev->type == LSIM_DEV_TYPE_SRLATCH, LSIM_ERR_INTERNAL);

  ERR_ASSRT(bit_offset == 0, LSIM_ERR_COMMAND);  /* Only one output. */
  if (strcmp(in_id, "S0") == 0) {
    *in_terminal = dev->srlatch.S_terminal;
  }
  else if (strcmp(in_id, "R0") == 0) {
    *in_terminal = dev->srlatch.R_terminal;
  }
  else ERR_THROW(LSIM_ERR_COMMAND, "Unrecognized in_id '%s'", in_id);

  return ERR_OK;
}  /* lsim_dev_srlatch_get_in_terminal */


ERR_F lsim_dev_srlatch_power(lsim_t *lsim, lsim_dev_t *dev) {
  (void)lsim;
  ERR_ASSRT(dev->type == LSIM_DEV_TYPE_SRLATCH, LSIM_ERR_INTERNAL);

  /* This is a composite device. The underlying devices will be
   * processed on their own. Nothing to be done here. */

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

  free(dev->name);
  free(dev);

  return ERR_OK;
}  /* lsim_dev_srlatch_delete */


ERR_F lsim_dev_srlatch_create(lsim_t *lsim, char *dev_name) {
  /* Make sure name doesn't already exist. */
  err_t *err;
  err = hmap_slookup(lsim->devs, dev_name, NULL);
  ERR_ASSRT(err && err->code == HMAP_ERR_NOTFOUND, LSIM_ERR_EXIST);

  lsim_dev_t *dev;
  ERR(err_calloc((void **)&dev, 1, sizeof(lsim_dev_t)));
  ERR(err_strdup(&(dev->name), dev_name));
  dev->type = LSIM_DEV_TYPE_SRLATCH;

  char *nand_q_name;
  ERR(err_asprintf(&nand_q_name, "%s.nand_q", dev_name));
  ERR(lsim_dev_nand_create(lsim, nand_q_name, 2));
  lsim_dev_t *nand_q_dev;
  ERR(hmap_slookup(lsim->devs, nand_q_name, (void**)&nand_q_dev));

  char *nand_Q_name;
  ERR(err_asprintf(&nand_Q_name, "%s.nand_Q", dev_name));
  ERR(lsim_dev_nand_create(lsim, nand_Q_name, 2));
  lsim_dev_t *nand_Q_dev;
  ERR(hmap_slookup(lsim->devs, nand_Q_name, (void**)&nand_Q_dev));

  /* Make connections. */
  ERR(lsim_dev_connect(lsim, nand_q_name, "o0", nand_Q_name, "i1", 0));
  ERR(lsim_dev_connect(lsim, nand_Q_name, "o0", nand_q_name, "i1", 0));

  /* Save the "external" output terminals. */
  dev->srlatch.q_terminal = nand_q_dev->nand.o_terminal;
  dev->srlatch.Q_terminal = nand_Q_dev->nand.o_terminal;
  /* Save the "external" output terminals. */
  ERR(lsim_dev_in_chain_add(&dev->srlatch.S_terminal, nand_q_dev->nand.i_terminals[0], NULL));
  ERR(lsim_dev_in_chain_add(&dev->srlatch.R_terminal, nand_Q_dev->nand.i_terminals[0], NULL));

  /* Type-specific methods (inheritance). */
  dev->get_out_terminal = lsim_dev_srlatch_get_out_terminal;
  dev->get_in_terminal = lsim_dev_srlatch_get_in_terminal;
  dev->power = lsim_dev_srlatch_power;
  dev->run_logic = lsim_dev_srlatch_run_logic;
  dev->propagate_outputs = lsim_dev_srlatch_propagate_outputs;
  dev->delete = lsim_dev_srlatch_delete;

  /* Write the srlatch dev. */
  ERR(hmap_swrite(lsim->devs, dev_name, dev));

  free(nand_q_name);
  free(nand_Q_name);

  return ERR_OK;
}  /* lsim_dev_srlatch_create */

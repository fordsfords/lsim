/* lsim_devs_addbit.c */
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


ERR_F lsim_devs_addbit_get_out_terminal(lsim_t *lsim, lsim_dev_t *dev, const char *out_id, lsim_dev_out_terminal_t **out_terminal, int bit_offset) {
  (void)lsim;
  ERR_ASSRT(dev->type == LSIM_DEV_TYPE_ADDBIT, LSIM_ERR_INTERNAL);

  ERR_ASSRT(bit_offset == 0, LSIM_ERR_COMMAND);  /* Only one output. */
  if (strcmp(out_id, "s0") == 0) {
    *out_terminal = dev->addbit.s_terminal;
  }
  else if (strcmp(out_id, "o0") == 0) {
    *out_terminal = dev->addbit.o_terminal;
  }
  else ERR_THROW(LSIM_ERR_INTERNAL, "unrecognized out_id '%s'", out_id);

  return ERR_OK;
}  /* lsim_devs_addbit_get_out_terminal */


ERR_F lsim_devs_addbit_get_in_terminal(lsim_t *lsim, lsim_dev_t *dev, const char *in_id, lsim_dev_in_terminal_t **in_terminal, int bit_offset) {
  (void)lsim;
  ERR_ASSRT(dev->type == LSIM_DEV_TYPE_ADDBIT, LSIM_ERR_INTERNAL);

  ERR_ASSRT(bit_offset == 0, LSIM_ERR_COMMAND);  /* Only one output. */
  if (strcmp(in_id, "a0") == 0) {
    *in_terminal = dev->addbit.a_terminal;
  }
  else if (strcmp(in_id, "b0") == 0) {
    *in_terminal = dev->addbit.b_terminal;
  }
  else if (strcmp(in_id, "i0") == 0) {
    *in_terminal = dev->addbit.i_terminal;
  }
  else ERR_THROW(LSIM_ERR_INTERNAL, "unrecognized in_id '%s'", in_id);

  return ERR_OK;
}  /* lsim_devs_addbit_get_in_terminal */


ERR_F lsim_devs_addbit_power(lsim_t *lsim, lsim_dev_t *dev) {
  (void)lsim;
  ERR_ASSRT(dev->type == LSIM_DEV_TYPE_ADDBIT, LSIM_ERR_INTERNAL);

  /* This is a composite device. The underlying devices will be
   * processed on their own. Nothing to be done here. */

  return ERR_OK;
}  /* lsim_devs_addbit_power */


ERR_F lsim_devs_addbit_run_logic(lsim_t *lsim, lsim_dev_t *dev) {
  (void)lsim;
  ERR_ASSRT(dev->type == LSIM_DEV_TYPE_ADDBIT, LSIM_ERR_INTERNAL);

  ERR_THROW(LSIM_ERR_INTERNAL, "run logic should not be called for addbit");

  return ERR_OK;
}  /* lsim_devs_addbit_run_logic */


ERR_F lsim_devs_addbit_propagate_outputs(lsim_t *lsim, lsim_dev_t *dev) {
  (void)lsim;
  ERR_ASSRT(dev->type == LSIM_DEV_TYPE_ADDBIT, LSIM_ERR_INTERNAL);

  ERR_THROW(LSIM_ERR_INTERNAL, "propagate outputs should not be called for addbit");

  return ERR_OK;
}  /* lsim_devs_addbit_propagate_outputs */


ERR_F lsim_devs_addbit_delete(lsim_t *lsim, lsim_dev_t *dev) {
  (void)lsim;
  ERR_ASSRT(dev->type == LSIM_DEV_TYPE_ADDBIT, LSIM_ERR_INTERNAL);

  free(dev->name);
  free(dev);

  return ERR_OK;
}  /* lsim_devs_addbit_delete */


/* The addbit is a bit complex. Refer to the schematic:
 * https://raw.githubusercontent.com/fordsfords/lsim/refs/heads/main/addbit.svg */
ERR_F lsim_devs_addbit_create(lsim_t *lsim, char *dev_name) {
  /* Make sure name doesn't already exist. */
  err_t *err;
  err = hmap_slookup(lsim->devs, dev_name, NULL);
  ERR_ASSRT(err && err->code == HMAP_ERR_NOTFOUND, LSIM_ERR_EXIST);

  lsim_dev_t *dev;
  ERR(err_calloc((void **)&dev, 1, sizeof(lsim_dev_t)));
  ERR(err_strdup(&(dev->name), dev_name));
  dev->type = LSIM_DEV_TYPE_ADDBIT;

  char *nand_1_name;
  ERR(err_asprintf(&nand_1_name, "%s.nand_1", dev_name));
  ERR(lsim_devs_nand_create(lsim, nand_1_name, 2));
  lsim_dev_t *nand_1_dev;
  ERR(hmap_slookup(lsim->devs, nand_1_name, (void**)&nand_1_dev));

  char *nand_2_name;
  ERR(err_asprintf(&nand_2_name, "%s.nand_2", dev_name));
  ERR(lsim_devs_nand_create(lsim, nand_2_name, 2));
  lsim_dev_t *nand_2_dev;
  ERR(hmap_slookup(lsim->devs, nand_2_name, (void**)&nand_2_dev));

  char *nand_3_name;
  ERR(err_asprintf(&nand_3_name, "%s.nand_3", dev_name));
  ERR(lsim_devs_nand_create(lsim, nand_3_name, 2));
  lsim_dev_t *nand_3_dev;
  ERR(hmap_slookup(lsim->devs, nand_3_name, (void**)&nand_3_dev));

  char *nand_4_name;
  ERR(err_asprintf(&nand_4_name, "%s.nand_4", dev_name));
  ERR(lsim_devs_nand_create(lsim, nand_4_name, 2));
  lsim_dev_t *nand_4_dev;
  ERR(hmap_slookup(lsim->devs, nand_4_name, (void**)&nand_4_dev));

  char *nand_5_name;
  ERR(err_asprintf(&nand_5_name, "%s.nand_5", dev_name));
  ERR(lsim_devs_nand_create(lsim, nand_5_name, 2));
  lsim_dev_t *nand_5_dev;
  ERR(hmap_slookup(lsim->devs, nand_5_name, (void**)&nand_5_dev));

  char *nand_6_name;
  ERR(err_asprintf(&nand_6_name, "%s.nand_6", dev_name));
  ERR(lsim_devs_nand_create(lsim, nand_6_name, 2));
  lsim_dev_t *nand_6_dev;
  ERR(hmap_slookup(lsim->devs, nand_6_name, (void**)&nand_6_dev));

  char *nand_7_name;
  ERR(err_asprintf(&nand_7_name, "%s.nand_7", dev_name));
  ERR(lsim_devs_nand_create(lsim, nand_7_name, 2));
  lsim_dev_t *nand_7_dev;
  ERR(hmap_slookup(lsim->devs, nand_7_name, (void**)&nand_7_dev));

  char *nand_s_name;
  ERR(err_asprintf(&nand_s_name, "%s.nand_s", dev_name));
  ERR(lsim_devs_nand_create(lsim, nand_s_name, 2));
  lsim_dev_t *nand_s_dev;
  ERR(hmap_slookup(lsim->devs, nand_s_name, (void**)&nand_s_dev));

  char *nand_o_name;
  ERR(err_asprintf(&nand_o_name, "%s.nand_o", dev_name));
  ERR(lsim_devs_nand_create(lsim, nand_o_name, 2));
  lsim_dev_t *nand_o_dev;
  ERR(hmap_slookup(lsim->devs, nand_o_name, (void**)&nand_o_dev));

  /* Make connections. */
  ERR(lsim_dev_connect(lsim, nand_1_name, "o0", nand_2_name, "i1", 0));
  ERR(lsim_dev_connect(lsim, nand_1_name, "o0", nand_3_name, "i0", 0));
  ERR(lsim_dev_connect(lsim, nand_1_name, "o0", nand_o_name, "i1", 0));

  ERR(lsim_dev_connect(lsim, nand_2_name, "o0", nand_4_name, "i0", 0));
  ERR(lsim_dev_connect(lsim, nand_3_name, "o0", nand_4_name, "i1", 0));

  ERR(lsim_dev_connect(lsim, nand_4_name, "o0", nand_5_name, "i0", 0));
  ERR(lsim_dev_connect(lsim, nand_4_name, "o0", nand_6_name, "i0", 0));

  ERR(lsim_dev_connect(lsim, nand_5_name, "o0", nand_6_name, "i1", 0));
  ERR(lsim_dev_connect(lsim, nand_5_name, "o0", nand_7_name, "i0", 0));
  ERR(lsim_dev_connect(lsim, nand_5_name, "o0", nand_o_name, "i0", 0));

  ERR(lsim_dev_connect(lsim, nand_6_name, "o0", nand_s_name, "i0", 0));
  ERR(lsim_dev_connect(lsim, nand_7_name, "o0", nand_s_name, "i1", 0));

  /* Save references to the "external" output terminals. */
  dev->addbit.s_terminal = nand_s_dev->nand.o_terminal;
  dev->addbit.o_terminal = nand_o_dev->nand.o_terminal;

  /* Save references to the "external" input terminals. */
  ERR(lsim_dev_in_chain_add(&dev->addbit.a_terminal, nand_1_dev->nand.i_terminals[0], NULL));
  ERR(lsim_dev_in_chain_add(&dev->addbit.a_terminal, nand_2_dev->nand.i_terminals[0], NULL));
  ERR(lsim_dev_in_chain_add(&dev->addbit.b_terminal, nand_1_dev->nand.i_terminals[1], NULL));
  ERR(lsim_dev_in_chain_add(&dev->addbit.b_terminal, nand_3_dev->nand.i_terminals[1], NULL));
  ERR(lsim_dev_in_chain_add(&dev->addbit.i_terminal, nand_5_dev->nand.i_terminals[1], NULL));
  ERR(lsim_dev_in_chain_add(&dev->addbit.i_terminal, nand_7_dev->nand.i_terminals[1], NULL));

  /* Type-specific methods (inheritance). */
  dev->get_out_terminal = lsim_devs_addbit_get_out_terminal;
  dev->get_in_terminal = lsim_devs_addbit_get_in_terminal;
  dev->power = lsim_devs_addbit_power;
  dev->run_logic = lsim_devs_addbit_run_logic;
  dev->propagate_outputs = lsim_devs_addbit_propagate_outputs;
  dev->delete = lsim_devs_addbit_delete;

  /* Write the addbit dev. */
  ERR(hmap_swrite(lsim->devs, dev_name, dev));

  free(nand_1_name);  free(nand_2_name);
  free(nand_3_name);  free(nand_4_name);
  free(nand_5_name);  free(nand_6_name);
  free(nand_7_name);  free(nand_s_name);
  free(nand_o_name);

  return ERR_OK;
}  /* lsim_devs_addbit_create */

/* lsim_devs_addword.c */
/*
# This code and its documentation is Copyright 2024-2025 Steven Ford, http://geeky-boy.com
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


ERR_F lsim_devs_addword_get_out_terminal(lsim_t *lsim, lsim_dev_t *dev, const char *out_id, lsim_dev_out_terminal_t **out_terminal, int bit_offset) {
  (void)lsim;
  ERR_ASSRT(dev->type == LSIM_DEV_TYPE_ADDWORD, LSIM_ERR_INTERNAL);

  if (strcmp(out_id, "o0") == 0) {
    *out_terminal = dev->addword.o_terminal;
  }
  else if (out_id[0] == 's') {
    long bit_num;
    ERR(err_atol(out_id + 1, &bit_num));
    bit_num += bit_offset;
    if (bit_num >= dev->addword.num_bits) { /* Use throw instead of assert for more useful error message. */
      ERR_THROW(LSIM_ERR_COMMAND, "addword %s output %s plus offset %d larger than last bit %d",
                dev->name, out_id, bit_offset, dev->addword.num_bits - 1);
    }
    *out_terminal = dev->addword.s_terminals[bit_num];
  }
  else ERR_THROW(LSIM_ERR_COMMAND, "Unrecognized out_id '%s'", out_id);

  return ERR_OK;
}  /* lsim_devs_addword_get_out_terminal */


ERR_F lsim_devs_addword_get_in_terminal(lsim_t *lsim, lsim_dev_t *dev, const char *in_id, lsim_dev_in_terminal_t **in_terminal, int bit_offset) {
  (void)lsim;
  ERR_ASSRT(dev->type == LSIM_DEV_TYPE_ADDWORD, LSIM_ERR_INTERNAL);

  if (strcmp(in_id, "i0") == 0) {
    *in_terminal = dev->addword.i_terminal;
  }
  else if (in_id[0] == 'a') {
    long bit_num;
    ERR(err_atol(in_id + 1, &bit_num));
    bit_num += bit_offset;
    if (bit_num >= dev->addword.num_bits) { /* Use throw instead of assert for more useful error message. */
      ERR_THROW(LSIM_ERR_COMMAND, "addword %s output %s plus offset %d larger than last bit %d",
                dev->name, in_id, bit_offset, dev->addword.num_bits - 1);
    }
    *in_terminal = dev->addword.a_terminals[bit_num];
  }
  else if (in_id[0] == 'b') {
    long bit_num;
    ERR(err_atol(in_id + 1, &bit_num));
    bit_num += bit_offset;
    if (bit_num >= dev->addword.num_bits) { /* Use throw instead of assert for more useful error message. */
      ERR_THROW(LSIM_ERR_COMMAND, "addword %s output %s plus offset %d larger than last bit %d",
                dev->name, in_id, bit_offset, dev->addword.num_bits - 1);
    }
    *in_terminal = dev->addword.b_terminals[bit_num];
  }
  else {
    ERR_THROW(LSIM_ERR_COMMAND, "Invalid addword input ID %s", in_id);
  }

  return ERR_OK;
}  /* lsim_devs_addword_get_in_terminal */


ERR_F lsim_devs_addword_power(lsim_t *lsim, lsim_dev_t *dev) {
  (void)lsim;
  ERR_ASSRT(dev->type == LSIM_DEV_TYPE_ADDWORD, LSIM_ERR_INTERNAL);

  /* This is a composite device. The underlying devices will be
   * processed on their own. Nothing to be done here. */

  return ERR_OK;
}  /* lsim_devs_addword_power */


ERR_F lsim_devs_addword_run_logic(lsim_t *lsim, lsim_dev_t *dev) {
  (void)lsim;
  ERR_ASSRT(dev->type == LSIM_DEV_TYPE_ADDWORD, LSIM_ERR_INTERNAL);

  ERR_THROW(LSIM_ERR_INTERNAL, "run logic should not be called for addword");

  return ERR_OK;
}  /* lsim_devs_addword_run_logic */


ERR_F lsim_devs_addword_propagate_outputs(lsim_t *lsim, lsim_dev_t *dev) {
  (void)lsim;
  ERR_ASSRT(dev->type == LSIM_DEV_TYPE_ADDWORD, LSIM_ERR_INTERNAL);

  ERR_THROW(LSIM_ERR_INTERNAL, "propagate outputs should not be called for addword");

  return ERR_OK;
}  /* lsim_devs_addword_propagate_outputs */


ERR_F lsim_devs_addword_delete(lsim_t *lsim, lsim_dev_t *dev) {
  (void)lsim;
  ERR_ASSRT(dev->type == LSIM_DEV_TYPE_ADDWORD, LSIM_ERR_INTERNAL);

  free(dev->name);
  free(dev);

  return ERR_OK;
}  /* lsim_devs_addword_delete */


ERR_F lsim_devs_addword_create(lsim_t *lsim, char *dev_name, long num_bits) {
  ERR_ASSRT(num_bits >= 1, LSIM_ERR_PARAM);

  /* Make sure name doesn't already exist. */
  err_t *err;
  err = hmap_slookup(lsim->devs, dev_name, NULL);
  ERR_ASSRT(err && err->code == HMAP_ERR_NOTFOUND, LSIM_ERR_EXIST);

  lsim_dev_t *dev;
  ERR(err_calloc((void **)&dev, 1, sizeof(lsim_dev_t)));
  ERR(err_strdup(&(dev->name), dev_name));
  dev->type = LSIM_DEV_TYPE_ADDWORD;
  dev->addword.num_bits = num_bits;

  ERR(err_calloc((void **)&dev->addword.s_terminals, num_bits, sizeof(lsim_dev_out_terminal_t *)));
  ERR(err_calloc((void **)&dev->addword.a_terminals, num_bits, sizeof(lsim_dev_in_terminal_t *)));
  ERR(err_calloc((void **)&dev->addword.b_terminals, num_bits, sizeof(lsim_dev_in_terminal_t *)));

  /* Create N addbits. */
  int i;
  for (i = 0; i < num_bits; i++) {
    char *addbit_name;
    ERR(err_asprintf(&addbit_name, "%s.addbit.%d", dev_name, i));
    ERR(lsim_devs_addbit_create(lsim, addbit_name));
    lsim_dev_t *addbit_dev;
    ERR(hmap_slookup(lsim->devs, addbit_name, (void **)&addbit_dev));

    /* Save the "external" terminals. */
    dev->addword.s_terminals[i] = addbit_dev->addbit.s_terminal;
    dev->addword.a_terminals[i] = addbit_dev->addbit.a_terminal;
    dev->addword.b_terminals[i] = addbit_dev->addbit.b_terminal;

    if (i == 0) {
      dev->addword.i_terminal = addbit_dev->addbit.i_terminal;  /* carry in. */
    } else {
      char *prev_addbit_name;
      ERR(err_asprintf(&prev_addbit_name, "%s.addbit.%d", dev_name, i - 1));
      ERR(lsim_dev_connect(lsim, prev_addbit_name, "o0", addbit_name, "i0", 0));  /* chain carry. */

      if (i == (num_bits - 1)) {  /* last addbit. */
        dev->addword.o_terminal = addbit_dev->addbit.o_terminal; /* carry out. */
      }
      free(prev_addbit_name);
    }

    free(addbit_name);
  }

  /* Type-specific methods (inheritance). */
  dev->get_out_terminal = lsim_devs_addword_get_out_terminal;
  dev->get_in_terminal = lsim_devs_addword_get_in_terminal;
  dev->power = lsim_devs_addword_power;
  dev->run_logic = lsim_devs_addword_run_logic;
  dev->propagate_outputs = lsim_devs_addword_propagate_outputs;
  dev->delete = lsim_devs_addword_delete;

  ERR(hmap_swrite(lsim->devs, dev_name, dev));

  return ERR_OK;
}  /* lsim_devs_addword_create */

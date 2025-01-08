/* lsim_mem.c */
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


ERR_F lsim_dev_mem_get_out_terminal(lsim_t *lsim, lsim_dev_t *dev, const char *out_id, lsim_dev_out_terminal_t **out_terminal, int bit_offset) {
  (void)lsim;
  ERR_ASSRT(dev->type == LSIM_DEV_TYPE_MEM, LSIM_ERR_INTERNAL);

  long bit_num;
  ERR(err_atol(out_id+1, &bit_num));
  bit_num += bit_offset;
  if (bit_num >= dev->mem.num_data) {  /* Use throw instead of assert for more useful error message. */
    ERR_THROW(LSIM_ERR_COMMAND, "mem %s output %s plus offset %d larger than last bit %d",
              dev->name, out_id, bit_offset, dev->mem.num_data-1);
  }
  if (out_id[0] == 'o') {
    *out_terminal = dev->mem.o_terminals[bit_num];
  }
  else {
    ERR_THROW(LSIM_ERR_COMMAND, "Invalid reg output ID %s", out_id);
  }

  return ERR_OK;
}  /* lsim_dev_mem_get_out_terminal */


ERR_F lsim_dev_mem_get_in_terminal(lsim_t *lsim, lsim_dev_t *dev, const char *in_id, lsim_dev_in_terminal_t **in_terminal, int bit_offset) {
  (void)lsim;
  ERR_ASSRT(dev->type == LSIM_DEV_TYPE_MEM, LSIM_ERR_INTERNAL);

  switch (in_id[0]) {
    case 'i': {
      long input_num;
      ERR(err_atol(in_id + 1, &input_num));
      input_num += bit_offset;
      if (input_num >= dev->mem.num_data) { /* Use throw instead of assert for more useful error message. */
        ERR_THROW(LSIM_ERR_COMMAND, "mem %s input %s plus offset %d larger than last bit %d",
                  dev->name, in_id, bit_offset, dev->mem.num_data-1);
      }
      *in_terminal = dev->mem.i_terminals[input_num];
      break;
    }
    case 'a': {
      long input_num;
      ERR(err_atol(in_id + 1, &input_num));
      input_num += bit_offset;
      if (input_num >= dev->mem.num_addr) { /* Use throw instead of assert for more useful error message. */
        ERR_THROW(LSIM_ERR_COMMAND, "mem %s input %s plus offset %d larger than last bit %d",
                  dev->name, in_id, bit_offset, dev->mem.num_addr-1);
      }
      *in_terminal = dev->mem.a_terminals[input_num];
      break;
    }
    case 'w': {
      long input_num;
      ERR(err_atol(in_id + 1, &input_num));
      input_num += bit_offset;
      if (input_num >= 1) { /* Use throw instead of assert for more useful error message. */
        ERR_THROW(LSIM_ERR_COMMAND, "mem %s input %s plus offset %d larger than last bit %d",
                  dev->name, in_id, bit_offset, 1-1);
      }
      *in_terminal = dev->mem.w_terminal;
      break;
    }
  }


  return ERR_OK;
}  /* lsim_dev_mem_get_in_terminal */


ERR_F lsim_dev_mem_power(lsim_t *lsim, lsim_dev_t *dev) {
  ERR_ASSRT(dev->type == LSIM_DEV_TYPE_MEM, LSIM_ERR_INTERNAL);

  int out_index;
  for (out_index = 0; out_index < dev->mem.num_data; out_index++) {
    dev->mem.o_terminals[out_index]->state = 0;
  }

  int in_index;
  for (in_index = 0; in_index < dev->mem.num_data; in_index++) {
    dev->mem.i_terminals[in_index]->state = 0;
  }
  for (in_index = 0; in_index < dev->mem.num_addr; in_index++) {
    dev->mem.a_terminals[in_index]->state = 0;
  }

  ERR(lsim_dev_in_changed(lsim, dev));  /* Trigger to run the logic. */

  long num_words = 1<<dev->mem.num_addr;
  int i;
  for (i = 0; i < num_words; i++) {
    dev->mem.words[i] = 0;
  }

  return ERR_OK;
}  /* lsim_dev_mem_power */


ERR_F lsim_dev_mem_run_logic(lsim_t *lsim, lsim_dev_t *dev) {
  ERR_ASSRT(dev->type == LSIM_DEV_TYPE_MEM, LSIM_ERR_INTERNAL);

  /* Get an integer value representing the address bits. */
  long addr_val = 0;
  int in_index;
  for (in_index = 0; in_index < dev->mem.num_addr; in_index++) {
    /* Check for floating inputs. */
    if (dev->mem.a_terminals[in_index]->driving_out_terminal == NULL) {
      ERR_THROW(LSIM_ERR_COMMAND, "Mem %s: input a%d is floating", dev->name, in_index);
    }
    if (dev->mem.a_terminals[in_index]->state == 1) {
      addr_val |= (1<<in_index);
    }
  }

  /* Get an integer value representing the data bits. */
  uint64_t data_val = 0;

  /* Write (if requested). */
  if (dev->mem.w_terminal->state) {
    for (in_index = 0; in_index < dev->mem.num_data; in_index++) {
      /* Check for floating inputs. */
      if (dev->mem.i_terminals[in_index]->driving_out_terminal == NULL) {
        ERR_THROW(LSIM_ERR_COMMAND, "Mem %s: input i%d is floating", dev->name, in_index);
      }
      if (dev->mem.i_terminals[in_index]->state) {
        data_val |= (1 << in_index);
      }
    }
    dev->mem.words[addr_val] = data_val;
  } else {
    data_val = dev->mem.words[addr_val];
  }
  ERR_ASSRT((data_val & (~dev->mem.word_mask)) == 0, LSIM_ERR_INTERNAL);

  /* Set output bits, checking if output changed. */
  int out_changed = 0;
  int out_index;
  for (out_index = 0; out_index < dev->mem.num_data; out_index++) {
    int new_val = 0;
    if (data_val & (1<<out_index)) {
      new_val = 1;
    }
    if (dev->mem.o_terminals[out_index]->state != new_val) {
      out_changed = 1;
      dev->mem.o_terminals[out_index]->state = new_val;
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
    printf("  mem %s: o=%ld (0x%lx)\n", dev->name, data_val, data_val);
  }

  return ERR_OK;
}  /* lsim_dev_mem_run_logic */


ERR_F lsim_dev_mem_propagate_outputs(lsim_t *lsim, lsim_dev_t *dev) {
  ERR_ASSRT(dev->type == LSIM_DEV_TYPE_MEM, LSIM_ERR_INTERNAL);

  /* Propagate each output bit. */
  int out_index;
  for (out_index = 0; out_index < dev->mem.num_data; out_index++) {
    int out_state = dev->mem.o_terminals[out_index]->state;
    lsim_dev_in_terminal_t *dst_in_terminal = dev->mem.o_terminals[out_index]->in_terminal_list;

    while (dst_in_terminal) {
      if (dst_in_terminal->state != out_state) {
        dst_in_terminal->state = out_state;
        lsim_dev_t *dst_dev = dst_in_terminal->dev;
        ERR(lsim_dev_in_changed(lsim, dst_dev));
      }

      /* Propagate output to next connected device. */
      dst_in_terminal = dst_in_terminal->next_in_terminal;
    }
  }

  return ERR_OK;
}  /* lsim_dev_mem_propagate_outputs */


ERR_F lsim_dev_mem_delete(lsim_t *lsim, lsim_dev_t *dev) {
  (void)lsim;
  ERR_ASSRT(dev->type == LSIM_DEV_TYPE_MEM, LSIM_ERR_INTERNAL);

  int i;
  for (i = 0; i < dev->mem.num_data; i++) {
    free(dev->mem.o_terminals[i]);
    free(dev->mem.i_terminals[i]);
  }
  free(dev->mem.o_terminals);
  free(dev->mem.i_terminals);

  for (i = 0; i < dev->mem.num_addr; i++) {
    free(dev->mem.a_terminals[i]);
  }
  free(dev->mem.a_terminals);

  free(dev->mem.w_terminal);

  free(dev->mem.words);

  free(dev->name);
  free(dev);

  return ERR_OK;
}  /* lsim_dev_mem_delete */


ERR_F lsim_dev_mem_create(lsim_t *lsim, char *dev_name, long num_addr, long num_data) {
  ERR_ASSRT((num_data >= 1) && (num_data <= 64), LSIM_ERR_PARAM);
  ERR_ASSRT((num_addr >= 1) && (num_addr <= 18), LSIM_ERR_PARAM);

  /* Make sure name doesn't already exist. */
  err_t *err;
  err = hmap_slookup(lsim->devs, dev_name, NULL);
  ERR_ASSRT(err && err->code == HMAP_ERR_NOTFOUND, LSIM_ERR_EXIST);

  lsim_dev_t *dev;
  ERR(err_calloc((void **)&dev, 1, sizeof(lsim_dev_t)));
  ERR(err_strdup(&(dev->name), dev_name));
  dev->type = LSIM_DEV_TYPE_MEM;
  dev->mem.num_data = num_data;
  dev->mem.num_addr = num_addr;
  dev->mem.word_mask = (1<<(uint64_t)num_data) - 1;

  /* Allocate actual memory. */
  long num_words = 1<<num_addr;
  ERR(err_calloc((void **)&(dev->mem.words), num_words, sizeof(uint64_t)));

  /* data output terminals. */
  ERR(err_calloc((void **)&(dev->mem.o_terminals), num_data, sizeof(lsim_dev_out_terminal_t *)));
  long in_index;
  for (in_index = 0; in_index < dev->mem.num_data; in_index++) {
    ERR(err_calloc((void **)&dev->mem.o_terminals[in_index], 1, sizeof(lsim_dev_out_terminal_t)));
    dev->mem.o_terminals[in_index]->dev = dev;
  }

  /* data input terminals. */
  ERR(err_calloc((void **)&(dev->mem.i_terminals), num_data, sizeof(lsim_dev_in_terminal_t *)));
  for (in_index = 0; in_index < dev->mem.num_data; in_index++) {
    ERR(err_calloc((void **)&dev->mem.i_terminals[in_index], 1, sizeof(lsim_dev_in_terminal_t)));
    dev->mem.i_terminals[in_index]->dev = dev;
  }

  /* address input terminals. */
  ERR(err_calloc((void **)&(dev->mem.a_terminals), num_addr, sizeof(lsim_dev_in_terminal_t *)));
  for (in_index = 0; in_index < dev->mem.num_addr; in_index++) {
    ERR(err_calloc((void **)&dev->mem.a_terminals[in_index], 1, sizeof(lsim_dev_in_terminal_t)));
    dev->mem.a_terminals[in_index]->dev = dev;
  }

  /* write input terminal. */
  ERR(err_calloc((void **)&dev->mem.w_terminal, 1, sizeof(lsim_dev_in_terminal_t)));
  dev->mem.w_terminal->dev = dev;

  /* Type-specific methods (inheritance). */
  dev->get_out_terminal = lsim_dev_mem_get_out_terminal;
  dev->get_in_terminal = lsim_dev_mem_get_in_terminal;
  dev->power = lsim_dev_mem_power;
  dev->run_logic = lsim_dev_mem_run_logic;
  dev->propagate_outputs = lsim_dev_mem_propagate_outputs;
  dev->delete = lsim_dev_mem_delete;

  ERR(hmap_swrite(lsim->devs, dev_name, dev));

  return ERR_OK;
}  /* lsim_dev_mem_create */

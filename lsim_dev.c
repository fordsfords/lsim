/* lsim_dev.c - logic simulator command processing. */
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


ERR_F lsim_dev_in_chain_add(lsim_dev_in_terminal_t **head, lsim_dev_in_terminal_t *in_terminal, lsim_dev_out_terminal_t *driving_out_terminal) {
  ERR_ASSRT(in_terminal, LSIM_ERR_INTERNAL);

  /* Assume in_terminal is a sub-chain. Find its tail.
   * Update driving_out_terminal as we go. */
  lsim_dev_in_terminal_t *in_tail = in_terminal;
  while (in_tail->next_in_terminal) {
    in_tail->driving_out_terminal = driving_out_terminal;
    in_tail = in_tail->next_in_terminal;
  }
  in_tail->driving_out_terminal = driving_out_terminal;

  /* Insert chain at head. */
  in_tail->next_in_terminal = *head;
  *head = in_terminal;

  return ERR_OK;
}  /* lsim_dev_in_chain_add */


ERR_F lsim_dev_out_changed(lsim_t *lsim, lsim_dev_t *dev) {
  /* If not already on the output changed list, add it. */
  if (! dev->out_changed) {
    dev->out_changed = 1;
    dev->next_out_changed = lsim->out_changed_list;
    lsim->out_changed_list = dev;
  }

  return ERR_OK;
}  /* lsim_dev_out_changed */


ERR_F lsim_dev_in_changed(lsim_t *lsim, lsim_dev_t *dev) {
  /* If not already on the input changed list, add it. */
  if (! dev->in_changed) {
    dev->in_changed = 1;
    dev->next_in_changed = lsim->in_changed_list;
    lsim->in_changed_list = dev;
  }

  return ERR_OK;
}  /* lsim_dev_in_changed */


ERR_F lsim_dev_connect(lsim_t *lsim, const char *src_dev_name, const char *src_out_id, const char *dst_dev_name, const char *dst_in_id, int bit_offset) {
  lsim_dev_t *src_dev;
  ERR(hmap_slookup(lsim->devs, src_dev_name, (void**)&src_dev));
  lsim_dev_t *dst_dev;
  ERR(hmap_slookup(lsim->devs, dst_dev_name, (void**)&dst_dev));

  lsim_dev_out_terminal_t *src_out_terminal;
  ERR(src_dev->get_out_terminal(lsim, src_dev, src_out_id, &src_out_terminal, bit_offset));
  lsim_dev_in_terminal_t *dst_in_terminal;
  ERR(dst_dev->get_in_terminal(lsim, dst_dev, dst_in_id, &dst_in_terminal, bit_offset));

  /* Can't have two outputs driving the same input. */
  if (dst_in_terminal->driving_out_terminal != NULL) {
    ERR_THROW(LSIM_ERR_COMMAND, "Can't connect %s;%s to %s;%s, it's already connected to %s",
              src_dev->name, src_out_id, dst_dev->name, dst_in_id, dst_in_terminal->driving_out_terminal->dev->name);
  }
  /* Make sure two outputs aren't driving the same input. */
  ERR_ASSRT(dst_in_terminal->driving_out_terminal == NULL, LSIM_ERR_COMMAND);

  /* This input might be the head of a chain of inputs that need to be connected to this output. */
  ERR(lsim_dev_in_chain_add(&src_out_terminal->in_terminal_list, dst_in_terminal, src_out_terminal));

  return ERR_OK;
}  /* lsim_dev_connect */


ERR_F lsim_dev_delete(lsim_t *lsim, lsim_dev_t *dev) {
  ERR(dev->delete(lsim, dev));

  return ERR_OK;
}  /* lsim_dev_delete */


ERR_F lsim_dev_delete_all(lsim_t *lsim) {
  /* Loop through entire hash map, starting with first entry. */
  hmap_entry_t *dev_entry = NULL;
  do {
    ERR(hmap_next(lsim->devs, &dev_entry));
    if (dev_entry) {
      lsim_dev_t *dev = dev_entry->value;
      ERR(lsim_dev_delete(lsim, dev));
    }
  } while (dev_entry);

  return ERR_OK;
}  /* lsim_dev_delete_all */


ERR_F lsim_dev_run_logic(lsim_t *lsim) {
  /* When starting to run the logic, output changed list should be empty. */
  ERR_ASSRT(lsim->out_changed_list == NULL, LSIM_ERR_INTERNAL);

  /* This loop visits every device on the "in_changed_list". Note that the
   * "run_logic" function does not add devices to that list (it adds
   * them to "out_changed_list"). So this can't loop infinitely. */
  while (lsim->in_changed_list) {
    /* Remove from input changed list. */
    lsim_dev_t *cur_dev = lsim->in_changed_list;
    lsim->in_changed_list = cur_dev->next_in_changed;
    cur_dev->next_in_changed = NULL;
    cur_dev->in_changed = 0;

    ERR(cur_dev->run_logic(lsim, cur_dev));
  }  /* while in_changed_list */

  return ERR_OK;
}  /* lsim_dev_run_logic */


ERR_F lsim_dev_propagate_outputs(lsim_t *lsim) {
  ERR_ASSRT(lsim->in_changed_list == NULL, LSIM_ERR_INTERNAL);

  /* This loop visits every device on the "out_changed_list". Note that the
   * "propagate_outputs" function does not add devices to that list (it adds
   * them to "in_changed_list"). So this can't loop infinitely. */
  while (lsim->out_changed_list) {
    /* Remove from output changed list. */
    lsim_dev_t *cur_dev = lsim->out_changed_list;
    lsim->out_changed_list = cur_dev->next_out_changed;
    cur_dev->next_out_changed = NULL;
    cur_dev->out_changed = 0;

    ERR(cur_dev->propagate_outputs(lsim, cur_dev));
  }  /* while out_changed_list */

  return ERR_OK;
}  /* lsim_dev_propagate_outputs */


ERR_F lsim_dev_engine_run(lsim_t *lsim) {
  if (! lsim->power_on) {
    return ERR_OK;
  }
  long max_propagate_cycles;
  ERR(cfg_get_long_val(lsim->cfg, "max_propagate_cycles", &max_propagate_cycles));
  ERR_ASSRT(max_propagate_cycles > 0, LSIM_ERR_CONFIG);

  lsim->cur_step++;
  if (lsim->verbosity_level > 0) {
    printf(" Step %ld:\n", lsim->cur_step);
  }

  lsim->cur_cycle = 0;
  /* Loop while the logic states are still stabilizing. Note that this can
   * loop infinitely (e.g. a NAND oscillator), so the "max_propagate_cycles"
   * configuration parameter limits the loop count. */
  while (lsim->in_changed_list) {
    lsim->cur_cycle++;
    /* Prevent infinite loops. */
    ERR_ASSRT(lsim->cur_cycle <= max_propagate_cycles, LSIM_ERR_MAXLOOPS);

    ERR(lsim_dev_run_logic(lsim));
    ERR(lsim_dev_propagate_outputs(lsim));
  }

  return ERR_OK;
}  /* lsim_dev_engine_run */


ERR_F lsim_dev_power(lsim_t *lsim) {
  lsim->power_on = 1;
  lsim->cur_ticklet = -1;
  lsim->cur_step = -1;

  /* Loop through entire hash map, starting with first entry. */
  hmap_entry_t *dev_entry = NULL;
  do {
    ERR(hmap_next(lsim->devs, &dev_entry));
    if (dev_entry) {
      lsim_dev_t *cur_dev = dev_entry->value;
      ERR_ASSRT(cur_dev->next_out_changed == NULL, LSIM_ERR_INTERNAL);
      ERR_ASSRT(cur_dev->next_in_changed == NULL, LSIM_ERR_INTERNAL);
      ERR(cur_dev->power(lsim, cur_dev));
    }
  } while (dev_entry);

  ERR(lsim_dev_engine_run(lsim));

  return ERR_OK;
}  /* lsim_dev_power */


ERR_F lsim_dev_loadmem(lsim_t *lsim, const char *dev_name, long addr, int num_words, uint64_t *words) {
  lsim_dev_t *dev;
  ERR(hmap_slookup(lsim->devs, dev_name, (void**)&dev));

  ERR_ASSRT(dev->type == LSIM_DEV_TYPE_MEM, LSIM_ERR_COMMAND);

  int i;
  for (i = 0; i < num_words; i++) {
    dev->mem.words[addr + i] = words[i];
  }

  return ERR_OK;
}  /* lsim_dev_loadmem */


ERR_F lsim_dev_move(lsim_t *lsim, const char *dev_name, long new_state) {
  lsim_dev_t *dev;
  ERR(hmap_slookup(lsim->devs, dev_name, (void**)&dev));

  ERR_ASSRT(dev->type == LSIM_DEV_TYPE_SWTCH, LSIM_ERR_COMMAND);

  if (dev->swtch.swtch_state != new_state) {
    dev->swtch.swtch_state = new_state;
    ERR(lsim_dev_in_changed(lsim, dev));  /* Trigger to run the logic. */
  }

  ERR(lsim_dev_engine_run(lsim));

  return ERR_OK;
}  /* lsim_dev_move */


ERR_F lsim_dev_ticklet(lsim_t *lsim) {
  long max_propagate_cycles;
  ERR(cfg_get_long_val(lsim->cfg, "max_propagate_cycles", &max_propagate_cycles));
  ERR_ASSRT(max_propagate_cycles > 0, LSIM_ERR_CONFIG);
  ERR_ASSRT(lsim->active_clk_dev, LSIM_ERR_COMMAND);

  lsim->cur_ticklet++;
  if (lsim->verbosity_level > 0) {
    printf(" Ticklet %ld\n", lsim->cur_ticklet);
  }

  ERR(lsim_dev_in_changed(lsim, lsim->active_clk_dev));

  ERR(lsim_dev_engine_run(lsim));

  return ERR_OK;
}  /* lsim_dev_ticklet */


ERR_F lsim_dev_watch(lsim_t *lsim, const char *dev_name, int watch_level) {
  lsim_dev_t *dev;
  ERR(hmap_slookup(lsim->devs, dev_name, (void**)&dev));

  dev->watch_level = watch_level;

  return ERR_OK;
}  /* lsim_dev_watch */

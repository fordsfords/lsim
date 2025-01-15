/* lsim_dev.h */
/*
# This code and its documentation is Copyright 2024-2024 Steven Ford, http://geeky-boy.com
# and licensed "public domain" style under Creative Commons "CC0": http://creativecommons.org/publicdomain/zero/1.0/
# To the extent possible under law, the contributors to this project have
# waived all copyright and related or neighboring rights to this work.
# In other words, you can use this code for any purpose without any
# restrictions.  This work is published from: United States.  The project home
# is https://github.com/fordsfords/lsim
*/

#ifndef LSIM_DEV_H
#define LSIM_DEV_H

#include <stdint.h>
#include "err.h"
#include "hmap.h"

#ifdef __cplusplus
extern "C" {
#endif


/* Forward declarations. */
typedef struct lsim_s lsim_t;
typedef struct lsim_dev_s lsim_dev_t;
typedef struct lsim_dev_out_terminal_s lsim_dev_out_terminal_t;
typedef struct lsim_dev_in_terminal_s lsim_dev_in_terminal_t;


/* Full definitions. */
struct lsim_dev_out_terminal_s {
  lsim_dev_t *dev;
  lsim_dev_in_terminal_t *in_terminal_list;
  char id_prefix;
  int id_index;
  int state;  /* 0 or 1 */
};

struct lsim_dev_in_terminal_s {
  lsim_dev_t *dev;
  lsim_dev_in_terminal_t *next_in_terminal;
  lsim_dev_out_terminal_t *driving_out_terminal;
  char id_prefix;
  int id_index;
  int state;  /* 0 or 1 */
};


ERR_F lsim_dev_in_chain_add(lsim_dev_in_terminal_t **head, lsim_dev_in_terminal_t *in_terminal, lsim_dev_out_terminal_t *driving_out_terminal);
ERR_F lsim_dev_out_changed(lsim_t *lsim, lsim_dev_t *dev);
ERR_F lsim_dev_in_changed(lsim_t *lsim, lsim_dev_t *dev);
ERR_F lsim_dev_connect(lsim_t *lsim, const char *src_dev_name, const char *src_out_id, const char *dst_dev_name, const char *dst_in_id, int bit_offset);
ERR_F lsim_dev_power(lsim_t *lsim);
ERR_F lsim_dev_loadmem(lsim_t *lsim, const char *name, long addr, int num_words, uint64_t *words);
ERR_F lsim_dev_move(lsim_t *lsim, const char *name, long new_state);
ERR_F lsim_dev_run_logic(lsim_t *lsim);
ERR_F lsim_dev_propagate_outputs(lsim_t *lsim);
ERR_F lsim_dev_watch(lsim_t *lsim, const char *dev_name, int watch_level);
ERR_F lsim_dev_ticklet(lsim_t *lsim);
ERR_F lsim_dev_delete_all(lsim_t *lsim);

#ifdef __cplusplus
}
#endif

#endif // LSIM_DEV_H

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
#include "lsim.h"

#ifdef __cplusplus
extern "C" {
#endif


/* Forward declarations. */
typedef struct lsim_dev_out_terminal_s lsim_dev_out_terminal_t;
typedef struct lsim_dev_in_terminal_s lsim_dev_in_terminal_t;
typedef struct lsim_dev_vcc_s lsim_dev_vcc_t;
typedef struct lsim_dev_gnd_s lsim_dev_gnd_t;
typedef struct lsim_dev_clk1_s lsim_dev_clk1_t;
typedef struct lsim_dev_nand_s lsim_dev_nand_t;
typedef struct lsim_dev_mem_s lsim_dev_mem_t;
typedef struct lsim_dev_s lsim_dev_t;


/* Full definitions. */


struct lsim_dev_out_terminal_s {
  lsim_dev_t *device;
  lsim_dev_in_terminal_t *in_terminal_list;
  char id_prefix;
  int id_index;
  int state;  /* 0 or 1 */
};

struct lsim_dev_in_terminal_s {
  lsim_dev_t *device;
  lsim_dev_in_terminal_t *next_in_terminal;
  lsim_dev_out_terminal_t *driving_out_terminal;
  char id_prefix;
  int id_index;
  int state;  /* 0 or 1 */
};


struct lsim_dev_vcc_s {
  lsim_dev_out_terminal_t *out_terminal;
};

struct lsim_dev_gnd_s {
  lsim_dev_out_terminal_t *out_terminal;
};

struct lsim_dev_clk1_s {
  lsim_dev_out_terminal_t *out_terminal;
};

struct lsim_dev_nand_s {
  lsim_dev_out_terminal_t *out_terminal;
  long num_inputs;
  lsim_dev_out_terminal_t *in_terminals;  /* Allocated array of in_terminal_t. */
};

struct lsim_dev_mem_s {
  long num_outputs;
  lsim_dev_out_terminal_t *out_terminals;  /* Allocated array of out_terminal_t. */
  long num_inputs;
  lsim_dev_out_terminal_t *in_terminals;  /* Allocated array of in_terminal_t. */
};


struct lsim_dev_s {
  char *name;
  lsim_dev_t *next_out_changed;
  lsim_dev_t *next_in_changed;
  ERR_F (*get_output_wire_p)(lsim_t *lsim, lsim_dev_t *dev, const char *output_id, lsim_wire_t **wire_p);
  ERR_F (*get_input_state_p)(lsim_t *lsim, lsim_dev_t *dev, const char *input_id, int **state_p);
  ERR_F (*reset)(lsim_t *lsim, lsim_dev_t *dev);
  ERR_F (*run_logic)(lsim_t *lsim, lsim_dev_t *dev, int *outputs_changed);
  ERR_F (*propogate_outputs)(lsim_t *lsim, lsim_dev_t *dev);
  int type;  /* DEV_TYPE_... */
  union {
    lsim_dev_vcc_t vcc;
    lsim_dev_gnd_t gnd;
    lsim_dev_clk1_t clk1;
    lsim_dev_nand_t nand;
    lsim_dev_mem_t mem;
  };
};


ERR_F lsim_dev_vcc_create(lsim_t *lsim, char *name);
ERR_F lsim_dev_nand_create(lsim_t *lsim, char *name, long num_inputs);
ERR_F lsim_dev_reset(lsim_t *lsim);

#ifdef __cplusplus
}
#endif

#endif // LSIM_DEV_H

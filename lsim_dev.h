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

#define LSIM_DEV_TYPE_GND 1
#define LSIM_DEV_TYPE_VCC 2
#define LSIM_DEV_TYPE_SWTCH 3
#define LSIM_DEV_TYPE_LED 4
#define LSIM_DEV_TYPE_CLK1 5
#define LSIM_DEV_TYPE_NAND 6
#define LSIM_DEV_TYPE_MEM 7


/* Forward declarations. */
typedef struct lsim_dev_out_terminal_s lsim_dev_out_terminal_t;
typedef struct lsim_dev_in_terminal_s lsim_dev_in_terminal_t;
typedef struct lsim_dev_gnd_s lsim_dev_gnd_t;
typedef struct lsim_dev_vcc_s lsim_dev_vcc_t;
typedef struct lsim_dev_swtch_s lsim_dev_swtch_t;
typedef struct lsim_dev_led_s lsim_dev_led_t;
typedef struct lsim_dev_clk1_s lsim_dev_clk1_t;
typedef struct lsim_dev_nand_s lsim_dev_nand_t;
typedef struct lsim_dev_mem_s lsim_dev_mem_t;
typedef struct lsim_dev_s lsim_dev_t;


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


struct lsim_dev_gnd_s {
  lsim_dev_out_terminal_t *out_terminal;
};

struct lsim_dev_vcc_s {
  lsim_dev_out_terminal_t *out_terminal;
};

struct lsim_dev_swtch_s {
  long swtch_state;
  lsim_dev_out_terminal_t *out_terminal;
};

struct lsim_dev_led_s {
  lsim_dev_in_terminal_t *in_terminal;
};

struct lsim_dev_clk1_s {
  lsim_dev_out_terminal_t *out_terminal;
};

struct lsim_dev_nand_s {
  lsim_dev_out_terminal_t *out_terminal;   /* Allocated output terminal (one). */
  long num_inputs;
  lsim_dev_in_terminal_t *in_terminals;  /* Allocated array of input terminals. */
};

struct lsim_dev_mem_s {
  long num_outputs;
  lsim_dev_out_terminal_t *out_terminals;  /* Allocated array of output terminals. */
  long num_inputs;
  lsim_dev_out_terminal_t *in_terminals;  /* Allocated array of input terminals. */
};


struct lsim_dev_s {
  char *name;
  int out_changed;
  int in_changed;
  lsim_dev_t *next_out_changed;
  lsim_dev_t *next_in_changed;
  int type;  /* DEV_TYPE_... */
  union {
    lsim_dev_gnd_t gnd;
    lsim_dev_vcc_t vcc;
    lsim_dev_swtch_t swtch;
    lsim_dev_led_t led;
    lsim_dev_clk1_t clk1;
    lsim_dev_nand_t nand;
    lsim_dev_mem_t mem;
  };
  /* Type-specific methods (inheritence). */
  ERR_F (*get_out_terminal)(lsim_t *lsim, lsim_dev_t *dev, const char *out_id, lsim_dev_out_terminal_t **out_terminal);
  ERR_F (*get_in_terminal)(lsim_t *lsim, lsim_dev_t *dev, const char *in_id, lsim_dev_in_terminal_t **in_terminal);
  ERR_F (*reset)(lsim_t *lsim, lsim_dev_t *dev);
  ERR_F (*run_logic)(lsim_t *lsim, lsim_dev_t *dev);
  ERR_F (*propagate_outputs)(lsim_t *lsim, lsim_dev_t *dev);
};


ERR_F lsim_dev_gnd_create(lsim_t *lsim, char *name);
ERR_F lsim_dev_vcc_create(lsim_t *lsim, char *name);
ERR_F lsim_dev_swtch_create(lsim_t *lsim, char *name, long init_state);
ERR_F lsim_dev_led_create(lsim_t *lsim, char *name);
ERR_F lsim_dev_nand_create(lsim_t *lsim, char *name, long num_inputs);
ERR_F lsim_dev_connect(lsim_t *lsim, const char *src_dev_name, const char *src_out_id, const char *dst_dev_name, const char *dst_in_id);
ERR_F lsim_dev_reset(lsim_t *lsim);
ERR_F lsim_dev_move(lsim_t *lsim, char *name, long new_state);
ERR_F lsim_dev_run_logic(lsim_t *lsim);
ERR_F lsim_dev_propagate_outputs(lsim_t *lsim);
ERR_F lsim_dev_step(lsim_t *lsim);

#ifdef __cplusplus
}
#endif

#endif // LSIM_DEV_H

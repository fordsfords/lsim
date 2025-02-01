/* lsim_devs.h */
/*
# This code and its documentation is Copyright 2024-2024 Steven Ford, http://geeky-boy.com
# and licensed "public domain" style under Creative Commons "CC0": http://creativecommons.org/publicdomain/zero/1.0/
# To the extent possible under law, the contributors to this project have
# waived all copyright and related or neighboring rights to this work.
# In other words, you can use this code for any purpose without any
# restrictions.  This work is published from: United States.  The project home
# is https://github.com/fordsfords/lsim
*/

#ifndef LSIM_DEVS_H
#define LSIM_DEVS_H

#include "err.h"
#include "hmap.h"
#include "lsim_dev.h"

#ifdef __cplusplus
extern "C" {
#endif

#define LSIM_DEV_TYPE_PROBE 1
#define LSIM_DEV_TYPE_GND 2
#define LSIM_DEV_TYPE_VCC 3
#define LSIM_DEV_TYPE_SWTCH 4
#define LSIM_DEV_TYPE_LED 5
#define LSIM_DEV_TYPE_CLK 6
#define LSIM_DEV_TYPE_NAND 7
#define LSIM_DEV_TYPE_MEM 8
#define LSIM_DEV_TYPE_SRLATCH 9
#define LSIM_DEV_TYPE_DFLIPFLOP 10
#define LSIM_DEV_TYPE_REG 11
#define LSIM_DEV_TYPE_PANEL 12
#define LSIM_DEV_TYPE_ADDBIT 13
#define LSIM_DEV_TYPE_ADDWORD 14


/* Forward declarations. */
typedef struct lsim_dev_probe_s lsim_dev_probe_t;
typedef struct lsim_dev_gnd_s lsim_dev_gnd_t;
typedef struct lsim_dev_vcc_s lsim_dev_vcc_t;
typedef struct lsim_dev_swtch_s lsim_dev_swtch_t;
typedef struct lsim_dev_clk_s lsim_dev_clk_t;
typedef struct lsim_dev_led_s lsim_dev_led_t;
typedef struct lsim_dev_nand_s lsim_dev_nand_t;
typedef struct lsim_dev_mem_s lsim_dev_mem_t;
typedef struct lsim_dev_srlatch_s lsim_dev_srlatch_t;
typedef struct lsim_dev_dflipflop_s lsim_dev_dflipflop_t;
typedef struct lsim_dev_reg_s lsim_dev_reg_t;
typedef struct lsim_dev_panel_s lsim_dev_panel_t;
typedef struct lsim_dev_addbit_s lsim_dev_addbit_t;
typedef struct lsim_dev_addword_s lsim_dev_addword_t;

typedef struct lsim_dev_s lsim_dev_t;


#define LSIM_DEV_PROBE_FLAGS_RISING_EDGE 0x1
struct lsim_dev_probe_s {
  long flags;
  long cur_step;
  lsim_dev_in_terminal_t *d_terminal;
  long prev_d_state;
  long d_changes_in_step;
  lsim_dev_in_terminal_t *c_terminal;
  long prev_c_state;
  long c_changes_in_step;
  long c_triggers_in_step;
};

struct lsim_dev_gnd_s {
  lsim_dev_out_terminal_t *o_terminal;
};

struct lsim_dev_vcc_s {
  lsim_dev_out_terminal_t *o_terminal;
};

struct lsim_dev_swtch_s {
  int swtch_state;
  lsim_dev_out_terminal_t *o_terminal;
};

struct lsim_dev_clk_s {
  lsim_dev_in_terminal_t *R_terminal;
  lsim_dev_out_terminal_t *q_terminal;
  lsim_dev_out_terminal_t *Q_terminal;
};

struct lsim_dev_led_s {
  int illuminated;
  long cur_step;
  long changes_in_step;
  lsim_dev_in_terminal_t *i_terminal;
};

struct lsim_dev_nand_s {
  lsim_dev_out_terminal_t *o_terminal;   /* Allocated output terminal (one). */
  long num_inputs;
  lsim_dev_in_terminal_t **i_terminals;  /* Allocated array of input terminal ptrs. */
};

struct lsim_dev_mem_s {
  long num_data;
  lsim_dev_out_terminal_t **o_terminals;  /* Allocated array of [num_data] output terminal ptrs. */
  long num_addr;
  lsim_dev_in_terminal_t **i_terminals;  /* Allocated array of [num_data] input terminal ptrs. */
  lsim_dev_in_terminal_t **a_terminals;  /* Allocated array of address terminal ptrs. */
  lsim_dev_in_terminal_t *w_terminal;
  uint64_t *words;                       /* Allocated array of integers. */
  uint64_t word_mask;
};

struct lsim_dev_srlatch_s {
  lsim_dev_out_terminal_t *q_terminal;
  lsim_dev_out_terminal_t *Q_terminal;
  lsim_dev_in_terminal_t *S_terminal;
  lsim_dev_in_terminal_t *R_terminal;
};

struct lsim_dev_dflipflop_s {
  lsim_dev_out_terminal_t *q_terminal;
  lsim_dev_out_terminal_t *Q_terminal;
  lsim_dev_in_terminal_t *S_terminal;
  lsim_dev_in_terminal_t *R_terminal;
  lsim_dev_in_terminal_t *d_terminal;
  lsim_dev_in_terminal_t *c_terminal;
};

struct lsim_dev_reg_s {
  long num_bits;
  lsim_dev_out_terminal_t **q_terminals;  /* Allocated array of output terminals. */
  lsim_dev_out_terminal_t **Q_terminals;  /* Allocated array of inverted output terminals. */
  lsim_dev_in_terminal_t **d_terminals;  /* Allocated array of input terminals. */
  lsim_dev_in_terminal_t *R_terminal;
  lsim_dev_in_terminal_t *c_terminal;
};

struct lsim_dev_panel_s {
  long num_bits;
  lsim_dev_out_terminal_t **o_terminals;  /* Allocated array of output terminals. */
  lsim_dev_in_terminal_t **i_terminals;  /* Allocated array of input terminals. */
};

struct lsim_dev_addbit_s {
  lsim_dev_out_terminal_t *s_terminal;
  lsim_dev_out_terminal_t *o_terminal;  /* Carry out */
  lsim_dev_in_terminal_t *a_terminal;
  lsim_dev_in_terminal_t *b_terminal;
  lsim_dev_in_terminal_t *i_terminal;  /* Carry in */
};

struct lsim_dev_addword_s {
  long num_bits;
  lsim_dev_out_terminal_t **s_terminals; /* Allocated array of output terminals. */
  lsim_dev_out_terminal_t *o_terminal;   /* Carry out */
  lsim_dev_in_terminal_t **a_terminals;  /* Allocated array of input terminals. */
  lsim_dev_in_terminal_t **b_terminals;  /* Allocated array of input terminals. */
  lsim_dev_in_terminal_t *i_terminal;    /* Carry in */
};


struct lsim_dev_s {
  char *name;
  int out_changed;
  int in_changed;
  lsim_dev_t *next_out_changed;
  lsim_dev_t *next_in_changed;
  int type;  /* DEV_TYPE_... */
  int watch_level;  /* 0=none, 1=output change, 2=always print. */
  union {
    lsim_dev_probe_t probe;
    lsim_dev_gnd_t gnd;
    lsim_dev_vcc_t vcc;
    lsim_dev_swtch_t swtch;
    lsim_dev_clk_t clk;
    lsim_dev_led_t led;
    lsim_dev_nand_t nand;
    lsim_dev_mem_t mem;
    lsim_dev_srlatch_t srlatch;
    lsim_dev_dflipflop_t dflipflop;
    lsim_dev_reg_t reg;
    lsim_dev_panel_t panel;
    lsim_dev_addbit_t addbit;
    lsim_dev_addword_t addword;
  };
  /* Type-specific methods (inheritance). */
  ERR_F (*get_out_terminal)(lsim_t *lsim, lsim_dev_t *dev, const char *out_id, lsim_dev_out_terminal_t **out_terminal, int bit_offset);
  ERR_F (*get_in_terminal)(lsim_t *lsim, lsim_dev_t *dev, const char *in_id, lsim_dev_in_terminal_t **in_terminal, int bit_offset);
  ERR_F (*power)(lsim_t *lsim, lsim_dev_t *dev);
  ERR_F (*run_logic)(lsim_t *lsim, lsim_dev_t *dev);
  ERR_F (*propagate_outputs)(lsim_t *lsim, lsim_dev_t *dev);
  ERR_F (*delete)(lsim_t *lsim, lsim_dev_t *dev);
};


ERR_F lsim_devs_probe_create(lsim_t *lsim, char *name, long flags);
ERR_F lsim_devs_gnd_create(lsim_t *lsim, char *name);
ERR_F lsim_devs_vcc_create(lsim_t *lsim, char *name);
ERR_F lsim_devs_swtch_create(lsim_t *lsim, char *name, int init_state);
ERR_F lsim_devs_clk_create(lsim_t *lsim, char *name);
ERR_F lsim_devs_led_create(lsim_t *lsim, char *name);
ERR_F lsim_devs_nand_create(lsim_t *lsim, char *name, long num_inputs);
ERR_F lsim_devs_mem_create(lsim_t *lsim, char *dev_name, long num_addr, long num_data);
ERR_F lsim_devs_srlatch_create(lsim_t *lsim, char *name);
ERR_F lsim_devs_dflipflop_create(lsim_t *lsim, char *name);
ERR_F lsim_devs_reg_create(lsim_t *lsim, char *name, long num_bits);
ERR_F lsim_devs_panel_create(lsim_t *lsim, char *name, long num_bits);
ERR_F lsim_devs_addbit_create(lsim_t *lsim, char *name);
ERR_F lsim_devs_addword_create(lsim_t *lsim, char *name, long num_bits);

#ifdef __cplusplus
}
#endif

#endif // LSIM_DEVS_H

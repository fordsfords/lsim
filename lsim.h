/* lsim.h - simple haslsim. */
/*
# This code and its documentation is Copyright 2024-2024 Steven Ford, http://geeky-boy.com
# and licensed "public domain" style under Creative Commons "CC0": http://creativecommons.org/publicdomain/zero/1.0/
# To the extent possible under law, the contributors to this project have
# waived all copyright and related or neighboring rights to this work.
# In other words, you can use this code for any purpose without any
# restrictions.  This work is published from: United States.  The project home
# is https://github.com/fordsfords/lsim
*/

#ifndef LSIM_H
#define LSIM_H

#include <stdint.h>
#include "err.h"
#include "hmap.h"

#ifdef __cplusplus
extern "C" {
#endif

#define LSIM_ERR_INTERNAL 1
#define LSIM_ERR_PARAM 2
#define LSIM_ERR_NOMEM 3

typedef struct wire_s wire_t;

typedef struct dev_s dev_t;  /* Generic device. */
typedef struct dev_vcc_s dev_vcc_t;
typedef struct dev_gnd_s dev_gnd_t;
typedef struct dev_clk1_s dev_clk1_t;
typedef struct dev_nand_s dev_nand_t;

struct dev_s {
  int type;  /* DEV_TYPE_... */
  union {
    dev_vcc_t vcc;
    dev_gnd_t gnd;
    dev_clk1_t clk1;
    dev_nand_t nand;
  };
  dev_t *next_out_changed;
  def_t *next_in_changed;
};

struct dev_vcc_s {
  int out_state;
  wire_t out_conn;
};

struct dev_gnd_s {
  int out_state;
  wire_t out_conn;
};

typedef struct lsim_s lsim_t;
struct lsim_s {
  cfg_t *cfg;
  hmap_t *devs;
};

ERR_F lsim_create(lsim_t **rtn_lsim, char *config_filename);
ERR_F lsim_delete(lsim_t *lsim);

#ifdef __cplusplus
}
#endif

#endif // LSIM_H

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

#define LSIM_DEV_TYPE_VCC 1
#define LSIM_DEV_TYPE_GND 2
#define LSIM_DEV_TYPE_CLK1 3
#define LSIM_DEV_TYPE_NAND 4

typedef struct wire_segment_s wire_segment_t;
struct wire_segment_s {
  device_t *sink_device;
  char *input_name;
  int *input_state;
  wire_segment_t *next_segment;
};

typedef struct wire_s wire_t;
struct wire_s {
  device_t *source_device;
  wire_segment_t *sink;
};

typedef struct device_s device_t;  /* Generic device. */
typedef struct device_vcc_s device_vcc_t;
typedef struct device_gnd_s device_gnd_t;
typedef struct device_clk1_s device_clk1_t;
typedef struct device_nand_s device_nand_t;

struct device_vcc_s {
  int out_state;
  wire_t out_wire;
};

struct device_gnd_s {
  int out_state;
  wire_t out_wire;
};

struct device_clk1_s {
  int out_state;
  wire_t out_wire;
};

struct device_nand_s {
  int out_state;
  wire_t out_wire;
  int num_inputs;
  int *in_state;  /* Array of num_inputs. */
};

struct device_s {
  char *name;
  device_t *next_out_changed;
  device_t *next_in_changed;
  int type;  /* DEV_TYPE_... */
  union {
    device_vcc_t vcc;
    device_gnd_t gnd;
    device_clk1_t clk1;
    device_nand_t nand;
  };
};

typedef struct lsim_s lsim_t;
struct lsim_s {
  cfg_t *cfg;
  hmap_t *devs;
  device_t *out_changed_list;
  device_t *in_changed_list;
};

ERR_F lsim_create(lsim_t **rtn_lsim, char *config_filename);
ERR_F lsim_delete(lsim_t *lsim);

#ifdef __cplusplus
}
#endif

#endif // LSIM_H

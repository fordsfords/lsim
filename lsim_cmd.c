/* lsim_cmd.c - logic simulator command processing. */
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
#include "lsim_devices.h"
#include "lsim_cmd.h"


ERR_F lsim_valid_name(const char *name) {
  ERR_ASSRT(name, LSIM_ERR_INTERNAL);

  /* Validate first character (cannot be digit). */
  if (!(isalpha(*name) || *name == '_' || *name == '-')) {
    ERR_THROW(LSIM_ERR_COMMAND, "invalid name '%s'", name);
  }

  /* Validate rest of characters. */
  name++;
  while (*name != '\0') {
    if (!(isalnum(*name) || *name == '_' || *name == '-')) {
      ERR_THROW(LSIM_ERR_COMMAND, "invalid name '%s'", name);
    }
    name++;
  }

  return ERR_OK;
}  /* lsim_valid_name */


/*******************************************************************************/


/* Define device "probe":
 * d;probe;dev_name;flags;
 * cmd_line points at dev_name. */
ERR_F lsim_cmd_define_probe(lsim_t *lsim, char *cmd_line) {
  char *semi_colon;

  char *dev_name = cmd_line;
  ERR_ASSRT(semi_colon = strchr(dev_name, ';'), LSIM_ERR_COMMAND);
  *semi_colon = '\0';
  ERR(lsim_valid_name(dev_name));

  char *flags_s = semi_colon + 1;
  ERR_ASSRT(semi_colon = strchr(flags_s, ';'), LSIM_ERR_COMMAND);
  *semi_colon = '\0';  /* Overwrite semicolon. */

  /* Make sure we're at the end of the line. */
  char *end_field = semi_colon + 1;
  ERR_ASSRT(strlen(end_field) == 0, LSIM_ERR_COMMAND);

  long flags;
  ERR(err_atol(flags_s, &flags));
  ERR_ASSRT(flags >= 0, LSIM_ERR_COMMAND);

  ERR(lsim_dev_probe_create(lsim, dev_name, flags));

  return ERR_OK;
}  /* lsim_cmd_define_probe */


/* Define device "gnd":
 * d;gnd;dev_name;
 * cmd_line points at dev_name. */
ERR_F lsim_cmd_define_gnd(lsim_t *lsim, char *cmd_line) {
  char *semi_colon;

  char *dev_name = cmd_line;
  ERR_ASSRT(semi_colon = strchr(dev_name, ';'), LSIM_ERR_COMMAND);
  *semi_colon = '\0';
  ERR(lsim_valid_name(dev_name));

  /* Make sure we're at the end of the line. */
  char *end_field = semi_colon + 1;
  ERR_ASSRT(strlen(end_field) == 0, LSIM_ERR_COMMAND);

  ERR(lsim_dev_gnd_create(lsim, dev_name));

  return ERR_OK;
}  /* lsim_cmd_define_gnd */


/* Define device "vcc":
 * d;vcc;dev_name;
 * cmd_line points at dev_name. */
ERR_F lsim_cmd_define_vcc(lsim_t *lsim, char *cmd_line) {
  char *semi_colon;

  char *dev_name = cmd_line;
  ERR_ASSRT(semi_colon = strchr(dev_name, ';'), LSIM_ERR_COMMAND);
  *semi_colon = '\0';
  ERR(lsim_valid_name(dev_name));

  /* Make sure we're at the end of the line. */
  char *end_field = semi_colon + 1;
  ERR_ASSRT(strlen(end_field) == 0, LSIM_ERR_COMMAND);

  ERR(lsim_dev_vcc_create(lsim, dev_name));

  return ERR_OK;
}  /* lsim_cmd_define_vcc */


/* Define device "swtch":
 * d;swtch;dev_name;init_state;
 * cmd_line points at dev_name. */
ERR_F lsim_cmd_define_swtch(lsim_t *lsim, char *cmd_line) {
  char *semi_colon;

  char *dev_name = cmd_line;
  ERR_ASSRT(semi_colon = strchr(dev_name, ';'), LSIM_ERR_COMMAND);
  *semi_colon = '\0';
  ERR(lsim_valid_name(dev_name));

  char *init_state_s = semi_colon + 1;
  ERR_ASSRT(semi_colon = strchr(init_state_s, ';'), LSIM_ERR_COMMAND);
  *semi_colon = '\0';  /* Overwrite semicolon. */

  /* Make sure we're at the end of the line. */
  char *end_field = semi_colon + 1;
  ERR_ASSRT(strlen(end_field) == 0, LSIM_ERR_COMMAND);

  long init_state;
  ERR(err_atol(init_state_s, &init_state));
  ERR_ASSRT(init_state == 0 || init_state == 1, LSIM_ERR_COMMAND);

  ERR(lsim_dev_swtch_create(lsim, dev_name, (int)init_state));

  return ERR_OK;
}  /* lsim_cmd_define_swtch */


/* Define device "clk":
 * d;clk;dev_name;
 * cmd_line points at dev_name. */
ERR_F lsim_cmd_define_clk(lsim_t *lsim, char *cmd_line) {
  char *semi_colon;

  char *dev_name = cmd_line;
  ERR_ASSRT(semi_colon = strchr(dev_name, ';'), LSIM_ERR_COMMAND);
  *semi_colon = '\0';
  ERR(lsim_valid_name(dev_name));

  /* Make sure we're at the end of the line. */
  char *end_field = semi_colon + 1;
  ERR_ASSRT(strlen(end_field) == 0, LSIM_ERR_COMMAND);

  ERR(lsim_dev_clk_create(lsim, dev_name));

  return ERR_OK;
}  /* lsim_cmd_define_clk */


/* Define device "led":
 * d;led;dev_name;
 * cmd_line points at dev_name. */
ERR_F lsim_cmd_define_led(lsim_t *lsim, char *cmd_line) {
  char *semi_colon;

  char *dev_name = cmd_line;
  ERR_ASSRT(semi_colon = strchr(dev_name, ';'), LSIM_ERR_COMMAND);
  *semi_colon = '\0';
  ERR(lsim_valid_name(dev_name));

  /* Make sure we're at the end of the line. */
  char *end_field = semi_colon + 1;
  ERR_ASSRT(strlen(end_field) == 0, LSIM_ERR_COMMAND);

  ERR(lsim_dev_led_create(lsim, dev_name));

  return ERR_OK;
}  /* lsim_cmd_define_led */


/* Define device "nand":
 * d;nand;dev_name;num_addr;num_data;
 * cmd_line points at dev_name. */
ERR_F lsim_cmd_define_nand(lsim_t *lsim, char *cmd_line) {
  char *semi_colon;

  char *dev_name = cmd_line;
  ERR_ASSRT(semi_colon = strchr(dev_name, ';'), LSIM_ERR_COMMAND);
  *semi_colon = '\0';
  ERR(lsim_valid_name(dev_name));

  char *num_inputs_s = semi_colon + 1;
  ERR_ASSRT(semi_colon = strchr(num_inputs_s, ';'), LSIM_ERR_COMMAND);
  *semi_colon = '\0';  /* Overwrite semicolon. */

  /* Make sure we're at end of line. */
  char *end_field = semi_colon + 1;
  ERR_ASSRT(strlen(end_field) == 0, LSIM_ERR_COMMAND);

  long num_inputs;
  ERR(err_atol(num_inputs_s, &num_inputs));
  ERR_ASSRT(num_inputs > 0, LSIM_ERR_COMMAND);

  ERR(lsim_dev_nand_create(lsim, dev_name, num_inputs));

  return ERR_OK;
}  /* lsim_cmd_define_nand */


/* Define device "mem":
 * d;mem;dev_name;num_addr;num_data;
 * cmd_line points at dev_name. */
ERR_F lsim_cmd_define_mem(lsim_t *lsim, char *cmd_line) {
  char *semi_colon;

  char *dev_name = cmd_line;
  ERR_ASSRT(semi_colon = strchr(dev_name, ';'), LSIM_ERR_COMMAND);
  *semi_colon = '\0';
  ERR(lsim_valid_name(dev_name));

  char *num_addr_s = semi_colon + 1;
  ERR_ASSRT(semi_colon = strchr(num_addr_s, ';'), LSIM_ERR_COMMAND);
  *semi_colon = '\0';  /* Overwrite semicolon. */

  char *num_data_s = semi_colon + 1;
  ERR_ASSRT(semi_colon = strchr(num_data_s, ';'), LSIM_ERR_COMMAND);
  *semi_colon = '\0';  /* Overwrite semicolon. */

  /* Make sure we're at end of line. */
  char *end_field = semi_colon + 1;
  ERR_ASSRT(strlen(end_field) == 0, LSIM_ERR_COMMAND);

  long num_addr;
  ERR(err_atol(num_addr_s, &num_addr));
  ERR_ASSRT(num_addr > 0, LSIM_ERR_COMMAND);

  long num_data;
  ERR(err_atol(num_data_s, &num_data));
  ERR_ASSRT(num_data > 0, LSIM_ERR_COMMAND);

  ERR(lsim_dev_mem_create(lsim, dev_name, num_addr, num_data));

  return ERR_OK;
}  /* lsim_cmd_define_mem */


/* Define device "srlatch":
 * d;srlatch;dev_name;
 * cmd_line points at dev_name. */
ERR_F lsim_cmd_define_srlatch(lsim_t *lsim, char *cmd_line) {
  char *semi_colon;

  char *dev_name = cmd_line;
  ERR_ASSRT(semi_colon = strchr(dev_name, ';'), LSIM_ERR_COMMAND);
  *semi_colon = '\0';
  ERR(lsim_valid_name(dev_name));

  /* Make sure we're at end of line. */
  char *end_field = semi_colon + 1;
  ERR_ASSRT(strlen(end_field) == 0, LSIM_ERR_COMMAND);

  ERR(lsim_dev_srlatch_create(lsim, dev_name));

  return ERR_OK;
}  /* lsim_cmd_define_srlatch */


/* Define device "dflipflop":
 * d;dflipflop;dev_name;
 * cmd_line points at dev_name. */
ERR_F lsim_cmd_define_dflipflop(lsim_t *lsim, char *cmd_line) {
  char *semi_colon;

  char *dev_name = cmd_line;
  ERR_ASSRT(semi_colon = strchr(dev_name, ';'), LSIM_ERR_COMMAND);
  *semi_colon = '\0';
  ERR(lsim_valid_name(dev_name));

  /* Make sure we're at end of line. */
  char *end_field = semi_colon + 1;
  ERR_ASSRT(strlen(end_field) == 0, LSIM_ERR_COMMAND);

  ERR(lsim_dev_dflipflop_create(lsim, dev_name));

  return ERR_OK;
}  /* lsim_cmd_define_dflipflop */


/* Define device "reg":
 * d;reg;dev_name;num_bits;
 * cmd_line points at dev_name. */
ERR_F lsim_cmd_define_reg(lsim_t *lsim, char *cmd_line) {
  char *semi_colon;

  char *dev_name = cmd_line;
  ERR_ASSRT(semi_colon = strchr(dev_name, ';'), LSIM_ERR_COMMAND);
  *semi_colon = '\0';
  ERR(lsim_valid_name(dev_name));

  char *num_bits_s = semi_colon + 1;
  ERR_ASSRT(semi_colon = strchr(num_bits_s, ';'), LSIM_ERR_COMMAND);
  *semi_colon = '\0';  /* Overwrite semicolon. */

  /* Make sure we're at end of line. */
  char *end_field = semi_colon + 1;
  ERR_ASSRT(strlen(end_field) == 0, LSIM_ERR_COMMAND);

  long num_bits;
  ERR(err_atol(num_bits_s, &num_bits));
  ERR_ASSRT(num_bits > 0, LSIM_ERR_COMMAND);

  ERR(lsim_dev_reg_create(lsim, dev_name, num_bits));

  return ERR_OK;
}  /* lsim_cmd_define_reg */


/* Define device "panel":
 * d;panel;dev_name;num_bits;
 * cmd_line points at dev_name. */
ERR_F lsim_cmd_define_panel(lsim_t *lsim, char *cmd_line) {
  char *semi_colon;

  char *dev_name = cmd_line;
  ERR_ASSRT(semi_colon = strchr(dev_name, ';'), LSIM_ERR_COMMAND);
  *semi_colon = '\0';
  ERR(lsim_valid_name(dev_name));

  char *num_bits_s = semi_colon + 1;
  ERR_ASSRT(semi_colon = strchr(num_bits_s, ';'), LSIM_ERR_COMMAND);
  *semi_colon = '\0';  /* Overwrite semicolon. */

  /* Make sure we're at end of line. */
  char *end_field = semi_colon + 1;
  ERR_ASSRT(strlen(end_field) == 0, LSIM_ERR_COMMAND);

  long num_bits;
  ERR(err_atol(num_bits_s, &num_bits));
  ERR_ASSRT(num_bits > 0, LSIM_ERR_COMMAND);

  ERR(lsim_dev_panel_create(lsim, dev_name, num_bits));

  return ERR_OK;
}  /* lsim_cmd_define_panel */


/* Define device:
 * d;dev_type;...
 * cmd_line points at dev_type. */
ERR_F lsim_cmd_define(lsim_t *lsim, char *cmd_line) {
  char *semi_colon;

  char *dev_type = cmd_line;
  ERR_ASSRT(semi_colon = strchr(dev_type, ';'), LSIM_ERR_COMMAND);
  *semi_colon = '\0';

  char *next_field = semi_colon + 1;

  if (strcmp(dev_type, "probe") == 0) {
    ERR(lsim_cmd_define_probe(lsim, next_field));
  }
  else if (strcmp(dev_type, "gnd") == 0) {
    ERR(lsim_cmd_define_gnd(lsim, next_field));
  }
  else if (strcmp(dev_type, "vcc") == 0) {
    ERR(lsim_cmd_define_vcc(lsim, next_field));
  }
  else if (strcmp(dev_type, "swtch") == 0) {
    ERR(lsim_cmd_define_swtch(lsim, next_field));
  }
  else if (strcmp(dev_type, "clk") == 0) {
    ERR(lsim_cmd_define_clk(lsim, next_field));
  }
  else if (strcmp(dev_type, "led") == 0) {
    ERR(lsim_cmd_define_led(lsim, next_field));
  }
  else if (strcmp(dev_type, "nand") == 0) {
    ERR(lsim_cmd_define_nand(lsim, next_field));
  }
  else if (strcmp(dev_type, "mem") == 0) {
    ERR(lsim_cmd_define_mem(lsim, next_field));
  }
  else if (strcmp(dev_type, "srlatch") == 0) {
    ERR(lsim_cmd_define_srlatch(lsim, next_field));
  }
  else if (strcmp(dev_type, "dflipflop") == 0) {
    ERR(lsim_cmd_define_dflipflop(lsim, next_field));
  }
  else if (strcmp(dev_type, "reg") == 0) {
    ERR(lsim_cmd_define_reg(lsim, next_field));
  }
  else if (strcmp(dev_type, "panel") == 0) {
    ERR(lsim_cmd_define_panel(lsim, next_field));
  }
  else {
    ERR_THROW(LSIM_ERR_COMMAND, "Unrecognized device type '%s'", dev_type);
  }

  return ERR_OK;
}  /* lsim_cmd_define */


/* Define connection:
 * c;src_dev_name;src_output_id;dst_dev_name;dst_input_id;
 * cmd_line points at src_dev_name. */
ERR_F lsim_cmd_connect(lsim_t *lsim, char *cmd_line) {
  char *semi_colon;

  char *src_dev_name = cmd_line;
  ERR_ASSRT(semi_colon = strchr(src_dev_name, ';'), LSIM_ERR_COMMAND);
  *semi_colon = '\0';
  ERR(lsim_valid_name(src_dev_name));

  char *src_output_id = semi_colon + 1;
  ERR_ASSRT(semi_colon = strchr(src_output_id, ';'), LSIM_ERR_COMMAND);
  *semi_colon = '\0';

  char *dst_dev_name = semi_colon + 1;
  ERR_ASSRT(semi_colon = strchr(dst_dev_name, ';'), LSIM_ERR_COMMAND);
  *semi_colon = '\0';
  ERR(lsim_valid_name(dst_dev_name));

  char *dst_input_id = semi_colon + 1;
  ERR_ASSRT(semi_colon = strchr(dst_input_id, ';'), LSIM_ERR_COMMAND);
  *semi_colon = '\0';

  /* Make sure we're at end of line. */
  char *end_field = semi_colon + 1;
  ERR_ASSRT(strlen(end_field) == 0, LSIM_ERR_COMMAND);

  ERR(lsim_dev_connect(lsim, src_dev_name, src_output_id, dst_dev_name, dst_input_id, 0));

  return ERR_OK;
}  /* lsim_cmd_connect */


/* Bus connection:
 * b;src_dev_name;src_output_id;dst_dev_name;dst_input_id;num_bits;
 * cmd_line points at src_dev_name. */
ERR_F lsim_cmd_busconn(lsim_t *lsim, char *cmd_line) {
  char *semi_colon;

  char *src_dev_name = cmd_line;
  ERR_ASSRT(semi_colon = strchr(src_dev_name, ';'), LSIM_ERR_COMMAND);
  *semi_colon = '\0';
  ERR(lsim_valid_name(src_dev_name));

  char *src_output_id = semi_colon + 1;
  ERR_ASSRT(semi_colon = strchr(src_output_id, ';'), LSIM_ERR_COMMAND);
  *semi_colon = '\0';

  char *dst_dev_name = semi_colon + 1;
  ERR_ASSRT(semi_colon = strchr(dst_dev_name, ';'), LSIM_ERR_COMMAND);
  *semi_colon = '\0';
  ERR(lsim_valid_name(dst_dev_name));

  char *dst_input_id = semi_colon + 1;
  ERR_ASSRT(semi_colon = strchr(dst_input_id, ';'), LSIM_ERR_COMMAND);
  *semi_colon = '\0';

  char *num_bits_s = semi_colon + 1;
  ERR_ASSRT(semi_colon = strchr(num_bits_s, ';'), LSIM_ERR_COMMAND);
  *semi_colon = '\0';  /* Overwrite semicolon. */

  /* Make sure we're at end of line. */
  char *end_field = semi_colon + 1;
  ERR_ASSRT(strlen(end_field) == 0, LSIM_ERR_COMMAND);

  long num_bits;
  ERR(err_atol(num_bits_s, &num_bits));
  ERR_ASSRT(num_bits > 0, LSIM_ERR_COMMAND);

  int i;
  for (i = 0; i < num_bits; i++) {
    ERR(lsim_dev_connect(lsim, src_dev_name, src_output_id, dst_dev_name, dst_input_id, i));
  }

  return ERR_OK;
}  /* lsim_cmd_busconn */


/* Reset:
 * r;
 * cmd_line points past first semi-colon. */
ERR_F lsim_cmd_power(lsim_t *lsim, char *cmd_line) {
  /* Make sure we're at end of line. */
  char *end_field = cmd_line;
  ERR_ASSRT(strlen(end_field) == 0, LSIM_ERR_COMMAND);

  ERR(lsim_dev_power(lsim));

  return ERR_OK;
}  /* lsim_cmd_power */


/* Loadmem:
 * l;mem_device_name;addr;val;... (up to 32 values)
 * cmd_line points past first semi-colon. */
ERR_F lsim_cmd_loadmem(lsim_t *lsim, char *cmd_line) {
  char *semi_colon;

  char *dev_name = cmd_line;
  ERR_ASSRT(semi_colon = strchr(dev_name, ';'), LSIM_ERR_COMMAND);
  *semi_colon = '\0';
  /* Don't call lsim_valid_name(dev_name); user needs to be able to use
   * switch component within a panel. */

  char *addr_s = semi_colon + 1;
  ERR_ASSRT(semi_colon = strchr(addr_s, ';'), LSIM_ERR_COMMAND);
  *semi_colon = '\0';  /* Overwrite semicolon. */
  long addr;
  ERR(err_atol(addr_s, &addr));

  /* Loop till end of line. */
  char *end_field = semi_colon + 1;
  int num_words = 0;
# define LOADMEM_MAX_WORDS 64
  uint64_t words[LOADMEM_MAX_WORDS];
  while (strlen(end_field) != 0) {
    ERR_ASSRT(num_words < LOADMEM_MAX_WORDS, LSIM_ERR_COMMAND);

    /* Not end of line yet, convert value. */
    char *val_s = end_field;
    ERR_ASSRT(semi_colon = strchr(val_s, ';'), LSIM_ERR_COMMAND);
    *semi_colon = '\0'; /* Overwrite semicolon. */
    long val;
    ERR(err_atol(val_s, &val));
    words[num_words] = (uint64_t)val;
    num_words++;

    end_field = semi_colon + 1;
  }

  ERR(lsim_dev_loadmem(lsim, dev_name, addr, num_words, words));

  return ERR_OK;
}  /* lsim_cmd_loadmem */


/* Move:
 * m;swtch_device_name;new_state;
 * cmd_line points past first semi-colon. */
ERR_F lsim_cmd_movesw(lsim_t *lsim, char *cmd_line) {
  char *semi_colon;

  char *dev_name = cmd_line;
  ERR_ASSRT(semi_colon = strchr(dev_name, ';'), LSIM_ERR_COMMAND);
  *semi_colon = '\0';
  /* Don't call lsim_valid_name(dev_name); user needs to be able to use
   * switch component within a panel. */

  char *new_state_s = semi_colon + 1;
  ERR_ASSRT(semi_colon = strchr(new_state_s, ';'), LSIM_ERR_COMMAND);
  *semi_colon = '\0';  /* Overwrite semicolon. */

  /* Make sure we're at end of line. */
  char *end_field = semi_colon + 1;
  ERR_ASSRT(strlen(end_field) == 0, LSIM_ERR_COMMAND);

  long new_state;
  ERR(err_atol(new_state_s, &new_state));
  ERR_ASSRT(new_state == 0 || new_state == 1, LSIM_ERR_COMMAND);

  ERR(lsim_dev_move(lsim, dev_name, new_state));

  return ERR_OK;
}  /* lsim_cmd_movesw */


/* ticklet:
 * t;num_ticklets;
 * cmd_line points past first semi-colon. */
ERR_F lsim_cmd_ticklet(lsim_t *lsim, char *cmd_line) {
  char *semi_colon;

  char *num_ticklets_s = cmd_line;
  ERR_ASSRT(semi_colon = strchr(num_ticklets_s, ';'), LSIM_ERR_COMMAND);
  *semi_colon = '\0';  /* Overwrite semicolon. */

  /* Make sure we're at end of line. */
  char *end_field = semi_colon + 1;
  ERR_ASSRT(strlen(end_field) == 0, LSIM_ERR_COMMAND);

  long num_ticklets;
  ERR(err_atol(num_ticklets_s, &num_ticklets));
  ERR_ASSRT(num_ticklets > 0, LSIM_ERR_COMMAND);

  err_t *err;
  long ticklet_num;
  for (ticklet_num = 0; ticklet_num < num_ticklets; ticklet_num++) {
    err = lsim_dev_ticklet(lsim);
    if (err) {
      ERR_RETHROW(err, "Step command '%s' had error %s in ticklet %ld", cmd_line, err->code, ticklet_num);
    }
  }

  return ERR_OK;
}  /* lsim_cmd_ticklet */


/* verbosity:
 * v;verbosity_level;
 * cmd_line points past first semi-colon. */
ERR_F lsim_cmd_verbosity(lsim_t *lsim, char *cmd_line) {
  char *semi_colon;

  char *verbosity_level_s = cmd_line;
  ERR_ASSRT(semi_colon = strchr(verbosity_level_s, ';'), LSIM_ERR_COMMAND);
  *semi_colon = '\0';  /* Overwrite semicolon. */

  /* Make sure we're at end of line. */
  char *end_field = semi_colon + 1;
  ERR_ASSRT(strlen(end_field) == 0, LSIM_ERR_COMMAND);

  long verbosity_level;
  ERR(err_atol(verbosity_level_s, &verbosity_level));
  ERR_ASSRT(verbosity_level >= 0 && verbosity_level <= 2, LSIM_ERR_COMMAND);
  lsim->verbosity_level = (int)verbosity_level;

  return ERR_OK;
}  /* lsim_cmd_verbosity */


/* include:
 * i;filename;
 * cmd_line points past first semi-colon. */
ERR_F lsim_cmd_include(lsim_t *lsim, char *cmd_line) {
  char *semi_colon;

  char *filename = cmd_line;
  ERR_ASSRT(semi_colon = strchr(filename, ';'), LSIM_ERR_COMMAND);
  *semi_colon = '\0';  /* Overwrite semicolon. */

  /* Make sure we're at end of line. */
  char *end_field = semi_colon + 1;
  ERR_ASSRT(strlen(end_field) == 0, LSIM_ERR_COMMAND);

  ERR(lsim_cmd_file(lsim, filename));

  return ERR_OK;
}  /* lsim_cmd_include */


/* Watch:
 * w;device_name;watch_level;
 * cmd_line points past first semi-colon. */
ERR_F lsim_cmd_watchdev(lsim_t *lsim, char *cmd_line) {
  char *semi_colon;

  char *dev_name = cmd_line;
  ERR_ASSRT(semi_colon = strchr(dev_name, ';'), LSIM_ERR_COMMAND);
  *semi_colon = '\0';
  ERR(lsim_valid_name(dev_name));

  char *watch_level_s = semi_colon + 1;
  ERR_ASSRT(semi_colon = strchr(watch_level_s, ';'), LSIM_ERR_COMMAND);
  *semi_colon = '\0';  /* Overwrite semicolon. */

  /* Make sure we're at end of line. */
  char *end_field = semi_colon + 1;
  ERR_ASSRT(strlen(end_field) == 0, LSIM_ERR_COMMAND);

  long watch_level;
  ERR(err_atol(watch_level_s, &watch_level));
  ERR_ASSRT(watch_level >= 0 || watch_level <= 2, LSIM_ERR_COMMAND);

  ERR(lsim_dev_watch(lsim, dev_name, (int)watch_level));

  return ERR_OK;
}  /* lsim_cmd_watchdev */


/* Quit:
 * q;
 * cmd_line points past first semi-colon. */
ERR_F lsim_cmd_quit(lsim_t *lsim, char *cmd_line) {
  /* Make sure we're at end of line. */
  char *end_field = cmd_line;
  ERR_ASSRT(strlen(end_field) == 0, LSIM_ERR_COMMAND);

  lsim->quit = 1;

  return ERR_OK;
}  /* lsim_cmd_quit */


/*******************************************************************************/


ERR_F lsim_cmd_line(lsim_t *lsim, const char *cmd_line) {

  /* Find index of last character that isn't a line ending. */
  int last_c = strlen(cmd_line) - 1;
  while (last_c >= 0 && (cmd_line[last_c] == '\r' || cmd_line[last_c] == '\n')) {
    last_c--;
  }
  /* Ignore "empty" lines. */
  if (last_c < 0 || cmd_line[0] == '#') {
    return ERR_OK;
  }

  /* Need local copy because we modify the string later. */
  char *local_cmd_line;
  ERR(err_strdup(&local_cmd_line, cmd_line));
  /* Strip line endings. */
  local_cmd_line[last_c + 1] = '\0';

  err_t *err;
  if (strstr(local_cmd_line, "b;") == local_cmd_line) {
    err = lsim_cmd_busconn(lsim, &local_cmd_line[2]);
  }
  else if (strstr(local_cmd_line, "c;") == local_cmd_line) {
    err = lsim_cmd_connect(lsim, &local_cmd_line[2]);
  }
  else if (strstr(local_cmd_line, "d;") == local_cmd_line) {
    err = lsim_cmd_define(lsim, &local_cmd_line[2]);
  }
  else if (strstr(local_cmd_line, "i;") == local_cmd_line) {
    err = lsim_cmd_include(lsim, &local_cmd_line[2]);
  }
  else if (strstr(local_cmd_line, "l;") == local_cmd_line) {
    err = lsim_cmd_loadmem(lsim, &local_cmd_line[2]);
  }
  else if (strstr(local_cmd_line, "m;") == local_cmd_line) {
    err = lsim_cmd_movesw(lsim, &local_cmd_line[2]);
  }
  else if (strstr(local_cmd_line, "p;") == local_cmd_line) {
    err = lsim_cmd_power(lsim, &local_cmd_line[2]);
  }
  else if (strstr(local_cmd_line, "q;") == local_cmd_line) {
    err = lsim_cmd_quit(lsim, &local_cmd_line[2]);
  }
  else if (strstr(local_cmd_line, "t;") == local_cmd_line) {
    err = lsim_cmd_ticklet(lsim, &local_cmd_line[2]);
  }
  else if (strstr(local_cmd_line, "v;") == local_cmd_line) {
    err = lsim_cmd_verbosity(lsim, &local_cmd_line[2]);
  }
  else if (strstr(local_cmd_line, "w;") == local_cmd_line) {
    err = lsim_cmd_watchdev(lsim, &local_cmd_line[2]);
  }
  else {
    free(local_cmd_line);
    ERR_THROW(LSIM_ERR_COMMAND, "Unrecognized command '%s'", cmd_line);
  }

  free(local_cmd_line);

  if (err) {
    /* Improve error reporting. */
    ERR_RETHROW(err, "Error processing '%s'", cmd_line);
  }

  return ERR_OK;
}  /* lsim_cmd_line */


ERR_F lsim_cmd_file(lsim_t *lsim, const char *filename) {
  FILE *cmd_file_fp;
  char iline[1024];

  ERR_ASSRT(lsim, LSIM_ERR_PARAM);
  ERR_ASSRT(filename, LSIM_ERR_PARAM);

  if (lsim->quit) {
    return ERR_OK;
  }

  if (strcmp(filename, "-") == 0) {
    cmd_file_fp = stdin;
  } else {
    cmd_file_fp = fopen(filename, "r");
  }
  ERR_ASSRT(cmd_file_fp, LSIM_ERR_BADFILE);

  int line_num = 0;
  err_t *err = ERR_OK;
  while (fgets(iline, sizeof(iline), cmd_file_fp)) {
    line_num++;
    size_t len = strlen(iline);
    ERR_ASSRT(len < sizeof(iline) - 1, LSIM_ERR_LINETOOLONG);  /* Line too long. */

    while (len > 0 && (iline[len-1] == '\n' || iline[len-1] == '\r')) {
      len--;
      iline[len] = '\0';
    }
    if (len > 0) {
      if (lsim->verbosity_level > 0) {
        printf("Trace: %s:%d, '%s'\n", filename, line_num, iline);
      }
      err = lsim_cmd_line(lsim, iline);
      if (err) {
        fprintf(stderr, "Error %s:%d '%s':\n", filename, line_num, iline);
        switch (global_error_reaction) {
        case 0: ERR_ABRT_ON_ERR(err, stderr); break;
        case 1: ERR_EXIT_ON_ERR(err, stderr); break;
        case 2:
          ERR_WARN_ON_ERR(err, stderr);
          break;
        }
      }
    }
    if (lsim->quit) { break; }
  }  /* while */

  if (strcmp(filename, "-") == 0) {
    /* Don't close stdin. */
  } else {
    fclose(cmd_file_fp);
  }

  if (err) {
    ERR_RETHROW(err, "file:line='%s:%d", filename, line_num);
  }
  return ERR_OK;
}  /* lsim_cmd_file */

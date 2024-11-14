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


ERR_F lsim_valid_name(const char *name) {
  ERR_ASSRT(name, LSIM_ERR_NAME);
  ERR_ASSRT(isalpha(*name) || *name == '_' || *name == '-', LSIM_ERR_NAME);

  name++;
  while (*name != '\0') {
    ERR_ASSRT(isalnum(*name) || *name == '_' || *name == '-', LSIM_ERR_NAME);
    name++;
  }

  return ERR_OK;
}  /* lsim_valid_name */


ERR_F lsim_device_vcc_create(lsim_t *lsim, char *name) {
  err_t *err;
  device_t *dev;
  ERR_ASSRT(dev = calloc(1, sizeof(device_t)), LSIM_ERR_NOMEM);
  ERR_ASSRT(dev->name = strdup(name), LSIM_ERR_NOMEM);
  dev->type = LSIM_DEV_TYPE_VCC;

  /* Make sure name doesn't already exist. */
  err = hmap_lookup(lsim->devs, name, strlen(name), NULL);
  ERR_ASSRT(err && err->code == HMAP_ERR_NOTFOUND, LSIM_ERR_EXIST);

  ERR(hmap_write(lsim->devs, name, strlen(name), dev));

  return ERR_OK;
}  /* lsim_device_vcc_create */


ERR_F lsim_device_vcc_reset(lsim_t *lsim, device_t *dev) {
  ERR_ASSRT(dev->type == LSIM_DEV_TYPE_VCC, LSIM_ERR_PARAM);

  dev->vcc.out_state = 1;
  dev->next_out_changed = lsim->out_changed_list;
  lsim->out_changed_list = dev;

  return ERR_OK;
}  /* lsim_device_vcc_reset */


ERR_F lsim_device_nand_create(lsim_t *lsim, char *name, long num_inputs) {
  ERR_ASSRT(num_inputs >= 1 && num_inputs <= 1024, LSIM_ERR_PARAM);

  device_t *dev;
  ERR_ASSRT(dev = calloc(1, sizeof(device_t)), LSIM_ERR_NOMEM);
  ERR_ASSRT(dev->name = strdup(name), LSIM_ERR_NOMEM);
  dev->type = LSIM_DEV_TYPE_NAND;
  dev->nand.num_inputs = num_inputs;
  ERR_ASSRT(dev->nand.in_states = calloc(num_inputs, sizeof(int)), LSIM_ERR_NOMEM);

  ERR(hmap_write(lsim->devs, name, strlen(name), dev));

  return ERR_OK;
}  /* lsim_device_nand_create */


ERR_F lsim_device_nand_reset(lsim_t *lsim, device_t *dev) {
  ERR_ASSRT(dev->type == LSIM_DEV_TYPE_NAND, LSIM_ERR_COMMAND);

  int input_index;
  dev->nand.out_state = 0;
  for (input_index = 0; input_index < dev->nand.num_inputs; input_index++) {
    dev->nand.in_states[input_index] = 0;
  }
  dev->next_in_changed = lsim->in_changed_list;
  lsim->in_changed_list = dev;
  dev->next_out_changed = lsim->out_changed_list;
  lsim->out_changed_list = dev;

  return ERR_OK;
}  /* lsim_device_nand_reset */


ERR_F lsim_interp_d_vcc(lsim_t *lsim, const char *cmd_line) {
  char *semi_colon;

  /* Need local copy because strtoc modifies string. */
  char *local_cmd_line;
  ERR_ASSRT(local_cmd_line = strdup(cmd_line), CFG_ERR_NOMEM);

  char *device_name = local_cmd_line;
  ERR_ASSRT(semi_colon = strchr(device_name, ';'), LSIM_ERR_COMMAND);
  *semi_colon = '\0';
  ERR(lsim_valid_name(device_name));

  /* Make sure we're at the end of the line. */
  char *end_field = semi_colon + 1;
  ERR_ASSRT(strchr("\r\n\0", *end_field), LSIM_ERR_COMMAND);

  ERR(lsim_device_vcc_create(lsim, device_name));

  free(local_cmd_line);
  return ERR_OK;
}  /* lsim_interp_d_vcc */


ERR_F lsim_interp_d_nand(lsim_t *lsim, const char *cmd_line) {
  char *semi_colon;

  /* Need local copy because we modify the string. */
  char *local_cmd_line;
  ERR_ASSRT(local_cmd_line = strdup(cmd_line), CFG_ERR_NOMEM);

  char *device_name = local_cmd_line;
  ERR_ASSRT(semi_colon = strchr(device_name, ';'), LSIM_ERR_COMMAND);
  *semi_colon = '\0';
  ERR(lsim_valid_name(device_name));

  char *num_inputs_field = semi_colon + 1;
  ERR_ASSRT(semi_colon = strchr(num_inputs_field, ';'), LSIM_ERR_COMMAND);
  semi_colon = '\0';  /* Overwrite semicolon. */

  char *end_field = semi_colon + 1;
  ERR_ASSRT(strchr("\r\n\0", *end_field), LSIM_ERR_COMMAND);

  long num_inputs;
  ERR(cfg_atol(num_inputs_field, &num_inputs));

  ERR(lsim_device_nand_create(lsim, device_name, num_inputs));

  free(local_cmd_line);
  return ERR_OK;
}  /* lsim_interp_d_nand */


ERR_F lsim_interp_d(lsim_t *lsim, const char *cmd_line) {
  char *semi_colon;
  ERR_ASSRT(semi_colon = strchr(cmd_line, ';'), LSIM_ERR_COMMAND);

  char *next_field = semi_colon + 1;
  if (strstr(cmd_line, "vcc;") == cmd_line) {
    ERR(lsim_interp_d_vcc(lsim, next_field));
  }
/*???
 *else if (strstr(cmd_line, "gnd;") == cmd_line) {
 *  ERR(lsim_interp_d_gnd(lsim, next_field));
 *}
 *else if (strstr(cmd_line, "clk1;") == cmd_line) {
 *   ERR(lsim_interp_d_clk1(lsim, next_field));
 * }
 */
  else if (strstr(cmd_line, "nand;") == cmd_line) {
    ERR(lsim_interp_d_nand(lsim, next_field));
  }
/*???
 *else if (strstr(cmd_line, "mem;") == cmd_line) {
 *  ERR(lsim_interp_d_mem(lsim, next_field));
 *}
 */
  else { ERR_THROW(LSIM_ERR_COMMAND, "Unrecognized device type"); }

  return ERR_OK;
}  /* lsim_interp_d */


ERR_F lsim_interp_w(lsim_t *lsim, const char *cmd_line) {
  char *semi_colon;
  ERR_ASSRT(semi_colon = strchr(cmd_line, ';'), LSIM_ERR_COMMAND);

  char *next_field = semi_colon + 1;
  if (strstr(cmd_line, "vcc;") == cmd_line) {
    ERR(lsim_interp_d_vcc(lsim, next_field));
  }
/*???
 *else if (strstr(cmd_line, "gnd;") == cmd_line) {
 *  ERR(lsim_interp_d_gnd(lsim, next_field));
 *}
 *else if (strstr(cmd_line, "clk1;") == cmd_line) {
 *   ERR(lsim_interp_d_clk1(lsim, next_field));
 * }
 */
  else if (strstr(cmd_line, "nand;") == cmd_line) {
    ERR(lsim_interp_d_nand(lsim, next_field));
  }
/*???
 *else if (strstr(cmd_line, "mem;") == cmd_line) {
 *  ERR(lsim_interp_d_mem(lsim, next_field));
 *}
 */
  else { ERR_THROW(LSIM_ERR_COMMAND, "Unrecognized device type"); }

  return ERR_OK;
}  /* lsim_interp_w */


ERR_F lsim_interp_cmd_line(lsim_t *lsim, const char *cmd_line) {
  /* Ignore "empty" lines. */
  if (strchr("#\r\n\0", cmd_line[0])) { return ERR_OK; }

  if (strstr(cmd_line, "d;") == cmd_line) {
    ERR(lsim_interp_d(lsim, &cmd_line[2]));
  }
  else if (strstr(cmd_line, "w;") == cmd_line) {
    ERR(lsim_interp_w(lsim, &cmd_line[2]));
  }
/*???
 *else if (strstr(cmd_line, "i;") == cmd_line) {
    ERR(lsim_interp_i(lsim, &cmd_line[2]));
  }
 *else if (strstr(cmd_line, "r;") == cmd_line) {
    ERR(lsim_interp_r(lsim, &cmd_line[2]));
  }
 *else if (strstr(cmd_line, "s;") == cmd_line) {
    ERR(lsim_interp_s(lsim, &cmd_line[2]));
  }
 *else if (strstr(cmd_line, "t;") == cmd_line) {
    ERR(lsim_interp_t(lsim, &cmd_line[2]));
  }
 *else if (strstr(cmd_line, "p;") == cmd_line) {
    ERR(lsim_interp_p(lsim, &cmd_line[2]));
  }
 *else if (strstr(cmd_line, "D;") == cmd_line) {
    ERR(lsim_interp_D(lsim, &cmd_line[2]));
  }
 *else if (strstr(cmd_line, "q;") == cmd_line) {
    ERR(lsim_interp_q(lsim, &cmd_line[2]));
  }
 */
  else { ERR_THROW(LSIM_ERR_COMMAND, "Unrecognized command"); }

  return ERR_OK;
}  /* lsim_interp_cmd_line */


ERR_F lsim_interp_cmd_file(lsim_t *lsim, const char *cmd_file_name) {
  FILE *cmd_file_fp;
  char iline[1024];

  ERR_ASSRT(lsim, LSIM_ERR_PARAM);
  ERR_ASSRT(cmd_file_name, LSIM_ERR_PARAM);
  if (strcmp(cmd_file_name, "-") == 0) {
    cmd_file_fp = stdin;
  } else {
    cmd_file_fp = fopen(cmd_file_name, "r");
  }
  ERR_ASSRT(cmd_file_fp, LSIM_ERR_BADFILE);

  err_t *err = ERR_OK;
  while (fgets(iline, sizeof(iline), cmd_file_fp)) {
    size_t len = strlen(iline);
    ERR_ASSRT(len < sizeof(iline) - 1, LSIM_ERR_LINETOOLONG);  /* Line too long. */

    err = lsim_interp_cmd_line(lsim, iline);
    if (err) { break; }
  }  /* while */

  if (strcmp(cmd_file_name, "-") == 0) {
    /* Don't close stdin. */
  } else {
    fclose(cmd_file_fp);
  }

  if (err) {
    ERR_RETHROW(err, err->code);
  }
  return ERR_OK;
}  /* lsim_interp_cmd_file */

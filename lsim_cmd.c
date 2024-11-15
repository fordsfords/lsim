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
  ERR_ASSRT(name, LSIM_ERR_INTERNAL);

  if (!(isalpha(*name) || *name == '_' || *name == '-')) {
    ERR_THROW(LSIM_ERR_COMMAND, "invalid name '%s'", name);
  }

  name++;
  while (*name != '\0') {
    if (!(isalnum(*name) || *name == '_' || *name == '-')) {
      ERR_THROW(LSIM_ERR_COMMAND, "invalid name '%s'", name);
    }
    name++;
  }

  return ERR_OK;
}  /* lsim_valid_name */


ERR_F lsim_device_vcc_get_input_state(lsim_t *lsim, lsim_device_t *dev, const char *input_id, int **rtn_state_p) {
  ERR_THROW(LSIM_ERR_COMMAND, "Attempt to get input state for vcc, which has no inputs");

  return ERR_OK;
}  /* lsim_device_vcc_get_input_state */


ERR_F lsim_device_vcc_get_output_wire(lsim_t *lsim, lsim_device_t *dev, const char *output_id, lsim_wire_t **rtn_wire_p) {
  /* There's only one output; ignore output_id. */
  *rtn_wire_p = &dev->vcc.out_wire;

  return ERR_OK;
}  /* lsim_device_vcc_get_output_wire */


ERR_F lsim_device_vcc_create(lsim_t *lsim, char *name) {
  /* Make sure name doesn't already exist. */
  err_t *err;
  err = hmap_lookup(lsim->devs, name, strlen(name), NULL);
  ERR_ASSRT(err && err->code == HMAP_ERR_NOTFOUND, LSIM_ERR_EXIST);

  lsim_device_t *dev;
  ERR_ASSRT(dev = calloc(1, sizeof(lsim_device_t)), LSIM_ERR_NOMEM);
  ERR_ASSRT(dev->name = strdup(name), LSIM_ERR_NOMEM);
  dev->type = LSIM_DEV_TYPE_VCC;
  dev->get_input_state_p = lsim_device_vcc_get_input_state;
  dev->get_output_wire_p = lsim_device_vcc_get_output_wire;

  ERR(hmap_write(lsim->devs, name, strlen(name), dev));

  return ERR_OK;
}  /* lsim_device_vcc_create */


ERR_F lsim_device_vcc_reset(lsim_t *lsim, lsim_device_t *dev) {
  ERR_ASSRT(dev->type == LSIM_DEV_TYPE_VCC, LSIM_ERR_PARAM);

  dev->vcc.out_state = 1;
  dev->next_out_changed = lsim->out_changed_list;
  lsim->out_changed_list = dev;

  return ERR_OK;
}  /* lsim_device_vcc_reset */


ERR_F lsim_device_nand_get_input_state(lsim_t *lsim, lsim_device_t *dev, const char *input_id, int **rtn_state_p) {
  long input_num;
  ERR(cfg_atol(input_id, &input_num));
  if (input_num > dev->nand.num_inputs) {
    ERR_THROW(LSIM_ERR_COMMAND, "input number %d larger than number of inputs %d", (int)input_num, dev->nand.num_inputs);
  }

  *rtn_state_p = &dev->nand.in_states[input_num];

  return ERR_OK;
}  /* lsim_device_nand_get_input_state */


ERR_F lsim_device_nand_get_output_wire(lsim_t *lsim, lsim_device_t *dev, const char *output_id, lsim_wire_t **rtn_wire_p) {
  /* There's only one output; ignore output_id. */
  *rtn_wire_p = &dev->nand.out_wire;

  return ERR_OK;
}  /* lsim_device_nand_get_output_wire */


ERR_F lsim_device_nand_create(lsim_t *lsim, char *name, long num_inputs) {
  ERR_ASSRT(num_inputs >= 1, LSIM_ERR_PARAM);

  /* Make sure name doesn't already exist. */
  err_t *err;
  err = hmap_lookup(lsim->devs, name, strlen(name), NULL);
  ERR_ASSRT(err && err->code == HMAP_ERR_NOTFOUND, LSIM_ERR_EXIST);

  lsim_device_t *dev;
  ERR_ASSRT(dev = calloc(1, sizeof(lsim_device_t)), LSIM_ERR_NOMEM);
  ERR_ASSRT(dev->name = strdup(name), LSIM_ERR_NOMEM);
  dev->type = LSIM_DEV_TYPE_NAND;
  dev->nand.num_inputs = num_inputs;
  ERR_ASSRT(dev->nand.in_states = calloc(num_inputs, sizeof(int)), LSIM_ERR_NOMEM);
  dev->get_input_state_p = lsim_device_nand_get_input_state;
  dev->get_output_wire_p = lsim_device_nand_get_output_wire;

  ERR(hmap_write(lsim->devs, name, strlen(name), dev));

  return ERR_OK;
}  /* lsim_device_nand_create */


ERR_F lsim_device_nand_reset(lsim_t *lsim, lsim_device_t *dev) {
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


ERR_F lsim_wire_create(lsim_t *lsim, const char *src_device_name, const char *src_output_id, const char *dst_device_name, const char *dst_input_id) {
  lsim_device_t *src_device, *dst_device;

  /* Find the two devices. */
  ERR(hmap_lookup(lsim->devs, src_device_name, strlen(src_device_name), (void **)&src_device));
  ERR(hmap_lookup(lsim->devs, dst_device_name, strlen(dst_device_name), (void **)&dst_device));

  lsim_wire_t *src_wire;
  ERR(src_device->get_output_wire_p(lsim, src_device, src_output_id, &src_wire));
  int *input_state;
  ERR(dst_device->get_input_state_p(lsim, dst_device, dst_input_id, &input_state));

  lsim_wire_segment_t *src_wire_segment;
  ERR_ASSRT(src_wire_segment = calloc(1, sizeof(lsim_wire_segment_t)), LSIM_ERR_NOMEM);
  src_wire_segment->src_device = src_device;
  src_wire_segment->dst_device = dst_device;
  src_wire_segment->input_state = input_state;
  src_wire_segment->next_segment = src_wire->dst_segment;
  src_wire->dst_segment = src_wire_segment;

  return ERR_OK;
}  /* lsim_wire_create */


/* Define device "vcc":
 * d;vcc;device_name;
 * cmd_line points at device_name. */
ERR_F lsim_interp_d_vcc(lsim_t *lsim, char *cmd_line) {
  char *semi_colon;

  char *device_name = cmd_line;
  ERR_ASSRT(semi_colon = strchr(device_name, ';'), LSIM_ERR_COMMAND);
  *semi_colon = '\0';
  ERR(lsim_valid_name(device_name));

  /* Make sure we're at the end of the line. */
  char *end_field = semi_colon + 1;
  ERR_ASSRT(strlen(end_field) == 0, LSIM_ERR_COMMAND);

  ERR(lsim_device_vcc_create(lsim, device_name));

  return ERR_OK;
}  /* lsim_interp_d_vcc */


/* Define device "nand":
 * d;mem;device_name;num_addr;num_data;
 * cmd_line points at device_name. */
ERR_F lsim_interp_d_nand(lsim_t *lsim, char *cmd_line) {
  char *semi_colon;

  char *device_name = cmd_line;
  ERR_ASSRT(semi_colon = strchr(device_name, ';'), LSIM_ERR_COMMAND);
  *semi_colon = '\0';
  ERR(lsim_valid_name(device_name));

  char *num_inputs_s = semi_colon + 1;
  ERR_ASSRT(semi_colon = strchr(num_inputs_s, ';'), LSIM_ERR_COMMAND);
  *semi_colon = '\0';  /* Overwrite semicolon. */

  /* Make sure we're at end of line. */
  char *end_field = semi_colon + 1;
  ERR_ASSRT(strlen(end_field) == 0, LSIM_ERR_COMMAND);

  long num_inputs;
  ERR(cfg_atol(num_inputs_s, &num_inputs));

  ERR(lsim_device_nand_create(lsim, device_name, num_inputs));

  return ERR_OK;
}  /* lsim_interp_d_nand */


/* Define device:
 * d;device_type;...
 * cmd_line points at device_type. */
ERR_F lsim_interp_d(lsim_t *lsim, char *cmd_line) {
  char *semi_colon;

  char *device_type = cmd_line;
  ERR_ASSRT(semi_colon = strchr(device_type, ';'), LSIM_ERR_COMMAND);
  *semi_colon = '\0';

  char *next_field = semi_colon + 1;

  if (strcmp(device_type, "vcc") == 0) {
    ERR(lsim_interp_d_vcc(lsim, next_field));
  }
/*???
 *else if (strcmp(device_type, "gnd") == 0) {
 *  ERR(lsim_interp_d_gnd(lsim, next_field));
 *}
 *else if (strcmp(device_type, "clk1") == 0) {
 *   ERR(lsim_interp_d_clk1(lsim, next_field));
 * }
 */
  else if (strcmp(device_type, "nand") == 0) {
    ERR(lsim_interp_d_nand(lsim, next_field));
  }
/*???
 *else if (strcmp(device_type, "mem") == 0) {
 *  ERR(lsim_interp_d_mem(lsim, next_field));
 *}
 */
  else {
    ERR_THROW(LSIM_ERR_COMMAND, "Unrecognized device type '%s'", device_type);
  }

  return ERR_OK;
}  /* lsim_interp_d */


/* Define wire:
 * w;src_device_name;src_output_id;dst_device_name;dst_input_id;
 * cmd_line points at src_device_name. */
ERR_F lsim_interp_w(lsim_t *lsim, char *cmd_line) {
  char *semi_colon;

  char *src_device_name = cmd_line;
  ERR_ASSRT(semi_colon = strchr(src_device_name, ';'), LSIM_ERR_COMMAND);
  *semi_colon = '\0';
  ERR(lsim_valid_name(src_device_name));

  char *src_output_id = semi_colon + 1;
  ERR_ASSRT(semi_colon = strchr(src_output_id, ';'), LSIM_ERR_COMMAND);
  *semi_colon = '\0';

  char *dst_device_name = semi_colon + 1;
  ERR_ASSRT(semi_colon = strchr(dst_device_name, ';'), LSIM_ERR_COMMAND);
  *semi_colon = '\0';
  ERR(lsim_valid_name(src_device_name));

  char *dst_input_id = semi_colon + 1;
  ERR_ASSRT(semi_colon = strchr(dst_input_id, ';'), LSIM_ERR_COMMAND);
  *semi_colon = '\0';

  /* Make sure we're at end of line. */
  char *end_field = semi_colon + 1;
  ERR_ASSRT(strlen(end_field) == 0, LSIM_ERR_COMMAND);

  ERR(lsim_wire_create(lsim, src_device_name, src_output_id, dst_device_name, dst_input_id));

  return ERR_OK;
}  /* lsim_interp_w */


ERR_F lsim_interp_cmd_line(lsim_t *lsim, const char *cmd_line) {

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
  ERR_ASSRT(local_cmd_line = strdup(cmd_line), CFG_ERR_NOMEM);
  /* Strip line endings. */
  local_cmd_line[last_c + 1] = '\0';

  err_t *err;
  if (strstr(local_cmd_line, "d;") == local_cmd_line) {
    err = lsim_interp_d(lsim, &local_cmd_line[2]);
  }
  else if (strstr(local_cmd_line, "w;") == local_cmd_line) {
    err = lsim_interp_w(lsim, &local_cmd_line[2]);
  }
/*???
 *else if (strstr(local_cmd_line, "i;") == local_cmd_line) {
    err = lsim_interp_i(lsim, &local_cmd_line[2]);
  }
 *else if (strstr(local_cmd_line, "r;") == local_cmd_line) {
    err = lsim_interp_r(lsim, &local_cmd_line[2]);
  }
 *else if (strstr(local_cmd_line, "s;") == local_cmd_line) {
    err = lsim_interp_s(lsim, &local_cmd_line[2]);
  }
 *else if (strstr(local_cmd_line, "t;") == local_cmd_line) {
    err = lsim_interp_t(lsim, &local_cmd_line[2]);
  }
 *else if (strstr(local_cmd_line, "p;") == local_cmd_line) {
    err = lsim_interp_p(lsim, &local_cmd_line[2]);
  }
 *else if (strstr(local_cmd_line, "D;") == local_cmd_line) {
    err = lsim_interp_D(lsim, &local_cmd_line[2]);
  }
 *else if (strstr(local_cmd_line, "q;") == local_cmd_line) {
    err = lsim_interp_q(lsim, &local_cmd_line[2]);
  }
 */
  else {
    free(local_cmd_line);
    ERR_THROW(LSIM_ERR_COMMAND, "Unrecognized command '%s'", cmd_line);
  }

  free(local_cmd_line);

  if (err) {
    /* Improve error reporting. */
    ERR_RETHROW(err, "Error parsing '%s'", cmd_line);
  }

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

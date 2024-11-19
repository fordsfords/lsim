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
#include "lsim_cmd.h"


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


/*******************************************************************************/


/* Define device "vcc":
 * d;vcc;dev_name;
 * cmd_line points at dev_name. */
ERR_F lsim_cmd_d_vcc(lsim_t *lsim, char *cmd_line) {
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
}  /* lsim_cmd_d_vcc */


/* Define device "led":
 * d;led;dev_name;
 * cmd_line points at dev_name. */
ERR_F lsim_cmd_d_led(lsim_t *lsim, char *cmd_line) {
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
}  /* lsim_cmd_d_led */


/* Define device "nand":
 * d;mem;dev_name;num_addr;num_data;
 * cmd_line points at dev_name. */
ERR_F lsim_cmd_d_nand(lsim_t *lsim, char *cmd_line) {
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
  ERR(cfg_atol(num_inputs_s, &num_inputs));

  ERR(lsim_dev_nand_create(lsim, dev_name, num_inputs));

  return ERR_OK;
}  /* lsim_cmd_d_nand */


/* Define device:
 * d;dev_type;...
 * cmd_line points at dev_type. */
ERR_F lsim_cmd_d(lsim_t *lsim, char *cmd_line) {
  char *semi_colon;

  char *dev_type = cmd_line;
  ERR_ASSRT(semi_colon = strchr(dev_type, ';'), LSIM_ERR_COMMAND);
  *semi_colon = '\0';

  char *next_field = semi_colon + 1;

  if (strcmp(dev_type, "gnd") == 0) {
    ERR(lsim_cmd_d_gnd(lsim, next_field));
  }
  else if (strcmp(dev_type, "vcc") == 0) {
    ERR(lsim_cmd_d_vcc(lsim, next_field));
  }
  else if (strcmp(dev_type, "led") == 0) {
    ERR(lsim_cmd_d_led(lsim, next_field));
  }
/*???
 *else if (strcmp(dev_type, "clk1") == 0) {
 *   ERR(lsim_cmd_d_clk1(lsim, next_field));
 * }
 */
  else if (strcmp(dev_type, "nand") == 0) {
    ERR(lsim_cmd_d_nand(lsim, next_field));
  }
/*???
 *else if (strcmp(dev_type, "mem") == 0) {
 *  ERR(lsim_cmd_d_mem(lsim, next_field));
 *}
 */
  else {
    ERR_THROW(LSIM_ERR_COMMAND, "Unrecognized device type '%s'", dev_type);
  }

  return ERR_OK;
}  /* lsim_cmd_d */


/* Define connection:
 * c;src_dev_name;src_output_id;dst_dev_name;dst_input_id;
 * cmd_line points at src_dev_name. */
ERR_F lsim_cmd_c(lsim_t *lsim, char *cmd_line) {
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
  ERR(lsim_valid_name(src_dev_name));

  char *dst_input_id = semi_colon + 1;
  ERR_ASSRT(semi_colon = strchr(dst_input_id, ';'), LSIM_ERR_COMMAND);
  *semi_colon = '\0';

  /* Make sure we're at end of line. */
  char *end_field = semi_colon + 1;
  ERR_ASSRT(strlen(end_field) == 0, LSIM_ERR_COMMAND);

  ERR(lsim_dev_connect(lsim, src_dev_name, src_output_id, dst_dev_name, dst_input_id));

  return ERR_OK;
}  /* lsim_cmd_c */


/* Reset:
 * r;
 * cmd_line points past semi-colon. */
ERR_F lsim_cmd_r(lsim_t *lsim, char *cmd_line) {
  /* Make sure we're at end of line. */
  char *end_field = cmd_line;
  ERR_ASSRT(strlen(end_field) == 0, LSIM_ERR_COMMAND);

  ERR(lsim_dev_reset(lsim));

  return ERR_OK;
}  /* lsim_cmd_r */


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
  ERR_ASSRT(local_cmd_line = strdup(cmd_line), CFG_ERR_NOMEM);
  /* Strip line endings. */
  local_cmd_line[last_c + 1] = '\0';

  err_t *err;
  if (strstr(local_cmd_line, "d;") == local_cmd_line) {
    err = lsim_cmd_d(lsim, &local_cmd_line[2]);
  }
  else if (strstr(local_cmd_line, "c;") == local_cmd_line) {
    err = lsim_cmd_w(lsim, &local_cmd_line[2]);
  }
  else if (strstr(local_cmd_line, "r;") == local_cmd_line) {
    err = lsim_cmd_r(lsim, &local_cmd_line[2]);
  }
/*???
 *else if (strstr(local_cmd_line, "i;") == local_cmd_line) {
    err = lsim_cmd_i(lsim, &local_cmd_line[2]);
  }
 *else if (strstr(local_cmd_line, "s;") == local_cmd_line) {
    err = lsim_cmd_s(lsim, &local_cmd_line[2]);
  }
 *else if (strstr(local_cmd_line, "t;") == local_cmd_line) {
    err = lsim_cmd_t(lsim, &local_cmd_line[2]);
  }
 *else if (strstr(local_cmd_line, "p;") == local_cmd_line) {
    err = lsim_cmd_p(lsim, &local_cmd_line[2]);
  }
 *else if (strstr(local_cmd_line, "D;") == local_cmd_line) {
    err = lsim_cmd_D(lsim, &local_cmd_line[2]);
  }
 *else if (strstr(local_cmd_line, "q;") == local_cmd_line) {
    err = lsim_cmd_q(lsim, &local_cmd_line[2]);
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
}  /* lsim_cmd_line */


ERR_F lsim_cmd_file(lsim_t *lsim, const char *cmd_file_name) {
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

    err = lsim_cmd_line(lsim, iline);
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
}  /* lsim_cmd_file */

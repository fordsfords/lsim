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


/* Define device "gnd":
 * d;gnd;dev_name;
 * cmd_line points at dev_name. */
ERR_F lsim_cmd_d_gnd(lsim_t *lsim, char *cmd_line) {
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
}  /* lsim_cmd_d_gnd */


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


/* Define device "swtch":
 * d;swtch;dev_name;init_state;
 * cmd_line points at dev_name. */
ERR_F lsim_cmd_d_swtch(lsim_t *lsim, char *cmd_line) {
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
  ERR(cfg_atol(init_state_s, &init_state));
  ERR_ASSRT(init_state == 0 || init_state == 1, LSIM_ERR_COMMAND);

  ERR(lsim_dev_swtch_create(lsim, dev_name, (int)init_state));

  return ERR_OK;
}  /* lsim_cmd_d_swtch */


/* Define device "clk":
 * d;clk;dev_name;
 * cmd_line points at dev_name. */
ERR_F lsim_cmd_d_clk(lsim_t *lsim, char *cmd_line) {
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
}  /* lsim_cmd_d_clk */


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
  ERR_ASSRT(num_inputs > 0, LSIM_ERR_COMMAND);

  ERR(lsim_dev_nand_create(lsim, dev_name, num_inputs));

  return ERR_OK;
}  /* lsim_cmd_d_nand */


/* Define device "srlatch":
 * d;mem;dev_name;num_addr;num_data;
 * cmd_line points at dev_name. */
ERR_F lsim_cmd_d_srlatch(lsim_t *lsim, char *cmd_line) {
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
}  /* lsim_cmd_d_srlatch */


/* Define device "dlatch":
 * d;mem;dev_name;num_addr;num_data;
 * cmd_line points at dev_name. */
ERR_F lsim_cmd_d_dlatch(lsim_t *lsim, char *cmd_line) {
  char *semi_colon;

  char *dev_name = cmd_line;
  ERR_ASSRT(semi_colon = strchr(dev_name, ';'), LSIM_ERR_COMMAND);
  *semi_colon = '\0';
  ERR(lsim_valid_name(dev_name));

  /* Make sure we're at end of line. */
  char *end_field = semi_colon + 1;
  ERR_ASSRT(strlen(end_field) == 0, LSIM_ERR_COMMAND);

  ERR(lsim_dev_dlatch_create(lsim, dev_name));

  return ERR_OK;
}  /* lsim_cmd_d_dlatch */


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
  else if (strcmp(dev_type, "swtch") == 0) {
    ERR(lsim_cmd_d_swtch(lsim, next_field));
  }
  else if (strcmp(dev_type, "clk") == 0) {
    ERR(lsim_cmd_d_clk(lsim, next_field));
  }
  else if (strcmp(dev_type, "led") == 0) {
    ERR(lsim_cmd_d_led(lsim, next_field));
  }
  else if (strcmp(dev_type, "nand") == 0) {
    ERR(lsim_cmd_d_nand(lsim, next_field));
  }
  else if (strcmp(dev_type, "srlatch") == 0) {
    ERR(lsim_cmd_d_srlatch(lsim, next_field));
  }
  else if (strcmp(dev_type, "dlatch") == 0) {
    ERR(lsim_cmd_d_dlatch(lsim, next_field));
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
  ERR(lsim_valid_name(dst_dev_name));

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
 * cmd_line points past first semi-colon. */
ERR_F lsim_cmd_p(lsim_t *lsim, char *cmd_line) {
  /* Make sure we're at end of line. */
  char *end_field = cmd_line;
  ERR_ASSRT(strlen(end_field) == 0, LSIM_ERR_COMMAND);

  ERR(lsim_dev_power(lsim));

  return ERR_OK;
}  /* lsim_cmd_p */


/* Move:
 * m;device_name;new_state;
 * cmd_line points past first semi-colon. */
ERR_F lsim_cmd_m(lsim_t *lsim, char *cmd_line) {
  char *semi_colon;

  char *dev_name = cmd_line;
  ERR_ASSRT(semi_colon = strchr(dev_name, ';'), LSIM_ERR_COMMAND);
  *semi_colon = '\0';
  ERR(lsim_valid_name(dev_name));

  char *new_state_s = semi_colon + 1;
  ERR_ASSRT(semi_colon = strchr(new_state_s, ';'), LSIM_ERR_COMMAND);
  *semi_colon = '\0';  /* Overwrite semicolon. */

  /* Make sure we're at end of line. */
  char *end_field = semi_colon + 1;
  ERR_ASSRT(strlen(end_field) == 0, LSIM_ERR_COMMAND);

  long new_state;
  ERR(cfg_atol(new_state_s, &new_state));
  ERR_ASSRT(new_state == 0 || new_state == 1, LSIM_ERR_COMMAND);

  ERR(lsim_dev_move(lsim, dev_name, new_state));

  return ERR_OK;
}  /* lsim_cmd_m */


/* tick:
 * t;num_ticks;
 * cmd_line points past first semi-colon. */
ERR_F lsim_cmd_t(lsim_t *lsim, char *cmd_line) {
  char *semi_colon;

  char *num_ticks_s = cmd_line;
  ERR_ASSRT(semi_colon = strchr(num_ticks_s, ';'), LSIM_ERR_COMMAND);
  *semi_colon = '\0';  /* Overwrite semicolon. */

  /* Make sure we're at end of line. */
  char *end_field = semi_colon + 1;
  ERR_ASSRT(strlen(end_field) == 0, LSIM_ERR_COMMAND);

  long num_ticks;
  ERR(cfg_atol(num_ticks_s, &num_ticks));
  ERR_ASSRT(num_ticks > 0, LSIM_ERR_COMMAND);

  err_t *err;
  long tick_num;
  for (tick_num = 0; tick_num < num_ticks; tick_num++) {
    err = lsim_dev_tick(lsim);
    if (err) {
      ERR_RETHROW(err, "Step command '%s' had error %s in tick %ld", cmd_line, err->code, tick_num);
    }
  }

  return ERR_OK;
}  /* lsim_cmd_t */


/* verbosity:
 * v;verbosity_level;
 * cmd_line points past first semi-colon. */
ERR_F lsim_cmd_v(lsim_t *lsim, char *cmd_line) {
  char *semi_colon;

  char *verbosity_level_s = cmd_line;
  ERR_ASSRT(semi_colon = strchr(verbosity_level_s, ';'), LSIM_ERR_COMMAND);
  *semi_colon = '\0';  /* Overwrite semicolon. */

  /* Make sure we're at end of line. */
  char *end_field = semi_colon + 1;
  ERR_ASSRT(strlen(end_field) == 0, LSIM_ERR_COMMAND);

  long verbosity_level;
  ERR(cfg_atol(verbosity_level_s, &verbosity_level));
  ERR_ASSRT(verbosity_level >= 0 && verbosity_level <= 2, LSIM_ERR_COMMAND);
  lsim->verbosity_level = (int)verbosity_level;

  return ERR_OK;
}  /* lsim_cmd_v */


/* include:
 * i;filename;
 * cmd_line points past first semi-colon. */
ERR_F lsim_cmd_i(lsim_t *lsim, char *cmd_line) {
  char *semi_colon;

  char *filename = cmd_line;
  ERR_ASSRT(semi_colon = strchr(filename, ';'), LSIM_ERR_COMMAND);
  *semi_colon = '\0';  /* Overwrite semicolon. */

  /* Make sure we're at end of line. */
  char *end_field = semi_colon + 1;
  ERR_ASSRT(strlen(end_field) == 0, LSIM_ERR_COMMAND);

  ERR(lsim_cmd_file(lsim, filename));

  return ERR_OK;
}  /* lsim_cmd_i */


/* Watch:
 * w;device_name;watch_level;
 * cmd_line points past first semi-colon. */
ERR_F lsim_cmd_w(lsim_t *lsim, char *cmd_line) {
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
  ERR(cfg_atol(watch_level_s, &watch_level));
  ERR_ASSRT(watch_level >= 0 || watch_level <= 2, LSIM_ERR_COMMAND);

  ERR(lsim_dev_watch(lsim, dev_name, (int)watch_level));

  return ERR_OK;
}  /* lsim_cmd_w */


/* Quit:
 * q;
 * cmd_line points past first semi-colon. */
ERR_F lsim_cmd_q(lsim_t *lsim, char *cmd_line) {
  /* Make sure we're at end of line. */
  char *end_field = cmd_line;
  ERR_ASSRT(strlen(end_field) == 0, LSIM_ERR_COMMAND);

  lsim->quit = 1;

  return ERR_OK;
}  /* lsim_cmd_q */


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
  if (strstr(local_cmd_line, "d;") == local_cmd_line) {
    err = lsim_cmd_d(lsim, &local_cmd_line[2]);
  }
  else if (strstr(local_cmd_line, "c;") == local_cmd_line) {
    err = lsim_cmd_c(lsim, &local_cmd_line[2]);
  }
  else if (strstr(local_cmd_line, "p;") == local_cmd_line) {
    err = lsim_cmd_p(lsim, &local_cmd_line[2]);
  }
  else if (strstr(local_cmd_line, "m;") == local_cmd_line) {
    err = lsim_cmd_m(lsim, &local_cmd_line[2]);
  }
  else if (strstr(local_cmd_line, "t;") == local_cmd_line) {
    err = lsim_cmd_t(lsim, &local_cmd_line[2]);
  }
  else if (strstr(local_cmd_line, "v;") == local_cmd_line) {
    err = lsim_cmd_v(lsim, &local_cmd_line[2]);
  }
  else if (strstr(local_cmd_line, "i;") == local_cmd_line) {
    err = lsim_cmd_i(lsim, &local_cmd_line[2]);
  }
  else if (strstr(local_cmd_line, "w;") == local_cmd_line) {
    err = lsim_cmd_w(lsim, &local_cmd_line[2]);
  }
  else if (strstr(local_cmd_line, "q;") == local_cmd_line) {
    err = lsim_cmd_q(lsim, &local_cmd_line[2]);
  }
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

  long error_level;
  ERR(cfg_get_long_val(lsim->cfg, "error_level", &error_level));
  ERR_ASSRT(error_level >= 0 && error_level <= 2, LSIM_ERR_CONFIG);

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
        switch (error_level) {
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

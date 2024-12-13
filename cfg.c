/* cfg.c - support functions for simple error handler. */
/*
# This code and its documentation is Copyright 2024-2024 Steven Ford, http://geeky-boy.com
# and licensed "public domain" style under Creative Commons "CC0": http://creativecommons.org/publicdomain/zero/1.0/
# To the extent possible under law, the contributors to this project have
# waived all copyright and related or neighboring rights to this work.
# In other words, you can use this code for any purpose without any
# restrictions.  This work is published from: United States.  The project home
# is https://github.com/fordsfords/cfg
*/

#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <errno.h>
#include <ctype.h>
#include "err.h"
#include "hmap.h"
#define CFG_C
#include "cfg.h"


/* This prime number tells hmap how big of a table to allocate.
 * I wouldn't expect more than a few hundred options at the most. */
#define CFG_OPTION_MAP_SIZE 1009


char *cfg_trim(char *in_str) {
  /* Skip over initial whitespace. */
  char *first_nonspace = in_str;
  while(isspace((unsigned char)*first_nonspace)) {
    first_nonspace++;
  }
  int len = strlen(first_nonspace);
  if (len == 0) {  /* empty line. */
    return first_nonspace;
  }

  /* Trim final whitespace. */
  while (len > 0 && isspace((unsigned char)first_nonspace[len - 1])) {
    len --;
  }
  first_nonspace[len] = '\0';

  return first_nonspace;
}  /* cfg_trim */


ERR_F cfg_key_valid(char *key) {
  ERR_ASSRT(key, CFG_ERR_INTERNAL);

  /* Validate first character (cannot be digit). */
  if (!(isalpha(*key) || *key == '_' || *key == '-')) {
    ERR_THROW(CFG_ERR_BADKEY, "invalid key '%s'", key);
  }

  /* Validate rest of characters. */
  key++;
  while (*key != '\0') {
    if (!(isalnum(*key) || *key == '_' || *key == '-')) {
      ERR_THROW(CFG_ERR_BADKEY, "invalid key '%s'", key);
    }
    key++;
  }

  return ERR_OK;
}  /* cfg_key_valid */


ERR_F cfg_create(cfg_t **rtn_cfg) {
  cfg_t *cfg;
  ERR_ASSRT(cfg = calloc(1, sizeof(cfg_t)), CFG_ERR_NOMEM);

  ERR(hmap_create(&(cfg->option_vals), CFG_OPTION_MAP_SIZE));

  err_t *err;
  err = hmap_create(&(cfg->option_locations), CFG_OPTION_MAP_SIZE);
  if (err) {
    ERR(hmap_delete(cfg->option_vals));
    ERR_RETHROW(err, err->code);
  }

  *rtn_cfg = cfg;
  return ERR_OK;
}  /* cfg_create */


ERR_F cfg_delete(cfg_t *cfg) {
  hmap_entry_t *entry;

  /* Free the option values. */
  entry = NULL;  /* Start at beginning. */
  do {
    ERR(hmap_next(cfg->option_vals, &entry));
    if (entry) {
        ERR_ASSRT(entry->value != NULL, CFG_ERR_INTERNAL);
        free(entry->value);
    }
  } while (entry);
  ERR(hmap_delete(cfg->option_vals));

  /* Free the option locations. */
  entry = NULL;  /* Start at beginning. */
  do {
    ERR(hmap_next(cfg->option_locations, &entry));
    if (entry) {
        ERR_ASSRT(entry->value != NULL, CFG_ERR_INTERNAL);
        free(entry->value);
    }
  } while (entry);
  ERR(hmap_delete(cfg->option_locations));

  free(cfg);

  return ERR_OK;
}  /* cfg_delete */


ERR_F cfg_parse_line(cfg_t *cfg, int mode, const char *iline, const char *filename, int line_num) {
  char *local_iline;
  err_t *err;

  switch (mode) {  /* Check for valid mode. */
  case CFG_MODE_UPDATE: break;
  case CFG_MODE_ADD: break;
  default: ERR_THROW(CFG_ERR_PARAM, "unrecognized mode %d", mode);
  }

  ERR(err_strdup(&local_iline, iline));  /* Need local copy because we modify string. */

  /* Strip any comment from iline. */
  char *hash = strchr(local_iline, '#');
  if (hash) { *hash = '\0'; }

  /* Skip blank lines. */
  char *trimmed_iline = cfg_trim(local_iline);  /* Trim whitespace. */
  if (*trimmed_iline == '\0') {
    free(local_iline);
    return ERR_OK;
  }

  /* Use equals sign to split key and value. */
  char *equals = strchr(trimmed_iline, '=');
  ERR_ASSRT(equals, CFG_ERR_NOEQUALS);
  *equals = '\0';  /* Split into two strings. */

  char *key = cfg_trim(trimmed_iline);
  ERR(cfg_key_valid(key));
  ERR_ASSRT(strlen(key) > 0, CFG_ERR_NOKEY);
  /* See if key already exists. */
  err = hmap_slookup(cfg->option_vals, key, NULL);
  if (err && err->code != HMAP_ERR_NOTFOUND) { /* An unexpected error. */
    free(local_iline);  /* Clean up local copy. */
    ERR_RETHROW(err, err->code);
  }
  int key_exists = (err == NULL);  /* Key exists if no error. */
  if (err) { err_dispose(err); }

  switch (mode) {
  case CFG_MODE_UPDATE:
    if (! key_exists) { /* Key not exist is an error for UPDATE mode. */
      free(local_iline);  /* Clean up local copy. */
      ERR_THROW(CFG_ERR_UPDATE_KEY_NOT_FOUND, "");
    }
    break;
  case CFG_MODE_ADD:
    if (key_exists) { /* Key exist is an error for ADD mode. */
      free(local_iline);  /* Clean up local copy. */
      ERR_THROW(CFG_ERR_ADD_KEY_ALREADY_EXIST, "");
    }
    break;
  default:
    free(local_iline);  /* Clean up local copy. */
    ERR_THROW(CFG_ERR_INTERNAL, "mode");
  }

  /* Get value into its own mem segment to store in hash. */
  char *value = equals + 1;
  value = cfg_trim(value);
  ERR(err_strdup(&value, value));

  ERR(hmap_swrite(cfg->option_vals, key, value));

  /* Remember location for this option. */
  char *location;
  ERR(err_asprintf(&location, "%s:%d", filename, line_num));
  ERR(hmap_swrite(cfg->option_locations, key, location));

  free(local_iline);  /* Clean up local copy. */
  return ERR_OK;
}  /* cfg_parse_line */


ERR_F cfg_parse_file(cfg_t *cfg, int mode, const char *filename) {
  char iline[CFG_MAX_LINE_LEN + 3];  /* Room for cr/lf/null. */
  FILE *file_fp;

  ERR_ASSRT(cfg, CFG_ERR_PARAM);
  ERR_ASSRT(filename, CFG_ERR_PARAM);
  if (strcmp(filename, "-") == 0) {
    file_fp = stdin;
  } else {
    file_fp = fopen(filename, "r");
  }
  ERR_ASSRT(file_fp, CFG_ERR_BADFILE);

  int line_num = 0;
  err_t *parse_err = ERR_OK;
  while (fgets(iline, sizeof(iline), file_fp)) {
    line_num++;
    size_t len = strlen(iline);
    ERR_ASSRT(len <= CFG_MAX_LINE_LEN, CFG_ERR_LINETOOLONG);

    parse_err = cfg_parse_line(cfg, mode, iline, filename, line_num);
    if (parse_err) { break; }
  }  /* while */
  int save_errno = errno;
  if (ferror(file_fp)) {
    ERR_THROW(CFG_ERR_READ_ERROR, "Error reading file %s: %s", filename, strerror(save_errno));
  }

  if (strcmp(filename, "-") == 0) {
    /* Don't close stdin. */
  } else {
    fclose(file_fp);
  }

  if (parse_err) {
    ERR_RETHROW(parse_err, parse_err->code);
  }
  return ERR_OK;
}  /* cfg_parse_file */


ERR_F cfg_parse_string_list(cfg_t *cfg, int mode, char **string_list) {
  char *iline;

  ERR_ASSRT(cfg, CFG_ERR_PARAM);
  ERR_ASSRT(string_list, CFG_ERR_PARAM);

  int line_num = 0;
  while ((iline = string_list[line_num])) {
    line_num++;

    ERR(cfg_parse_line(cfg, mode, iline, "string_list", line_num));
  }  /* while */

  return ERR_OK;
}  /* cfg_parse_string_list */


ERR_F cfg_get_str_val(cfg_t *cfg, const char *key, char **rtn_value) {
  char *val_str;

  ERR(hmap_slookup(cfg->option_vals, key, (void **)&val_str));

  *rtn_value = val_str;
  return ERR_OK;
}  /* cfg_get_str_val */


void cfg_remove_spaces(char *in_str) {
  char *dst = in_str;
  char *src = in_str;
  while (*src != '\0') {
    if (*src != ' ') {
      *dst = *src;
      dst++;
    }
    src ++;
  }
  *dst = '\0';
}  /* cfg_remove_spaces */


ERR_F cfg_atol(const char *in_str, long *rtn_value) {
  int base = 10;

  if (in_str[0] == '0' && (in_str[1] == 'x' || in_str[1] == 'X')) {
    base = 16;
    in_str += 2;  /* Step past the "0x". */
  }

  errno = 0;  /* Best practice when using strtol. */
  char *p = NULL;
  long value = strtol(in_str, &p, base);
  /* Check for error. */
  if (errno != 0 || p == in_str || p == NULL || *p != '\0') {
    ERR_THROW(CFG_ERR_BAD_NUMBER, in_str);
  }

  *rtn_value = value;
  return ERR_OK;
}  /* cfg_atol */


ERR_F cfg_get_long_val(cfg_t *cfg, const char *key, long *rtn_value) {
  long value;
  err_t *err;

  char *val_str;
  ERR(hmap_slookup(cfg->option_vals, key, (void **)&val_str));

  char *local_val_str;  /* Local copy to remove spaces. */
  ERR(err_strdup(&local_val_str, val_str));

  cfg_remove_spaces(local_val_str);
  err = cfg_atol(local_val_str, &value);
  if (err) {
    free(local_val_str);
    ERR_RETHROW(err, err->code);
  }

  free(local_val_str);  /* Clean up local copy. */
  *rtn_value = value;
  return ERR_OK;
}  /* cfg_get_long_val */

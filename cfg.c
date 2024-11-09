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
#include <stdarg.h>
#include <string.h>
#include <stdint.h>
#include <errno.h>
#include <ctype.h>
#include "err.h"
#include "hmap.h"
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


ERR_F cfg_create(cfg_t **rtn_cfg) {
  err_t *err;

  cfg_t *cfg;
  ERR_ASSRT(cfg = calloc(1, sizeof(cfg_t)), CFG_ERR_NOMEM);

  ERR(hmap_create(&(cfg->option_vals), CFG_OPTION_MAP_SIZE));

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

  entry = NULL;  /* Start at beginning. */
  do {
    ERR(hmap_next(cfg->option_vals, &entry));
    if (entry) {
        ERR_ASSRT(entry->value != NULL, CFG_ERR_INTERNAL);
        free(entry->value);
    }
  } while (entry);
  ERR(hmap_delete(cfg->option_vals));

  entry = NULL;  /* Start at beginning. */
  do {
    ERR(hmap_next(cfg->option_locations, &entry));
    if (entry) {
        ERR_ASSRT(entry->value != NULL, CFG_ERR_INTERNAL);
        free(entry->value);
    }
  } while (entry);
  ERR(hmap_delete(cfg->option_locations));

  return ERR_OK;
}  /* cfg_delete */


ERR_F cfg_parse_line(cfg_t *cfg, int mode, const char *iline, const char *filename, int line_num) {
  char *local_iline;
  err_t *err;

  switch (mode) {  /* Check for valid mode. */
  case CFG_MODE_UPDATE: break;
  case CFG_MODE_ADD: break;
  default: ERR_THROW(CFG_ERR_PARAM, "mode");
  }

  ERR_ASSRT(local_iline = strdup(iline), CFG_ERR_NOMEM);  /* Need local copy because strtoc modifies string. */

  /* Strip (optional) comment from iline. */
  char *hash = strchr(local_iline, '#');
  if (hash) { *hash = '\0'; }

  /* Skip blank lines. */
  char *trimmed_iline = cfg_trim(local_iline);  /* Trim whitespace. */
  if (*trimmed_iline == '\0') {
    return ERR_OK;
  }

  /* Use equals sign to split key and value. */
  char *equals = strchr(trimmed_iline, '=');
  ERR_ASSRT(equals, CFG_ERR_NOEQUALS);
  *equals = '\0';  /* Split into two strings. */

  char *key = cfg_trim(trimmed_iline);
  size_t key_len = strlen(key);
  ERR_ASSRT(key_len > 0, CFG_ERR_NOKEY);
  /* See if key already exists. */
  err = hmap_lookup(cfg->option_vals, key, key_len, NULL);
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
    ERR_THROW(CFG_ERR_INTERNAL, "mode");
  }

  /* Get value into its own mem segment to store in hash. */
  char *value = equals + 1;
  value = cfg_trim(value);
  ERR_ASSRT(value = strdup(value), CFG_ERR_NOMEM);

  ERR(hmap_write(cfg->option_vals, key, key_len, value));

  /* Remember location for this option. */
  char *location;
  ERR_ASSRT(location = err_asprintf("%s:%d", filename, line_num), CFG_ERR_NOMEM);
  ERR(hmap_write(cfg->option_locations, key, key_len, location));

  free(local_iline);  /* Clean up local copy. */
  return ERR_OK;
}  /* cfg_parse_line */


ERR_F cfg_parse_file(cfg_t *cfg, int mode, const char *filename) {
  char iline[1024];
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
  err_t *err = ERR_OK;
  while (fgets(iline, sizeof(iline), file_fp)) {
    line_num++;
    size_t len = strlen(iline);
    ERR_ASSRT(len < sizeof(iline) - 1, CFG_ERR_LINETOOLONG);  /* Line too long. */

    err = cfg_parse_line(cfg, mode, iline, filename, line_num);
    if (err) { break; }
  }  /* while */

  if (strcmp(filename, "-") == 0) {
    /* Don't close stdin. */
  } else {
    fclose(file_fp);
  }

  if (err) {
    ERR_RETHROW(err, err->code);
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

  ERR(hmap_lookup(cfg->option_vals, key, strlen(key), (void **)&val_str));

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


ERR_F cfg_get_long_val(cfg_t *cfg, const char *key, long *rtn_value) {
  char *val_str;
  char *local_val_str;
  long value;
  int base = 10;
  err_t *err;

  ERR(hmap_lookup(cfg->option_vals, key, strlen(key), (void **)&val_str));
  ERR_ASSRT(local_val_str = strdup(val_str), CFG_ERR_NOMEM);  /* Local copy to remove spaces. */
  val_str = local_val_str;
  cfg_remove_spaces(val_str);
  if (val_str[0] == '0' && (val_str[1] == 'x' || val_str[1] == 'X')) {
    base = 16;
    val_str += 2;
  }
  errno = 0;  /* Good practice when using strtol. */
  char *p = NULL;
  value = strtol(val_str, &p, base);
  int strtol_errno = errno;
  /* Check for conversation errors. */
  if (strtol_errno != 0 || p == val_str || p == NULL || *p != '\0') {
    free(local_val_str);  /* Clean up local copy. */
    char *location;
    err = hmap_lookup(cfg->option_locations, key, strlen(key), (void **)&location);
    if (err) {
      location = NULL;
      err_dispose(err);
    }
    ERR_THROW(CFG_ERR_BAD_NUMBER, location);
  } else {
    free(local_val_str);  /* Clean up local copy. */
  }

  *rtn_value = value;
  return ERR_OK;
}  /* cfg_get_int_val */

/* cfg.h - simple error handler. */
/*
# This code and its documentation is Copyright 2024-2024 Steven Ford, http://geeky-boy.com
# and licensed "public domain" style under Creative Commons "CC0": http://creativecommons.org/publicdomain/zero/1.0/
# To the extent possible under law, the contributors to this project have
# waived all copyright and related or neighboring rights to this work.
# In other words, you can use this code for any purpose without any
# restrictions.  This work is published from: United States.  The project home
# is https://github.com/fordsfords/cfg
*/

#ifndef CFG_H
#define CFG_H

#include "err.h"
#include "hmap.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct cfg_s cfg_t;
struct cfg_s {
  hmap_t *option_vals;
  hmap_t *option_locations;
};

#define CFG_MODE_ADD 1
#define CFG_MODE_UPDATE 2

/* Maximum length of configuration line content (not including CR, LF, null). */
#define CFG_MAX_LINE_LEN 1000  

#ifdef CFG_C
#  define ERR_CODE(err__code) ERR_API char *err__code = #err__code
#else
#  define ERR_CODE(err__code) ERR_API extern char *err__code
#endif

ERR_CODE(CFG_ERR_INTERNAL);
ERR_CODE(CFG_ERR_PARAM);
ERR_CODE(CFG_ERR_NOMEM);
ERR_CODE(CFG_ERR_BADKEY);
ERR_CODE(CFG_ERR_BADFILE);
ERR_CODE(CFG_ERR_READ_ERROR);
ERR_CODE(CFG_ERR_LINETOOLONG);
ERR_CODE(CFG_ERR_NOEQUALS);
ERR_CODE(CFG_ERR_NOKEY);
ERR_CODE(CFG_ERR_UPDATE_KEY_NOT_FOUND);
ERR_CODE(CFG_ERR_ADD_KEY_ALREADY_EXIST);
ERR_CODE(CFG_ERR_BAD_NUMBER);
#undef ERR_CODE

ERR_F cfg_create(cfg_t **rtn_cfg);
ERR_F cfg_delete(cfg_t *cfg);
ERR_F cfg_parse_line(cfg_t *cfg, int mode, const char *iline, const char *filename, int line_num);
ERR_F cfg_parse_file(cfg_t *cfg, int mode, const char *filename);
ERR_F cfg_parse_string_list(cfg_t *cfg, int mode, char **string_list);
ERR_F cfg_get_str_val(cfg_t *cfg, const char *key, char **rtn_value);
ERR_F cfg_atol(const char *in_str, long *rtn_value);
ERR_F cfg_get_long_val(cfg_t *cfg, const char *key, long *rtn_value);

#ifdef __cplusplus
}
#endif

#endif  /* CFG_H */

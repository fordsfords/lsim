/* lsim_cmd.h */

/* This work is dedicated to the public domain under CC0 1.0 Universal:
 * http://creativecommons.org/publicdomain/zero/1.0/
 * 
 * To the extent possible under law, Steven Ford has waived all copyright
 * and related or neighboring rights to this work. In other words, you can 
 * use this code for any purpose without any restrictions.
 * This work is published from: United States.
 * Project home: https://github.com/fordsfords/lsim
 */

#ifndef LSIM_CMD_H
#define LSIM_CMD_H

#include <stdint.h>
#include "err.h"
#include "hmap.h"

#ifdef __cplusplus
extern "C" {
#endif

ERR_F lsim_cmd_line(lsim_t *lsim, const char *cmd_line);
ERR_F lsim_cmd_file(lsim_t *lsim, const char *cmd_file_name);

#ifdef __cplusplus
}
#endif

#endif // LSIM_CMD_H

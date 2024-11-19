/* lsim_cmd.h */
/*
# This code and its documentation is Copyright 2024-2024 Steven Ford, http://geeky-boy.com
# and licensed "public domain" style under Creative Commons "CC0": http://creativecommons.org/publicdomain/zero/1.0/
# To the extent possible under law, the contributors to this project have
# waived all copyright and related or neighboring rights to this work.
# In other words, you can use this code for any purpose without any
# restrictions.  This work is published from: United States.  The project home
# is https://github.com/fordsfords/lsim
*/

#ifndef LSIM_CMD_H
#define LSIM_CMD_H

#include <stdint.h>
#include "err.h"
#include "hmap.h"
#include "lsim.h"

#ifdef __cplusplus
extern "C" {
#endif

ERR_F lsim_cmd_line(lsim_t *lsim, const char *cmd_line);
ERR_F lsim_cmd_file(lsim_t *lsim, const char *cmd_file_name);

#ifdef __cplusplus
}
#endif

#endif // LSIM_CMD_H

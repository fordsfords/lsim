/* lsim_cmd.h - simple haslsim. */
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

ERR_F lsim_device_vcc_create(lsim_t *lsim, char *name);
ERR_F lsim_device_vcc_reset(lsim_t *lsim, device_t *dev);
ERR_F lsim_device_nand_create(lsim_t *lsim, char *name, long num_inputs);
ERR_F lsim_device_nand_reset(lsim_t *lsim, device_t *dev);
ERR_F lsim_interp_d_vcc(lsim_t *lsim, const char *cmd_line);
ERR_F lsim_interp_d_nand(lsim_t *lsim, const char *cmd_line);
ERR_F lsim_interp_d(lsim_t *lsim, const char *cmd_line);
ERR_F lsim_interp_cmd_line(lsim_t *lsim, const char *cmd_line);
ERR_F lsim_interp_cmd_file(lsim_t *lsim, const char *cmd_file_name);

#ifdef __cplusplus
}
#endif

#endif // LSIM_CMD_H

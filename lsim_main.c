/* lsim_main.c */
/*
 *
 * This code and its documentation is Copyright 2019, 2019 Steven Ford,
 * http://geeky-boy.com and licensed "public domain" style under Creative
 * Commons "CC0": http://creativecommons.org/publicdomain/zero/1.0/
 * To the extent possible under law, the contributors to this project have
 * waived all copyright and related or neighboring rights to this work.
 * In other words, you can use this code for any purpose without any
 * restrictions. This work is published from: United States. The project home
 * is https://github.com/fordsfords/lsim
 */

#include <stdio.h>
#include <string.h>
#if ! defined(_WIN32)
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#endif
#include "err.h"
#include "hmap.h"
#include "cfg.h"
#include "lsim.h"
#include "lsim_cmd.h"

#if defined(_WIN32)
#define MY_SLEEP_MS(msleep_msecs) Sleep(msleep_msecs)
#else
#define MY_SLEEP_MS(msleep_msecs) usleep((msleep_msecs)/1000)
#endif


/* Options */
char *o_config_file = NULL;

/* Positional parameters. */
char *p_cmd_file = NULL;


char usage_str[] = "Usage: lsim_main [-h] [-c config_file] command_file";
void usage(char *msg) {
  if (msg) fprintf(stderr, "\n%s\n\n", msg);
  fprintf(stderr, "%s\n", usage_str);
  exit(1);
}  /* usage */

void help() {
  printf("%s\n"
    "where:\n"
    "  -h - print help\n"
    "  -c config_file - configuration file.\n"
    "command_file - can be set to '-' for stdin\n"
    "For details, see https://github.com/fordsfords/lsim\n",
    usage_str);
  exit(0);
}  /* help */


ERR_F parse_cmdline(int argc, char **argv) {
  int opt;

  /* Since this is Unix and Windows, don't use getopts(). */
  for (opt = 1; opt < argc; opt++) {
    if (strcmp(argv[opt], "-h") == 0) {
      help();  exit(0);

    } else if (strcmp(argv[opt], "-c") == 0) {
      opt++;  /* Step past -c to get to teh config_file. */
      ERR_ASSRT(opt < argc, LSIM_ERR_PARAM);  /* Make there is a config_file. */
      if (o_config_file) { free(o_config_file); }  /* Free a previous setting. */
      ERR(err_strdup(&o_config_file, argv[opt]));

    } else if (strcmp(argv[opt], "--") == 0) {
      opt++;  /* Step past "--". */
      break;  /* End of options. */

    } else if (*argv[opt] != '-') {
      /* argv[opt] has first positional parameter; leave "opt" alone. */
      break;  /* End of options */
    } else {
      ERR_THROW(LSIM_ERR_PARAM, "Unknown option");
    }
  }  /* for opt */

  if (opt < argc) {  /* if have positional parameters. */
    ERR(err_strdup(&p_cmd_file, argv[opt]));
    opt++;
  }
  else {
    ERR(err_strdup(&p_cmd_file, "-"));
  }

  if (opt < argc) {
    ERR_THROW(LSIM_ERR_PARAM, "Unexpected command-line parameter");
  }

  return ERR_OK;
}  /* parse_cmdline */


ERR_F lsim_main(int argc, char **argv) {
  lsim_t *lsim;

  ERR(parse_cmdline(argc, argv));

  ERR(lsim_create(&lsim, o_config_file));

  ERR(lsim_cmd_file(lsim, p_cmd_file));

  ERR(lsim_delete(lsim));

  return ERR_OK;
}  /* lsim_main */


int main(int argc, char **argv) {
  ERR_ABRT_ON_ERR(lsim_main(argc, argv), stderr);

  return 0;
}  /* main */

/* lsim_tst.c - self-test.
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

#define E(e__test) do { \
  err_t *e__err = (e__test); \
  if (e__err != ERR_OK) { \
    printf("ERROR [%s:%d]: '%s' returned error\n", __FILE__, __LINE__, #e__test); \
    ERR_ABRT(e__err, stdout); \
    exit(1); \
  } \
} while (0)

#define ASSRT(assrt__cond) do { \
  if (! (assrt__cond)) { \
    printf("ERROR [%s:%d]: assert '%s' failed\n", __FILE__, __LINE__, #assrt__cond); \
    exit(1); \
  } \
} while (0)


/* Options */
int o_testnum;


char usage_str[] = "Usage: err_test [-h] [-t testnum]";
void usage(char *msg) {
  if (msg) fprintf(stderr, "\n%s\n\n", msg);
  fprintf(stderr, "%s\n", usage_str);
  exit(1);
}  /* usage */

void help() {
  printf("%s\n"
    "where:\n"
    "  -h - print help\n"
    "  -t testnum - Specify which test to run [1].\n"
    "For details, see https://github.com/fordsfords/err\n",
    usage_str);
  exit(0);
}  /* help */


void parse_cmdline(int argc, char **argv) {
  int i;

  /* Since this is Unix and Windows, don't use getopts(). */
  for (i = 1; i < argc; i++) {
    if (strcmp(argv[i], "-h") == 0) {
      help();  exit(0);

    } else if (strcmp(argv[i], "-t") == 0) {
      if ((i + 1) < argc) {
        i++;
        o_testnum = atoi(argv[i]);
      } else { fprintf(stderr, "Error, -t requires test number\n");  exit(1); }

    } else { fprintf(stderr, "Error, unknown option '%s'\n", argv[i]);  exit(1); }
  }  /* for i */
}  /* parse_cmdline */


void test1() {
  lsim_t *lsim;
  err_t *err;

  E(lsim_create(&lsim, NULL));

  E(lsim_interp_cmd_line(lsim, "d;vcc;MyVcc;"));
  E(lsim_interp_cmd_line(lsim, "d;vcc;-My_Vcc2;"));

  err = lsim_interp_cmd_line(lsim, "d;vcc;MyVcc;");  /* Duplicate. */
  ASSRT(err);
  ASSRT(err->code == LSIM_ERR_EXIST);
  err = lsim_interp_cmd_line(lsim, "d;vcc; MyVcc;");
  ASSRT(err);
  ASSRT(err->code == LSIM_ERR_NAME);
  err = lsim_interp_cmd_line(lsim, "d;vcc;9MyVcc;");
  ASSRT(err);
  ASSRT(err->code == LSIM_ERR_NAME);
  err = lsim_interp_cmd_line(lsim, "d;vcc;.MyVcc;");
  ASSRT(err);
  ASSRT(err->code == LSIM_ERR_NAME);

  E(lsim_delete(lsim));
}  /* test1 */


int main(int argc, char **argv) {
  parse_cmdline(argc, argv);

  if (o_testnum == 0 || o_testnum == 1) {
    test1();
    printf("test1: success\n");
  }

  return 0;
}  /* main */

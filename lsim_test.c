/* lsim_test.c - self-test.
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
#include "lsim_dev.h"
#include "lsim_cmd.h"
#include "lsim_devs.h"

#if defined(_WIN32)
#define MY_SLEEP_MS(msleep_msecs) Sleep(msleep_msecs)
#else
#define MY_SLEEP_MS(msleep_msecs) usleep((msleep_msecs)/1000)
#endif

#define E(e__test)                                                                  \
  do {                                                                              \
    err_t *e__err = (e__test);                                                      \
    if (e__err != ERR_OK) {                                                         \
      printf("ERROR [%s:%d]: '%s' returned error\n", __FILE__, __LINE__, #e__test); \
      /* include ERR_ABRT() caller in stack trace. */                               \
      e__err = err_rethrow_v(__FILE__, __LINE__, __func__, e__err, #e__test);       \
      fprintf(stdout, "Stack trace:\n----------------\n");                          \
      err_print(e__err, stdout);                                                    \
      err_dispose(e__err);                                                          \
      fflush(stdout);                                                               \
      switch (global_error_reaction) {                                              \
        case 1:                                                                     \
          exit(1);                                                                  \
          break;                                                                    \
        case 2:                                                                     \
          /* Warn - keep running. */                                                \
          break;                                                                    \
        default:                                                                    \
          abort();                                                                  \
          break;                                                                    \
      }                                                                             \
    }                                                                               \
  } while (0)

#define ASSRT(assrt__cond)                                                             \
  do {                                                                                 \
    if (!(assrt__cond)) {                                                              \
      printf("ERROR [%s:%d]: assert '%s' failed\n", __FILE__, __LINE__, #assrt__cond); \
      fflush(stdout);                                                                  \
      switch (global_error_reaction) {                                                    \
        case 1:                                                                        \
          exit(1);                                                                     \
          break;                                                                       \
        case 2:                                                                        \
          /* Warn - keep running. */                                                   \
          break;                                                                       \
        default:                                                                       \
          abort();                                                                     \
          break;                                                                       \
      }                                                                                \
    }                                                                                  \
  } while (0)


/* Options. */
char *o_config_file = NULL;
int o_testnum;

/* Globals. */
long global_error_reaction = 1;


char usage_str[] = "Usage: err_test [-h] [-c config_file] [-t testnum]";
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
    "  -t testnum - Specify which test to run [1].\n"
    "For details, see https://github.com/fordsfords/lsim\n",
    usage_str);
  exit(0);
}  /* help */


void parse_cmdline(int argc, char **argv) {
  int opt;

  /* Since this is Unix and Windows, don't use getopts(). */
  for (opt = 1; opt < argc; opt++) {
    if (strcmp(argv[opt], "-h") == 0) {
      help();  exit(0);

    } else if (strcmp(argv[opt], "-c") == 0) {
      opt++;  /* Step past -c to get to the config_file. */
      if (opt >= argc) { fprintf(stderr, "ERROR, -c option requires config file\n"); exit(1); }
      if (o_config_file) { free(o_config_file); }  /* Free a previous setting. */
      E(err_strdup(&o_config_file, argv[opt]));

    } else if (strcmp(argv[opt], "-t") == 0) {
      if ((opt + 1) < argc) {
        opt++;
        o_testnum = atoi(argv[opt]);
      } else { fprintf(stderr, "Error, -t requires test number\n");  exit(1); }

    } else if (strcmp(argv[opt], "--") == 0) {
      opt++;  /* Step past "--". */
      break;  /* End of options. */

    } else { fprintf(stderr, "Error, unknown option '%s'\n", argv[opt]);  exit(1); }
  }  /* for opt */

  if (opt < argc) {
    fprintf(stderr, "Unexpected command-line parameter '%s'\n", argv[opt]); exit(1);
  }
}  /* parse_cmdline */


void test1() {
  lsim_t *lsim;
  err_t *err;

  E(lsim_create(&lsim, NULL));

  E(lsim_cmd_line(lsim, "d;vcc;MyVcc;"));
  E(lsim_cmd_line(lsim, "d;vcc;-My_Vcc2;"));

  err = lsim_cmd_line(lsim, "d;vcc;MyVcc;");  /* Duplicate. */
  ASSRT(err);
  ASSRT(err->code == LSIM_ERR_EXIST);
  err = lsim_cmd_line(lsim, "d;vcc; MyVcc;");
  ASSRT(err);
  ASSRT(err->code == LSIM_ERR_COMMAND);
  err = lsim_cmd_line(lsim, "d;vcc;9MyVcc;");
  ASSRT(err);
  ASSRT(err->code == LSIM_ERR_COMMAND);
  err = lsim_cmd_line(lsim, "d;vcc;.MyVcc;");
  ASSRT(err);
  ASSRT(err->code == LSIM_ERR_COMMAND);
  /* Can't route an output to a vcc. */
  err = lsim_cmd_line(lsim, "w;MyVcc;;-My_Vcc2;;");
  ASSRT(err);
  ASSRT(err->code == LSIM_ERR_COMMAND);

  E(lsim_cmd_line(lsim, "d;nand;MyNand;2;"));

  lsim_dev_t *nand_dev;
  E(hmap_slookup(lsim->devs, "MyNand", (void **)&nand_dev));
  ASSRT(nand_dev);
  ASSRT(nand_dev->type == LSIM_DEV_TYPE_NAND);
  ASSRT(nand_dev->nand.num_inputs == 2);
  ASSRT(nand_dev->nand.o_terminal->state == 0);
  ASSRT(nand_dev->nand.o_terminal->in_terminal_list == NULL);
  ASSRT(nand_dev->nand.i_terminals[0]->state == 0);
  ASSRT(nand_dev->nand.i_terminals[1]->state == 0);

  lsim_dev_t *vcc_dev;
  E(hmap_slookup(lsim->devs, "MyVcc", (void **)&vcc_dev));
  ASSRT(vcc_dev);
  ASSRT(vcc_dev->type == LSIM_DEV_TYPE_VCC);
  ASSRT(vcc_dev->vcc.o_terminal->state == 0);
  ASSRT(vcc_dev->vcc.o_terminal->in_terminal_list == NULL);

  lsim_dev_t *vcc2_dev;
  E(hmap_slookup(lsim->devs, "-My_Vcc2", (void **)&vcc2_dev));
  ASSRT(vcc2_dev);
  ASSRT(vcc2_dev->type == LSIM_DEV_TYPE_VCC);
  ASSRT(vcc2_dev->vcc.o_terminal->state == 0);
  ASSRT(vcc2_dev->vcc.o_terminal->in_terminal_list == NULL);

  E(lsim_cmd_line(lsim, "c;-My_Vcc2;o0;MyNand;i0;"));

  lsim_dev_t *tmp_dev;
  E(hmap_slookup(lsim->devs, "-My_Vcc2", (void **)&tmp_dev));
  ASSRT(tmp_dev == vcc2_dev);
  ASSRT(vcc2_dev->type == LSIM_DEV_TYPE_VCC);
  ASSRT(vcc2_dev->vcc.o_terminal->state == 0);
  ASSRT(vcc2_dev->vcc.o_terminal->in_terminal_list == nand_dev->nand.i_terminals[0]);
  ASSRT(nand_dev->nand.i_terminals[0]->next_in_terminal == NULL);
  ASSRT(nand_dev->nand.i_terminals[0]->driving_out_terminal == vcc2_dev->vcc.o_terminal);
  ASSRT(nand_dev->nand.i_terminals[1]->driving_out_terminal == NULL);
  ASSRT(nand_dev->nand.o_terminal->in_terminal_list == NULL);  /* Nand isn't driving anything yet. */

  E(lsim_cmd_line(lsim, "c;MyNand;o0;MyNand;i1;"));
  ASSRT(nand_dev->nand.o_terminal->in_terminal_list == nand_dev->nand.i_terminals[1]);

  ASSRT(lsim->quit == 0);
  E(lsim_cmd_line(lsim, "q;"));
  ASSRT(lsim->quit == 1);

  E(lsim_delete(lsim));

  E(lsim_create(&lsim, NULL));
  E(lsim_cmd_file(lsim, "srlatch.lsim"));

  E(lsim_cmd_line(lsim, "d;gnd;my_gnd;"));
  E(lsim_cmd_line(lsim, "d;led;my_led;"));
  E(lsim_cmd_line(lsim, "c;my_gnd;o0;my_led;i0;"));

  lsim_dev_t *led_dev;
  E(hmap_slookup(lsim->devs, "my_led", (void **)&led_dev));
  ASSRT(led_dev);
  ASSRT(led_dev->type == LSIM_DEV_TYPE_LED);
  ASSRT(led_dev->led.illuminated == 0);
  ASSRT(led_dev->led.i_terminal->state == 0);

  ASSRT(led_dev);
  ASSRT(led_dev->type == LSIM_DEV_TYPE_LED);
  ASSRT(led_dev->led.illuminated == 0);
  ASSRT(led_dev->led.i_terminal->state == 0);

  E(lsim_cmd_line(lsim, "v;1;"));
  E(lsim_cmd_line(lsim, "d;clk;my_clk;"));
  E(lsim_cmd_line(lsim, "d;swtch;Reset_sw;0;"));
  E(lsim_cmd_line(lsim, "d;led;qled;"));
  E(lsim_cmd_line(lsim, "d;led;Qled;"));
  E(lsim_cmd_line(lsim, "c;my_clk;q0;qled;i0;"));
  E(lsim_cmd_line(lsim, "c;my_clk;Q0;Qled;i0;"));
  E(lsim_cmd_line(lsim, "c;Reset_sw;o0;my_clk;R0;"));

  lsim_dev_t *qled_dev;
  E(hmap_slookup(lsim->devs, "qled", (void **)&qled_dev));
  ASSRT(qled_dev);
  ASSRT(qled_dev->led.illuminated == 0);
  lsim_dev_t *Qled_dev;
  E(hmap_slookup(lsim->devs, "Qled", (void **)&Qled_dev));
  ASSRT(Qled_dev);
  ASSRT(Qled_dev->led.illuminated == 0);
  lsim_dev_t *clk_dev;
  E(hmap_slookup(lsim->devs, "my_clk", (void **)&clk_dev));
  ASSRT(clk_dev);

  E(lsim_cmd_line(lsim, "t;1;"));

  ASSRT(qled_dev->led.illuminated == 0);
  ASSRT(Qled_dev->led.illuminated == 1);

  E(lsim_cmd_line(lsim, "m;Reset_sw;1;"));
  ASSRT(qled_dev->led.illuminated == 0);
  ASSRT(Qled_dev->led.illuminated == 1);

  E(lsim_cmd_line(lsim, "t;1;"));
  ASSRT(qled_dev->led.illuminated == 1);
  ASSRT(Qled_dev->led.illuminated == 0);

  E(lsim_cmd_line(lsim, "t;1;"));
  ASSRT(qled_dev->led.illuminated == 0);
  ASSRT(Qled_dev->led.illuminated == 1);

  E(lsim_delete(lsim));
}  /* test1 */


void test2() {
  lsim_t *lsim;

  E(lsim_create(&lsim, NULL));

  E(lsim_cmd_line(lsim, "v;1;"));
  E(lsim_cmd_line(lsim, "i;srlatch.lsim;"));

  lsim_dev_t *nand1_dev;
  E(hmap_slookup(lsim->devs, "nand1", (void **)&nand1_dev));
  ASSRT(nand1_dev->nand.o_terminal->state == 1);

  lsim_dev_t *nand2_dev;
  E(hmap_slookup(lsim->devs, "nand2", (void **)&nand2_dev));
  ASSRT(nand2_dev->nand.o_terminal->state == 0);

  E(lsim_delete(lsim));
}  /* test2 */


void test3() {
  lsim_t *lsim;

  E(lsim_create(&lsim, NULL));

  E(lsim_cmd_line(lsim, "d;swtch;swS;1;"));
  E(lsim_cmd_line(lsim, "d;swtch;swR;0;"));
  E(lsim_cmd_line(lsim, "d;led;ledq;"));
  E(lsim_cmd_line(lsim, "d;led;ledQ;"));
  E(lsim_cmd_line(lsim, "d;srlatch;srlatch1;"));
  E(lsim_cmd_line(lsim, "c;swS;o0;srlatch1;S0;"));
  E(lsim_cmd_line(lsim, "c;swR;o0;srlatch1;R0;"));
  E(lsim_cmd_line(lsim, "c;srlatch1;q0;ledq;i0;"));
  E(lsim_cmd_line(lsim, "c;srlatch1;Q0;ledQ;i0;"));
  E(lsim_cmd_line(lsim, "v;1;"));  /* Trace. */
  E(lsim_cmd_line(lsim, "p;"));  /* Power up. */
  E(lsim_cmd_line(lsim, "m;swR;1;"));
  E(lsim_cmd_line(lsim, "m;swS;0;"));
  E(lsim_cmd_line(lsim, "m;swS;1;"));

  lsim_dev_t *ledq_dev;
  E(hmap_slookup(lsim->devs, "ledq", (void **)&ledq_dev));
  ASSRT(ledq_dev->led.i_terminal->state == 1);

  lsim_dev_t *ledQ_dev;
  E(hmap_slookup(lsim->devs, "ledQ", (void **)&ledQ_dev));
  ASSRT(ledQ_dev->led.i_terminal->state == 0);

  E(lsim_delete(lsim));
}  /* test3 */


void test4() {
  lsim_t *lsim;

  E(lsim_create(&lsim, NULL));

  E(lsim_cmd_line(lsim, "d;swtch;swd;0;"));
  E(lsim_cmd_line(lsim, "d;swtch;swR;0;"));
  E(lsim_cmd_line(lsim, "d;swtch;swc;1;"));
  E(lsim_cmd_line(lsim, "d;vcc;vcc;"));
  E(lsim_cmd_line(lsim, "d;led;ledq;"));
  E(lsim_cmd_line(lsim, "d;led;ledQ;"));
  E(lsim_cmd_line(lsim, "d;dflipflop;dflipflop1;"));
  E(lsim_cmd_line(lsim, "c;vcc;o0;dflipflop1;S0;"));
  E(lsim_cmd_line(lsim, "c;swd;o0;dflipflop1;d0;"));
  E(lsim_cmd_line(lsim, "c;swR;o0;dflipflop1;R0;"));
  E(lsim_cmd_line(lsim, "c;swc;o0;dflipflop1;c0;"));
  E(lsim_cmd_line(lsim, "c;dflipflop1;q0;ledq;i0;"));
  E(lsim_cmd_line(lsim, "c;dflipflop1;Q0;ledQ;i0;"));
  E(lsim_cmd_line(lsim, "v;1;"));  /* Trace. */

  lsim_dev_t *ledq_dev;
  E(hmap_slookup(lsim->devs, "ledq", (void **)&ledq_dev));
  lsim_dev_t *ledQ_dev;
  E(hmap_slookup(lsim->devs, "ledQ", (void **)&ledQ_dev));

  E(lsim_cmd_line(lsim, "p;"));  /* Power-up. */
  ASSRT(ledq_dev->led.i_terminal->state == 0);
  ASSRT(ledQ_dev->led.i_terminal->state == 1);
  E(lsim_cmd_line(lsim, "m;swR;1;"));  /* not reset. */
  ASSRT(ledq_dev->led.i_terminal->state == 0);
  ASSRT(ledQ_dev->led.i_terminal->state == 1);
  E(lsim_cmd_line(lsim, "m;swd;1;"));
  ASSRT(ledq_dev->led.i_terminal->state == 0);
  ASSRT(ledQ_dev->led.i_terminal->state == 1);
  E(lsim_cmd_line(lsim, "m;swc;0;"));  /* Clock falling edge. */
  ASSRT(ledq_dev->led.i_terminal->state == 0);
  ASSRT(ledQ_dev->led.i_terminal->state == 1);
  E(lsim_cmd_line(lsim, "m;swc;1;"));  /* Clock rising edge. */
  ASSRT(ledq_dev->led.i_terminal->state == 1);
  ASSRT(ledQ_dev->led.i_terminal->state == 0);
  E(lsim_cmd_line(lsim, "m;swd;0;"));  /* Data in 0. */
  ASSRT(ledq_dev->led.i_terminal->state == 1);
  ASSRT(ledQ_dev->led.i_terminal->state == 0);
  E(lsim_cmd_line(lsim, "m;swc;0;"));  /* Clock falling edge. */
  ASSRT(ledq_dev->led.i_terminal->state == 1);
  ASSRT(ledQ_dev->led.i_terminal->state == 0);
  E(lsim_cmd_line(lsim, "m;swc;1;"));  /* Clock rising edge. */
  ASSRT(ledq_dev->led.i_terminal->state == 0);
  ASSRT(ledQ_dev->led.i_terminal->state == 1);

  E(lsim_delete(lsim));
}  /* test4 */


void test5() {
  lsim_t *lsim;

  E(lsim_create(&lsim, NULL));

  E(lsim_cmd_line(lsim, "d;swtch;swd;0;"));
  E(lsim_cmd_line(lsim, "d;swtch;swS;1;"));
  E(lsim_cmd_line(lsim, "d;swtch;swR;0;"));
  E(lsim_cmd_line(lsim, "d;clk;clock;"));
  E(lsim_cmd_line(lsim, "d;vcc;vcc;"));
  E(lsim_cmd_line(lsim, "d;led;ledq;"));
  E(lsim_cmd_line(lsim, "d;led;ledQ;"));
  E(lsim_cmd_line(lsim, "d;dflipflop;dflipflop1;"));
  E(lsim_cmd_line(lsim, "c;swS;o0;dflipflop1;S0;"));
  E(lsim_cmd_line(lsim, "c;swd;o0;dflipflop1;d0;"));
  E(lsim_cmd_line(lsim, "c;swR;o0;dflipflop1;R0;"));
  E(lsim_cmd_line(lsim, "c;swR;o0;clock;R0;"));
  E(lsim_cmd_line(lsim, "c;clock;q0;dflipflop1;c0;"));
  E(lsim_cmd_line(lsim, "c;dflipflop1;q0;ledq;i0;"));
  E(lsim_cmd_line(lsim, "c;dflipflop1;Q0;ledQ;i0;"));
  E(lsim_cmd_line(lsim, "v;1;"));  /* Trace. */

  lsim_dev_t *ledq_dev;
  E(hmap_slookup(lsim->devs, "ledq", (void **)&ledq_dev));
  lsim_dev_t *ledQ_dev;
  E(hmap_slookup(lsim->devs, "ledQ", (void **)&ledQ_dev));

  E(lsim_cmd_line(lsim, "p;"));  /* Power-up. */
  ASSRT(ledq_dev->led.i_terminal->state == 0);
  ASSRT(ledQ_dev->led.i_terminal->state == 1);
  E(lsim_cmd_line(lsim, "t;1;"));  /* reset. */
  ASSRT(ledq_dev->led.i_terminal->state == 0);
  ASSRT(ledQ_dev->led.i_terminal->state == 1);
  E(lsim_cmd_line(lsim, "m;swR;1;"));  /* not reset. */
  ASSRT(ledq_dev->led.i_terminal->state == 0);
  ASSRT(ledQ_dev->led.i_terminal->state == 1);
  E(lsim_cmd_line(lsim, "t;1;"));
  ASSRT(ledq_dev->led.i_terminal->state == 0);
  ASSRT(ledQ_dev->led.i_terminal->state == 1);
  E(lsim_cmd_line(lsim, "m;swd;1;"));
  ASSRT(ledq_dev->led.i_terminal->state == 0);
  ASSRT(ledQ_dev->led.i_terminal->state == 1);
  E(lsim_cmd_line(lsim, "t;2;"));
  ASSRT(ledq_dev->led.i_terminal->state == 1);
  ASSRT(ledQ_dev->led.i_terminal->state == 0);
  E(lsim_cmd_line(lsim, "m;swd;0;"));
  ASSRT(ledq_dev->led.i_terminal->state == 1);
  ASSRT(ledQ_dev->led.i_terminal->state == 0);
  E(lsim_cmd_line(lsim, "t;2;"));
  ASSRT(ledq_dev->led.i_terminal->state == 0);
  ASSRT(ledQ_dev->led.i_terminal->state == 1);
  E(lsim_cmd_line(lsim, "m;swS;0;"));
  ASSRT(ledq_dev->led.i_terminal->state == 1);
  ASSRT(ledQ_dev->led.i_terminal->state == 0);

  E(lsim_delete(lsim));
}  /* test5 */


void test6() {
  lsim_t *lsim;

  E(lsim_create(&lsim, NULL));

  E(lsim_cmd_line(lsim, "d;panel;pan1;3;"));

  err_t *err;
  err = lsim_cmd_line(lsim, "c;pan1;o3;pan1;i3;");
  ASSRT(err);
  ASSRT(err->code == LSIM_ERR_COMMAND);

  E(lsim_cmd_line(lsim, "d;reg;reg1;4;"));  /* one more than panel. */
  E(lsim_cmd_line(lsim, "d;led;led_reg0;"));  /* one more than panel. */
  E(lsim_cmd_line(lsim, "d;swtch;swR;0;"));
  E(lsim_cmd_line(lsim, "d;clk;clk1;"));
  E(lsim_cmd_line(lsim, "c;swR;o0;reg1;R0;"));
  E(lsim_cmd_line(lsim, "c;swR;o0;reg1;d0;"));
  E(lsim_cmd_line(lsim, "c;swR;o0;clk1;R0;"));
  E(lsim_cmd_line(lsim, "c;clk1;q0;reg1;c0;"));
  E(lsim_cmd_line(lsim, "b;pan1;o0;reg1;d1;3;"));  /* Bus. */
  E(lsim_cmd_line(lsim, "c;reg1;q0;led_reg0;i0;"));
  E(lsim_cmd_line(lsim, "b;reg1;q1;pan1;i0;3;"));  /* Bus. */

  lsim_dev_t *led_reg0_dev;
  E(hmap_slookup(lsim->devs, "led_reg0", (void **)&led_reg0_dev));
  lsim_dev_t *led0_dev;
  E(hmap_slookup(lsim->devs, "pan1.led.0", (void **)&led0_dev));
  lsim_dev_t *led1_dev;
  E(hmap_slookup(lsim->devs, "pan1.led.1", (void **)&led1_dev));
  lsim_dev_t *led2_dev;
  E(hmap_slookup(lsim->devs, "pan1.led.2", (void **)&led2_dev));

  /* E(lsim_cmd_line(lsim, "v;1;")); */  /* Trace. */
  E(lsim_cmd_line(lsim, "p;"));  /* Power-up. */

  E(lsim_cmd_line(lsim, "t;2;"));      /* Allow reset to settle. */

  ASSRT(led_reg0_dev->led.illuminated == 0);
  ASSRT(led0_dev->led.illuminated == 0);
  ASSRT(led1_dev->led.illuminated == 0);
  ASSRT(led2_dev->led.illuminated == 0);

  E(lsim_cmd_line(lsim, "m;swR;1;"));  /* not reset. */
  E(lsim_cmd_line(lsim, "t;2;"));      /* Allow reset to settle. */

  ASSRT(led_reg0_dev->led.illuminated == 1);
  ASSRT(led0_dev->led.illuminated == 0);
  ASSRT(led1_dev->led.illuminated == 0);
  ASSRT(led2_dev->led.illuminated == 0);

  E(lsim_cmd_line(lsim, "m;pan1.swtch.0;1;"));
  E(lsim_cmd_line(lsim, "t;2;"));

  ASSRT(led_reg0_dev->led.illuminated == 1);
  ASSRT(led0_dev->led.illuminated == 1);
  ASSRT(led1_dev->led.illuminated == 0);
  ASSRT(led2_dev->led.illuminated == 0);

  E(lsim_cmd_line(lsim, "m;pan1.swtch.1;1;"));
  E(lsim_cmd_line(lsim, "t;2;"));

  ASSRT(led_reg0_dev->led.illuminated == 1);
  ASSRT(led0_dev->led.illuminated == 1);
  ASSRT(led1_dev->led.illuminated == 1);
  ASSRT(led2_dev->led.illuminated == 0);

  E(lsim_cmd_line(lsim, "m;pan1.swtch.2;1;"));
  E(lsim_cmd_line(lsim, "t;1;"));

  ASSRT(led_reg0_dev->led.illuminated == 1);
  ASSRT(led0_dev->led.illuminated == 1);
  ASSRT(led1_dev->led.illuminated == 1);
  ASSRT(led2_dev->led.illuminated == 1);

  E(lsim_delete(lsim));
}  /* test6 */


void test7() {
  lsim_t *lsim;

  E(lsim_create(&lsim, NULL));

  E(lsim_cmd_line(lsim, "v;1;"));  /* Trace. */

  E(lsim_cmd_line(lsim, "d;mem;mem1;4;3;"));  /* 3 data bits, 4 address bits. */
  E(lsim_cmd_line(lsim, "d;panel;d_pan;3;"));
  E(lsim_cmd_line(lsim, "d;panel;a_pan;4;"));
  E(lsim_cmd_line(lsim, "d;swtch;w_sw;0;"));

  E(lsim_cmd_line(lsim, "b;d_pan;o0;mem1;i0;3;"));
  E(lsim_cmd_line(lsim, "b;mem1;o0;d_pan;i0;3;"));
  E(lsim_cmd_line(lsim, "b;a_pan;o0;mem1;a0;4;"));
  E(lsim_cmd_line(lsim, "b;a_pan;o0;a_pan;i0;4;"));  /* Leds just match switches. */
  E(lsim_cmd_line(lsim, "c;w_sw;o0;mem1;w0;"));

  E(lsim_cmd_line(lsim, "p;"));  /* Power up. */

  E(lsim_cmd_line(lsim, "m;a_pan.swtch.1;1;"));  /* Memory location 2. */
  E(lsim_cmd_line(lsim, "m;d_pan.swtch.2;1;"));
  E(lsim_cmd_line(lsim, "m;w_sw;1;"));
  E(lsim_cmd_line(lsim, "m;w_sw;0;"));

  E(lsim_cmd_line(lsim, "m;a_pan.swtch.1;0;"));  /* Memory location 0. */
  E(lsim_cmd_line(lsim, "m;a_pan.swtch.1;1;"));  /* Memory location 2. */

  lsim_dev_t *mem_dev;
  E(hmap_slookup(lsim->devs, "mem1", (void **)&mem_dev));
  ASSRT(mem_dev);
  ASSRT(mem_dev->type == LSIM_DEV_TYPE_MEM);
  ASSRT(mem_dev->mem.words[0] == 0);
  ASSRT(mem_dev->mem.words[1] == 0);
  ASSRT(mem_dev->mem.words[2] == 4);
  ASSRT(mem_dev->mem.words[3] == 0);
  ASSRT(mem_dev->mem.words[4] == 0);
  ASSRT(mem_dev->mem.words[5] == 0);
  ASSRT(mem_dev->mem.words[6] == 0);
  ASSRT(mem_dev->mem.words[7] == 0);

  E(lsim_cmd_line(lsim, "l;mem1;1;0xb;"));
  E(lsim_cmd_line(lsim, "l;mem1;0x3;4;5;6;"));
  ASSRT(mem_dev->mem.words[0] == 0);
  ASSRT(mem_dev->mem.words[1] == 0x0b);
  ASSRT(mem_dev->mem.words[2] == 4);
  ASSRT(mem_dev->mem.words[3] == 4);
  ASSRT(mem_dev->mem.words[4] == 5);
  ASSRT(mem_dev->mem.words[5] == 6);
  ASSRT(mem_dev->mem.words[6] == 0);
  ASSRT(mem_dev->mem.words[7] == 0);

  E(lsim_delete(lsim));
}  /* test7 */


/* Test8 is for the "probe" device. We want to detect glitches and data
   changes during same step as control trigger. To make a glitch,
   use a single-input NAND gate "nand1" connected to a switch. Add a dual-input
   NAND "nand2" with its inputs connected to nand1's input and output. When the
   switch is at 0, nand1 has a 0 input and a 1 output. So nand2 has a 1 output.
   When the switch is moved to 1, the first cycle of the new step sees nand1's
   input at a 1. It's output takes a cycle to flip. So for that first cycle,
   both inputs of nand2 are 1, flipping its output to 0. But the next cycle of
   the same step has nand1's output flipping to 0, which flips nand2's output
   back to 0.  I.e. nand2's output glitches to 1 for a nand propagation delay.
   The probe should detect and warn about this. */
void test8() {
  lsim_t *lsim;

  E(lsim_create(&lsim, NULL));

  E(lsim_cmd_line(lsim, "v;1;"));  /* Trace. */

  E(lsim_cmd_line(lsim, "d;nand;nand1;1;"));
  E(lsim_cmd_line(lsim, "d;nand;nand2;1;"));
  E(lsim_cmd_line(lsim, "d;swtch;swtch;0;"));

  E(lsim_cmd_line(lsim, "c;swtch;o0;nand1;i0;"));
  E(lsim_cmd_line(lsim, "c;nand1;o0;nand2;i0;"));

  E(lsim_cmd_line(lsim, "d;nand;nand3;2;"));
  E(lsim_cmd_line(lsim, "c;swtch;o0;nand3;i0;"));
  E(lsim_cmd_line(lsim, "c;nand1;o0;nand3;i1;"));

  E(lsim_cmd_line(lsim, "d;probe;probe;0;"));
  E(lsim_cmd_line(lsim, "d;gnd;gnd;"));
  E(lsim_cmd_line(lsim, "c;gnd;o0;probe;d0;"));
  E(lsim_cmd_line(lsim, "c;nand3;o0;probe;c0;"));

  ASSRT(lsim->total_warnings == 0);

  E(lsim_cmd_line(lsim, "p;"));  /* Power up. */

  E(lsim_cmd_line(lsim, "m;swtch;1;"));  /* Power up. */
  ASSRT(lsim->total_warnings == 1);
  E(lsim_cmd_line(lsim, "m;swtch;0;"));  /* Power up. */
  ASSRT(lsim->total_warnings == 1);

  E(lsim_delete(lsim));

  /****************************************************/

  E(lsim_create(&lsim, NULL));

  E(lsim_cmd_line(lsim, "v;1;"));  /* Trace. */

  E(lsim_cmd_line(lsim, "d;nand;nand1;1;"));
  E(lsim_cmd_line(lsim, "d;nand;nand2;1;"));
  E(lsim_cmd_line(lsim, "d;swtch;swtch;0;"));

  E(lsim_cmd_line(lsim, "c;swtch;o0;nand1;i0;"));
  E(lsim_cmd_line(lsim, "c;nand1;o0;nand2;i0;"));

  E(lsim_cmd_line(lsim, "d;nand;nand3;2;"));
  E(lsim_cmd_line(lsim, "c;swtch;o0;nand3;i0;"));
  E(lsim_cmd_line(lsim, "c;nand1;o0;nand3;i1;"));

  E(lsim_cmd_line(lsim, "d;probe;probe;1;"));
  E(lsim_cmd_line(lsim, "c;swtch;o0;probe;d0;"));
  E(lsim_cmd_line(lsim, "c;nand3;o0;probe;c0;"));

  ASSRT(lsim->total_warnings == 0);

  E(lsim_cmd_line(lsim, "p;"));  /* Power up. */

  E(lsim_cmd_line(lsim, "m;swtch;1;"));  /* Power up. */
  ASSRT(lsim->total_warnings == 2);
  E(lsim_cmd_line(lsim, "m;swtch;0;"));  /* Power up. */
  ASSRT(lsim->total_warnings == 2);

  E(lsim_delete(lsim));
}  /* test8 */


void test9() {
  lsim_t *lsim;

  E(lsim_create(&lsim, NULL));

  E(lsim_cmd_line(lsim, "d;addbit;adder;"));

  E(lsim_cmd_line(lsim, "d;swtch;sw_a;0;"));
  E(lsim_cmd_line(lsim, "w;sw_a;1;"));
  E(lsim_cmd_line(lsim, "d;swtch;sw_b;0;"));
  E(lsim_cmd_line(lsim, "w;sw_b;1;"));
  E(lsim_cmd_line(lsim, "d;swtch;sw_i;0;"));
  E(lsim_cmd_line(lsim, "w;sw_i;1;"));

  E(lsim_cmd_line(lsim, "d;led;led_s;"));
  lsim_dev_t *led_s;
  E(hmap_slookup(lsim->devs, "led_s", (void **)&led_s));
  ASSRT(led_s);
  ASSRT(led_s->type == LSIM_DEV_TYPE_LED);
  ASSRT(led_s->led.illuminated == 0);
  ASSRT(led_s->led.i_terminal->state == 0);

  E(lsim_cmd_line(lsim, "d;led;led_o;"));
  lsim_dev_t *led_o;
  E(hmap_slookup(lsim->devs, "led_o", (void **)&led_o));
  ASSRT(led_o);
  ASSRT(led_o->type == LSIM_DEV_TYPE_LED);
  ASSRT(led_o->led.illuminated == 0);
  ASSRT(led_o->led.i_terminal->state == 0);

  E(lsim_cmd_line(lsim, "c;sw_a;o0;adder;a0;"));
  E(lsim_cmd_line(lsim, "c;sw_b;o0;adder;b0;"));
  E(lsim_cmd_line(lsim, "c;sw_i;o0;adder;i0;"));
  E(lsim_cmd_line(lsim, "c;adder;s0;led_s;i0;"));
  E(lsim_cmd_line(lsim, "c;adder;o0;led_o;i0;"));

  E(lsim_cmd_line(lsim, "p;"));  /* Power up. */
  ASSRT(led_s->led.illuminated == 0);
  ASSRT(led_o->led.illuminated == 0);

  E(lsim_cmd_line(lsim, "m;sw_a;1;"));
  ASSRT(led_s->led.illuminated == 1);
  ASSRT(led_o->led.illuminated == 0);

  E(lsim_cmd_line(lsim, "m;sw_b;1;"));
  ASSRT(led_s->led.illuminated == 0);
  ASSRT(led_o->led.illuminated == 1);

  E(lsim_cmd_line(lsim, "m;sw_a;0;"));
  ASSRT(led_s->led.illuminated == 1);
  ASSRT(led_o->led.illuminated == 0);

  E(lsim_cmd_line(lsim, "m;sw_i;1;"));
  ASSRT(led_s->led.illuminated == 0);
  ASSRT(led_o->led.illuminated == 1);

  E(lsim_cmd_line(lsim, "m;sw_a;1;"));
  ASSRT(led_o->led.illuminated == 1);
  ASSRT(led_o->led.illuminated == 1);

  E(lsim_delete(lsim));
}  /* test9 */


void test10() {
  lsim_t *lsim;

  E(lsim_create(&lsim, NULL));

  E(lsim_cmd_line(lsim, "d;addword;adder;3;"));
  E(lsim_cmd_line(lsim, "d;panel;panela;3;"));
  E(lsim_cmd_line(lsim, "d;panel;panelb;3;"));
  E(lsim_cmd_line(lsim, "d;swtch;carry_in_sw;0;"));
  E(lsim_cmd_line(lsim, "d;gnd;gnd;"));

  E(lsim_cmd_line(lsim, "b;panela;o0;adder;a0;3;"));
  E(lsim_cmd_line(lsim, "b;panelb;o0;adder;b0;3;"));
  E(lsim_cmd_line(lsim, "b;adder;s0;panela;i0;3;"));
  E(lsim_cmd_line(lsim, "c;carry_in_sw;o0;adder;i0;"));
  E(lsim_cmd_line(lsim, "c;adder;o0;panelb;i0;"));
  E(lsim_cmd_line(lsim, "c;gnd;o0;panelb;i1;"));
  E(lsim_cmd_line(lsim, "c;gnd;o0;panelb;i2;"));

  lsim_dev_t *led0_dev;
  E(hmap_slookup(lsim->devs, "panela.led.0", (void **)&led0_dev));
  lsim_dev_t *led1_dev;
  E(hmap_slookup(lsim->devs, "panela.led.1", (void **)&led1_dev));
  lsim_dev_t *led2_dev;
  E(hmap_slookup(lsim->devs, "panela.led.2", (void **)&led2_dev));

  E(lsim_cmd_line(lsim, "w;panela.swtch.0;1;"));
  E(lsim_cmd_line(lsim, "w;panela.swtch.1;1;"));
  E(lsim_cmd_line(lsim, "w;panela.swtch.2;1;"));
  E(lsim_cmd_line(lsim, "w;panelb.swtch.0;1;"));
  E(lsim_cmd_line(lsim, "w;panelb.swtch.1;1;"));
  E(lsim_cmd_line(lsim, "w;panelb.swtch.2;1;"));
  E(lsim_cmd_line(lsim, "w;carry_in_sw;1;"));
  E(lsim_cmd_line(lsim, "v;3;"));
  E(lsim_cmd_line(lsim, "p;"));  /* Power up. */
  ASSRT(led0_dev->led.illuminated == 0);
  ASSRT(led1_dev->led.illuminated == 0);
  ASSRT(led2_dev->led.illuminated == 0);

  E(lsim_cmd_line(lsim, "m;panela.swtch.0;1;"));
  ASSRT(led0_dev->led.illuminated == 1);
  ASSRT(led1_dev->led.illuminated == 0);
  ASSRT(led2_dev->led.illuminated == 0);
  E(lsim_cmd_line(lsim, "m;panelb.swtch.0;1;"));
  ASSRT(led0_dev->led.illuminated == 0);
  ASSRT(led1_dev->led.illuminated == 1);
  ASSRT(led2_dev->led.illuminated == 0);
  E(lsim_cmd_line(lsim, "m;carry_in_sw;1;"));
  ASSRT(led0_dev->led.illuminated == 1);
  ASSRT(led1_dev->led.illuminated == 1);
  ASSRT(led2_dev->led.illuminated == 0);
  E(lsim_cmd_line(lsim, "m;panelb.swtch.1;1;"));
  ASSRT(led0_dev->led.illuminated == 1);
  ASSRT(led1_dev->led.illuminated == 0);
  ASSRT(led2_dev->led.illuminated == 1);
  E(lsim_cmd_line(lsim, "m;panela.swtch.1;1;"));
  ASSRT(led0_dev->led.illuminated == 1);
  ASSRT(led1_dev->led.illuminated == 1);
  ASSRT(led2_dev->led.illuminated == 1);

  E(lsim_delete(lsim));
}  /* test10 */


int main(int argc, char **argv) {
  parse_cmdline(argc, argv);

  if (o_testnum == 0 || o_testnum == 1) {
    test1();
    printf("test1: success\n");
  }

  if (o_testnum == 0 || o_testnum == 2) {
    test2();
    printf("test2: success\n");
  }

  if (o_testnum == 0 || o_testnum == 3) {
    test3();
    printf("test3: success\n");
  }

  if (o_testnum == 0 || o_testnum == 4) {
    test4();
    printf("test4: success\n");
  }

  if (o_testnum == 0 || o_testnum == 5) {
    test5();
    printf("test5: success\n");
  }

  if (o_testnum == 0 || o_testnum == 6) {
    test6();
    printf("test6: success\n");
  }

  if (o_testnum == 0 || o_testnum == 7) {
    test7();
    printf("test7: success\n");
  }

  if (o_testnum == 0 || o_testnum == 8) {
    test8();
    printf("test8: success\n");
  }

  if (o_testnum == 0 || o_testnum == 9) {
    test9();
    printf("test9: success\n");
  }

  if (o_testnum == 0 || o_testnum == 10) {
    test10();
    printf("test10: success\n");
  }

  return 0;
}  /* main */

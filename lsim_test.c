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

#if defined(_WIN32)
#define MY_SLEEP_MS(msleep_msecs) Sleep(msleep_msecs)
#else
#define MY_SLEEP_MS(msleep_msecs) usleep((msleep_msecs)/1000)
#endif

#define E(e__test) do { \
  err_t *e__err = (e__test); \
  if (e__err != ERR_OK) { \
    printf("ERROR [%s:%d]: '%s' returned error\n", __FILE__, __LINE__, #e__test); \
    ERR_ABRT_ON_ERR(e__err, stdout); \
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
    "For details, see https://github.com/fordsfords/lsim\n",
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
  E(hmap_lookup(lsim->devs, "MyNand", strlen("MyNand"), (void **)&nand_dev));
  ASSRT(nand_dev);
  ASSRT(nand_dev->type == LSIM_DEV_TYPE_NAND);
  ASSRT(nand_dev->nand.num_inputs == 2);
  ASSRT(nand_dev->nand.out_terminal->state == 0);
  ASSRT(nand_dev->nand.out_terminal->in_terminal_list == NULL);
  ASSRT(nand_dev->nand.in_terminals[0].state == 0);
  ASSRT(nand_dev->nand.in_terminals[1].state == 0);

  lsim_dev_t *vcc_dev;
  E(hmap_lookup(lsim->devs, "MyVcc", strlen("MyVcc"), (void **)&vcc_dev));
  ASSRT(vcc_dev);
  ASSRT(vcc_dev->type == LSIM_DEV_TYPE_VCC);
  ASSRT(vcc_dev->vcc.out_terminal->state == 0);
  ASSRT(vcc_dev->vcc.out_terminal->in_terminal_list == NULL);

  lsim_dev_t *vcc2_dev;
  E(hmap_lookup(lsim->devs, "-My_Vcc2", strlen("-My_Vcc2"), (void **)&vcc2_dev));
  ASSRT(vcc2_dev);
  ASSRT(vcc2_dev->type == LSIM_DEV_TYPE_VCC);
  ASSRT(vcc2_dev->vcc.out_terminal->state == 0);
  ASSRT(vcc2_dev->vcc.out_terminal->in_terminal_list == NULL);

  E(lsim_cmd_line(lsim, "c;-My_Vcc2;o0;MyNand;i0;"));

  lsim_dev_t *tmp_dev;
  E(hmap_lookup(lsim->devs, "-My_Vcc2", strlen("-My_Vcc2"), (void **)&tmp_dev));
  ASSRT(tmp_dev == vcc2_dev);
  ASSRT(vcc2_dev->type == LSIM_DEV_TYPE_VCC);
  ASSRT(vcc2_dev->vcc.out_terminal->state == 0);
  ASSRT(vcc2_dev->vcc.out_terminal->in_terminal_list == &nand_dev->nand.in_terminals[0]);
  ASSRT(nand_dev->nand.in_terminals[0].next_in_terminal == NULL);
  ASSRT(nand_dev->nand.in_terminals[0].driving_out_terminal == vcc2_dev->vcc.out_terminal);
  ASSRT(nand_dev->nand.in_terminals[1].driving_out_terminal == NULL);
  ASSRT(nand_dev->nand.out_terminal->in_terminal_list == NULL);  /* Nand isn't driving anything yet. */

  E(lsim_cmd_line(lsim, "c;MyNand;o0;MyNand;i1;"));
  ASSRT(nand_dev->nand.out_terminal->in_terminal_list == &nand_dev->nand.in_terminals[1]);

  E(lsim_cmd_line(lsim, "p;"));  /* Power. */
  /* Three devices with changed input. */
  ASSRT(lsim->in_changed_list);
  ASSRT(lsim->in_changed_list->next_in_changed);
  ASSRT(lsim->in_changed_list->next_in_changed->next_in_changed);
  ASSRT(lsim->in_changed_list->next_in_changed->next_in_changed->next_in_changed == NULL);
  /* No output changes yet. */
  ASSRT(lsim->out_changed_list == NULL);
  ASSRT(vcc_dev->vcc.out_terminal->state == 0);
  ASSRT(vcc2_dev->vcc.out_terminal->state == 0);
  ASSRT(nand_dev->nand.out_terminal->state == 0);

  /* First calculate new outputs. */
  E(lsim_dev_run_logic(lsim));

  /* Three devices with changed output. */
  ASSRT(lsim->out_changed_list);
  ASSRT(lsim->out_changed_list->next_out_changed);
  ASSRT(lsim->out_changed_list->next_out_changed->next_out_changed);
  ASSRT(lsim->out_changed_list->next_out_changed->next_out_changed->next_out_changed == NULL);
  ASSRT(vcc_dev->vcc.out_terminal->state == 1);
  ASSRT(vcc2_dev->vcc.out_terminal->state == 1);
  ASSRT(nand_dev->nand.out_terminal->state == 1);
  /* No inputs changes yet. */
  ASSRT(lsim->in_changed_list == NULL);
  ASSRT(nand_dev->nand.in_terminals[0].state == 0);
  ASSRT(nand_dev->nand.in_terminals[1].state == 0);

  /* Propagate outputs to inputs. */
  E(lsim_dev_propagate_outputs(lsim));

  /* One device with changed input. */
  ASSRT(lsim->in_changed_list);
  ASSRT(lsim->in_changed_list->next_in_changed == NULL);
  ASSRT(nand_dev->nand.in_terminals[0].state == 1);
  ASSRT(nand_dev->nand.in_terminals[1].state == 1);
  /* No outputs changes yet. */
  ASSRT(lsim->out_changed_list == NULL);
  ASSRT(vcc2_dev->vcc.out_terminal->state == 1);
  ASSRT(nand_dev->nand.out_terminal->state == 1);

  /* Calculate new outputs. */
  E(lsim_dev_run_logic(lsim));

  /* One device with changed output. */
  ASSRT(lsim->out_changed_list);
  ASSRT(lsim->out_changed_list->next_out_changed == NULL);
  ASSRT(vcc2_dev->vcc.out_terminal->state == 1);
  ASSRT(nand_dev->nand.out_terminal->state == 0);
  /* No inputs changes yet. */
  ASSRT(lsim->in_changed_list == NULL);
  ASSRT(nand_dev->nand.in_terminals[0].state == 1);
  ASSRT(nand_dev->nand.in_terminals[1].state == 1);

  /* Propagate outputs to inputs. */
  E(lsim_dev_propagate_outputs(lsim));

  /* One device with changed input. */
  ASSRT(lsim->in_changed_list);
  ASSRT(lsim->in_changed_list->next_in_changed == NULL);
  ASSRT(nand_dev->nand.in_terminals[0].state == 1);
  ASSRT(nand_dev->nand.in_terminals[1].state == 0);
  /* No outputs changes yet. */
  ASSRT(lsim->out_changed_list == NULL);
  ASSRT(vcc2_dev->vcc.out_terminal->state == 1);
  ASSRT(nand_dev->nand.out_terminal->state == 0);

  err = lsim_cmd_line(lsim, "s;1;");
  ASSRT(err);
  ASSRT(err->code == LSIM_ERR_MAXLOOPS);

  ASSRT(lsim->quit == 0);
  E(lsim_cmd_line(lsim, "q;"));
  ASSRT(lsim->quit == 1);

  E(lsim_delete(lsim));

  E(lsim_create(&lsim, NULL));

  E(lsim_cmd_file(lsim, "srlatch.lsim"));

  E(lsim_cmd_line(lsim, "d;gnd;my_gnd;"));
  E(lsim_cmd_line(lsim, "d;led;my_led;"));
  E(lsim_cmd_line(lsim, "c;my_gnd;o0;my_led;i0;"));

  E(lsim_cmd_line(lsim, "s;1;"));

  lsim_dev_t *led_dev;
  E(hmap_lookup(lsim->devs, "my_led", strlen("my_led"), (void **)&led_dev));
  ASSRT(led_dev);
  ASSRT(led_dev->type == LSIM_DEV_TYPE_LED);
  ASSRT(led_dev->led.illuminated == 0);
  ASSRT(led_dev->led.in_terminal->state == 0);

  E(lsim_cmd_line(lsim, "s;1;"));

  ASSRT(led_dev);
  ASSRT(led_dev->type == LSIM_DEV_TYPE_LED);
  ASSRT(led_dev->led.illuminated == 0);
  ASSRT(led_dev->led.in_terminal->state == 0);

  E(lsim_cmd_line(lsim, "t;1;"));
  E(lsim_cmd_line(lsim, "d;clk1;my_clk;"));
  E(lsim_cmd_line(lsim, "d;swtch;Reset_sw;0;"));
  E(lsim_cmd_line(lsim, "d;led;qled;"));
  E(lsim_cmd_line(lsim, "d;led;Qled;"));
  E(lsim_cmd_line(lsim, "c;my_clk;q0;qled;i0;"));
  E(lsim_cmd_line(lsim, "c;my_clk;Q0;Qled;i0;"));
  E(lsim_cmd_line(lsim, "c;Reset_sw;o0;my_clk;R0;"));

  lsim_dev_t *qled_dev;
  E(hmap_lookup(lsim->devs, "qled", strlen("qled"), (void **)&qled_dev));
  ASSRT(qled_dev);
  ASSRT(qled_dev->led.illuminated == 0);
  lsim_dev_t *Qled_dev;
  E(hmap_lookup(lsim->devs, "Qled", strlen("Qled"), (void **)&Qled_dev));
  ASSRT(Qled_dev);
  ASSRT(Qled_dev->led.illuminated == 0);
  lsim_dev_t *clk_dev;
  E(hmap_lookup(lsim->devs, "my_clk", strlen("my_clk"), (void **)&clk_dev));
  ASSRT(clk_dev);

  E(lsim_cmd_line(lsim, "s;1;"));

  ASSRT(qled_dev->led.illuminated == 0);
  ASSRT(Qled_dev->led.illuminated == 0);

  E(lsim_cmd_line(lsim, "m;Reset_sw;1;"));
  E(lsim_cmd_line(lsim, "s;1;"));

  ASSRT(qled_dev->led.illuminated == 0);
  ASSRT(Qled_dev->led.illuminated == 1);

  E(lsim_cmd_line(lsim, "s;1;"));

  ASSRT(qled_dev->led.illuminated == 1);
  ASSRT(Qled_dev->led.illuminated == 0);

  E(lsim_cmd_line(lsim, "s;1;"));

  ASSRT(qled_dev->led.illuminated == 0);
  ASSRT(Qled_dev->led.illuminated == 1);

  E(lsim_delete(lsim));
}  /* test1 */


void test2() {
  lsim_t *lsim;

  E(lsim_create(&lsim, NULL));

  E(lsim_cmd_line(lsim, "t;1;"));
  E(lsim_cmd_line(lsim, "i;srlatch.lsim;"));

  E(lsim_delete(lsim));
}  /* test2 */


int main(int argc, char **argv) {
  parse_cmdline(argc, argv);

  if (o_testnum == 0 || o_testnum == 1) {
    test1();
    printf("test1: success\n");
  }

  if (o_testnum == 0 || o_testnum == 2) {
    test2();
    printf("test1: success\n");
  }

  return 0;
}  /* main */

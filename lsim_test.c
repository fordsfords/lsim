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

  lsim_dev_t *nand_device;
  E(hmap_lookup(lsim->devs, "MyNand", strlen("MyNand"), (void **)&nand_device));
  ASSRT(nand_device);
  ASSRT(nand_device->type == LSIM_DEV_TYPE_NAND);
  ASSRT(nand_device->nand.num_inputs == 2);
  ASSRT(nand_device->nand.out_terminal->state == 0);
  ASSRT(nand_device->nand.out_terminal->in_terminal_list == NULL);
  ASSRT(nand_device->nand.in_terminals[0].state == 0);
  ASSRT(nand_device->nand.in_terminals[1].state == 0);

  lsim_dev_t *vcc_device;
  E(hmap_lookup(lsim->devs, "MyVcc", strlen("MyVcc"), (void **)&vcc_device));
  ASSRT(vcc_device);
  ASSRT(vcc_device->type == LSIM_DEV_TYPE_VCC);
  ASSRT(vcc_device->vcc.out_terminal->state == 0);
  ASSRT(vcc_device->vcc.out_terminal->in_terminal_list == NULL);

  lsim_dev_t *vcc2_device;
  E(hmap_lookup(lsim->devs, "-My_Vcc2", strlen("-My_Vcc2"), (void **)&vcc2_device));
  ASSRT(vcc2_device);
  ASSRT(vcc2_device->type == LSIM_DEV_TYPE_VCC);
  ASSRT(vcc2_device->vcc.out_terminal->state == 0);
  ASSRT(vcc2_device->vcc.out_terminal->in_terminal_list == NULL);

  E(lsim_cmd_line(lsim, "c;-My_Vcc2;o0;MyNand;i0;"));

  lsim_dev_t *tmp_device;
  E(hmap_lookup(lsim->devs, "-My_Vcc2", strlen("-My_Vcc2"), (void **)&tmp_device));
  ASSRT(tmp_device == vcc2_device);
  ASSRT(vcc2_device->type == LSIM_DEV_TYPE_VCC);
  ASSRT(vcc2_device->vcc.out_terminal->state == 0);
  ASSRT(vcc2_device->vcc.out_terminal->in_terminal_list == &nand_device->nand.in_terminals[0]);
  ASSRT(nand_device->nand.in_terminals[0].next_in_terminal == NULL);
  ASSRT(nand_device->nand.in_terminals[0].driving_out_terminal == vcc2_device->vcc.out_terminal);
  ASSRT(nand_device->nand.in_terminals[1].driving_out_terminal == NULL);
  ASSRT(nand_device->nand.out_terminal->in_terminal_list == NULL);  /* Nand isn't driving anything yet. */

  E(lsim_cmd_line(lsim, "c;MyNand;o0;MyNand;i1;"));
  ASSRT(nand_device->nand.out_terminal->in_terminal_list == &nand_device->nand.in_terminals[1]);

  E(lsim_cmd_line(lsim, "p;"));  /* Power. */
  /* Three devices with changed input. */
  ASSRT(lsim->in_changed_list);
  ASSRT(lsim->in_changed_list->next_in_changed);
  ASSRT(lsim->in_changed_list->next_in_changed->next_in_changed);
  ASSRT(lsim->in_changed_list->next_in_changed->next_in_changed->next_in_changed == NULL);
  /* No output changes yet. */
  ASSRT(lsim->out_changed_list == NULL);
  ASSRT(vcc_device->vcc.out_terminal->state == 0);
  ASSRT(vcc2_device->vcc.out_terminal->state == 0);
  ASSRT(nand_device->nand.out_terminal->state == 0);

  /* First calculate new outputs. */
  E(lsim_dev_run_logic(lsim));

  /* Three devices with changed output. */
  ASSRT(lsim->out_changed_list);
  ASSRT(lsim->out_changed_list->next_out_changed);
  ASSRT(lsim->out_changed_list->next_out_changed->next_out_changed);
  ASSRT(lsim->out_changed_list->next_out_changed->next_out_changed->next_out_changed == NULL);
  ASSRT(vcc_device->vcc.out_terminal->state == 1);
  ASSRT(vcc2_device->vcc.out_terminal->state == 1);
  ASSRT(nand_device->nand.out_terminal->state == 1);
  /* No inputs changes yet. */
  ASSRT(lsim->in_changed_list == NULL);
  ASSRT(nand_device->nand.in_terminals[0].state == 0);
  ASSRT(nand_device->nand.in_terminals[1].state == 0);

  /* Propogate outputs to inputs. */
  E(lsim_dev_propagate_outputs(lsim));

  /* One device with changed input. */
  ASSRT(lsim->in_changed_list);
  ASSRT(lsim->in_changed_list->next_in_changed == NULL);
  ASSRT(nand_device->nand.in_terminals[0].state == 1);
  ASSRT(nand_device->nand.in_terminals[1].state == 1);
  /* No outputs changes yet. */
  ASSRT(lsim->out_changed_list == NULL);
  ASSRT(vcc2_device->vcc.out_terminal->state == 1);
  ASSRT(nand_device->nand.out_terminal->state == 1);

  /* Calculate new outputs. */
  E(lsim_dev_run_logic(lsim));

  /* One device with changed output. */
  ASSRT(lsim->out_changed_list);
  ASSRT(lsim->out_changed_list->next_out_changed == NULL);
  ASSRT(vcc2_device->vcc.out_terminal->state == 1);
  ASSRT(nand_device->nand.out_terminal->state == 0);
  /* No inputs changes yet. */
  ASSRT(lsim->in_changed_list == NULL);
  ASSRT(nand_device->nand.in_terminals[0].state == 1);
  ASSRT(nand_device->nand.in_terminals[1].state == 1);

  /* Propogate outputs to inputs. */
  E(lsim_dev_propagate_outputs(lsim));

  /* One device with changed input. */
  ASSRT(lsim->in_changed_list);
  ASSRT(lsim->in_changed_list->next_in_changed == NULL);
  ASSRT(nand_device->nand.in_terminals[0].state == 1);
  ASSRT(nand_device->nand.in_terminals[1].state == 0);
  /* No outputs changes yet. */
  ASSRT(lsim->out_changed_list == NULL);
  ASSRT(vcc2_device->vcc.out_terminal->state == 1);
  ASSRT(nand_device->nand.out_terminal->state == 0);

  err = lsim_cmd_line(lsim, "s;1;");
  ASSRT(err);
  ASSRT(err->code == LSIM_ERR_MAXLOOPS);

  ASSRT(lsim->quit == 0);
  E(lsim_cmd_line(lsim, "q;"));
  ASSRT(lsim->quit == 1);

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

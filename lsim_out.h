/* lsim_out.h
 * DRAFT: proposed output API for lsim. Not yet included by any .c file.
 *
 * Replaces the scattered printf() calls in lsim_dev.c and lsim_devs_*.c.
 * lsim calls these functions to report events; it never prints them itself.
 * Each front-end supplies its own implementation:
 *   - lsim_main / lsim_test: lsim_out_printf (same text as today's printf's).
 *   - the library front-end for ldraw: queues structured events for the host.
 *
 * Error reporting (err_t, stderr) and the "Trace:" echo of command lines
 * are NOT events; they stay as they are.
 *
 */

/* This work is dedicated to the public domain under CC0 1.0 Universal:
 * http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Steven Ford has waived all copyright
 * and related or neighboring rights to this work. In other words, you can
 * use this code for any purpose without any restrictions.
 * This work is published from: United States.
 * Project home: https://github.com/fordsfords/lsim
 */
#ifndef LSIM_OUT_H
#define LSIM_OUT_H

#ifdef __cplusplus
extern "C" {
#endif

/* Forward declaration; full definition is in lsim.h. */
typedef struct lsim_s lsim_t;

/* Kinds of warning reported through the `warning` callback. */
typedef enum {
  LSIM_OUT_WARN_PROBE_CTRL_GLITCH = 1,      /* control trigger glitch */
  LSIM_OUT_WARN_PROBE_DATA_BEFORE_RISE,     /* data changed before rising trigger */
  LSIM_OUT_WARN_PROBE_DATA_BEFORE_FALL      /* data changed before falling trigger */
} lsim_out_warn_kind_t;

/* Each callback may be NULL, meaning "ignore this event". Callbacks must not
 * call back into lsim, and must not keep the `name` pointers (they belong to
 * lsim and are valid only for the duration of the call). */
typedef struct lsim_out_s lsim_out_t;
struct lsim_out_s {
  void *ctx;  /* Owned by the front-end; lsim never touches it. */

  /* --- Engine progress. Today: lsim_dev.c. --- */

  /* A ticklet (half clock cycle) is starting. Today: " Ticklet N". */
  void (*ticklet_begin)(lsim_out_t *out, lsim_t *lsim, long ticklet);

  /* A step is starting. Today: " Step N:". Also announced per verbosity. */
  void (*step_begin)(lsim_out_t *out, lsim_t *lsim, long step);

  /* A propagate cycle within a step is starting. Today: "  Cycle N:". */
  void (*cycle_begin)(lsim_out_t *out, lsim_t *lsim, long cycle);

  /* NEW (no printf today): the circuit has settled; the step is over.
   * Viewers should apply buffered LED changes here. Called whether or not
   * anything changed. Requires a new call site in lsim_dev.c. */
  void (*step_end)(lsim_out_t *out, lsim_t *lsim, long step);

  /* --- Device events. --- */

  /* An LED's input changed. Today: "Led NAME: on|off (ticklet N)[ glitch]".
   * Always reported, regardless of verbosity. `glitch` is nonzero if the LED
   * changed more than once in the same step. Panel LEDs arrive with their
   * full internal name, "<panel>.led.<i>". Today: lsim_devs_led.c. */
  void (*led)(lsim_out_t *out, lsim_t *lsim,
      const char *name, int state, long ticklet, int glitch);

  /* Verbose device dumps (only called when the matching verbosity bit is
   * set; the device decides, not the front-end). */

  /* Devices with a single output o0: nand, swtch, vcc, gnd.
   * `type` is the device type name, e.g. "nand". */
  void (*dev_o0)(lsim_out_t *out, lsim_t *lsim,
      const char *type, const char *name, int state);

  /* clk: both outputs. Today: "  clk NAME: q0=%d, Q0=%d". */
  void (*dev_clk)(lsim_out_t *out, lsim_t *lsim,
      const char *name, int q0, int Q0);

  /* mem: current data output. Today: "  mem NAME: o=%ld (0x%lx)". */
  void (*dev_mem)(lsim_out_t *out, lsim_t *lsim,
      const char *name, long data_val);

  /* --- Warnings. Today: lsim_devs_probe.c ("Warning: probe ..."). --- */

  void (*warning)(lsim_out_t *out, lsim_t *lsim,
      lsim_out_warn_kind_t kind, const char *dev_name, long step);
};

/* The printf implementation used by lsim_main and lsim_test. Produces the
 * same text as the printf() calls it replaces. stdout stays fully buffered;
 * it never flushes. */
extern lsim_out_t lsim_out_printf;

#ifdef __cplusplus
}
#endif

#endif  /* LSIM_OUT_H */

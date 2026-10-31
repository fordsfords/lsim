# Connecting lsim to ldraw (live, interactive)

Status: **design decided; no lsim code changed yet.** The previous version of
this document (all the options considered, with the author's replies) is in
git history: `git show ef2d642:ldraw_connect.md`.

## Goal

ldraw (browser schematic editor, `../ldraw`) drives and displays a running
lsim simulation: click a switch, step a ticklet or a step, and see LEDs
change. Editing and running are one application with a prog/run mode switch
(below). Used mainly for low-level visual debugging and demos, so the unit of
progress is a single ticklet or step, not continuous real-time running.

## Decisions

* **Interactive and single-step.** ldraw sends commands (`m;`, `t;`, ...) and
  receives the resulting events. Two-way, so ldraw controls lsim, not the
  reverse. lsim never sleeps or paces itself.
* **lsim becomes a library with a third front-end.** The engine is called,
  not run as a process, so no stdin/stdout pipe and no networking in C.
* **Output goes through an API, not `printf`.** See `lsim_out.h`. Every place
  lsim now prints an event calls a function in a `lsim_out_t` table instead.
  Events are structured (one function per kind), so the ldraw front-end does
  not parse text.
* **Bridge in Python using `ctypes`.** Python loads the lsim shared library,
  calls into it, and talks WebSocket (or SSE+POST) to the browser. Python was
  chosen over Node because `ctypes` needs no build step or extra package,
  whereas Node needs `ffi-napi` or a native addon.
* **ldraw and the run mode are unified; the bridge is required.** ldraw has a
  prog/run switch. Prog mode is today's editor. Flipping to run exports the
  `.lsim` text, sends it over the socket, and the bridge creates a fresh
  `lsim_t` and feeds it through `lsim_cmd_line`, one line per call. In run
  mode the drawing is read-only; switches are clickable, LEDs are live, and
  step/ticklet/run controls are active. Flipping back to prog deletes the
  `lsim_t` (lsim has no undefine/disconnect, so any edit means a rebuild).
  Because the drawing is the source of truth, no stale-export detection is
  needed. There is no editor-only mode without the bridge.
* **ldraw no longer opens from `file://`.** The bridge serves `ldraw.html`
  from `http://127.0.0.1:PORT`. Bind to 127.0.0.1 only, because `i;` lets a
  command read files. ldraw's README/CLAUDE.md "single HTML file" rule must
  be updated when this is built.
* **Two repos, one-way dependency.** lsim stays standalone (current mains
  unchanged in behavior) and knows nothing of ldraw. ldraw requires lsim.
  Not merged: different languages, different owners (Steven authors lsim;
  Claude maintains ldraw), and the coupling is just one header plus one
  shared library.
* Flushing is only for the ldraw front-end. `lsim_main` and `lsim_test` keep
  full buffering for speed.

## Where things live

* **lsim repo:** `lsim_out.h` (engine event API), the library front-end
  (`lsim_lib.c` / `lsim_lib.h`, the stable C API called by ctypes), and the
  `bld.sh` step that builds `liblsim.so` (`.dll` on Windows). The front-end
  compiles against lsim internals, so it is built and tested with them via
  `tst.sh`.
* **ldraw repo:** `bridge.py` (serves `ldraw.html`, WebSocket to the browser,
  ctypes into the lsim library) and the ldraw simulation mode. The
  browser-to-bridge protocol is private to ldraw.
* **Finding lsim:** sibling checkout `../lsim`, overridable with `LSIM_DIR`.
  ldraw's start script runs lsim's `bld.sh` if the library is missing or
  stale. No submodule, no vendored lsim sources, no committed binaries.
* **Version check:** the library exports `lsim_lib_api_version()`; the bridge
  checks it at startup and fails with a clear "rebuild lsim" message.

## Division of work

* **Steven (author)** edits the existing lsim modules to call the API, and
  implements the API in `lsim_main.c` / `lsim_test.c` (printf behavior).
* **Claude** writes and maintains the library front-end, the Python bridge and
  the ldraw side, and keeps `lsim_out.h` in step with what lsim needs.

## Architecture

```
ldraw.html  <--WebSocket-->  bridge.py  --ctypes-->  liblsim.so
 (browser)                  (Python)                 (engine + library
                                                      front-end + lsim_out impl)
```

* `liblsim.so` = all lsim sources except `lsim_main.c` / `lsim_test.c`,
  plus the new front-end file. It exposes a small C API, roughly:
  `open(config, file)`, `command(line)` (one `lsim_cmd_line`), and
  `poll_events()` returning the events queued since the last call (a
  caller-supplied buffer of fixed-size records, or one JSON string per call).
  Polling beats upcalls: no callback-from-C-into-Python threading issues, and
  the bridge sends one browser message per command.
* The front-end installs a `lsim_out_t` that appends to an event queue.
  `step_end` is the point where a viewer applies buffered LED changes.
* Commands enter only through `lsim_cmd_line()` (public, `lsim_cmd.h`); the
  bridge reads the `.lsim` text itself and sends line by line, so `lsim_cmd_file`
  is never used. A returned `err_t` becomes an error reply and is freed; the
  simulator stays alive. `q;` sets `lsim->quit`, which the front-end reports
  without exiting.
* Initial state: the front-end walks the LED devices in `lsim->devs` and
  reports each through the `led` callback (glitch = 0), so ldraw can start
  from a known state. LED names equal the drawing's (`<panel>.led.<i>` for
  panel LEDs). If walking `lsim->devs` from outside the engine is unwanted, a
  small `lsim_dev_foreach_led()` helper in `lsim_dev.c` can replace it.
* Later: the same API can gain events for `w;`-watched terminals so ldraw
  can color wires by value.

## Shared-library facts (verified)

* All sources compile with `-fPIC -shared` under the project's flags
  (`-std=c11 -Wall -Wextra -pedantic -Werror`) with no changes.
* The only undefined symbol is `global_error_reaction`, which each existing
  main defines. The library front-end must define it too.
* The library must call `lsim_cmd_line` and handle the returned `err_t`
  itself, reporting it to ldraw. `lsim_cmd_file` applies
  `global_error_reaction`, which can `exit(1)` or `abort()` and would kill the
  Python process, so the library never calls it.
* The only other global data is read-only: the error-code constants and
  `lsim_cfg_defaults`. No mutable per-process state, so several `lsim_t`
  instances could coexist (except `global_error_reaction`, shared by all).
  ldraw needs only one at a time: flipping to run creates it, flipping back
  to prog calls `lsim_delete`. The bridge holds one optional handle and
  rejects a second run request (e.g. from another browser tab).
* Because a long-lived Python process repeatedly creates and deletes
  simulators, `lsim_delete` must free everything. Check with a
  create/delete loop under valgrind when the front-end is written.
* A throwaway shim loaded with `ctypes` created a simulator and returned
  command error status to Python without ending the process. Most of the
  test commands errored because the command syntax was guessed wrong, so a
  fully successful circuit run through ctypes is still unverified (it is
  plain C calls, so low risk).

## Notes for implementers

* Only about 15 real `printf` event sites exist (`lsim_dev.c`, `led`, `clk`,
  `mem`, `swtch`, `nand`, `vcc`, `gnd`, `probe`). A grep for `printf` over-counts
  because it also matches `err_asprintf` in the composite devices. The `w;`
  watch output site was not located when `lsim_out.h` was drafted, so the
  API may be missing an event for it; check while converting.
* `lsim_out.h` is a draft written from a read of the existing `printf`
  calls, not from compiling against lsim. Expect adjustments.

## ldraw side (to do)

* Simulation mode: per-LED on/off state keyed by device name, rendered on the
  drawing; switches clickable.
* Controls: step ticklet, step, run/pause with a speed setting (the browser
  or bridge paces by sending `t;1;`).
* Prog/run mode switch, with the drawing read-only in run mode.
* Must remain compatible with `.lsim` export; see `../ldraw/CLAUDE.md`.

## Next steps

0. Decided: generic `dev_o0` callback for single-output devices (nand, swtch,
   vcc, gnd) is fine.
1. Steven reviews `lsim_out.h` and edits lsim to use it (add the
   `lsim_out_t *` to `lsim_t`; add a `step_end` call site in `lsim_dev.c`).
2. Claude writes the library front-end and `lsim_out` queue implementation,
   adds a `bld.sh` target for `liblsim.so`, and a test.
3. Claude writes `bridge.py` and the ldraw simulation mode.
4. Update ldraw's README and CLAUDE.md for the required bridge and the end of
   the `file://` single-file rule.

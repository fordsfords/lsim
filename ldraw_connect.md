# Connecting lsim to ldraw (live, interactive)

Status: **design decided; no lsim code changed yet.** The previous version of
this document (all the options considered, with the author's replies) is in
git history: `git show ef2d642:ldraw_connect.md`.

## Goal

ldraw (browser schematic editor, `../ldraw`) drives and displays a running
lsim simulation: click a switch, step a ticklet or a step, and see LEDs
change. Used mainly for low-level visual debugging and demos, so the unit of
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
* **ldraw need not run from `file://`.** The bridge may serve `ldraw.html`
  from `http://127.0.0.1:PORT`. Bind to 127.0.0.1 only, because `i;` lets a
  command read files.
* Flushing is only for the ldraw front-end. `lsim_main` and `lsim_test` keep
  full buffering for speed.

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
* Initial state: on request the front-end emits every LED's current state, so
  ldraw can start mid-run and detect a stale export by comparing LED names to
  the drawing (`<panel>.led.<i>` for panel LEDs; names equal the drawing's).
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
  instances can coexist (except `global_error_reaction`, shared by all).
* A throwaway shim loaded with `ctypes` created a simulator, ran commands and
  returned error status to Python without ending the process.

## ldraw side (to do)

* Simulation mode: per-LED on/off state keyed by device name, rendered on the
  drawing; switches clickable.
* Controls: step ticklet, step, run/pause with a speed setting (the browser
  or bridge paces by sending `t;1;`).
* Stale-export detection (names in the drawing vs. LEDs reported by lsim).
* Must remain compatible with `.lsim` export; see `../ldraw/CLAUDE.md`.

## Next steps

1. Steven reviews `lsim_out.h` and edits lsim to use it (add the
   `lsim_out_t *` to `lsim_t`; add a `step_end` call site in `lsim_dev.c`).
2. Claude writes the library front-end and `lsim_out` queue implementation,
   adds a `bld.sh` target for `liblsim.so`, and a test.
3. Claude writes `bridge.py` and the ldraw simulation mode.
4. Verify early that Chrome connects to the bridge as planned (WebSocket from
   a page served by the bridge itself avoids `file://`/CORS questions).

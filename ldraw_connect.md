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

## Design review (Claude, 2026-10-31)

**Method.** I read this document, `lsim_out.h`, the lsim engine and device
code, and ldraw's export code. I then linked a small driver against the
unmodified lsim sources, in a scratch directory, to model the planned
front-end: one `lsim_cmd_line` per line, with errors reported and execution
continuing. "Verified" means I ran it that way. Nothing was tested in a browser
or through ctypes, and no lsim code was changed.

**Summary.** The overall shape works, but the design has errors that need
fixing before anyone builds it. The most serious are that an `i;` line can kill
the host process (E1), that an engine error can leave the simulator permanently
broken (E2), and that binding to 127.0.0.1 does not keep other web pages out
(E5). It can also be much smaller: S1, S2, S3 and S5 together remove the
WebSocket, the shared library, the event queue, the snapshot and every edit to
the existing lsim modules that this feature currently requires. Those
simplifications reverse decisions you made, so they are your call; see
"Decisions for Steven".

### Errors

**E1. `i;` reaches `lsim_cmd_file`, which can kill the process.** This document
says the library never calls `lsim_cmd_file`, but `lsim_cmd_line("i;...")` does
(`lsim_cmd.c:705`). Verified: including a file containing one bad line ended the
driver with `exit(1)`. Setting `global_error_reaction` before `lsim_create` does
not help, because `lsim_create` overwrites it with the config default of 1
(`lsim.c:47`). Fix: reject `i;` in the front-end, and set the global to 2 after
`lsim_create`.

**E2. "The simulator stays alive" is not true after an engine error.** An error
in `run_logic` returns with the changed lists partly processed. Verified, with
one floating LED and eight inverters, changing only the LED's name (hash order):

* 9 of 15 names wedged the simulator. Every later `m;` failed with
  `LSIM_ERR_INTERNAL: lsim->out_changed_list == NULL`, and `p;` failed too.
* The other 6 kept accepting commands on a half-initialized circuit, which is
  worse for a visual debugger. In a similar circuit, the next `m;` reported LED
  changes left over from the failed `p;`.
* After `LSIM_ERR_MAXLOOPS`, `m;` still works, but `p;` fails with
  `LSIM_ERR_INTERNAL`, so `p;` is not a way to recover.
* A floating input may not show up at power-on. The NAND loop stops at the first
  0 input (`lsim_devs_nand.c:79-88`), so a later input's float appears only when
  the earlier inputs go high, mid-run.

Fix: treat any error from `p;`, `m;` or `t;` as the end of the run: show the
message and return to prog mode. Also refuse to enter run mode while
`generateLsim` reports unconnected pins, wires with floating ends or unresolved
nets. Panels count: every panel LED input must be driven, or `p;` errors.

***Comment***: I agree to refusing to allow run mode for generateLsim errors. Only allow run for clean generations.

**E3. Nothing sends `p;`.** ldraw's export does not emit it, and `seq1.lsim` has
none. Without it, `m;` changes the switch and returns without running the engine
(`lsim_dev.c:159`), so nothing lights and no error appears. Fix: state that the
bridge sends `p;` after the last exported line.

***Comment***" I guess I assumed that flipping to run mode would enable a "power on" button that sends the `p;` command interactively. That lets me re-power the circuit as needed during testing. Ldraw should remember if lsim has been powered on before allowing other commands, like step, etc. Does this make sense?

**E4. There is no place for a test bench.** `run_seq1.lsim` shows what a real run
adds to the export: probes, `w;` and `v;`, followed by stimulus. Exported
drawings cannot carry these. ldraw has no probe symbol, and a `mem` in a drawing
can never be loaded because the export has no `l;`, so it always starts at zero.
Fix: either add an optional per-drawing setup text, stored in `.ldraw` and sent
before `p;`, or record this as a known first-version limit. The switch and clock
stimulus is covered by clicking.

***Comment***" I agree that probe would need to be added to ldraw. I imagined that `w;` and `v;` would be triggered by button presses.

**E5. Binding to 127.0.0.1 is not enough.** WebSocket connections are not subject
to the same-origin policy, so any page open in the browser can connect to
`ws://127.0.0.1:PORT`. This is standard browser behavior; I did not test it here.
Such a page could send `i;` to probe files, reading their contents from error
text. It could also crash the bridge. Verified: `l;mm;100000000;1;` (no bounds
check, `lsim_dev.c:222`) and `c;pp;o-99999999;l;i0;` (no negative check,
`lsim_devs_panel.c:32-33`) both segfault. Fix:

* Check `Origin` and `Host` on every request. `Host` also covers DNS rebinding.
* While building, reject `i;`.
* After building, accept only `m;` and `t;` from the browser.

***Comment***: Is this an issue with security? I.e. I might browse to foo.bar.com, and that page might bind to my simulator? An this is an issue dealt with by the bridge, right?

**E6. "Step a ticklet or a step" is one control, not two.** `t;1;` runs exactly
one step (`lsim_dev.c:259`), and `m;` also runs one. lsim cannot advance by a step
without advancing a ticklet, so the two controls would behave identically. Stepping
a *cycle* inside a step is the finer unit that low-level debugging might really
want, but it would need `lsim_dev_engine_run` to be split. Fix: ship one step
control, and decide separately whether cycle stepping is wanted.

***Comment***: The intent is that `m;` moves a switch but does not advance a full ticklet. It just runs one propagation cycle. In contrast, `t;` is supposed to advance a full ticklet which can involve many propagation cycles until the circuit settles (or hits the cycle limit). I may have my vocabulary wrong. Please look closer at `t;` and `m;` and verify they conform to my intent.

**E7. `lsim_delete` does not free everything, as this document requires.**
Verified with AddressSanitizer on `seq1.lsim`: 102,169 bytes in 175 allocations
per create/delete. Causes:

* `hmap_delete` never frees `hmap->table` (`hmap.c:107`). This is most of the
  bytes.
* Each `*_create` drops the `err_t` from its "name not found" lookup, e.g.
  `lsim_devs_led.c:114`.
* From reading the code, `lsim_devs_panel_delete` does not free its terminal
  arrays.

The practical cost is small, at about 100 KB per flip to run mode. S2 removes the
requirement altogether.

***Comment***: Please create a `bugs.md` file with lsim bugs that you've encountered.

**E8. `lsim_out.h` problems.**

* `dev_mem` takes `long`, but `data_val` is `uint64_t` (`lsim_devs_mem.c:124`).
* `step_end` will not fire on an error return, or before power-on
  (`lsim_dev.c:159`, `:181`). Applying buffered LED changes there would lose them
  in exactly the case where the user most wants to see the state. The comment
  "Called whether or not anything changed" is also incomplete.
* The verbose comment is wrong. `dev_o0`, `dev_clk` and `dev_mem` fire on
  `watch_level` as well as the `OUT_CHG` bit (for example `lsim_devs_nand.c:101`).
* The header does not say who applies the verbosity gate for `ticklet_begin`,
  `step_begin` and `cycle_begin`. "Also announced per verbosity" can be read
  either way. Today the engine checks at the call site.
* The `ticklet` argument can be -1 (before the first `t;`), and it returns to -1
  whenever the clock's `R0` is low. The comment should say so.
* Nothing says how the table gets into `lsim_t`. The header's single
  `lsim_out_printf` also conflicts with this document's plan to implement it in
  both mains, which would duplicate the code. See S4.

**E9. This document contradicts itself or the code in smaller ways.**

* `open(config, file)` conflicts with the feed-line-by-line decision. Also, "the
  bridge reads the `.lsim` text itself" conflicts with "sends it over the socket".
  The API should be `open(config)`, and the text comes from the browser.
* "Python needs no extra package" covers only ctypes. WebSocket is not in the
  Python standard library. The `websockets` package happens to be installed on
  this machine, but it is third-party (S1 avoids it).
* The flushing bullet is left over from the pipe design. A library queue has
  nothing to flush.
* The "`w;` site not located" note can go: those sites are the `watch_level` tests
  in nand, swtch, vcc, gnd, clk and mem. "Later: events for watched terminals"
  mostly exists already as `dev_o0`, `dev_clk` and `dev_mem`.
* The event site count is 13, plus the `Trace:` line.
* A returned `err_t` needs more handling than "becomes an error reply". Its
  top-level `mesg` is only `Error processing 'm;s;1;'`, and the cause is at the end
  of the `stacktrace` chain. For `ERR_ASSRT` failures the cause is C source text:
  `t;` with no clock reports `lsim->active_clk_dev`. ldraw should disable step
  controls when the drawing has no clock.
* `q;` only sets a flag, and `lsim_cmd_line` keeps executing later lines (verified).
  The front-end must check the flag itself.

### Simplifications

**S1. Use plain HTTP POST instead of WebSocket.** lsim never initiates anything.
Every browser action is one command with one reply, and the browser paces run mode
by sending `t;1;`, so nothing needs a push channel. Python's standard
`http.server` is enough, with no framing and no third-party package. A new run
simply replaces the old one, which removes the "reject a second tab" rule and the
closed-tab cleanup this document does not cover. Include a run id so a stale tab
gets an error instead of driving the new simulator.

**S2. Use a real third main with a pipe, instead of a shared library and ctypes.**
`lsim_ldraw_main.c` would read command lines from stdin and write one JSON reply
per command to stdout, flushing after each. The bridge starts one process per run
and kills it on the flip back to prog. This is close to your original "new main"
idea. The earlier objections to a pipe (parsing human text, buffering) do not
apply, because this main writes structured output and flushes only itself.

* Gone: the `-fPIC` target, `.dll` question, ctypes `argtypes`/`restype` setup
  (ctypes truncates returned pointers by default), and the need for `lsim_delete`
  to free everything (E7).
* Contained: a segfault, `exit` or `abort` kills only the child, not the bridge
  (E1, E5).
* Easier to test: `tst.sh` can pipe commands in and compare replies with no Python.
* Flushing: the new main must call `fflush(stdout)` after each reply. stdout is
  fully buffered on a pipe, so otherwise the bridge and the child wait on each other
  forever. `lsim_main` and `lsim_test` keep their buffering.
* Shared stdout: the existing `printf`s (every LED change, plus warnings and verbose
  text) go to the same stream as the replies. The bridge reads lines until one starts
  with `{`, which is the reply. No engine text starts with `{`, and the shared buffer
  keeps the order. The other lines can be forwarded to the browser as log text.
* Cost: the bridge must handle the child dying, which can lose that command's buffered
  text. It should leave the child's stderr connected to the terminal, because an
  unread stderr pipe could block the child.

**S3. Read LED state instead of emitting events.** After each command, the
front-end reads `dev->led.illuminated` for each LED, and uses
`changes_in_step > 1` (when `dev->led.cur_step` matches `lsim->cur_step`) as the
glitch flag. It can read `lsim->total_warnings` the same way. That needs no edits
to existing modules, and no `lsim_out.h`, `step_end`, queue or `poll_events`. Of
the 9 callbacks in the draft, ldraw's first version consumes only `led`: ldraw has
no probes, and no UI that sends `v;` or `w;`.

***Comment***: The frontend does not have the ability to read data structures inside of lsim. I guess you would implmenet a query/response, but that would be inefficient, with the frontend having to query every LED after every step. I think the LED changes need to be rolled up and provided in the response to the current command. Either that or go back to web sockets and let lsim send events. Am I misunderstanding your proposal?

It also suits the planned wire coloring better. ldraw names each wire by source
device and pin, and `get_out_terminal(dev, pin)` resolves that for composites too
(the same call `lsim_dev_connect` uses; from reading the code, not run). An event
stream reports internal names such as `ff1.nand3`, which the drawing does not know.

What this loses: the order of transitions within a step, intermediate states of
`t;N;` when N is above 1 (the browser sends `t;1;`), and structure in verbose and
warning output. With the pipe (S2), that output reaches the bridge as plain text, so
it can be shown in a log but not interpreted. If you later want a structured trace
view in the browser, `lsim_out.h` can be added then, with a real consumer to design
against.

**S4. If `lsim_out.h` stays, make it smaller.**

* Convert only `led` and `warning` now (4 of the 13 sites). The other sites can
  keep `printf` until something consumes them.
* Drop `step_end`. The viewer applies changes when the command's reply arrives,
  which also fixes the error case in E8.
* Drop the `out` parameter, since `lsim->out` is already available.
* Make callbacks non-NULL, with no-ops where unused, so each site needs no extra
  `if`.
* Have `lsim_create` default to `&lsim_out_printf`, defined once in
  `lsim_out_printf.c`. The 13 existing `lsim_create` calls and both mains then stay
  as they are, and the front-end overrides the pointer after creating.
* Return events from `poll_events` as one JSON string, not fixed-size records that
  Python must mirror.

**S5. Drop the initial snapshot.** This applies whichever output design you pick.
LEDs start off, and the first `p;` reports every LED that settles on (verified), so
"clear all, then apply replies" gives the correct state. That removes the LED walk,
`lsim_dev_foreach_led()`, and the double use of the `led` callback. With events, a
second `p;` could leave a stale LED, because `lsim_devs_led_power` clears
`illuminated` without reporting it (`lsim_devs_led.c:52`), and `p;` cannot recover
from errors anyway (E2). So make "Reset" a rebuild, and never send a second `p;`.

**S6. Keep prog mode working without the bridge.** The bridge serves the page for
run mode, but nothing requires it for editing. If `location.protocol` is `file:`,
disable Run with a short message. That keeps ldraw's "open the file and draw"
workflow and avoids rewriting its docs around a required server, at the cost of
one check. Run mode should still require the bridge-served page, because a `file://`
page sends `Origin: null`, which the check in E5 cannot trust.

***Comment***: OK.

**S7. Drop the stale-library check and auto-build.** lsim's `bld.sh` uses relative
paths, deletes the binaries before compiling, and rewrites README tables of
contents, so it is a poor thing for another repo's start script to run implicitly.
With one user, having the start script stop with "run `./bld.sh` in `../lsim`" when
the binary is missing is enough, and the version check catches API mismatches. The
`.dll` target can also go unless you plan to run the bridge outside WSL.

### Resulting design, if S1, S2, S3 and S5 are adopted

```
ldraw.html  --HTTP POST-->  bridge.py  --stdin/stdout-->  lsim_ldraw
 (browser)                  (stdlib)    (one per run)     (new main)
```

```
> t;1;
< {"ok":true,"step":7,"ticklet":6,"warnings":0,"leds":{"led1":[1,0],"led2a":[0,0]}}
```

Each LED entry is `[state, glitch]`. An error reply carries the root cause message
and ends the run. Claude writes the new main, the bridge and the ldraw side, as
already agreed. The feature no longer depends on any edit to existing lsim modules,
so your bug fixes (below and E2) can be done on your own schedule.

### What I checked and found sound

* Rebuilding on every flip to run, a read-only drawing in run mode, the drawing as
  the source of truth, and the browser pacing with `t;1;`.
* Name mapping. Panel switches are reachable by `m;<panel>.swtch.<i>;` (verified),
  and panel LEDs report as `<panel>.led.<i>`. Run-mode switch state must stay out
  of `state.devices`, so that it does not touch undo or the dirty flag.
* The shared-library facts, if you keep that design. The sources build with
  `-fPIC -shared` under the project's flags, and `global_error_reaction` is the only
  undefined lsim symbol (verified).

Still untested, and worth trying early: whether Chrome on Windows reaches a bridge
bound to 127.0.0.1 inside WSL2, and how the File System Access pickers behave from
an `http://127.0.0.1` origin. If WSL2 forwarding fails, binding to `0.0.0.0` is not
an acceptable fix.

### Decisions for Steven

1. **Pipe or library (S2)?** I recommend the pipe. It contains crashes and removes
   the most machinery. This reverses "no stdin/stdout pipe".
2. **Read state or emit events (S3)?** I recommend reading state for the first
   version. This reverses "output goes through an API", which you proposed so that
   events could reach ldraw. If you want the API for its own sake, use S4.
3. **What does stepping mean (E6)?** I recommend one ticklet control for now. Cycle
   stepping is a separate engine change that only you should decide on.
4. **Test bench (E4)?** I recommend recording it as a first-version limit, then
   adding setup text when you first need a loaded `mem` or a probe in run mode.
5. **Editor without the bridge (S6)?** I recommend keeping it. This reverses "no
   editor-only mode".

S1, S5 and S7 change no decision about what the feature does, so I would simply
adopt them. E1, E2, E3, E5 and E9 apply under every option.

### Bugs noticed in passing (lsim, not part of this design)

* `lsim_cmd.c:731`: `watch_level >= 0 || watch_level <= 2` is always true. It
  should use `&&`.
* `lsim_devs_mem.c:134`: `1 << in_index` is an `int` shift, so it breaks for data
  widths above 31 bits, even though `data_val` is 64 bits.
* `lsim_dev_loadmem` does not bounds-check its address, and terminal ids accept
  negative numbers (E5). These matter beyond this feature because bad input
  segfaults instead of reporting an error.

## Follow-up discussion and WASM test (2026-11-01)

This section records how the design moved from the review above to WebAssembly
(WASM), and why. It supersedes the review's recommendations where they conflict.

### Steven's preferences

1. Minimizing changes to the lsim core is a benefit, not a requirement. A library
   should not call `exit` or `abort`.
2. Separate processes are not a requirement. One process is simpler than two or
   three. If the commands leading to a crash are recorded, they can probably be
   replayed in standalone lsim and debugged there, and lsim being single-threaded
   makes that more reliable.
3. A new toolchain is a couple of installs, and a stale build is a workflow fault.
   Rebuild everything every time; add `make` only if the build gets annoying.
4. Cycle-by-cycle viewing belongs in the first version, though the first version
   can be built in phases.
5. Steven may hand Claude some of the small or mechanical lsim changes.

### Options reconsidered

**The library never had a real comparison.** The original pros and cons covered
replay, tailing a file, a bridge with pipes, and a server in C. The ctypes library
was added afterward, with no row in the comparison table, even though Steven asked
for skepticism. Its one recorded argument, "ctypes needs no build step", compares
Python with Node rather than a library with a pipe.

**WASM was rejected for reasons that no longer hold.** The first reason was
"separate processes", which Steven has now ruled out as a requirement. The second
was "ties ldraw to lsim's build", which the design had already accepted.

**Node.js is not an alternative to the browser.** Node has no display, so ldraw
still needs a browser and no rewrite would help. As a bridge, it would have no
advantage over Python. Electron would be heavy and would give up "open it in any
browser". A native addon would keep the ctypes crash risk. Node is useful only as a
development tool: Emscripten uses it, and tests can use it to run the WASM build
without a browser. It is bundled in `emsdk` and not needed at run time.

### Decisions and reasons

* **lsim runs inside the page as WASM. There is no bridge.** This removes Python,
  HTTP, `Origin` checks, WSL2 networking and child process management. It also keeps
  ldraw a single file, which was the original reason for choosing a browser.
* **Each Run creates a fresh lsim instance, and leaving run mode discards it.** That
  gives a clean state and makes the `lsim_delete` leaks irrelevant to this feature.
* **Read state rather than adding an event API.** After each command the front-end
  reads LED state and returns it in the reply. `lsim_out.h` is not needed, since
  `printf` text already reaches a JavaScript callback, separate from the reply.
* **Cycle stepping comes from splitting the engine loop, not from events.** The
  in-changed and out-changed lists already hold everything between cycles, so one
  pass of the `lsim_dev_engine_run` loop can become its own function. The page then
  reads true state after each cycle, which also suits wire coloring. This is from
  reading the code; it has not been prototyped.
* **The command log is always on.** See below.
* **`lsim_cmd_file` should stop and return errors, and only the mains exit.**
  `lsim_main.c:121` already has the abort/exit switch. An "abort on error" flag
  (below) covers the shallower core dump this would otherwise cause.
* **Steven's comments on the review stand:** run mode requires a clean generation;
  a Power button sends `p;` and the other controls stay disabled until then;
  `w;` and `v;` are buttons; a probe symbol gets added to ldraw.

### Command log

A button to save a log would be too late, and it would be clutter once the bugs are
shaken out. So the log should simply always exist, overwritten at each Run and
growing during it. A page opened from `file://` has no startup flags and cannot write
a file without a picker, so the plan is:

* Each command is saved to browser storage **before** it is passed to lsim, so the
  log includes the command that crashed.
* A storage failure, such as a full quota, must not stop the simulation.
* The log is valid lsim input: the exported circuit followed by the commands sent.
  After a problem, one menu item saves it, and `lsim_main` can then replay it under
  gdb or AddressSanitizer.

The save step only happens after a problem, and nothing has to be pressed beforehand.

### "Abort on error" flag

Steven proposed a global flag, possibly set by a command, for difficult problems, so
that a core dump keeps local variables and the original stack. This is a small change
and useful with log replay, with one catch: **thrown errors are also normal control
flow.** Every device `_create` expects `hmap_slookup` to throw `HMAP_ERR_NOTFOUND`
(for example `lsim_devs_led.c:114`), and that goes through `err_throw_v`. A flag
checked there would abort on the first device defined, so not-found has to be exempt,
or those lookups need a non-throwing form.

### What happens when lsim seg faults in WASM

The page survives. Results of deliberate faults, using Node's V8 engine:

| Fault | Native | WASM release build | WASM debug build (`-sSAFE_HEAP`) |
|---|---|---|---|
| Wild read, or `l;mm;100000000;1;` | Segfault | Catchable `RuntimeError` | `RuntimeError` |
| `c;pp;o-99999999;l;i0;` | Segfault | Catchable `RuntimeError` | `RuntimeError` |
| `l;mm;1000;1;` (nearby overrun) | No crash (corrupts silently) | **No error (corrupts silently)** | `RuntimeError` |
| NULL read or write | Segfault (standard; not run) | **No error** | `RuntimeError` |
| `abort()` | Core dump | `RuntimeError` | `RuntimeError` |
| `exit(1)` | Process ends | `ExitStatus` exception | `ExitStatus` exception |

What this means:

* There is no core dump, and no process dies. JavaScript catches the exception at the
  call into lsim.
* **The instance does not stop working after a fault,** so later commands appear to
  succeed on possibly damaged state. The front-end must treat any exception as the end
  of the run and discard the instance. A new instance worked in every case.
* **Address 0 is ordinary memory in WASM,** so a NULL dereference does nothing visible
  in a release build. This is the main debugging loss, and `SAFE_HEAP` covers it. Use
  that build until the bugs are shaken out.
* AddressSanitizer also works under Emscripten and caught the nearby overrun.

### Test build results

Nothing in the lsim sources was changed. The build used Emscripten 6.0.13 with a
throwaway wrapper in a scratch directory.

* **`lsim_test`:** tests 0-10 passed with byte-identical output between native and
  WASM. This ran on a copy of the sources with both binaries run directly, not through
  `tst.sh`, because `tst.sh` runs `bld.sh` in the repo.
* **Circuits:** `ctr`, `seq1`, `seq2`, `sr`, `srlatch` and `run_seq1` (with `i;`
  expanded), plus `seq1` with verbosity 15, power, stimulus and a re-power, all gave
  identical output.
* **Size and speed:** 158 KB embedded as a single JavaScript file. A rebuild takes about
  4 seconds, compared with about 2 for one native target. 100,000 ticklets of `seq1` took
  0.87 s including Node startup, against 0.17 s native.
* **Browser:** a self-checking page passed in Node with a stub DOM. **It has not been run
  in Chrome,** so loading from `file://` and browser storage there are still unverified.

**Differences found.** `long` is 32 bits in WASM. Only `mem` showed a difference:

* `mem` printing is wrong in WASM: `o=0 (0x5)` where native prints `o=5 (0x5)`, because
  `lsim_devs_mem.c:161` uses `%ld` for a `uint64_t`. The fix is `PRIu64`.
* `l;` rejects values of 2^31 and above in WASM, because it parses into a `long`.
* `mem` is already broken natively above 31 data bits (see `bugs.md`), so this costs no
  working behavior today.

**Also found:** `p;` zeroes `mem` contents (`lsim_devs_mem.c:97-101`), so `l;` must come
after `p;`, and each re-power needs a reload. That affects the Power button.

### Review items this settles

* **No longer apply:** E1, E5 and E7, and S1, S2, S3's pipe text, S5, S6 and S7.
* **Still apply:** E2 (end the run on an error), E4 (`mem` loading), E6 (one step command,
  now with cycle stepping added) and the `q;` and error-message points in E9.
* **E3** is replaced by the Power button.

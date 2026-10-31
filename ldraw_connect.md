# Connecting lsim to ldraw (live LED display)

Status: design thoughts only. No code has been changed anywhere.

Goal: while lsim runs a simulation, LEDs on the ldraw drawing light up and go
dark in real time. lsim and ldraw stay separate processes, and lsim events
flow to ldraw.

**Verdict: feasible, and not hard.**
The only real obstacle is that a web page in a browser cannot read another
process's stdout or open a pipe. Something has to bridge the gap. Every option
below is a different choice of bridge.

## What exists today (relevant facts)

### lsim

* An LED reports itself when its input changes, in `lsim_devs_led.c`
  (`lsim_devs_led_run_logic`):
  `Led <name>: on|off (ticklet N)`, with ` glitch` appended if it changed
  more than once in the same step.
  This is printed regardless of verbosity, to stdout, via plain `printf`.
* stdout is **fully buffered when it is a pipe or file**. Today nothing flushes
  it, so a consumer would see nothing until the buffer fills or lsim exits.
* lsim has no notion of wall-clock time. `t;N;` runs N ticklets as fast as the
  CPU allows, so a whole `.lsim` script finishes in milliseconds. A human can
  only watch "real time" if something *paces* the run.
* lsim reads commands line by line from a file or from stdin (`-`), so it
  can already be driven interactively: `m;sw;1;` and `t;1;` can be fed to it
  while it runs.
* LEDs inside a `panel` are named `<panel>.led.<i>` (`lsim_devs_panel.c`).
  Standalone LEDs use the name given in the drawing.
* LEDs print only on a *change*. Nothing announces the initial state, so a
  viewer that joins late knows nothing until each LED next changes.
* Step boundaries are visible only in verbose output (`v;` bits), and that is
  human-oriented text.

### ldraw

* A single self-contained HTML file opened from `file://` in Chrome. No server,
  no dependencies. Its README and `CLAUDE.md` treat this as a feature.
* It exports `.lsim` with the **same device names as the drawing**, so the
  mapping drawing-LED to simulator-LED is just the name. Panel LEDs map
  as `<panel>.led.<i>`. `netsource`/`netsink` pseudo-devices are not in lsim
  and don't matter for LEDs.
* It already uses the File System Access API (open/save pickers).

## Events and protocol (common to all options)

Whatever the transport, these requirements apply:

1. **Machine-readable events.** Parsing `Led x: on (ticklet 3)` works but is
   fragile, and it shares stdout with human trace text. Better to have a
   dedicated, stable event stream: e.g. a command-line option (`-e file`) or a
   verbosity bit that emits lines such as `LED <name> <0|1> <step>`.
2. **Step markers.** lsim's logic can glitch inside one step (the LED code
   already detects it). The display should apply changes atomically at the end
   of each step so it shows settled values, not transient ones. That needs a
   `STEP <n>` (end-of-step) line in the stream.

***Comment***: Disagree. The ldraw connection will be used mostly for low-level visual debugging and demos. I see it as more of a single-step operation, either at the ticklet or the step basis. This will reduce choices since a two-way communication has to happen so that ldraw can control operation of lsim. I see a new "main" program that has an output API. All the places that currently printf will call that API. For today's main programs, that new API will just call printf. In this new one, it will do whatever so that the event is delivered to ldraw. It might make sense to design the API to accept structured data so that the API isn't just pushing the printf string to ldraw but is instead pushing something easier to interpret. The API might have different functions for the different output forms, like one specifically for LED state changes, another for clock state changes, etc. The API is designed to know what to do (just printf for today's mains). However, I don't know how to do the communication to ldraw.

3. **Initial snapshot.** At power-on (and on request) emit every LED's current
   state, so a viewer can start mid-run. Emitting the *list* of LEDs also
   lets ldraw detect "the drawing no longer matches the `.lsim` that is
   running" (stale export) by comparing names.
4. **Flush.** Line-buffer (or `fflush`) after each step marker.

***Comment***: Flush, if needed, only for the ldraw main. Current mains should lean on buffering so that it can run faster.

5. **Pacing.** Choose who sets the speed (see "Who drives time" below).
6. **Optional, later:** the same stream could carry `w;`-watched terminals or
   net states so ldraw can color *wires* by value, not just LEDs. Design the
   line format so extra record types are easy to add.

## Both programs change

Every option needs changes on both sides; the options differ in how much.

**lsim (needed by all options)**
* A dedicated event stream (LED changes, step-end markers, initial state /
  device list), selectable by option or verbosity bit, separate from the
  human-readable output.
* Flush (line-buffer) the stream so a reader sees events promptly.
* Optional: pacing option (Option B only), watched-terminal records (wire
  coloring, later).

**ldraw (needed by all options)**
* A "simulation" mode that holds a per-LED on/off state keyed by device name
  (`<panel>.led.<i>` for panel bits) and renders LEDs accordingly.
* An event source: log player (A), file poller (B), or network connection
  (C), plus stale-drawing detection by comparing LED names.
* For C only: send commands (`m;`, `t;`) back, with switch clicks, run/pause/
  step and a speed control in the UI.

***Comment***: C.

**New, for C only:** the bridge script.

## Option A - Replay: lsim writes an event log, ldraw plays it back

lsim runs on its own and writes the event stream to a file. In ldraw you open
the log (file picker) and "play" it at an adjustable speed, with pause, step
and scrub.

* **Pros**
  * No network, no bridge process, nothing to keep running.
  * Fits the existing "single HTML file, no server" rule.
  * Pacing is entirely ldraw's problem, so lsim stays timeless.
  * Deterministic and repeatable. A log can be rewound, stepped back and
    compared; a live view can't do that.
  * Smallest change to lsim among the working options (just the event stream and flush).
  * Easy to test.
* **Cons**
  * Not live. You can't flip a switch in ldraw and see the circuit react.
  * Long runs make big logs (mitigate: only changes, only LEDs).
* **Verdict:** best first step, and valuable even if live mode comes later.
  It would settle the event format and the name mapping.

## Option B - Tail a growing log file from the browser (pseudo-live)

Same event file as A, but ldraw keeps the `FileSystemFileHandle` and re-reads
the file about every 50-100 ms (`handle.getFile()` returns a fresh snapshot),
applying new lines as they appear.

* **Pros**
  * Looks live, with no bridge or network, still a single HTML file.
  * The same format as A, so it is almost free once A exists.
* **Cons**
  * One-way only. No clicking a switch in ldraw.
  * Polling, so ~100 ms latency, with the user picking the file each session
    (Chrome permission prompt).
  * Someone must pace lsim (a sleep per ticklet in lsim, or a wrapper
    script). Otherwise the display just fast-forwards to the end.
  * File growth / truncation on a new run needs handling.
* **Verdict:** cheap, and good enough to watch a clock-driven circuit. Not a
  path to interactive control.

## Option C - Bridge process (lsim stdin/stdout <-> browser)

A tiny separate program (Python stdlib or Node, no third-party deps) starts
lsim with pipes. It forwards lsim's event lines to ldraw and forwards commands
from ldraw (`m;sw;1;`, `t;1;`) to lsim's stdin. Browser transport choices:

* **Server-Sent Events + HTTP POST** (simplest in stdlib): events stream down
  over SSE, commands go up as `fetch` POSTs.
* **WebSocket** (nicer, bidirectional on one connection): the handshake and
  framing are roughly 50 lines in Python without a library, or trivial in Node
  with `ws`.

Two sub-variants:

* **C1. Bridge also serves ldraw.html** over `http://localhost:PORT`.
  Same origin, so no CORS question; localhost is a secure context, so the File
  System Access API still works. Loses "just open the file".
* **C2. ldraw.html stays on `file://`** and connects to the bridge.
  Needs the bridge to send CORS headers (SSE/fetch) or use WebSocket (not
  subject to CORS). Chrome's treatment of `file://` pages reaching `localhost`
  (Private Network Access) should be checked experimentally before betting on
  it.

* **Pros**
  * Truly live and bidirectional: click a switch in ldraw, see LEDs react;
    ldraw can start/stop/step the run.
  * The bridge gives pacing for free: it (or ldraw) sends `t;1;` at whatever
    rate the user picks. lsim itself never needs a sleep or a "run" mode.
  * lsim stays a plain stdin/stdout program. The only needed lsim changes are
    the event stream and flushing.
  * Late-joining viewers: the bridge can cache the last LED states.
  * Keeps lsim and ldraw independent processes, as wanted.
* **Cons**
  * A third component to write, launch and keep running.
  * Browser-to-localhost plumbing (CORS / file origin / port choice) is the
    one place surprises are likely.
  * Process management: lsim crashes, quit (`q;`), restarts after the drawing
    is re-exported.
  * Security hygiene: bind to 127.0.0.1 only. Since the bridge forwards
    commands into a process that can read files (`i;`), don't expose it to the
    network.
* **Verdict:** best end state if interactive simulation is wanted.

***Comment***: Hmm ... what about the third "main" not being a main at all? But instead a callable interface to Python or Node. This mode basically leverages Python's or Node's extensive library for websockets, etc and calls into lsim's C interface (presented by the third "main") to step the simulator. Outputs could either be an upcall to Python, or just a return value as an array of output events, or concatonated strings of output events, or whatever. Be skeptical of this design idea since I don't have a firm grasp of how Python/C or Node/C interface works. I don't know enough about Node to have a preference in terms of the technology.

## Option D - Build the server into lsim

lsim itself listens on a TCP port (HTTP/SSE or WebSocket) and serves events
and accepts commands directly. No bridge.

* **Pros**
  * One fewer process.
* **Cons**
  * Pulls networking, and for WebSocket also SHA-1/base64/framing, into a
    deliberately tiny, portable, dependency-free C simulator (which also
    tries to stay Windows-friendly).
  * Threads or non-blocking I/O are needed, because lsim currently blocks in
    `fgets` on stdin. That is a significant architectural change.
  * Harder to test, and it works against the project's hobby-scale simplicity.
* **Verdict:** not recommended. Option C gets the same result with the
  complexity outside lsim.

## Option C-prime - Second `main` in C (refactor lsim, no bridge)

Refactor lsim so the engine is a pure library with two front-ends: the
existing keyboard/stdin `main`, and a second `main` (e.g. `lsim_ldraw_main.c`)
that serves ldraw directly. This is Option D done in a disciplined way.

**What already helps**
* `lsim_main.c` is ~135 lines (options, `lsim_create`, `lsim_cmd_file`,
  `lsim_delete`), and `lsim_test.c` is already a second `main` built from the
  same sources.
* `lsim_cmd_line()` is already public (`lsim_cmd.h`), so a second main can
  feed the engine one command at a time.

**The valuable part: an event sink instead of `printf`**
* About 40 `printf` calls are spread across `lsim_dev.c` and the
  `lsim_devs_*.c` files (LED changes, watch output, step/ticklet traces).
  That is why LED events can't be separated from human-readable text.
* Give `lsim_t` a small emit callback (e.g. `lsim->emit(lsim, event)`). Each
  main installs its own: text for the keyboard main, structured events for
  the ldraw main, a capture buffer for `lsim_test`.
* This is the same event-stream change every other option needs, so it is
  worth doing first and independently.

**What the second main must handle**
* A single-threaded event loop (`poll`/`select` over the client socket, stdin
  and a pacing timer), because `lsim_cmd_file` blocks in `fgets`.
* Errors: `lsim_cmd_file` applies `global_error_reaction` (may `exit(1)` or
  abort). A server main must call `lsim_cmd_line` itself and send the error to
  ldraw instead of dying.
* Transport code in C with no dependencies. SSE plus HTTP POST is much less
  code than WebSocket (which needs the HTTP upgrade, SHA-1, base64 and frame
  masking, roughly 300 lines). A Windows variant is needed since sockets
  differ.
* Bind to 127.0.0.1 only, since it accepts commands that can read files (`i;`).

* **Pros**
  * No third component; one self-contained program per use.
  * The refactor makes lsim cleaner and more testable regardless.
  * Bidirectional and live, like Option C.
* **Cons**
  * Network code in a deliberately small, portable C program.
  * The same browser-to-localhost questions as Option C still apply.
  * More lsim work (which you would write) than a bridge script.
* **Verdict:** reasonable end state if you prefer all-C. Do the event-sink
  refactor first; it keeps both this and Option C open, and the choice
  between them can be made afterwards.

## Options that don't fit

* **lsim compiled to WebAssembly and run inside ldraw.** Works well
  technically and needs no bridge, but it is not "separate processes", which
  was a stated requirement. It would also tie ldraw to lsim's build.
* **Chrome extension with native messaging.** Can launch a native process
  from the page, but it is heavy to install and maintain for a hobby tool.
* **Web Serial / WebUSB / WebRTC / FIFO or Unix socket read directly by the
  page.** A page can't open a FIFO or Unix socket, and the others are the
  wrong tool.

## Who drives time (applies to B and C)

| Pacing owner | How | Notes |
|---|---|---|
| ldraw / bridge sends `t;1;` at N Hz | lsim unchanged | Preferred. Speed slider, pause and single-step all fall out naturally. |
| lsim sleeps per ticklet | new option in lsim (`MY_SLEEP_MS` already exists in `lsim_main.c`, unused) | Works for B (no back-channel), but then lsim has a notion of real time and the sleep is not interruptible. |
| ldraw replays a log (A) | none | Pacing and scrubbing live entirely in the viewer. |

A circuit with a free-running clock is just `t;1;` over and over, so the
"clock speed" is simply the pacing rate.

## Comparison

| | A Replay | B Tail file | C Bridge | C' 2nd main (in C) / D |
|---|---|---|---|---|
| Live | no | ~yes (100 ms poll) | yes | yes |
| Interactive (switches, step) | no | no | yes | yes |
| Extra process | none | none (or a pacing wrapper) | one small script | none |
| Keeps ldraw single-file `file://` | yes | yes | C2 yes, C1 no | maybe |
| lsim changes | event stream, flush | same + pacing | same | large |
| ldraw changes | log player | + poll loop | + connection, send cmds | same as C |
| Risk | low | low | medium (browser plumbing) | high |

## Suggested path

1. **Define the event stream in lsim** (small, shared by all options): LED
   changes, step-end markers, initial state, flush. Keep it separate from the
   human-readable output. Best done as an event-sink callback replacing the
   scattered `printf`s (see Option C-prime).
2. **ldraw: replay (Option A).** Open a log, animate LEDs by name, step and
   speed controls. This validates name mapping (including `panel.led.N`) and
   stale-export detection.
3. **Bridge (Option C) or second main (C-prime)**, if interactive control is wanted. Start with SSE + POST
   (stdlib only) and verify `file://` to `localhost` in Chrome early. If that
   is flaky, serve ldraw.html from the bridge (C1).
4. **Later:** color wires by value (needs lsim to emit watched terminal
   states, plus a way to map ldraw wires to lsim terminals).

## Open questions for you

* Is interactive simulation (clicking ldraw switches) a goal, or is
  watching a scripted run enough? That decides between A/B and C.

***Comment***: Yes, interactive.

* Is "ldraw must open from `file://`" a hard requirement, or acceptable to
  relax when connected to a live simulation (C1)?

***Comment***: 'file://' is not a hard requirement.

* Where should a bridge script live (lsim or ldraw repo), and what language
  (Python is probably already on the machine; Node would need installing)?

***Comment***: I don't have an opinion. If Node would help you, I'm OK with it.

* Ownership: lsim is yours to author, ldraw is maintained by Claude Code
  going forward. Event-stream changes in lsim would be small, and I'd
  suggest you write those yourself and let Claude handle the ldraw and
  bridge sides.

***Comment***: I would want to modify the existing lsim modules. You would write and maintain the third "main". When we get to low-level design, you'll create a .h file for the APIs that lsim needs to call instead of calling printf. I'll do the implementation of those APIs in the existing mains and you'll do it for the new main. Regarding Python/Node choice, since you'll be maintaining it, the choice comes down to which helps you meet the requirements better?

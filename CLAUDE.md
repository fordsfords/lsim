# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Ownership

Steven Ford is the sole programmer and maintainer of this project, with a few small exceptions. It is a hobby project, and he wants to remain the primary author. Claude's role is code review, test support, and debugging support. Don't write large amounts of new code or take over design decisions unless asked.

## Commands

- Build: `./bld.sh` — compiles `lsim_test` and `lsim_main` with `gcc -std=c11 -Wall -Wextra -pedantic -Werror`, so any warning fails the build. It also regenerates the README table of contents if `mdtoc.pl` is available.
- Run all tests: `./tst.sh` (it runs `bld.sh` first). Always use this rather than running `./lsim_test` directly.
- Run a single test: `./tst.sh <N>`, which maps to `lsim_test -t N`. Test cases are numbered blocks in `tst.sh`, and each one passes only if its log contains `testN: success`.
- Run a circuit: `./lsim_main [-c config_file] file.lsim` (use `-` for stdin). Example: `./lsim_main sr.lsim`.

There is no linter and there are no external dependencies. The build is a single `gcc` command listing every `.c` file; `bld.sh` has an explicit `DEVS=` list. **A new `lsim_devs_*.c` file must be added to that list.**

## Architecture

lsim is a NAND-gate-centric digital logic simulator written in C11. Circuits are described in a line-based, semicolon-delimited command language (`d;` define device, `c;` connect, `b;` bus connect, `p;` power on, `m;` move switch, `t;` tick the clock, `w;` watch, `i;` include, `v;` verbosity, `q;` quit). See `README.md` for a cheat sheet and `circuit-language-docs.md` for the full spec. Parsing lives in `lsim_cmd.c`.

**Simulation engine** (`lsim_dev.c`, state in `lsim_t` in `lsim.h`)
- A "step" is one run of the logic engine. It loops over two phases until the circuit stabilizes:
  1. `run_logic` on every device with a changed input.
  2. `propagate_outputs` to connected inputs.
- The loop is capped by the config `max_propagate_cycles` (default 50), which catches oscillating circuits.
- Devices with changed inputs or outputs are tracked on the intrusive linked lists `in_changed_list` and `out_changed_list`.
- A "ticklet" is a half clock cycle, driven by the `t;` command. It is not a self-running oscillator, because the `clk` device has no internal timing.

**Devices** (`lsim_devs.h`, `lsim_devs_<type>.c`)
- `lsim_dev_t` is a tagged union plus a function-pointer vtable: `get_out_terminal`, `get_in_terminal`, `power`, `run_logic`, `propagate_outputs`, `delete`. Each `lsim_devs_<type>.c` implements one type and exposes a `_create` function declared in `lsim_devs.h`.
- Adding a device type needs the following:
  - a `LSIM_DEV_TYPE_*` constant, a struct, and a union member in `lsim_devs.h`
  - the `.c` file
  - an entry in the `lsim_cmd.c` define-command dispatch
  - an entry in `bld.sh`
- Primitive devices are `nand`, `gnd`, `vcc`, `swtch`, `led`, `clk`, `mem`, `probe` and `addbit`.
- Composite devices are `srlatch`, `dflipflop`, `reg`, `panel` and `addword`. They are only definition shortcuts: their `_create` instantiates and wires primitive devices, and the simulator has no special-case behavior for them.
- Internal sub-devices are named with a period (e.g. `ff1.nand3`). User-chosen names cannot contain a period, which avoids collisions.
- `mem` and `clk` are deliberate primitive exceptions, kept for simulation speed and for the ticklet design.
- Terminals have an id like `i0`, `o0`, `q0`, `d0`, `c0`, `S0`, `R0`. Upper case means active-low or inverted (`q`/`Q`, `S`/`R`). Bus connections (`b;`) pass a `bit_offset` to `get_*_terminal`.

**Support libraries** (vendored from the author's other projects, in-tree)
- `err.[ch]`: try/throw/catch-style error handling. Functions return `ERR_F` (an `err_t *`). Use `ERR(call)` to propagate, `ERR_ASSRT(cond, CODE)` to assert, `ERR_THROW(CODE, fmt, ...)` to raise, and `ERR_RETHROW` to add context. Error codes are `LSIM_ERR_*` in `lsim.h`.
- `hmap.[ch]`: hash map holding the devices by name. `device_hash_buckets` is a config parameter.
- `cfg.[ch]`: key = value config file loader (`-c`). `error_reaction` is 0=abort, 1=exit(1), 2=warn and continue, and is stored in the global `global_error_reaction`.

**Entry points**
- `lsim_main.c`: CLI.
- `lsim_test.c`: self-test driver, which is a separate binary built from the same sources.

## Companion project: ldraw

`../ldraw` (https://github.com/fordsfords/ldraw) is a browser-based schematic editor that is the graphical front-end for lsim. It saves `.ldraw` files (JSON) and exports `.lsim` files, so some `.lsim` files here (e.g. `seq1.lsim` from `seq1.ldraw`) are generated rather than hand-written. Changes to the command language in `lsim_cmd.c` or to device terminal ids must stay compatible with ldraw's export. See `../ldraw/CLAUDE.md`.

## Conventions

- Files in the repo root with names like `x`, `x.*`, `*.x`, `*.log` and `*:Zone.Identifier` are scratch files and are gitignored. `bld.sh` deletes `*:Zone.Identifier`.
- Every source file carries the CC0 public-domain header. Keep it on new files.

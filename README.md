# lsim
Logic simulator.

<!-- mdtoc-start -->
&bull; [lsim](#lsim)  
&bull; [WORK IN PROGRESS](#work-in-progress)  
&nbsp;&nbsp;&nbsp;&nbsp;&bull; [Introduction](#introduction)  
&nbsp;&nbsp;&nbsp;&nbsp;&bull; [Quick Start](#quick-start)  
&nbsp;&nbsp;&nbsp;&nbsp;&bull; [Composite Devices](#composite-devices)  
&nbsp;&nbsp;&nbsp;&nbsp;&bull; [Configuration](#configuration)  
&nbsp;&nbsp;&nbsp;&nbsp;&bull; [Design Notes](#design-notes)  
&nbsp;&nbsp;&nbsp;&nbsp;&bull; [Circuit Entry Language](#circuit-entry-language)  
&nbsp;&nbsp;&nbsp;&nbsp;&bull; [License](#license)  
<!-- TOC created by '../mdtoc/mdtoc.pl README.md' (see https://github.com/fordsfords/mdtoc) -->
<!-- mdtoc-end -->

# WORK IN PROGRESS

This project is undergoing rapid evolution.
If you are interested in making use of it,
I recommend waiting until March or April 2025,
by which time it should have stabilized somewhat.
Or contact me and we can collaborate.

FYI - I can't imagine anybody being that interested in this project.
There are much better hardware simulators out there;
I'm writing my own just as a hobby project.
But, hey, if you're genuinely interested, let me know.

## Introduction

I learned many years ago that any logic circuit can be made with only
[NAND gates](https://en.wikipedia.org/wiki/NAND_gate).
Strictly speaking, this isn't quite true.
You can't make an LED out of NAND gates, or an input switch,
or even a constant 0 source (a.k.a. "gnd") or constant 1 ("vcc").
But with some obvious exception, theoretically you can build a full
turing-complete CPU with only NAND gates.

I wanted to prove this to myself.

But actually trying to build a CPU out of hundreds (thousands?) of
[7400 chips](https://en.wikipedia.org/wiki/7400-series_integrated_circuits)
brings to mind rats nests of
[wire-wrap boards](https://www.edn.com/wp-content/uploads/contenteetimes-images-edn-tales-tales-wrapped-z80-computer-backplane-1977.png),
broken wires, out-of-control capacitances, and kilowatts of power consumption. 
And as it turns out, wire wrapping is
[barely even a thing anymore](https://www.reddit.com/r/electronics/comments/i2mqub/comment/g0apze8/).

So, being the ultimate DIY software developer,
I decided to write my own digital logic simulator and use it to execute a
NAND-based CPU of my own design. This project is just the simulator, not the CPU.

Thanks to Claude.ai for some help with the code and much help with the doc.
See https://blog.geeky-boy.com/2024/12/claude-as-coders-assistant.html for details.

## Quick Start

Want to take it for a spin?

I haven't tried to build this under Windows, and I'm confident it would need
lots of changes.
So Linux it is:

```
./bld.sh
./lsim_main sr.lsim
```

This will define and run a very simple shift register.

## Composite Devices

"Wait a minute!" I can hear you saying, "Your simulator has SR latches and D flipflops!
I thought you said it would be only NAND gates!"
I commend your observance, but challenge your conclusion.
My command language does indeed let you define SR latches and D flipflops,
but it simply responds by inserting properly-wired NAND gates.
I.e. if you actually look at the circuit being simulated, it is only NAND gates,
with no simulation-time allowances being given to the higher-level device.
Composite devices are only circuit definition shortcuts.

One glaring exception is the "mem" device.
Although it is perfectly possible to design RAM memory as NAND gates,
the sheer number of them would slow the simulation down to a crawl.
So I did NOT make "mem" a composite, it is a fundamental device.

The clock is another exception. I guess you could make a clock with only nand gates
by stringing them together in a ring to make an oscillator.
But that wouldn't serve my purpose in the simulator given my design
(no infinite loops allowed).
So I made the clock a fundamental device with an external input to
run it in the form of the "ticklet" command.

## Configuration

There are a few configurable parameters for lsim (defaults shown in [square brackets]):
  * **device_hash_buckets** - set to a prime number somewhat larger than the
  total number of logic devices in your circuit [10007]
  * **max_propagate_cycles** - prevent logic engine from infinite looping [50]
  * **error_level** - how to react if an error is detected: 0=abort, 1=exit(1),
  2=warn and continue [0].

To set one or more configs, create a file. For example:
```
error_level = 2
```

Then specify the file on the lsim command line using the "-c" option. For example:
```
./lsim_main -c mycfg.txt
```

## Design Notes

* The lsim project leverages three other much smaller projects:
  * https://github.com/fordsfords/err - error handling framework that
    implements a very primitive form of try/throw/catch.
    It's of my own invention, and lsim is the first reasonably-sized project I've
    used it on, but I'm rather fond of it.
  * https://github.com/fordsfords/hmap - hash map.
  * https://github.com/fordsfords/cfg - simple configuration loader.
* The "devices" subdirectory contains C files for the hardware devices available
  for simulation. E.g. the file "devices/nand.c" implements the nand gate.
* OO-style "inheritance" is implemented with function pointers.
* A "terminal" is an input or an output to a device.
(I thought of naming it a "pin", but many logic gates are not exposed
to the package pins. I thought of "wire", but a wire is something that connects
to a terminal. I thought of "connection" but that is when you connect two
terminals together (maybe with a wire). Terminal was the best I could think of.)
* I use upper-case in some naming conventions to indicate "not".
For example, a latch has "q" and "Q" outputs.
An sr-latch with active-low set and reset labels its inputs S and R.
(I thought about underscore for not, "_q" for not-Q, but it would have
complicated the code more.)
* I wanted a absolute minimum of different logic devices.
So some devices, like srlatch and dflipflop, are composite devices;
defining a dflipflop actually generates 6 nand gates wired as a
[classical d flip-flop](https://en.wikipedia.org/wiki/Flip-flop_(electronics)#Classical_positive-edge-triggered_D_flip-flop).
Note that it names the internal gates with a period (.) so that the name won't
conflict with any user-chosen names (which can't have a period).
* A single "run" of the logic engine consists of a loop containing two phases
  * Have each device with an input change re-calculate its output,
  * Propagate those outputs to the connected inputs.
  A single event (switch move, clock ticklet) can trigger the loop to run
until the circuit stabilizes.
One stabilized, the "run" is complete.
I call one execution of the logic engine a "step".
(In contrast, a "ticklet" is a half-cycle of the clock device.)
* A poorly-designed circuit can cause the logic engine to enter an
infinite loop.
For example, just make an inverter (single-input nand) and connect its
output to its input.
The circuit will never stabilize.
There is a configurable limit to this looping ("max_propagate_cycles")
that defaults to 50.

## Circuit Entry Language

I'm going for ease of parsing, not human friendliness.
Eventually it should have a circuit editor that is more human
friendly (presumably a GUI) that produces this primitive language as output.

For a full detailed description,
see [Logic Simulator Circuit Definition Language Documentation](circuit-language-docs.md) (Thanks Claude.ai!).
Here is a cheat sheet:

````
# Device definitions.
d;gnd;dev_name;
d;vcc;dev_name;
d;swtch;dev_name;init_state;
d;led;dev_name;
d;clk;dev_name;
d;nand;dev_name;num_inputs;
d;srlatch;dev_name;                # composite of nands
d;dflipflop;dev_name;              # composite of nands
d;reg;dev_name;num_bits;           # composite of dflipflops.
d;panel;dev_name;num_bits;         # composite of switches and LEDs
d;mem;dev_name;num_addr;num_data;  # not yet implemented.

# Connect devices.
c;src_dev_name;src_output_id;dst_dev_name;dst_input_id;
b;src_dev_name;src_output_id;dst_dev_name;dst_input_id;num_bits;  # bus (multiple connections)

# Include.
i;filename;

# Power on.
p;

# Move a switch
m;dev_name;new_state;

# Watch a device for debugging (watch_level: 0=none, 1=output change, 2=always print)
w;dev_name;watch_level;

# Tick
t;num_ticklets;
# Verbosity. (verbosity_level: 0=none, 1=output change, 2=always print)
v;verbosity_level;

# quit
q;
````

## License

Copyright 2024, 2024 Steven Ford http://geeky-boy.com and licensed
"public domain" style under
[CC0](http://creativecommons.org/publicdomain/zero/1.0/): 
![CC0](https://licensebuttons.net/p/zero/1.0/88x31.png "CC0")

To the extent possible under law, the contributors to this project have
waived all copyright and related or neighboring rights to this work.
In other words, you can use this code for any purpose without any
restrictions.  This work is published from: United States.  The project home
is https://github.com/fordsfords/lsim

To contact me, Steve Ford, project owner, you can find my email address
at http://geeky-boy.com.  Can't see it?  Keep looking.

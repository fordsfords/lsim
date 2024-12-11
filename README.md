# lsim
Logic simulator.

<!-- mdtoc-start -->
&bull; [lsim](#lsim)  
&nbsp;&nbsp;&nbsp;&nbsp;&bull; [License](#license)  
&nbsp;&nbsp;&nbsp;&nbsp;&bull; [Introduction](#introduction)  
&nbsp;&nbsp;&nbsp;&nbsp;&bull; [Design Notes](#design-notes)  
&nbsp;&nbsp;&nbsp;&nbsp;&bull; [Circuit Entry Language](#circuit-entry-language)  
<!-- TOC created by '../mdtoc/mdtoc.pl README.md' (see https://github.com/fordsfords/mdtoc) -->
<!-- mdtoc-end -->

## Introduction

tbd.

## Configuration

There are a few configurable parameters for lsim (defaults shown in [square brackets]):
  * device_hash_buckets - set to a prime number somewhat larger than the
  total number of logic devices in your circuit [10007]
  * max_propagate_cycles - prevent logic engine from infinite looping [50]
  * error_level - how to react if an error is detected: 0=abort, 1=exit(1),
  2=warn and continue [0].

To set one or more configs, create a file. For example:
```
# Format:
# config_name = value
error_level = 2
```

Then specify the file on the lsim command line using the "-c" option. For example:
```
./lsim -c mycfg.txt
```

## Design Notes

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
So some devices, like srlatch and dlatch, are composit devices;
defining a dlatch actually generates 6 nand gates wired as a
[classical d flip-flop](https://en.wikipedia.org/wiki/Flip-flop_(electronics)#Classical_positive-edge-triggered_D_flip-flop).
Note that it names the internal gates with a period (.) so that the name won't
conflict with any user-chosen names (which can't have a period).
* The "tick" command is the performance-critical part that actually simulates
the circuit.
I.e. "t;1000;" should simulate 1000 timer ticks without any mallocs/frees,
hashes, or any other time-consuming operations.
* A single "run" of the logic engine consists of a loop containing two phases
** Have each device with an input change re-calculate its output,
** Propagate those outputs to the connected inputs.
  A single event (switch move, clock tick) can trigger the loop to run multiple
times as the circuit stabilizes.
One stabilized, the "run" is complete.
* It is very easy to get the system into an unstable state
where the logic engine loops infinitely within a single event.
For example, wire two NAND gates into an SR latch and connect a switch to both
the S and R inputs.
Then just move the switch to 1.
This will trigger infinite looping (oscillation) in the logic engine.
Note that there is a configurable limit to this looping ("max_propagate_cycles")
that defaults to 50.
But in real life it is almost impossible for two independent events to happen at
exactly the same time, and an SR latch will behave properly if there is even the
slightest difference in event arrival time.
My solution is to have external events (each switch movement, each clock transition)
run the engine; i.e. you'll only have one external event per run of the engine.
The user still has a responsibility to avoid bad designs (like connecting both
S and R inputs to the same switch - it makes no sense - instead use two
switches; moving the first will run the engine and moving the second will run
it again, and all will be stable.)

## Circuit Entry Language

Going for ease of parsing, not human friendliness.
Eventually should have a circuit editor that is more human
friendly (GUI?) that produces this as output.

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
d;srlatch;dev_name;
d;dlatch;dev_name;
d;mem;dev_name;num_addr;num_data;  - not yet implemented.

# Connect devices.
c;src_dev_name;src_output_id;dst_dev_name;dst_input_id;

# Include.
i;filename;

# Power cycle.
p;

# Move a switch
m;dev_name;new_state;

# Watch a device (watch_level: 0=none, 1=output change, 2=always print)
w;dev_name;watch_level;

# Tick
t;num_ticks;
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

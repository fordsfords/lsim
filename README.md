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

## Introduction

tbd.

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
* The "step" command is the performance-critical part that actually simulates
the circuit.
* It is very easy to get the system into an unstable state.
For example, wire two NAND gates into an SR latch and connect two switches to the
S and R inputs respectively.
Then, in the same step, move both switches from 0 to 1.
This will trigger infinite oscillation.
But in real life it is almost impossible for two independent events to happen at
exactly the same time, and an SR latch will behave properly if there is even the
slightest difference in event arrival time.
My solution is to have external events (each switch movement) run the engine
before the clock tick.
This will still catch an incorrectly designed circuit by letting it go unstable,
but will make sure that the clock transitions asynchronously to any external
events, like switches being moved.

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

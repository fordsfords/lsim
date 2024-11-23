# lsim
Logic simulator.

<!-- mdtoc-start -->
&bull; [lsim](#lsim)  
&nbsp;&nbsp;&nbsp;&nbsp;&bull; [License](#license)  
&nbsp;&nbsp;&nbsp;&nbsp;&bull; [Introduction](#introduction)  
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

## Circuit Entry Language

Going for ease of parsing, not human friendliness.
Eventually should have a circuit editor that is more human
friendly (GUI?) that produces this as output.

````
# Device definitions.
d;gnd;dev_name;
d;vcc;dev_name;
d;switch;dev_name;init_state;
d;led;dev_name;
d;clk1;dev_name;
d;nand;dev_name;num_inputs;
d;mem;dev_name;num_addr;num_data;

# Connect devices.
c;src_dev_name;src_output_id;dst_dev_name;dst_input_id;

# Include.
i;filename;

# Reset system state.
r;

# Move a switch
m;dev_name;new_state;

# Watch a device (watch_level: 0=none, 1=output change, 2=always print)
w;dev_name;watch_level;

# Step.
s;num_steps;
# Trace. (trace_level: 0=none, 1=output change, 2=always print)
t;trace_level;

# Print single output.
p;dev_name;output_spec;
# Print all inputs/outputs.
P;dev_name;
# Dump system state (BIG!).
D;

# quit
q;
````

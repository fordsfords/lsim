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
d;gnd;device_name;
d;vcc;device_name;
d;switch;device_name;init_state;
d;led;device_name;
d;clk1;device_name;
d;nand;device_name;num_inputs;
d;mem;device_name;num_addr;num_data;

# Connect devices.
c;src_device_name;src_output_id;dst_device_name;dst_input_id;

# Include.
i;filename;

# Reset system state.
r;

# Move a switch
m;device_name;new_state;

# Step.
s;num_steps;
# Trace (for debugging) - lots of output.
t;num_steps;

# Print single output.
p;devicename;output_spec;
# Print all inputs/outputs.
P;devicename;
# Dump system state (BIG!).
D;

# quit
q;
````

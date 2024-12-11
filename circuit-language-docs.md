# Logic Simulator Circuit Definition Language Documentation

This document describes the command language for a digital logic circuit simulator. The simulator allows building circuits from basic components like NAND gates, latches, and input/output devices.

Except for this paragraph and some minor corrections, it was fully written by Claude.ai based only on the C files "lsim_cmd.c" and "lsim_dev.c" ... with all comments stripped! Good job, Claude! See [Claude-doc.md](Claude-doc.md) if you're interested in my conversation with Claude.

## Command Format

Each command consists of a single letter followed by parameters separated and terminated by semicolons. For example:

```
d;gnd;ground1;
```

The command above creates a ground component named "ground1".

## Available Commands

### d - Define Device
Creates a new logic device. Format: `d;device_type;device_name;[parameters...]`

Supported device types:

* `gnd` - Ground (logical 0)
  * Format: `d;gnd;name;`
  * Output: `o0` (always 0)

* `vcc` - Power (logical 1)
  * Format: `d;vcc;name;`
  * Output: `o0` (always 1)

* `swtch` - Switch
  * Format: `d;swtch;name;initial_state;`
  * Parameters:
    * initial_state: 0 or 1
  * Output: `o0`

* `led` - LED Indicator
  * Format: `d;led;name;`
  * Input: `i0`
  * Prints state changes to console

* `nand` - NAND Gate
  * Format: `d;nand;name;num_inputs;`
  * Parameters:
    * num_inputs: Number of inputs (must be > 0)
  * Inputs: `i0` through `i(num_inputs-1)`
  * Output: `o0`

* `clk` - Clock Generator
  * Format: `d;clk;name;`
  * Input: `R0` (reset)
  * Outputs: 
    * `q0` (normal output)
    * `Q0` (inverted output)
  * Alternates between 0 and 1 on each tick when not reset
  * A clock is not necessary and can be omitted from a simple circuit. However, no more than one clock is allowed.

* `srlatch` - SR Latch
  * Format: `d;srlatch;name;`
  * Inputs:
    * `S0` (Set)
    * `R0` (Reset)
  * Outputs:
    * `q0` (normal output)
    * `Q0` (inverted output)

* `dlatch` - D Latch
  * Format: `d;dlatch;name;`
  * Inputs:
    * `d0` (Data)
    * `c0` (Clock)
    * `S0` (Set)
    * `R0` (Reset)
  * Outputs:
    * `q0` (normal output)
    * `Q0` (inverted output)

Note the convention that upper-case letters represent "active-low" or inverted. I.e. a dlatch's "R0" input asserts the reset on value 0.

### c - Connect
Connects an output from one device to an input of another device.

Format: `c;source_device;source_output;destination_device;destination_input;`

Example:
```
c;clock1;q0;nand1;i0;
```

### p - Power On
Initializes and powers on the circuit. Must be called after defining all devices and connections.

Format: `p;`

### m - Move (Set) Switch
Changes the state of a switch.  Also runs the logic engine to propogate the state change through the circuit (does not represent a clock tick).

Format: `m;switch_name;new_state;`

Parameters:
- new_state: 0 or 1

### t - Tick
Advances the simulation by the specified number of clock cycles.

Format: `t;num_ticks;`

### v - Verbosity
Sets the debug output level.

Format: `v;level;`

Parameters:
- level: 0 (quiet), 1 (changes only), or 2 (all states)

### w - Watch
Sets watch level for a specific device.

Format: `w;device_name;level;`

Parameters:
- level: 0 (quiet), 1 (changes only), or 2 (all states)

### i - Include
Includes and processes commands from another file.

Format: `i;filename;`

### q - Quit
Exits the simulation.

Format: `q;`

## Name Constraints
Device names must:
- Start with a letter, underscore, or hyphen
- Contain only letters, numbers, underscores, or hyphens

## Example Circuit

Here's an example of a D flip-flop circuit using NAND gates:

```
# Create NAND gates
d;nand;nand1;2;
d;nand;nand2;2;
d;nand;nand3;2;
d;nand;nand4;2;

# Connect the gates
c;nand1;o0;nand2;i0;
c;nand2;o0;nand1;i1;
c;nand3;o0;nand4;i0;
c;nand4;o0;nand3;i1;

# Add clock and data inputs
d;clk;clock;
d;swtch;data;0;

# Connect inputs
c;clock;q0;nand1;i0;
c;clock;q0;nand3;i0;
c;data;o0;nand2;i1;
c;data;o0;nand4;i1;

# Power on and run
p;
t;10;
```

## Error Handling

The simulator performs validation and will report errors for:
- Invalid device names
- Undefined devices
- Invalid connection points
- Floating inputs
- Uninitialized devices
- Maximum propagation cycle exceeded

## Implementation Notes

- The simulator uses an event-driven architecture
- Changes propagate through the circuit until stability is reached
- A maximum propagation cycle limit prevents infinite oscillations
- Clock devices are synchronized to the global tick counter

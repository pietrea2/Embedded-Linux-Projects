# Embedded C Linux Projects

Developed By:
- Adam Pietrewicz
- Hooman Keshvari

A repository for Embedded Linux course projects for the DE1-SoC Development Board.

## Skills Demonstrated
- User-level and kernel-level programming in Linux
- Memory-mapped I/O with FPGA peripherals
- Handling hardware interrupts in the Linux kernel
- Writing Makefiles and building kernel modules
- Real-time embedded system design concepts

![DE1-SoC Block Diagram](/diagrams/de1_soc_block_diagram.png)

## Project 2
### Part 1: Scrolling Message
- Linux user-level C program that displays a scrolling message on the 7-segment display and terminal
- Pushbutton keys are used to pause/resume scrolling
- Communicate with SoC hardware through memory-mapped IO

### Part 2: Real-time Clock
- A kernel module that displays a real-time clock in the format MM:SS:DD on the 7-segment display and terminal
- Uses FPGA hardware timer register interface to generate periodic interrupts for clock

### Part 3: Stopwatch
- A kernel module that acts as a countdown stopwatch displayed as MM:SS:DD
- Can set start time using switches and pushbuttons
- Supports starting, pausing, and resetting the stopwatch via pushbutton interrupts


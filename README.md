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

## Project 3
### Part 1: Basic Character Device Driver
- Implemented a simple character device driver (chardev) as a Linux kernel module
- Supports read and write operations via /dev/chardev, allowing user programs to modify and retrieve a message

### Part 2: Input Drivers for Switches and Pushbuttons
- Created kernel modules providing /dev/KEY and /dev/SW interfaces
- Drivers read the states of DE1-SoC pushbuttons (KEY) and slider switches (SW) and returned their values to the user as ASCII-encoded data

### Part 3: Output Drivers for LEDs and 7-Segment Displays
- Developed drivers for controlling LEDs (/dev/LEDR) and 7-segment displays (/dev/HEX)
- Drivers accepted ASCII-encoded input from the user to display patterns and values on the hardware

### Part 4: User-Level Control Program
- Wrote a C user-space program to integrate all drivers
- When a KEY was pressed, the current state of the SW switches was displayed on the LEDR lights, and the accumulated total was shown on the HEX displays


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

## Project 2: Embedded Linux Program
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

## Project 3:  Character Device Drivers
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

## Project 4: Using Character Device Drivers
### Part 1: Stopwatch Driver
- Implemented a basic stopwatch driver supporting open, release, and read operations
- Displays stopwatch time in terminal via cat /dev/stopwatch

### Part 2: Enhanced Stopwatch Driver via Write Commands
- Extended the driver to support write operations, enabling commands:
    - run / stop — start or pause stopwatch
    - MM:SS:DD — set stopwatch time
    - disp / nodisp — enable/disable seven-segment display output

### Part 3: User-Level Stopwatch Control Program
- Developed a user-level C program to interact with the stopwatch driver
- Program runs in an endless loop, responding to hardware inputs:
    - KEY0: toggles between run and pause
    - KEY1–KEY3: set stopwatch time parts (MM, SS, DD) using SW switches
    - SW switches: specify the time values
- Read input from KEY and SW device drivers, and optionally displayed SW values on LEDs

### Part 4: Interactive Math Game
- A user-level C program that turns the stopwatch into a timed math quiz game
- Presents a series of math questions to answer before time runs out
- Tracks and reports statistics at the end (questions answered, average response time)
- Optionally displays stopwatch on seven-segment display during the game

## Project 5: ASCII Graphics for Animation
### Part 1: Static ASCII Graphics
- Used VT100 commands to send ASCII escape sequences to the Linux terminal to clear the screen, and draw colored characters at specific positions

### Part 2: Line Drawing with Bresenham’s Algorithm
- Implemented Bresenham’s algorithm in C to draw straight lines between two points on the terminal

### Part 3: Moving Line Animation
- Extended the program to create a moving horizontal line that bounces vertically
- Implemented basic collision detection with the top and bottom edges of the screen

### Part 4: Random Line Animation
- Extended the animation to include multiple objects moving and bouncing off screen edges
- Connected moving objects with lines to form a dynamic “chain"
- Used random initial positions and directions for variety

### Part 5: Interactive Animation Enhancements
- Enhanced the Part 4 animation to respond to hardware inputs (KEYs and SW switches) in live-time to:
    - Adjust animation speed up/down
    - Increase/decrease the number of animated objects
    - Toggle drawing of connecting lines on/off
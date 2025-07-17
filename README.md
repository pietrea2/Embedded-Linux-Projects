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

## [Project 2: Embedded Linux Programs](https://github.com/pietrea2/Embedded-Linux-Projects/tree/main/Project_02)
#### Part 1: Scrolling Message
- Linux user-level C program that displays a scrolling message on the 7-segment display and terminal
- Pushbutton keys are used to pause/resume scrolling
- Communicate with SoC hardware through memory-mapped IO

#### Part 2: Real-time Clock
- A kernel module that displays a real-time clock in the format MM:SS:DD on the 7-segment display and terminal
- Uses FPGA hardware timer register interface to generate periodic interrupts for clock

#### Part 3: Stopwatch
- A kernel module that acts as a countdown stopwatch displayed as MM:SS:DD
- Can set start time using switches and pushbuttons
- Supports starting, pausing, and resetting the stopwatch via pushbutton interrupts

## [Project 3:  Character Device Drivers](https://github.com/pietrea2/Embedded-Linux-Projects/tree/main/Project_03)
#### Part 1: Basic Character Device Driver
- Implemented a simple character device driver (chardev) as a Linux kernel module
- Supports read and write operations via /dev/chardev, allowing user programs to modify and retrieve a message

#### Part 2: Input Drivers for Switches and Pushbuttons
- Created kernel modules providing /dev/KEY and /dev/SW interfaces
- Drivers read the states of DE1-SoC pushbuttons (KEY) and slider switches (SW) and returned their values to the user as ASCII-encoded data

#### Part 3: Output Drivers for LEDs and 7-Segment Displays
- Developed drivers for controlling LEDs (/dev/LEDR) and 7-segment displays (/dev/HEX)
- Drivers accepted ASCII-encoded input from the user to display patterns and values on the hardware

#### Part 4: User-Level Control Program
- Wrote a C user-space program to integrate all drivers
- When a KEY was pressed, the current state of the SW switches was displayed on the LEDR lights, and the accumulated total was shown on the HEX displays

## [Project 4: Using Character Device Drivers](https://github.com/pietrea2/Embedded-Linux-Projects/tree/main/Project_04)
#### Part 1: Stopwatch Driver
- Implemented a basic stopwatch driver supporting open, release, and read operations
- Displays stopwatch time in terminal via cat /dev/stopwatch

#### Part 2: Enhanced Stopwatch Driver via Write Commands
- Extended the driver to support write operations, enabling commands:
    - run / stop — start or pause stopwatch
    - MM:SS:DD — set stopwatch time
    - disp / nodisp — enable/disable seven-segment display output

#### Part 3: User-Level Stopwatch Control Program
- Developed a user-level C program to interact with the stopwatch driver
- Program runs in an endless loop, responding to hardware inputs:
    - KEY0: toggles between run and pause
    - KEY1–KEY3: set stopwatch time parts (MM, SS, DD) using SW switches
    - SW switches: specify the time values
- Read input from KEY and SW device drivers, and optionally displayed SW values on LEDs

#### Part 4: Interactive Math Game
- A user-level C program that turns the stopwatch into a timed math quiz game
- Presents a series of math questions to answer before time runs out
- Tracks and reports statistics at the end (questions answered, average response time)
- Optionally displays stopwatch on seven-segment display during the game

## [Project 5: ASCII Graphics for Animation](https://github.com/pietrea2/Embedded-Linux-Projects/tree/main/Project_05)
#### Part 1: Static ASCII Graphics
- Used VT100 commands to send ASCII escape sequences to the Linux terminal to clear the screen, and draw colored characters at specific positions

#### Part 2: Line Drawing with Bresenham’s Algorithm
- Implemented Bresenham’s algorithm in C to draw straight lines between two points on the terminal

#### Part 3: Moving Line Animation
- Extended the program to create a moving horizontal line that bounces vertically
- Implemented basic collision detection with the top and bottom edges of the screen

#### Part 4: Random Line Animation
- Extended the animation to include multiple objects moving and bouncing off screen edges
- Connected moving objects with lines to form a dynamic “chain"
- Used random initial positions and directions for variety

#### Part 5: Interactive Animation Enhancements
- Enhanced the Part 4 animation to respond to hardware inputs (KEYs and SW switches) in live-time to:
    - Adjust animation speed up/down
    - Increase/decrease the number of animated objects
    - Toggle drawing of connecting lines on/off

## [Project 6: Graphics & Animation Using Linux Character Drivers](https://github.com/pietrea2/Embedded-Linux-Projects/tree/main/Project_06)
#### Part 1: VGA Driver
- Implemented a Linux character device driver (/dev/video) to interface with the VGA display to:
    - Clear the screen
    - Set individual pixel colours

#### Part 2: Line Drawing Command
- Extended the driver with a line drawing command (Bresenham’s algorithm)
- Reduced overhead by moving line computation into the driver rather than issuing many pixel commands

#### Part 3: Animation with Synchronization
- Added support for VGA vertical synchronization (sync command) to avoid tearing during animations by performing pixel buffer swapping
- Developed a user-space program to animate a horizontal line bouncing between the top and bottom of the screen

#### Part 4: Animated Boxes
- Enhanced the driver to support drawing boxes of any specified length
- Wrote a user-level program that animates eight bouncing rectangles, connected by lines to form a moving chain
- Implemented double-buffering for smooth animations without flickering by storing pixel buffers in FPGA on-chip memory and SDRAM

#### Part 5: Controlling Animated Boxes
- Added user interaction to control the animation using pushbuttons and switches in real time to:
    - Adjust animation speed
    - Increase/decrease the number of rectangles
    - Toggle lines between rectangles on/off

#### Part 6: VGA Text Display
- Extended the driver to work with the VGA character buffer to display a frame counter in the corner of the screen
- Added commands to:
    - Erase text from the screen
    - Display ASCII text at specified screen coordinates

## [Project 7: ADXL345 Accelerometer](https://github.com/pietrea2/Embedded-Linux-Projects/tree/main/Project_07)
#### Part 1: Configuring and Reading the ADXL345
- A user level program that configures the accelerometer, and prints real-time X, Y, Z acceleration values while tilting the board using virtual addresses to access the chip via memory-mapped I/O

#### Part 2: ADXL345 Character Device Driver
- Developed a character device driver for the ADXL345 to add support to the Linux kernel for accessing the device
- Created /dev/accel device file to allow user-space access to accelerometer data

#### Part 3: Device Driver Write Operations & Graphical Demo
- Extended the character device driver to handle write commands such as:
    - init: re-initializes the device
    - calibrate: runs calibration routine
    - format F G: sets resolution & range
- Implemented a user-space program with a graphical demo that displays a “bubble” that moves based on board tilt
- Supported smoothing of acceleration readings using a running average to reduce jitter

#### Part 4: Tap and Double-Tap Detection
- Enhanced the driver to detect tap and double-tap gestures using the ADXL345’s interrupt capabilities

## [Project 8: Audio and Multithreaded Applications on DE1-SoC](https://github.com/pietrea2/Embedded-Linux-Projects/tree/main/Project_08)
#### Part 1: Single Tone Playback
- Wrote a Linux user-level C program that generates and plays the middle C chromatic scale through the audio-out port of the
 DE1-SoC Computer system
- Used memory-mapped I/O to write samples to the DE1-SoC audio CODEC via the audio port registers

#### Part 2: Chord Playback
- Extended the program to play multiple tones (chords) simultaneously (without volume overflow) based on a 13-character command-line input

#### Part 3: Digital Piano with Multithreading
- Created a multithreaded digital piano that responds to USB keyboard input in real-time
- Used Pthreads to separate audio generation and input handling, improving responsiveness and timing accuracy

#### Part 4: Piano Waveform Visualization
- Added a third thread to display the currently played sound waveforms on the VGA screen using character device drivers to communicate with the VGA controller

#### Part 5: Recording and Playback
- Implemented a feature to record and play back sequences of notes, tracking piano key press/release timings
- Used the stopwatch and pushbuttons/LEDs to control and indicate recording/playback status

#### Part 6: Character Device Driver Version
- Modified the program to use the Linux character device driver interface (/dev/IntelFPGAUP/audio) instead of memory-mapped I/O for audio output

## [Project 9: Simple Oscilloscope Using DE1-SoC](https://github.com/pietrea2/Embedded-Linux-Projects/tree/main/Project_09)
#### Part 1: Reading ADC Pin Voltage
- C program that reads from the DE1-SoC ADC (AD7928) controller port registers and prints the voltage level of the ADC connector pin to the terminal

#### Part 2: Digital Signal Generator
- Developed a Linux kernel module that generates a configurable square wave on pin D0 of the JP1 connector of the DE1-SoC
- Used FPGA Timer0 to generate periodic interrupts and toggle the output pin at a frequency set by board switches (10–160 Hz)
- Displayed the selected frequency and switch settings on LEDs and seven-segment displays

#### Part 3: Oscilloscope Application
- Wrote a C program that implements an oscilloscope that samples ADC channel 0 periodically and displays the waveform on the VGA screen
- Triggered sweeps on rising or falling edges of the input signal based on switch SW0

#### Part 4: Enhanced Oscilloscope With Sweep Time Control
- Enhanced the oscilloscope to allow the user to adjust the sweep duration using the board’s KEY buttons (increase/decrease by 100 ms increments)

## [Project 10: Image Processing: Canny Edge Detection](https://github.com/pietrea2/Embedded-Linux-Projects/tree/main/Project_10)
#### Part 1: Software Implementation
- Developed an implementation of the Canny Edge Detection Algorithm in C which includes: grayscale conversion, Sobel operator, non-max suppresison and hysteresis
- tested edge detection on BMP images and visualized results using VGA display

#### Part 2: Hardware Setup and Image Display
- Reconfigured the FPGA with a hardware edge-detection system
- Wrote C function to properly pack 24-bit pixels into 32-bit SDRAM memory
- Implemented flip() that flips the image vertically before writing it into the SDRAM buffer so it appears correctly on the video output

#### Part 3: Hardware-Accelerated Edge Detection
- Extended the Part 2 code to enable and control the FPGA’s hardware edge-detection pipeline
- Used DMA controllers and their registers to:
    - Stream image data to and from the FPGA-based edge-detection circuit
    - Swap buffer addresses and wait for completion using status polling
    - Display the edge-detected output on the VGA monitor
    - Saved the hardware-processed edge-detected image back to a BMP file
- Compared the hardware-accelerated runtime against the CPU-only implementation to evaluate performance improvements


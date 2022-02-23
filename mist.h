/*

 */

#ifndef MIST_H
#define MIST_H

// FPGA spi cmommands
#define MIST_INVALID      0x00

// memory interface
#define MIST_SET_ADDRESS  0x01
#define MIST_WRITE_MEMORY 0x02
#define MIST_READ_MEMORY  0x03
#define MIST_SET_CONTROL  0x04

// control bits (all control bits have unknown state after core startup)
#define MIST_CONTROL_CPU_RESET 0x01

#endif // MIST_H

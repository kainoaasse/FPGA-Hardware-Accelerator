/* -----------------------------------------------------------------------------
 * Project: High-Performance FPGA Hardware Accelerator
 * File: sorting_core.h
 * Author: Kainoa Asse
 * Description:
 * Driver class for the Custom Sorting IP Core. Defines the register map,
 * control bits, and high-level methods for hardware-accelerated sorting.
 * -----------------------------------------------------------------------------
 */
 
#ifndef _SORTING_CORE_H_INCLUDED
#define _SORTING_CORE_H_INCLUDED

#include "chu_init.h"
 
class SortCore {
public:
/* Register map */
	enum {
		MEMW_ri_REG = 0, // Writing to MEM[ri] & ri++ (16 bits)
		MEMR_ri_REG = 1, // Reading from MEM[ri] & ri++ (16 bits)
		N_REG       = 2, // set N (16 bits)
	    CTRL_REG    = 3, // control register rw, init, s
		STATUS_REG  = 4 // Done (1-bit) register
	};

	/*masks*/
	enum {
		DATA_MASK = 0x0000FFFF, //mask for 16-bit sorting data
		S_BIT = 0x00000001, //mask for s bit 001
		INIT_BIT = 0x00000002, //mask for init bit 010
		RW_BIT = 0x00000004, //mask for RW bit 100
	};

	/**
	constructor: automatically called when an object of class SortCore is created
	Note: Constructor has no return value, takes in parameter core_base_addr
	to set initial attribute of base address
	*/
	SortCore(uint32_t core_base_addr);
	~SortCore(); // not used
	
	/* Methods*/
	
	/* Configuration */
	void set_n(uint16_t n); // initialize N for loop control aka how many integers to sort

	/* Control Flow */
	void init_write(); //initialize write conditions => 110 to ctrl_reg: rw=1, init=1, s=0 (0x06)
	void init_read(); //initialize read conditions (Start Readout/Read) => 010 to control_reg: rw=0, init=1, s=0 (0x02)
	void idle(); // all=0 -> 0x00 (Return to Idle/Stop Sorting)
	void sort(); //write '1' to control register => start sorting s=1 -> 0x01

	/* Data Transfer */
	void write(uint16_t data); //write a 16-bit data to MEMW_ri_REG
	uint16_t read(); //reads and returns a 8-bit data from a specified address in memory


	/* Status */
	bool done(); //reads and returns the status register as 8-bits returns true if done bit is 1
	
private: 
	uint32_t base_addr;
	uint32_t wr_data;

};
#endif

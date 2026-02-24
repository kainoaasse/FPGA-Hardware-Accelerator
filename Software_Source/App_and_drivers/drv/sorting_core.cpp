/* -----------------------------------------------------------------------------
 * Project: High-Performance FPGA Hardware Accelerator
 * File: sorting_core.cpp
 * Author: Kainoa Asse
 * Description:
 * Implements the SortCore class methods for low-level MMIO communication.
 * Handles hardware-to-software handshaking, data transfer, and
 * synchronization with the VHDL sorting logic.
 * -----------------------------------------------------------------------------
 */
 
#include "sorting_core.h"

SortCore::SortCore(uint32_t core_base_addr) {
	base_addr = core_base_addr;
	wr_data = 0; // Initialize shadow register aka copy register
}
SortCore::~SortCore() {
}

void SortCore::set_n(uint16_t n){
	io_write(base_addr, N_REG, (uint32_t)n);
}

void SortCore::init_write(){
	// Force IDLE to clear any previous sorting state
	io_write(base_addr, CTRL_REG, 0);
	// rw=1, init=1, s=0 -> 0x06
	io_write(base_addr, CTRL_REG, RW_BIT | INIT_BIT);
}

void SortCore::init_read(){
	// Exit computation mode
    io_write(base_addr, CTRL_REG, 0);

    // Reset the internal pointer (ri = 0)
    io_write(base_addr, CTRL_REG, INIT_BIT);

    // Small wait to let the hardware state stabilize
    for(volatile int i=0; i<10; i++);

}

void SortCore::idle() {
    io_write(base_addr, CTRL_REG, 0);
}

void SortCore::sort(){
	io_write(base_addr, CTRL_REG, S_BIT);
}
	
void SortCore::write(uint16_t data){
	// Store copy in shadow register (State Tracking)
	// Typecast to 32-bit for the system bus
	wr_data = (uint32_t)(data & DATA_MASK);
	// Perform actual hardware write
	io_write(base_addr, MEMW_ri_REG, wr_data);
}
	
uint16_t SortCore::read(){
	// Typecast back to 16-bit and mask for safety
	return (uint16_t)(io_read(base_addr, MEMR_ri_REG) & DATA_MASK);
}

	
bool SortCore::done(){
	// Read bit 0 of status register
	return (bool)(io_read(base_addr, STATUS_REG) & 0x01);
}

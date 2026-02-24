/*
 * By: Kainoa Asse
 * C++ main application for sorting in software and hardware
 * Using an FPro SoC with Standard and Custom Hardware IP Cores: Fast Sorting
 *
 * SUMMARY OF PROGRAM
 *
 * Solution supports sorting of N w-bit unsigned integers for at least the following values of k, N = 2^k
 * k = 4, 8, 9, 10, 11, 12, 13
 * w = 8, 8, 16, 16, 16, 16, 16
 * Switches SW3...SW0 should be used to enter k=log2(N), i.e., log2 of the number of elements to sort.
 *
 * Note: Total Time of HW-accelerated sorting(N) = Time(hw_data => core_mem)(N) + Time(sorting in HW)(N)
 * + Time (core_mem => hw_data)(N)
 *
 * 1) INITIALIZING MEMORY
 * After power-up, system should wait for the first press of BTNR or BTNL, all remaining buttons should be ignored
 * - Pressing BTNR initializes two arrays, sw_data[] and hw_data[], stored in Processor's RAM,
 *   with the same N pseudorandom values generated in software from LFSR
 * - Pressing BTNL initializes two arrays, sw_data[] and hw_data[], stored in Processor's RAM,
 * 	 with the same N values generated in software using equation sw_data[addr] = HW_data[addr] = N – 1 – addr.
 *
 * 2) DISPLAY MODE
 * In the Display Mode, for N=2k with k=4..8, a user should be able to browse the contents of the
 * arrays sw_data[ ] and hw_data[ ] using the following user interface:
 * - SW15=0 : browsing sw_data[]
 * - SW15=1 : browsing hw_data[]
 * Seven-Segment Display 3 and 2 => Current Address
 * Seven-Segment Display 1 and 0 => Value in sw_data[](SW15 = 0) or hw_data[](SW15 = 1) at the position given
 * by the Current Address
 * BTNU should increment the Current Address in a wrap-around fashion (e.g., for N=16, "0F" should be followed by "00")
 * BTND should decrement the Current Address in a wrap-around fashion (e.g., for N=16, "00" should be followed by "0F")
 * For values of N > 256, browsing should be disabled, and the Seven-Segment Displays should
 * show values of k and w, respectively, expressed in the decimal notation.
 *
 * 3) SORTING
 * Pressing BTNC (when SW12=0) should initiate sorting.
 * Sorting is performed in hardware and software. Sorting in software should be performed on the array sw_data[ ] stored in the
 * Processor memory. Sorting in hardware should involve transferring input data from the array
 * hw_data[ ] to the Sorting core, performing sorting, and transferring results back to the array
 * hw_data[ ]. The processed numbers should be treated as unsigned integers and should be sorted in ascending order
 * The total number of clock cycles required for software and hardware sorting should be measured and stored.
 * During sorting, “----“ should be displayed on the seven-segment displays
 * After sorting is completed, the seven-segment displays should show the number of mismatches
 * between the results of sorting in software and the results of sorting in hardware. Thus, 0000 will
 * represent a perfect match. For N=16 (0x0010), 0010 will mean that none of the corresponding
 * locations in each array matches.
 * After another press of BTNC, the system should come back to the Display Mode.
 *
 * 4) Cycle Count Mode
 * After sorting is completed, pressing BTNL should allow toggling between the Display Mode and the Cycle Count Mode.
 * In the Cycle Count Mode, the total number of clock cycles used for sorting should be displayed on the seven-segment displays.
 * The position of the switch SW14 should have the following meaning:
 * SW14=0 : displaying the least significant 16 bits of the Cycle Counter
 * SW14=1 : displaying the most significant 16 bits of the Cycle Counter
 * The position of the switch SW15 should have the following meaning:
 * SW15=0 : number of clock cycles used for sorting in software
 * SW15=1 : number of clock cycles used for sorting in hardware.
 *
 */
#include "drv/chu_init.h"
#include "drv/gpio_cores.h"
#include "drv/sseg_core.h"
#include "drv/sorting_core.h"
#include "drv/timer_core.h"
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <cmath>
#include <inttypes.h>
#include <unistd.h>

#define MAX_SIZE 8192 //2^13
#define NIBBLE_MASK 0x0F

// Button bit-mapping
#define BTN_UP     (1 << 0)
#define BTN_RIGHT  (1 << 1)
#define BTN_DOWN   (1 << 2)
#define BTN_LEFT   (1 << 3)
#define BTN_CENTER (1 << 4)

//State definitions
enum SystemState{
	STATE_IDLE,        // Waiting for starts
	STATE_DISPLAY,     // Data browsing mode (Small N) or k/w display (Large N)
	STATE_SORTING,     // Active sorting ("----" on SSEG)
	STATE_MISMATCH,    // Showing errors
	STATE_CYCLE_COUNT  // Showing timing
};


// Global variables
// volatile to avoid compiler over-optimization
volatile uint16_t sw_data[MAX_SIZE];
volatile uint16_t hw_data[MAX_SIZE];

uint16_t N = 16; // Current number of elements to sort
uint8_t k = 4; //log2(N)
uint16_t w = 8; // Data width (8 or 16 bits)
uint16_t current_address  = 0;
uint64_t sw_cycles = 0;
uint64_t hw_total_cycles = 0;
int mismatches = 0;
SystemState current_state = STATE_IDLE;

// Hardware Core Instances
// Instantiate timer, sseg, debounce core, sort core, switch
TimerCore timer(get_slot_addr(BRIDGE_BASE, S0_SYS_TIMER));
SsegCore sseg(get_slot_addr(BRIDGE_BASE, S8_SSEG));
DebounceCore btn(get_slot_addr(BRIDGE_BASE, S7_BTN));
SortCore sort(get_slot_addr(BRIDGE_BASE, S4_USER));
GpiCore sw(get_slot_addr(BRIDGE_BASE, S3_SW));

// Software LFSR Class
class LFSR {
public:
    LFSR(uint16_t seed = 0xACE1) : lfsr_state(seed) {}
    uint16_t next() {
        // Taps for 16-bit LFSR: 0, 2, 3, 5
        uint16_t bit = ((lfsr_state >> 0) ^ (lfsr_state >> 2) ^
                        (lfsr_state >> 3) ^ (lfsr_state >> 5)) & 1;
        lfsr_state = (lfsr_state >> 1) | (bit << 15);
        return lfsr_state;
    }
private:
    uint16_t lfsr_state;
};
LFSR software_lfsr;

//Reads SW3 ... SW0 to determine the number of elements to sort N
//Sets N, k, and w global variables
void update_config() {
    uint32_t sw_val = sw.read();
	k = sw_val & 0x0F; // SW3..0 defines k

	//Safety check
	if (k<4) k = 4;
	if (k>13) k = 13;
    N = (uint16_t)(1 << k);
    w = (k < 9) ? 8 : 16; // if k is greater than 8 then width must be 16
}

void init_arrays(bool random) {
    update_config();
    current_address = 0; // Reset address on init
    uart.disp("\r\n--- Initializing Data Structure ---\r\n");
    uart.disp("Array Size (N): "); uart.disp(N);
    uart.disp(" | Data Width (w): "); uart.disp(w); uart.disp(" bits\r\n");

    if (random) {
    	uart.disp("Pattern: Pseudo-random (LFSR)\r\n");
    } else {
    	uart.disp("Pattern: DESCENDING ORDER (Worst Case Scenario)\r\n");
        uart.disp("Note: This triggers maximum comparisons for Selection Sort.\r\n");
    }

    for (int i = 0; i < N; i++) {
        uint16_t val = random ? software_lfsr.next() : (N - 1 - i);
        if (w == 8) val &= 0xFF; // Mask for 8-bit mode
        sw_data[i] = hw_data[i] = val;
    }
    uart.disp("Memory Initialized. N="); uart.disp(N); uart.disp("\r\n");
}

// Software Sorting (Selection Sort Algorithm)
void software_sort() {
    timer.clear();
    timer.go();

    // Selection Sort
    for (int i = 0; i < N - 1; i++) {
        int min_idx = i;
        for (int j = i + 1; j < N; j++) {
            if (sw_data[j] < sw_data[min_idx]) min_idx = j;
        }
        //Swap
        uint16_t temp = sw_data[i];
        sw_data[i] = sw_data[min_idx];
        sw_data[min_idx] = temp;
    }
    timer.pause();
    sw_cycles = timer.read_tick();
}

// Hardware Sorting
void hardware_sort() {
    timer.clear();
    timer.go();

    // Write to Core (Host -> FPGA)
    sort.set_n(N);
    sort.init_write(); // rw=1, init=1, s=0 (Write Mode), Reset internal pointer ri=0
    for (int i = 0; i < N; i++){
    	sort.write(hw_data[i]);
    }

    // Start sorting
    sort.sort(); // s=1
    while(!sort.done()); // Wait for Done Signal

    // Readback (FPGA -> Host)
    sort.init_read(); // rw=0, init=1, s=0 (Read Mode), Resets internal pointer ri=0
    //sort.read();

    for (int i = 0; i < N; i++) {
        // Read twice as required for the specific MMIO wrapper
        //hw_data[i] = sort.read();
        hw_data[i] = sort.read();

    }

    timer.pause();
    hw_total_cycles = timer.read_tick();
}

// MAIN LOOP
int main() {
    init_fix();
    timer.sleep(500); // Wait 100ms for UART to stabilize
    uart.disp("\r\n--- Project by Kainoa L. Asse ---\r\n");
    uart.disp("\r\n--- HELLO WORLD, SYSTEM READY ---\r\n");
    uart.disp("State: IDLE. Press BTNR or BTNL to initialize memory.\r\n");
    sseg.set_dp(0x00); // Turns off all decimal points

    uint32_t db_old = btn.read_db();

    while (1) {
        // Read inputs
    	uint32_t sw_val = sw.read();
    	// Extract specific switch bits for easier use in logic
    	int sw15 = (sw_val >> 15) & 1;
    	int sw14 = (sw_val >> 14) & 1;
    	int sw12 = (sw_val >> 12) & 1;

        uint32_t db_new = btn.read_db(); // Read debounced buttons

        // Detect the moment a button is pushed down
        uint32_t pressed = (db_new ^ db_old) & db_new; // Edge detection
        db_old = db_new;

        // UART DEBUG: Print every button press immediately
        /*
        if (pressed) {
        	uart.disp("DEBUG -> BTN Pressed: ");
            if (pressed & BTN_UP)     uart.disp("UP ");
            if (pressed & BTN_DOWN)   uart.disp("DOWN ");
            if (pressed & BTN_LEFT)   uart.disp("LEFT ");
            if (pressed & BTN_RIGHT)  uart.disp("RIGHT ");
            if (pressed & BTN_CENTER) uart.disp("CENTER ");
            // Also print the raw hex for bit-mapping confirmation
            uart.disp("(0x");
            uart.disp((int)pressed, 16);
            uart.disp(")\r\n");
        }
*/
        // --- State Machine Logic ---
        switch (current_state) {
            case STATE_IDLE:
            	// Only allow initialization. Ignore all other buttons
            	if (pressed & (BTN_LEFT | BTN_RIGHT)) {
            		// BTNR (Right) = true (random), BTNL (Left) = false (descending)
            	    init_arrays(pressed & BTN_RIGHT);
            	    current_state = STATE_DISPLAY;
            	    uart.disp("State Switch: DISPLAY MODE\r\n");
            	}
                break;

            case STATE_DISPLAY:
            	// Allow re-initialization at any time of user wants to change N or data pattern
                if (pressed & BTN_RIGHT) init_arrays(true);
                if (pressed & BTN_LEFT) init_arrays(false);

                // Browsing Logic
                // Allow browsing only if N <= 256
                if (N <= 256) {
                	if (pressed & BTN_UP) {
                		current_address = (current_address + 1) % N; // Increment with wrap around
                        //uart.disp("Addr Up: "); uart.disp(current_address); uart.disp("\r\n");
                    }
                    if (pressed & BTN_DOWN) {
                    	current_address = (current_address == 0) ? N - 1 : current_address - 1;  //Decrement wrap
                        //uart.disp("Addr Down: "); uart.disp(current_address); uart.disp("\r\n");
                    }
                }

                // Sorting Trigger: Center button + SW12 == 0
                if ((pressed & BTN_CENTER) && sw12 == 0) {
                    current_state = STATE_SORTING;
                }
            break;

            case STATE_SORTING:
            {
            	// Display "----" immediately
            	uint8_t dash[4] = {0xBF, 0xBF, 0xBF, 0xBF};
            	sseg.write_8ptn(dash);


                uart.disp("Sorting...\r\n");
                uart.disp("1. Running Software Selection Sort on MicroBlaze CPU...\r\n");
                software_sort();

                uart.disp("2. Running Hardware-Accelerated Sort on FPGA Core...\r\n");
                hardware_sort();

                // Check Mismatches
                uart.disp("\r\n--- Verification Report ---\r\n");
                mismatches = 0;
                for (int i = 0; i < N; i++) {
                	if (sw_data[i] != hw_data[i]){
                		if (mismatches <= 10) {
                			// Only print the first 10 mismatches to avoid spamming UART
                			uart.disp("Mismatch at Index ["); uart.disp(i); uart.disp("]: ");
                			uart.disp("Expected(SW)="); uart.disp(sw_data[i]);
                			uart.disp("  Actual(HW)="); uart.disp(hw_data[i]);
                			uart.disp("\r\n");
                			if (i == 0 && hw_data[i] == 0) uart.disp(" <- (Check BRAM Latency)");
                			uart.disp("\r\n");
                		}
                		mismatches++;
                	}
                	// Only print the first 10 mismatches to avoid spamming UART

                }
                if (mismatches == 0) uart.disp("> SUCCESS: All values match!\r\n");
                else {
                	uart.disp("> FAIL: "); uart.disp(mismatches); uart.disp(" mismatches found.\r\n");
                }
                // Calculate Speedup: (SW - HW) / HW * 100
                double speedup = (hw_total_cycles > 0) ?
                	((double)sw_cycles - (double)hw_total_cycles) / (double)hw_total_cycles * 100.0 : 0.0;

                // Print Stats
                uart.disp("Done.\r\n");
                uart.disp("Mismatches: "); uart.disp(mismatches); uart.disp("\r\n");
                uart.disp("SW Cycles: "); uart.disp((int)sw_cycles); uart.disp("\r\n");
                uart.disp(" (0x"); uart.disp((int)sw_cycles, 16); uart.disp(")\r\n");

                uart.disp("HW Cycles: "); uart.disp((int)hw_total_cycles); uart.disp("\r\n");
                uart.disp(" (0x"); uart.disp((int)hw_total_cycles, 16); uart.disp(")\r\n");

                uart.disp("HW is "); uart.disp(speedup, 2); uart.disp("% Faster\r\n");
                uart.disp("---------------------------\r\n");
                current_state = STATE_MISMATCH;
            }

            break;

            case STATE_MISMATCH:
            	// Pressing BTNC again returns to Display Mode
            	if (pressed & BTN_CENTER) {
            		current_state = STATE_DISPLAY;
            	    uart.disp("Returning to DISPLAY\r\n");
            	}
            	// Pressing BTNL toggles to Cycle Count Mode
            	if (pressed & BTN_LEFT) {
            		current_state = STATE_CYCLE_COUNT;
            	    uart.disp("Mode Switch: CYCLE COUNT\r\n");
            	}
                break;

            case STATE_CYCLE_COUNT:
            	// Manual: Pressing BTNL again toggles back to Display Mode
            	if (pressed & BTN_LEFT) {
            		current_state = STATE_DISPLAY;
            	    uart.disp("Returning to DISPLAY\r\n");
            	}
            break;
        }

        // --- SSEG Display Logic ---
        // This runs every loop, so SSEG updates instantly when switches move
        uint8_t ptn[4];
        if (current_state == STATE_MISMATCH) {
        	// Show Mismatches in Hex
            for (int i = 0; i < 4; i++) ptn[i] = sseg.h2s((mismatches >> (i * 4)) & 0xF);
        }
        else if (current_state == STATE_CYCLE_COUNT) {
        	// SW15: 0=SW, 1=HW | SW14: 0=Lower 16bit, 1=Upper 16bit
            uint32_t val = sw15 ? (uint32_t)hw_total_cycles : (uint32_t)sw_cycles;
            if (sw14) val >>= 16;
            for (int i = 0; i < 4; i++) ptn[i] = sseg.h2s((val >> (i * 4)) & 0xF);
        }
        else if (current_state != STATE_SORTING) {
        	if (N > 256) {
        		// Show k and w (e.g., k=09, w=16)
                ptn[3] = sseg.h2s(k / 10); ptn[2] = sseg.h2s(k % 10);
                ptn[1] = sseg.h2s(w / 10); ptn[0] = sseg.h2s(w % 10);
            } else {
            	// Browsing: [Addr][Value]
            	uint16_t val = sw15 ? hw_data[current_address] : sw_data[current_address];
                ptn[3] = sseg.h2s((current_address >> 4) & 0xF);
                ptn[2] = sseg.h2s(current_address & 0xF);
                ptn[1] = sseg.h2s((val >> 4) & 0xF);
                ptn[0] = sseg.h2s(val & 0xF);
            }
        }
        if (current_state != STATE_SORTING) {
        	sseg.write_8ptn(ptn);
        }
    }
}



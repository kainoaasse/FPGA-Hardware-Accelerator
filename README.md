# FPGA Hardware-Accelerated Sorter
**Developed by: Kainoa L. Asse**

## Project Goal
This project compares a standard **Software Sort** (running on a MicroBlaze CPU) against a **Custom VHDL Hardware Accelerator**. The goal was to prove that dedicated FPGA logic can process data significantly faster than a general-purpose processor.

## Performance Results (The "Worst Case" Benchmark)
I tested the system using 8,192 elements initialized in **Descending Order** (the hardest scenario for a selection sort).

| Metric | Software (CPU) | Hardware (FPGA) |
| :--- | :--- | :--- |
| **Clock Cycles** | 1,460,019,229 | 68,051,405 |
| **Execution Time** | ~14.6 seconds | ~0.68 seconds |

**Result:** The Hardware Accelerator achieved a **21.4x Speedup** (over 2000% faster).

## Verification Screenshot
Below is the output from the Vitis Serial Terminal confirming the cycle counts and data accuracy:

![Benchmark Results](./Documentation/Verification_report_Vitis)

## How It Works
* **Hardware:** Basys 3 FPGA (Artix-7).
* **Communication:** Used MMIO (Memory-Mapped I/O) to send data from the processor to the FPGA RAM.
* **Accuracy:** The system compares the results of both sorts to ensure 0 mismatches.
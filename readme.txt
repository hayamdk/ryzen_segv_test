[About]

This is a test code to reproduce SEGV of RYZEN processor.


[Overview]

This is a "cross-modifying code".
There a three threads (thread1 and two threadx).
Thread1 executes code of specific address repeatedly, at the same time threadx modify data of this address.
The execution and modifing are protected with multithread synchronization.
In addition to this, a serializing instruction is inserted before call of modified function code.

About serializing instruction in "cross-modifying code":
	"8.1.3 Handling Self- and Cross-Modifying Code" of
	https://www.intel.com/content/www/us/en/architecture-and-technology/64-ia-32-architectures-software-developer-vol-3a-part-1-manual.html (Intel® 64 and IA-32 Architectures Software Developer’s Manual, Volume 3A: System Programming Guide, Part 1)


[Usage]

It works with gcc on Linux (Confirmed: Ubuntu 17.04) and VS2017 on Windows10(1703).

In Linux:
---
# make
# ./run.sh [n] [m]
---
n: Number of concurrent execution (e.g. 8)
m: Loop count (e.g. 2500000)

In Windows:
Build ryzen_segv_test and lancher with VS2017.
Place ryzen_segv_test.exe and lancher.exe in same directory, and open lancher.exe.
Input concurrency and loop count to command prompt.

Output log is in log.txt.

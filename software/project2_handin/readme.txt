
[Code Files we wrote]
* P2_PrototypeOS_WORKING\myScheduler.S
* P2_PrototypeOS_WORKING\P2PrototypeOS.h
* P2_PrototypeOS_WORKING\P2PrototypeOS.c
* P2_PrototypeOS_WORKING\ThreadQueue.c

Additional files are provided so that the project can easily be imported into Eclipse and run without any issues. 


[How to compile and run]
It's recommended that you use the DE2_NIOS-II-CUSTOM provided because it has been rebuilt for Quartus 12 and to have a timer that does 10 ticks per second as opposed to 8 ticks per second that the provided DE2_NIOS-II file does. 

1.)  Launch Nios II - Eclipse
	- When it asks for a workspace navigate to the root directory that this readme is in. 

2.) Import Existing Projects into Workspace
	• Browse to the root directory that this readme is in and click ok
	• Eclipse will automatically find P2_PrototypeOS_WORKING and P2_PrototypeOS_WORKING_bsp.  Ensure they are selected then press finish to import these two projects. 
	
3.) In Eclipse under the "Nios II" menu, launch the Quartus II 32-bit Programmer. 
	• Go to Add File, then add the DE2_Basic_Computer.sof located in DE2_NIOS-II-CUSTOM folder
	• Press Start to program the FPGA. 
	
4.) In Eclipse
	• Build ALL.
	• Open Run Configurations and add a new configuration under Nios II Hardware. 
		- Select Project name: P2_PrototypeOS_WORKING
		- In Target Connection, under System ID checks, check "Ignore mismatched system ID" and "Ignore mismatched system timestamp". Alternatively, take tylenol now to avoid headache. 
		- Press Run.	
		
5.) Be impressed by our handy work. 


[Tuneables] 
These are found in P2PrototypeOS.h.  There are more than mentioned here, but those are self explanatory. 

• RELATIVE_SPEED_MULTIPLIER - Changes the timing of various aspects by using this multiplier. 

• MAX - Controls the busy loop timing in mythread() function. Controls how often to print: "This is message x of thread #x" 

• MAIN_DELAY - Controls the loop in the main function and decides how often to print:"This is the prototype os for my exciting CSE351 course projects!"

• THREAD_DELAY - Controls busy loops in mythread_join()

• QUANTUM_LENGTH - Controls how often the timer interrupt is fired.  Measured in milliseconds.  However, the resolution is only 100ms using the custom DE2_Basic_Computer.sof file. 

• SHOW_THREAD_STATS - Whether or not the following is displayed. Also whether or not thread runtimes are displayed. Set 1 to enable. 
	---------> Running=1 | Ready=0 | Waiting=0 | Done=12 | No Queued Threads
	
• SHOW_ITERRUPT_STATS - Whether or not the following is displayed. Set 1 to enable. 
	---------> Interrupted! Elapsed=1362 ticks, Period=10, Main Loop Iterations= 16610959.
	
	NOTE: Main Loop Iterations is the number of time the main loop has looped.  Will remain 0 until threads have joined, unless thread join is commented out.  


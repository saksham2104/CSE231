A simple shell  && scheduler that manages multiple processes using a queue.
	•	Supports graceful termination and tracks execution history.

Functionality

	•	Process Management: Handles the execution of multiple processes using a queue data structure.
	•	Signal Handling: Responds to SIGINT signals, allowing processes to finish before exiting.
	•	Execution History: Maintains a record of executed commands, including start and end times.

Data Structures

	•	Process Structure: Stores process information such as PID, execution time, and wait time.
	•	Queue Structure: Implements a circular queue to manage processes efficiently.
	•	Dynamic Array: Facilitates dynamic management of processes and their execution details.
	•	Command History: Tracks executed commands and their details, including timing and PID.

Signal Handling

	•	SIGINT Handler: Sets a flag upon receiving a SIGINT signal to indicate termination.
	•	SIGCHLD Handler: Waits for child processes to terminate, updating the command history accordingly.

How to Run

	1.	Compile the source files using a C compiler (e.g., gcc).
	2.	Execute the compiled program.(simple_shell)
	3.	Interact with the scheduler through shared memory.
	4.	Terminate the scheduler using Ctrl + C to trigger graceful shutdown.


contribution- saksham-scheduling algo and work on bonus,creating data structures
              atin-creating data structures(strut and queues)for shell and scheduler , updating simple_shell an creating other required files)


https://github.com/saksham2104/OS-Assignment-3.git

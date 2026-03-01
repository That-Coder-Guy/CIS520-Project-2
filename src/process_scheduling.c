#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "dyn_array.h"
#include "processing_scheduling.h"


// You might find this handy.  I put it around unused parameters, but you should
// remove it before you submit. Just allows things to compile initially.
#define UNUSED(x) (void)(x)

// private function
void virtual_cpu(ProcessControlBlock_t *process_control_block) 
{
	// decrement the burst time of the pcb
	--process_control_block->remaining_burst_time;
}

// Defines a function pointer type for non-preemptive CPU scheduler algorithms that select and extract the next
// process to run from the ready queue.
// \param: ready_queue - A dyn_array containing the pool of available processes
// \param: current_time - The current simulated CPU time used to determine process eligibility
// \param: object - A pointer to the destination where the extracted process block will be stored
// \return: True if a process was successfully selected and extracted, false otherwise
typedef bool (*process_selector_function_t)(dyn_array_t* ready_queue, unsigned long current_time, ProcessControlBlock_t* const process);

// Simulates a non-preemptive CPU scheduler by executing processes from the ready queue to completion using a provided selection algorithm.
// \param: ready_queue - A dyn_array of type ProcessControlBlock_t containing the processes to be scheduled
// \param: result - A pointer to a ScheduleResult_t structure where the calculated scheduling statistics will be stored
// \param: selector - A function pointer used to extract the next process from the ready queue based on a specific scheduling algorithm
// \return: True if the scheduling simulation completed successfully, false otherwise
static bool nonpreemptive_scheduler(dyn_array_t* ready_queue, ScheduleResult_t* result, process_selector_function_t selector)
{
	// Validate input values
	if (ready_queue == NULL || result == NULL) { return false; }
	size_t process_count = dyn_array_size(ready_queue);
	if (process_count == 0) { return false; }

	// Create CPU variables
	unsigned long current_time = 0;
	unsigned long total_waiting = 0;
	unsigned long total_turnaround = 0;

	// Loop until the ready queue has been emptied
	while (!dyn_array_empty(ready_queue))
	{
		// Select the next process to run
		ProcessControlBlock_t target_process;
		if (!selector(ready_queue, current_time, &target_process)) { return false; }

		// Skip to the arrival time if needed
		if (current_time < target_process.arrival) { current_time = target_process.arrival; }
		else { total_waiting += current_time - target_process.arrival; }

		// If a process was found then run it to completion
		while (target_process.remaining_burst_time > 0)
		{
			virtual_cpu(&target_process);
			current_time++;
		}
		total_turnaround += current_time - target_process.arrival;
	}

	// Set the result values
	result->average_waiting_time = (float)total_waiting / process_count;
	result->average_turnaround_time = (float)total_turnaround / process_count;
	result->total_run_time = current_time;
	
	return true;
}

// Extracts the process with the earliest arrival time from the queue.
// \param: ready_queue - A dyn_array of type ProcessControlBlock_t containing up to N elements
// \param: current_time - The current simulated CPU time used to determine process eligibility
// \param: object - A pointer to the destination where the extracted process block will be stored
// \return: True if the extraction was successful, false otherwise
static bool select_earliest_arrival(dyn_array_t* ready_queue, unsigned long current_time, ProcessControlBlock_t* const process)
{
	// Mark current_time as intentionally unused for the compiler
	(void)current_time;

	// Validate input values
	if (ready_queue == NULL || dyn_array_empty(ready_queue) || process == NULL) { return false; }
	
	// Find the process with the earliest arrival time
	size_t prime_candidate_index = 0;
	for (size_t i = 1; i < dyn_array_size(ready_queue); i++)
	{
		ProcessControlBlock_t* prime_candidate_process = dyn_array_at(ready_queue, prime_candidate_index);
		ProcessControlBlock_t* candidate_process = dyn_array_at(ready_queue, i);
		if (prime_candidate_process->arrival > candidate_process->arrival)
		{
			prime_candidate_index = i;
		}
	}

	// Store the selected process in the memory location provided
	dyn_array_extract(ready_queue, prime_candidate_index, process);

	return true;
}

// Runs First Come First Served algorithm.
// \param: ready_queue - A dyn_array of type ProcessControlBlock_t containing up to N elements
// \param: result - Result used for stat tracking
// \return: True if function ran successful, false otherwise
bool first_come_first_serve(dyn_array_t* ready_queue, ScheduleResult_t* result) 
{
	return nonpreemptive_scheduler(ready_queue, result, select_earliest_arrival);
}

bool shortest_job_first(dyn_array_t *ready_queue, ScheduleResult_t *result) 
{
	UNUSED(ready_queue);
	UNUSED(result);
	return false;
}

// Extracts the highest priority process that is in the ready queue, or the earliest future process if the CPU is idle.
// \param: ready_queue - A dyn_array of type ProcessControlBlock_t containing up to N elements
// \param: current_time - The current simulated CPU time used to determine process eligibility
// \param: object - A pointer to the destination where the extracted process block will be stored
// \return: True if the extraction was successful, false otherwise
static bool select_highest_priority(dyn_array_t* ready_queue, unsigned long current_time, ProcessControlBlock_t* const process)
{
	// Validate input values
	if (ready_queue == NULL || dyn_array_empty(ready_queue) || process == NULL) { return false; }
	
	// Find the process with the highest priority
	size_t prime_candidate_index = 0;
	for (size_t i = 1; i < dyn_array_size(ready_queue); i++)
	{
		// Acquire two processes to compare
		ProcessControlBlock_t* prime_candidate_process = dyn_array_at(ready_queue, prime_candidate_index);
		ProcessControlBlock_t* candidate_process = dyn_array_at(ready_queue, i);
		bool prime_candidate_arrived = prime_candidate_process->arrival <= current_time;
		bool candidate_arrived = candidate_process->arrival <= current_time;
		
		// If both process have arrived determine which one has higher priority 
		if (prime_candidate_arrived && candidate_arrived)
		{
			if (prime_candidate_process->priority > candidate_process->priority || 
                (prime_candidate_process->priority == candidate_process->priority && 
                prime_candidate_process->arrival > candidate_process->arrival))
            {
                prime_candidate_index = i;
            }
		}
		// If neither have arrived determine which one has the earlier arrival time
		else if (!prime_candidate_arrived && !candidate_arrived && prime_candidate_process->arrival > candidate_process->arrival)
		{
			prime_candidate_index = i;
		}
		// If only one arrived then it gets priority
		else if (candidate_arrived)
		{
			prime_candidate_index = i;
		}
	}

	// Store the selected process in the memory location provided
	dyn_array_extract(ready_queue, prime_candidate_index, process);

	return true;
}

// Runs the non-preemptive Priority algorithm over the incoming ready_queue.
// \param: ready_queue - a dyn_array of type ProcessControlBlock_t that contain be up to N elements
// \param: result - used for shortest job first stat tracking \ref ScheduleResult_t
// \return: true if function ran successful else false for an error
bool priority(dyn_array_t* ready_queue, ScheduleResult_t* result) 
{
	return nonpreemptive_scheduler(ready_queue, result, select_highest_priority);
}

bool round_robin(dyn_array_t *ready_queue, ScheduleResult_t *result, size_t quantum) 
{
	if (!ready_queue || !result || quantum == 0) {
		return false;
	}

	dyn_array_t *rr_queue = dyn_array_create(0, sizeof(ProcessControlBlock_t), NULL);
	if (!rr_queue) {
		return false;
	}

	unsigned long current_time = 0;
	unsigned long total_waiting = 0;
	unsigned long total_turnaround = 0;
	size_t num_processes = dyn_array_size(ready_queue);
	if (num_processes == 0) {
		dyn_array_destroy(rr_queue);
		return false;
	}

	// Represents pcb at i
	ProcessControlBlock_t *pcb = malloc(sizeof(ProcessControlBlock_t));
	if (!pcb) {
		dyn_array_destroy(rr_queue);
		return false;
	}

	// Represents the pcb that we execute from rr_queue
	ProcessControlBlock_t *round = malloc(sizeof(ProcessControlBlock_t));
	if (!round) {
		dyn_array_destroy(rr_queue);
		free(pcb);
		return false;
	}

	// Tracks how many processes are still in ready_queue
	size_t i = 0;
	// Tracks if we executed a pcb from rr_queue
	bool flag = false;
	// Continue until rr_queue is empty and every pcb has arrived
	while (!dyn_array_empty(rr_queue) || i < num_processes) {
		// Execute the front pcb in rr_queue
		flag = false;
		if (!dyn_array_empty(rr_queue)) {
			flag = true;
			dyn_array_extract_front(rr_queue, round);
			size_t rr_size = dyn_array_size(rr_queue);
			if (round->remaining_burst_time > quantum) {
				for (size_t j = 0; j < quantum; j++) {
					virtual_cpu(round);
					current_time++;
					// For every unit of time this pcb is executed, every other pcb
					// in rr_queue waits that amount of time
					total_waiting += rr_size;
					// For every unit of time thsi pcb is executed, this pcb takes
					// that amount of time to execute (Hence the +1) and every other pcb in rr_queue
					// waits that amount of time
					total_turnaround += rr_size + 1;
				}
			} else {
				size_t remaining = round->remaining_burst_time;
				for (size_t j = 0; j < remaining; j++) {
					virtual_cpu(round);
					current_time++;
					// For every unit of time this pcb is executed, every other pcb
					// in rr_queue waits that amount of time
					total_waiting += rr_size;
					// For every unit of time thsi pcb is executed, this pcb takes
					// that amount of time to execute (Hence the +1) and every other pcb in rr_queue
					// waits that amount of time
					total_turnaround += rr_size + 1;
				}
			}
		} // If no pcb can be executed, fast-forward time
		else if (i < num_processes && ((ProcessControlBlock_t *)dyn_array_at(ready_queue, 0))->arrival > current_time) {
			current_time = ((ProcessControlBlock_t *)dyn_array_at(ready_queue, 0))->arrival;
		}

		// Add all pcb's that are waiting to the back of rr_queue
		while (i < num_processes && ((ProcessControlBlock_t *)dyn_array_at(ready_queue, 0))->arrival <= current_time) {
			dyn_array_extract_front(ready_queue, pcb);
			dyn_array_push_back(rr_queue, pcb);
			// This pcb had to wait until after 'round' finished executing
			total_waiting += current_time - pcb->arrival;
			total_turnaround += current_time - pcb->arrival;
			i++;
		}
		
		// If the pcb we executed has not terminated, put it at the back of the queue
		// Important that this happens after all pcb's that became available are loaded onto the queue
		if (flag && round->remaining_burst_time > 0) {
			dyn_array_push_back(rr_queue, round);
		}
	}

	dyn_array_destroy(rr_queue);
	free(pcb);
	free(round);

	result->average_waiting_time = (float)total_waiting / num_processes;
	result->average_turnaround_time = (float)total_turnaround / num_processes;
	result->total_run_time = current_time;

	return true;
}

// Selects and extracts the process from the ready queue with the shortest remaining time.
// \param: ready_queue - a pointer to the dynamic array containing the pool of processes
// \param: current_time - the current simulation time used to evaluate if a process has arrived
// \param: process - a pointer to the destination control block where the selected process data will be stored
// \return: true - if a process was successfully selected and extracted, else false on error or empty queue
static bool select_shortest_remaining_time(dyn_array_t* ready_queue, unsigned long current_time, ProcessControlBlock_t* const process)
{
	// Validate input values
	if (ready_queue == NULL || dyn_array_empty(ready_queue) || process == NULL) { return false; }

	// Find the process with the shortest burst time remaining 
	size_t prime_candidate_index = 0;
	for (size_t i = 1; i < dyn_array_size(ready_queue); i++)
	{
		// Acquire two processes to compare
		ProcessControlBlock_t* prime_candidate_process = dyn_array_at(ready_queue, prime_candidate_index);
		ProcessControlBlock_t* candidate_process = dyn_array_at(ready_queue, i);
		bool prime_candidate_arrived = prime_candidate_process->arrival <= current_time;
		bool candidate_arrived = candidate_process->arrival <= current_time;

		// If both process have arrived determine which one has the shorter burst time remaining
		if (prime_candidate_arrived && candidate_arrived)
		{
			if (prime_candidate_process->remaining_burst_time > candidate_process->remaining_burst_time || 
                (prime_candidate_process->remaining_burst_time == candidate_process->remaining_burst_time && 
                prime_candidate_process->arrival > candidate_process->arrival))
            {
                prime_candidate_index = i;
            }
		}
		// If neither have arrived determine which one has the earlier arrival time
		else if (!prime_candidate_arrived && !candidate_arrived)
		{
			if (prime_candidate_process->arrival > candidate_process->arrival)
            {
                prime_candidate_index = i;
            }
            else if (prime_candidate_process->arrival == candidate_process->arrival && 
                     prime_candidate_process->remaining_burst_time > candidate_process->remaining_burst_time)
            {
                prime_candidate_index = i;
            }
		}
		// If only one arrived then it gets priority
		else if (candidate_arrived)
		{
			prime_candidate_index = i;
		}
	}

	// Store the selected process in the memory location provided
	dyn_array_extract(ready_queue, prime_candidate_index, process);

	return true;
}

// Runs the preemptive Shortest Remaining Time First Process Scheduling algorithm over the incoming ready_queue
// \param: ready_queue - a dyn_array of type ProcessControlBlock_t that contain be up to N elements
// \param: result - used for shortest job first stat tracking \ref ScheduleResult_t
// \return: true if function ran successful else false for an error
// There is no guarantee that the passed dyn_array_t will be the result of your implementation of load_process_control_blocks
bool shortest_remaining_time_first(dyn_array_t* ready_queue, ScheduleResult_t* result) 
{
	// Validate input values
	if (ready_queue == NULL || result == NULL) { return false; }
	size_t process_count = dyn_array_size(ready_queue);
	if (process_count == 0) { return false; }

	// Create CPU variables
	unsigned long current_time = 0;
	unsigned long total_waiting = 0;
	unsigned long total_turnaround = 0;

	// Loop until the ready queue has been emptied
	while (!dyn_array_empty(ready_queue))
	{
		// Acquire the process with the shortest burst time remaining
		ProcessControlBlock_t target_process;
		if (!select_shortest_remaining_time(ready_queue, current_time, &target_process)) { return false; }

		// If the process has not been run on the CPU before then skip to the arrival time if needed and mark it as started
		if (!target_process.started)
		{
			if (current_time < target_process.arrival) { current_time = target_process.arrival; }
			else { total_waiting += current_time - target_process.arrival; }
			target_process.started = true;
		}
		
		// Run the process on the CPU
		virtual_cpu(&target_process);

		// For all preempted processes in the ready queue increment the total wait time by one
		for (size_t i = 0; i < dyn_array_size(ready_queue); i++)
		{
			ProcessControlBlock_t* process = dyn_array_at(ready_queue, i);
			if (process->started) { total_waiting++; }
		}

		// Increment the CPU clock
		current_time++;

		// Push the process back to the ready queue if it has not fnished
		if (target_process.remaining_burst_time > 0) { dyn_array_push_back(ready_queue, &target_process); }
		else { total_turnaround += current_time - target_process.arrival; }
	}

	// Set the result values
	result->average_waiting_time = (float)total_waiting / process_count;
	result->average_turnaround_time = (float)total_turnaround / process_count;
	result->total_run_time = current_time;

	return true;
}

// Reads a specified number of bytes from an open file descriptor into a destination buffer.
// \param: fd - the open file descriptor to read from
// \param: buffer - a pointer to the destination buffer where the data will be stored
// \param: count - the number of bytes to read from the file
// \return: true - if the requested number of bytes was successfully read, else false on error
static bool read_file_bytes(int fd, void* buffer, size_t count)
{
	// Validate input values
	if (fd == -1 || buffer == NULL) { return false; }

	// Loop and read until the requested number of bytes are read to the buffer
	uint8_t* buffer_bytes = (uint8_t*)buffer;
	size_t bytes_to_read = count;
	size_t total_bytes_read = 0;
	while (bytes_to_read > 0)
	{
		// Request the kernel to read up to the specified number of bytes from the file descriptor
		ssize_t bytes_read = read(fd, buffer_bytes + total_bytes_read, bytes_to_read);
		if (bytes_read > 0)
		{
			bytes_to_read -= bytes_read;
			total_bytes_read += bytes_read;
		}
		// If the read is stopped because of an interrupt ignore it
		else if (bytes_read == -1 && errno == EINTR) { continue; }
		// If the read fails return false
		else { return false; }
	}
	return true;
}

// Reads the PCB values from the binary file into ProcessControlBlock_t
// for N number of PCB entries stored in the file
// \param: input_file - the file containing the PCB burst times
// \return: a populated dyn_array of ProcessControlBlocks if function ran successful else NULL for an error
dyn_array_t* load_process_control_blocks(const char* input_file)
{
	// Validate input value
	if (input_file == NULL || input_file[0] == '\0' || (input_file[0] == '\n' && input_file[1] == '\0')) { return NULL; }

	// Acquire input file descriptor
	int fd = open(input_file, O_RDONLY);
	if (fd == -1) { return NULL; }

	// Calcualte file payload size then read payload to a dynamic array
	dyn_array_t* control_blocks = NULL;
	uint32_t control_block_count = 0;
	if (read_file_bytes(fd, &control_block_count, sizeof(uint32_t)) && control_block_count > 0) // Read first 4 bytes for control block count
	{
		// Allocate a storage array for the control blocks and read them into it
		control_blocks = dyn_array_create(control_block_count, sizeof(ProcessControlBlock_t), NULL);
		if (control_blocks != NULL)
		{
			for (uint32_t i = 0; i < control_block_count; i++)
			{
				// Allocate a control block struct to store the incoming data
				ProcessControlBlock_t* block = malloc(sizeof(ProcessControlBlock_t));
				if (block != NULL &&
					read_file_bytes(fd, &(block->remaining_burst_time), sizeof(uint32_t)) &&
					read_file_bytes(fd, &(block->priority), sizeof(uint32_t)) &&
					read_file_bytes(fd, &(block->arrival), sizeof(uint32_t))
					)
				{
					block->started = false;
					dyn_array_push_back(control_blocks, block);
					// dyn_array_push_back allocates memory to copy block into,
					// which is a separate space of memory, so we have to free block
					free(block);
				}
				else
				{
					dyn_array_destroy(control_blocks);
					free(block);
					control_blocks = NULL;
					break;
				}
			}
		}
	}

	// Close the file
	close(fd);

	// Return the control blocks
	return control_blocks;
}

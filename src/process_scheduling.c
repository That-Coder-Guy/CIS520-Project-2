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

bool first_come_first_serve(dyn_array_t *ready_queue, ScheduleResult_t *result) 
{
	UNUSED(ready_queue);
	UNUSED(result);
	return false;
}

bool shortest_job_first(dyn_array_t *ready_queue, ScheduleResult_t *result) 
{
	UNUSED(ready_queue);
	UNUSED(result);
	return false;
}

bool priority(dyn_array_t *ready_queue, ScheduleResult_t *result) 
{
	UNUSED(ready_queue);
	UNUSED(result);
	return false;
}

bool round_robin(dyn_array_t *ready_queue, ScheduleResult_t *result, size_t quantum) 
{
	UNUSED(ready_queue);
	UNUSED(result);
	UNUSED(quantum);
	return false;
}

dyn_array_t *load_process_control_blocks(const char *input_file) 
{
	// Validate input value
	if (input_file == NULL) { return NULL; }

	// Acquire input file descriptor
	int fd = open(input_file, O_RDONLY);
	if (fd == -1) { return NULL; }

	// Create a variable to store the number of control blocks in the input file
	uint32_t control_block_count = 0;
	uint8_t* count_bytes = (uint8_t*)&control_block_count; // Read friendly pointer

	// Determine the number of control blocks in the input file by reading the first 4 bytes
	size_t bytes_to_read = sizeof(uint32_t);
	size_t total_bytes_read = 0;
	while (bytes_to_read > 0)
	{
		ssize_t bytes_read = read(fd, count_bytes + total_bytes_read, bytes_to_read);
		if (bytes_read > 0)
		{
			bytes_to_read -= bytes_read;
			total_bytes_read += bytes_read;
		}
		else if (bytes_read == -1 && errno == EINTR) { continue; }
		else
		{
			close(fd);
			return NULL;
		}
	}

	// Allocate a storage array for the control blocks and read them from the input file
	size_t input_size = sizeof(ProcessControlBlock_t) * control_block_count;
	ProcessControlBlock_t* control_block_buffer = malloc(input_size);
	if (control_block_buffer == NULL)
	{
		close(fd);
		return NULL;
	}

	// Read all the control blocks into the storage array from the input file
	uint8_t* buffer_bytes = (uint8_t*)control_block_buffer;
	bytes_to_read = input_size;
	total_bytes_read = 0;
	while (bytes_to_read > 0)
	{
		ssize_t bytes_read = read(fd, buffer_bytes + total_bytes_read, bytes_to_read);
		if (bytes_read > 0)
		{
			bytes_to_read -= bytes_read;
			total_bytes_read += bytes_read;
		}
		else if (bytes_read == -1 && errno == EINTR) { continue; }
		else
		{
			free(control_block_buffer);
			close(fd);
			return NULL;
		}
	}

	// Transfer the control blocks to a dynamic array and free the temperary buffer
	dyn_array_t* control_blocks = dyn_array_import(control_block_buffer, control_block_count, sizeof(ProcessControlBlock_t), NULL);
	free(control_block_buffer);

	// Close the file
	close(fd);

	// Return control blocks
	return control_blocks;
}

bool shortest_remaining_time_first(dyn_array_t *ready_queue, ScheduleResult_t *result) 
{
	UNUSED(ready_queue);
	UNUSED(result);
	return false;
}

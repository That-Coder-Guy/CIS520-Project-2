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

static bool read_file_value(int fd, void* value, size_t count)
{
	// Validate input values
	if (fd == -1 || value == NULL) { return false; }

	// Loop and read until the requested number of bytes are read to the buffer
	uint8_t* value_bytes = (uint8_t*)value;
	size_t bytes_to_read = count;
	size_t total_bytes_read = 0;
	while (bytes_to_read > 0)
	{	
		// Request the kernel to read up to the specified number of bytes from the file descriptor
		ssize_t bytes_read = read(fd, value_bytes + total_bytes_read, bytes_to_read);
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

dyn_array_t* load_process_control_blocks(const char *input_file) 
{
	// Validate input value
	if (input_file == NULL || input_file[0] == '\0' || (input_file[0] == '\n' && input_file[1] == '\0')) { return NULL; }

	// Acquire input file descriptor
	int fd = open(input_file, O_RDONLY);
	if (fd == -1) { return NULL; }

	// Calcualte file payload size then read it to a dynamic array
	dyn_array_t* control_blocks = NULL;
	uint32_t control_block_count = 0;
	if (read_file_value(fd, &control_block_count, sizeof(uint32_t)) && control_block_count > 0) // Read first 4 bytes for control block count
	{
		// Allocate a temperary storage array for the control blocks and read them into it
		control_blocks = dyn_array_create(control_block_count, sizeof(ProcessControlBlock_t), NULL);
		if (control_blocks != NULL)
		{
			for (uint32_t i = 0; i < control_block_count; i++)
			{
				ProcessControlBlock_t* block = malloc(sizeof(ProcessControlBlock_t));
				if (block != NULL &&
					read_file_value(fd, &(block->remaining_burst_time), sizeof(uint32_t)) &&
					read_file_value(fd, &(block->priority), sizeof(uint32_t)) &&
					read_file_value(fd, &(block->arrival), sizeof(uint32_t))
				) 
				{
					block->started = false;
					dyn_array_push_back(control_blocks, block);
				}
				else
				{
					dyn_array_destroy(control_blocks);
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

bool shortest_remaining_time_first(dyn_array_t *ready_queue, ScheduleResult_t *result) 
{
	UNUSED(ready_queue);
	UNUSED(result);
	return false;
}

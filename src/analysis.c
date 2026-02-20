#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "dyn_array.h"
#include "processing_scheduling.h"

#define FCFS "FCFS"
#define P "P"
#define RR "RR"
#define SJF "SJF"
#define SRT "SRT"

// Add and comment your analysis code in this function.
// THIS IS NOT FINISHED.
int main(int argc, char **argv) 
{
	if (argc < 3) 
	{
		printf("%s <pcb file> <schedule algorithm> [quantum]\n", argv[0]);
		return EXIT_FAILURE;
	}

	// Assign and validate argument values
	char* filename = argv[1];
	char* algorithm_name = argv[2];
	if (filename == NULL || algorithm_name == NULL) { return EXIT_FAILURE; }

	// Assign optional argument
	size_t quantum = 0;
	if (argc > 3 && sscanf(argv[3], "%zu", &quantum) <= 0) { return EXIT_FAILURE; }

	// Extract schedule data from the provided file
	dyn_array_t* schedule = load_process_control_blocks(filename);
	if (schedule == NULL) { return EXIT_FAILURE; }

	// Allocate storage for the schedule algorithm result
	ScheduleResult_t* result = malloc(sizeof(ScheduleResult_t));
	if (result == NULL) { return EXIT_FAILURE; }

	// Call the correct scheduling algorithm on the provided file schedule data
	static const int MAX_ALGORITHM_NAME_LENGTH = 5;
	bool success = false;
	switch (strnlen(algorithm_name, MAX_ALGORITHM_NAME_LENGTH))
	{	
		case 1:
			if (strncmp(algorithm_name, P, 1) == 0)
			{
				success = priority(schedule, result);
			}
        	break;
		
		case 2:
			if (strncmp(algorithm_name, RR, 2) == 0)
			{
				if (quantum > 0) { success = round_robin(schedule, result, quantum); }
			}
        	break;

		case 3:
			if (strncmp(algorithm_name, SJF, 3) == 0)
			{
				success = shortest_job_first(schedule, result);
			}
			else if (strncmp(algorithm_name, SRT, 3) == 0)
			{
				success = shortest_remaining_time_first(schedule, result);
			}
			break;

		case 4:
			if (strncmp(algorithm_name, FCFS, 4) == 0)
			{
				success = first_come_first_serve(schedule, result);
			}
			break;
	}

	if (success) {
		printf("Average waiting time: %f\n", result->average_waiting_time);
		printf("Average turnaround time: %f\n", result->average_turnaround_time);
		printf("Total run time: %ld\n", result->total_run_time);
	}

	return success;
}

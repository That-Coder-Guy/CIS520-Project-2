#include <fcntl.h>
#include <stdio.h>
#include <pthread.h>
#include "gtest/gtest.h"
#include "../include/processing_scheduling.h"

// Using a C library requires extern "C" to prevent function mangling
extern "C"
{
#include <dyn_array.h>
}

#define NUM_PCB 30
#define QUANTUM 5 // Used for Robin Round for process as the run time limit

/*
unsigned int score;
unsigned int total;

class GradeEnvironment : public testing::Environment
{
	public:
		virtual void SetUp()
		{
			score = 0;
			total = 210;
		}

		virtual void TearDown()
		{
			::testing::Test::RecordProperty("points_given", score);
			::testing::Test::RecordProperty("points_total", total);
			std::cout << "SCORE: " << score << '/' << total << std::endl;
		}
};
*/

/*
*  FIRST COME FIRST SERVER UNIT TEST CASES
**/
TEST(FCFS, SingleProcess)
{
	dyn_array_t* ready_queue = dyn_array_create(0, sizeof(ProcessControlBlock_t) , NULL);
	ProcessControlBlock_t p1;
	p1.arrival = 0;
	p1.remaining_burst_time = 3;
	p1.priority = 0;
	p1.started = false; 

	dyn_array_push_back(ready_queue, &p1);

	ScheduleResult_t result;

	bool success = first_come_first_serve(ready_queue, &result);

	EXPECT_TRUE(success);
	EXPECT_NEAR(result.average_waiting_time, 0.0f , 0.01);
	EXPECT_NEAR(result.average_turnaround_time, 3.0f , 0.01);
	EXPECT_EQ(result.total_run_time, 3UL);

	dyn_array_destroy(ready_queue);
}

TEST(FCFS, SimpleSequentialArrival)
{
	dyn_array_t* ready_queue = dyn_array_create(0, sizeof(ProcessControlBlock_t) , NULL);
	ProcessControlBlock_t p1;
	p1.arrival = 0;
	p1.remaining_burst_time = 5;
	p1.priority = 0;
	p1.started = false; 
	ProcessControlBlock_t p2;
	p2.arrival = 1;
	p2.remaining_burst_time = 3;
	p2.priority = 0;
	p2.started = false; 
	ProcessControlBlock_t p3;
	p3.arrival = 2;
	p3.remaining_burst_time = 2;
	p3.priority = 0;
	p3.started = false; 

	dyn_array_push_back(ready_queue, &p1);
	dyn_array_push_back(ready_queue, &p2);
	dyn_array_push_back(ready_queue, &p3);

	ScheduleResult_t result;

	bool success = first_come_first_serve(ready_queue, &result);

	EXPECT_TRUE(success);
	EXPECT_NEAR(result.average_waiting_time, 3.33f , 0.01);
	EXPECT_NEAR(result.average_turnaround_time, 6.67f , 0.01);
	EXPECT_EQ(result.total_run_time, 10UL);

	dyn_array_destroy(ready_queue);
}

TEST(FCFS, HandlesIdleTime)
{
	dyn_array_t* ready_queue = dyn_array_create(0, sizeof(ProcessControlBlock_t) , NULL);
	ProcessControlBlock_t p1;
	p1.arrival = 0;
	p1.remaining_burst_time = 4;
	p1.priority = 0;
	p1.started = false; 
	ProcessControlBlock_t p2;
	p2.arrival = 6;
	p2.remaining_burst_time = 3;
	p2.priority = 0;
	p2.started = false; 


	dyn_array_push_back(ready_queue, &p1);
	dyn_array_push_back(ready_queue, &p2);

	ScheduleResult_t result;

	bool success = first_come_first_serve(ready_queue, &result);

	EXPECT_TRUE(success);
	EXPECT_NEAR(result.average_waiting_time, 0.0f , 0.01);
	EXPECT_NEAR(result.average_turnaround_time, 3.5f , 0.01);
	EXPECT_EQ(result.total_run_time, 9UL);

	dyn_array_destroy(ready_queue);
}

TEST(FCFS, EmptyQueue)
{
	dyn_array_t* ready_queue = dyn_array_create(0, sizeof(ProcessControlBlock_t) , NULL);

	ScheduleResult_t result;

	bool success = first_come_first_serve(ready_queue, &result);

	EXPECT_FALSE(success);

	dyn_array_destroy(ready_queue);
}

TEST(FCFS, SameTimeArrival)
{
	dyn_array_t* ready_queue = dyn_array_create(0, sizeof(ProcessControlBlock_t) , NULL);
	ProcessControlBlock_t p1;
	p1.arrival = 0;
	p1.remaining_burst_time = 3;
	p1.priority = 0;
	p1.started = false; 
	ProcessControlBlock_t p2;
	p2.arrival = 0;
	p2.remaining_burst_time = 5;
	p2.priority = 0;
	p2.started = false; 
	ProcessControlBlock_t p3;
	p3.arrival = 0;
	p3.remaining_burst_time = 2;
	p3.priority = 0;
	p3.started = false; 

	dyn_array_push_back(ready_queue, &p1);
	dyn_array_push_back(ready_queue, &p2);
	dyn_array_push_back(ready_queue, &p3);

	ScheduleResult_t result;

	bool success = first_come_first_serve(ready_queue, &result);

	EXPECT_TRUE(success);
	EXPECT_NEAR(result.average_waiting_time, 3.67f , 0.01);
	EXPECT_NEAR(result.average_turnaround_time, 7.0f , 0.01);
	EXPECT_EQ(result.total_run_time, 10UL);

	dyn_array_destroy(ready_queue);
}

TEST(FCFS, LargeGapBetweenArrival)
{
	dyn_array_t* ready_queue = dyn_array_create(0, sizeof(ProcessControlBlock_t) , NULL);
	ProcessControlBlock_t p1;
	p1.arrival = 0;
	p1.remaining_burst_time = 5;
	p1.priority = 0;
	p1.started = false; 
	ProcessControlBlock_t p2;
	p2.arrival = 100;
	p2.remaining_burst_time = 3;
	p2.priority = 0;
	p2.started = false; 

	dyn_array_push_back(ready_queue, &p1);
	dyn_array_push_back(ready_queue, &p2);

	ScheduleResult_t result;

	bool success = first_come_first_serve(ready_queue, &result);

	EXPECT_TRUE(success);
	EXPECT_NEAR(result.average_waiting_time, 0.0f , 0.01);
	EXPECT_NEAR(result.average_turnaround_time, 4.0f , 0.01);
	EXPECT_EQ(result.total_run_time, 103UL);

	dyn_array_destroy(ready_queue);
}

/*
*  PRIORITY UNIT TEST CASES
**/
TEST(priority, NullReadyQueue) {
	ScheduleResult_t result;
	EXPECT_FALSE(priority(NULL, &result));
}

TEST(priority, NullScheduleResult) {
	ProcessControlBlock_t data[] = {
		{ .remaining_burst_time = 10, .priority = 0, .arrival = 5, .started = false },
		{ .remaining_burst_time = 10, .priority = 1, .arrival = 6, .started = false }
	};
	dyn_array_t* ready_queue = dyn_array_import(data, 2, sizeof(ProcessControlBlock_t), NULL);

	EXPECT_FALSE(priority(ready_queue, NULL));

	dyn_array_destroy(ready_queue);
}

TEST(priority, EmptyReadyQueue)
{
	dyn_array_t* ready_queue = dyn_array_create(0, sizeof(ProcessControlBlock_t) , NULL);
	ScheduleResult_t result;

	EXPECT_FALSE(priority(ready_queue, &result));

	dyn_array_destroy(ready_queue);
}

TEST(priority, ValidReadyQueueData) {
	ProcessControlBlock_t data[] = {
		{ .remaining_burst_time = 5, .priority = 2, .arrival = 0, .started = false },
		{ .remaining_burst_time = 3, .priority = 1, .arrival = 1, .started = false },
		{ .remaining_burst_time = 4, .priority = 1, .arrival = 2, .started = false },
		{ .remaining_burst_time = 2, .priority = 0, .arrival = 15, .started = false }
	};
	dyn_array_t* ready_queue = dyn_array_import(data, 4, sizeof(ProcessControlBlock_t), NULL);
	ScheduleResult_t result;

	EXPECT_TRUE(priority(ready_queue, &result));

	dyn_array_destroy(ready_queue);

	EXPECT_NEAR(result.average_waiting_time, 2.5f , 0.01);
	EXPECT_NEAR(result.average_turnaround_time, 6.0f , 0.01);
	EXPECT_EQ(result.total_run_time, 17UL);
}

TEST(priority, IdenticalPriorityAndArrival) {
    ProcessControlBlock_t data[] = {
        { .remaining_burst_time = 4, .priority = 2, .arrival = 0, .started = false },
        { .remaining_burst_time = 3, .priority = 1, .arrival = 2, .started = false },
        { .remaining_burst_time = 3, .priority = 1, .arrival = 2, .started = false },
        { .remaining_burst_time = 5, .priority = 0, .arrival = 3, .started = false }
    };
    dyn_array_t* ready_queue = dyn_array_import(data, 4, sizeof(ProcessControlBlock_t), NULL);
    ScheduleResult_t result;

    EXPECT_TRUE(priority(ready_queue, &result));

    dyn_array_destroy(ready_queue);

    EXPECT_NEAR(result.average_waiting_time, 4.5f, 0.01);
    EXPECT_NEAR(result.average_turnaround_time, 8.25f, 0.01);
    EXPECT_EQ(result.total_run_time, 15UL);
}

TEST(priority, LargeReadyQueue) {
    ProcessControlBlock_t data[] = {
        { .remaining_burst_time = 2, .priority = 3, .arrival = 0, .started = false },
        { .remaining_burst_time = 3, .priority = 1, .arrival = 1, .started = false },
        { .remaining_burst_time = 3, .priority = 1, .arrival = 1, .started = false },
        { .remaining_burst_time = 4, .priority = 0, .arrival = 3, .started = false },
        { .remaining_burst_time = 2, .priority = 2, .arrival = 5, .started = false },
        { .remaining_burst_time = 2, .priority = 2, .arrival = 5, .started = false }, 
        { .remaining_burst_time = 5, .priority = 4, .arrival = 8, .started = false },
        { .remaining_burst_time = 5, .priority = 4, .arrival = 8, .started = false }
    };
    dyn_array_t* ready_queue = dyn_array_import(data, 8, sizeof(ProcessControlBlock_t), NULL);
    ScheduleResult_t result;

    EXPECT_TRUE(priority(ready_queue, &result));

    dyn_array_destroy(ready_queue);

    EXPECT_NEAR(result.average_waiting_time, 6.0f, 0.01);
    EXPECT_NEAR(result.average_turnaround_time, 9.25f, 0.01);
    EXPECT_EQ(result.total_run_time, 26UL);
}

/*
*  SHORTEST TIME REMAINING FIRST UNIT TEST CASES
**/
TEST(shortest_remaining_time_first, NullReadyQueue) {
	ScheduleResult_t result;
	EXPECT_FALSE(shortest_remaining_time_first(NULL, &result));
}

TEST(shortest_remaining_time_first, NullScheduleResult) {
	ProcessControlBlock_t data[] = {
		{ .remaining_burst_time = 10, .priority = 0, .arrival = 5, .started = false },
		{ .remaining_burst_time = 10, .priority = 1, .arrival = 6, .started = false }
	};
	dyn_array_t* ready_queue = dyn_array_import(data, 2, sizeof(ProcessControlBlock_t), NULL);

	EXPECT_FALSE(shortest_remaining_time_first(ready_queue, NULL));

	dyn_array_destroy(ready_queue);
}

TEST(shortest_remaining_time_first, EmptyReadyQueue)
{
	dyn_array_t* ready_queue = dyn_array_create(0, sizeof(ProcessControlBlock_t) , NULL);
	ScheduleResult_t result;

	EXPECT_FALSE(shortest_remaining_time_first(ready_queue, &result));

	dyn_array_destroy(ready_queue);
}

TEST(shortest_remaining_time_first, ValidReadyQueueData) {
    ProcessControlBlock_t data[] = {
        { .remaining_burst_time = 8, .priority = 0, .arrival = 0, .started = false },
        { .remaining_burst_time = 4, .priority = 0, .arrival = 1, .started = false },
        { .remaining_burst_time = 9, .priority = 0, .arrival = 2, .started = false },
        { .remaining_burst_time = 5, .priority = 0, .arrival = 3, .started = false }
    };
    dyn_array_t* ready_queue = dyn_array_import(data, 4, sizeof(ProcessControlBlock_t), NULL);
    ScheduleResult_t result;

    EXPECT_TRUE(shortest_remaining_time_first(ready_queue, &result));

    dyn_array_destroy(ready_queue);

    EXPECT_NEAR(result.average_waiting_time, 6.5f, 0.01); 
    EXPECT_NEAR(result.average_turnaround_time, 13.0f, 0.01);
    EXPECT_EQ(result.total_run_time, 26UL);
}

TEST(shortest_remaining_time_first, IdenticalTimeRemainingAndArrival) {
    ProcessControlBlock_t data[] = {
        { .remaining_burst_time = 10, .priority = 0, .arrival = 0, .started = false },
        { .remaining_burst_time = 2,  .priority = 0, .arrival = 2, .started = false },
        { .remaining_burst_time = 2,  .priority = 0, .arrival = 2, .started = false },
        { .remaining_burst_time = 1,  .priority = 0, .arrival = 5, .started = false }
    };
    dyn_array_t* ready_queue = dyn_array_import(data, 4, sizeof(ProcessControlBlock_t), NULL);
    ScheduleResult_t result;

    EXPECT_TRUE(shortest_remaining_time_first(ready_queue, &result));

    dyn_array_destroy(ready_queue);

    EXPECT_NEAR(result.average_waiting_time, 2.0f, 0.01);
    EXPECT_NEAR(result.average_turnaround_time, 5.75f, 0.01);
    EXPECT_EQ(result.total_run_time, 15UL);
}

TEST(shortest_remaining_time_first, LargeReadyQueueWithIdleTime) {
    ProcessControlBlock_t data[] = {
        { .remaining_burst_time = 3, .priority = 0, .arrival = 0,  .started = false },
        { .remaining_burst_time = 2, .priority = 0, .arrival = 1,  .started = false },
        { .remaining_burst_time = 1, .priority = 0, .arrival = 2,  .started = false },
        { .remaining_burst_time = 4, .priority = 0, .arrival = 3,  .started = false },
        { .remaining_burst_time = 2, .priority = 0, .arrival = 4,  .started = false },
        { .remaining_burst_time = 1, .priority = 0, .arrival = 5,  .started = false },
        { .remaining_burst_time = 2, .priority = 0, .arrival = 15, .started = false },
        { .remaining_burst_time = 2, .priority = 0, .arrival = 15, .started = false }
    };
    dyn_array_t* ready_queue = dyn_array_import(data, 8, sizeof(ProcessControlBlock_t), NULL);
    ScheduleResult_t result;

    EXPECT_TRUE(shortest_remaining_time_first(ready_queue, &result));

    dyn_array_destroy(ready_queue);

    EXPECT_NEAR(result.average_waiting_time, 2.0f, 0.01);
    EXPECT_NEAR(result.average_turnaround_time, 4.125f, 0.01);
    EXPECT_EQ(result.total_run_time, 19UL); 
}

/*
 * ROUND ROBIN UNIT TEST CASES
*/
TEST(RR, SingleProcessEqualToQuantum) {
	ProcessControlBlock_t data[] = {
		{ .remaining_burst_time = QUANTUM, .priority = 0, .arrival = 0, .started = false }
	};
	dyn_array_t *ready_queue = dyn_array_import(data, 1, sizeof(ProcessControlBlock_t), NULL);
	ScheduleResult_t result;
	
	EXPECT_TRUE(round_robin(ready_queue, &result, QUANTUM));
	EXPECT_NEAR(result.average_waiting_time, 0.0f, 0.01);
	EXPECT_NEAR(result.average_turnaround_time, 5.0f, 0.01);
	EXPECT_EQ(result.total_run_time, 5UL);
	
	dyn_array_destroy(ready_queue);
}
TEST(RR, SingleProcessShorterThanQuantum) {
	ProcessControlBlock_t data[] = {
		{ .remaining_burst_time = QUANTUM - 3, .priority = 0, .arrival = 0, .started = false }
	};
	dyn_array_t *ready_queue = dyn_array_import(data, 1, sizeof(ProcessControlBlock_t), NULL);
	ScheduleResult_t result;

	EXPECT_TRUE(round_robin(ready_queue, &result, QUANTUM));
	EXPECT_NEAR(result.average_waiting_time, 0.0f, 0.01);
	EXPECT_NEAR(result.average_turnaround_time, 2.0f, 0.01);
	EXPECT_EQ(result.total_run_time, 2UL);

	dyn_array_destroy(ready_queue);
}
TEST(RR, SingleProcessLongerThanQuantum) {
	ProcessControlBlock_t data[] = {
		{ .remaining_burst_time = QUANTUM + 3, .priority = 0, .arrival = 0, .started = false }
	};
	dyn_array_t *ready_queue = dyn_array_import(data, 1, sizeof(ProcessControlBlock_t), NULL);
	ScheduleResult_t result;

	EXPECT_TRUE(round_robin(ready_queue, &result, QUANTUM));
	EXPECT_NEAR(result.average_waiting_time, 0.0f, 0.01);
	EXPECT_NEAR(result.average_turnaround_time, 8.0f, 0.01);
	EXPECT_EQ(result.total_run_time, 8UL);

	dyn_array_destroy(ready_queue);
}
TEST(RR, SimpleSequentialArrivalEqualQuantum) {
	ProcessControlBlock_t data[] = {
		{ .remaining_burst_time = QUANTUM, .priority = 0, .arrival = 0, .started = false },
		{ .remaining_burst_time = QUANTUM, .priority = 0, .arrival = 1, .started = false },
		{ .remaining_burst_time = QUANTUM, .priority = 0, .arrival = 2, .started = false }
	};
	dyn_array_t *ready_queue = dyn_array_import(data, 3, sizeof(ProcessControlBlock_t), NULL);
	ScheduleResult_t result;

	EXPECT_TRUE(round_robin(ready_queue, &result, QUANTUM));
	EXPECT_NEAR(result.average_waiting_time, 4.0f, 0.1);
	EXPECT_NEAR(result.average_turnaround_time, 9.0f, 0.1);
	EXPECT_EQ(result.total_run_time, 15UL);

	dyn_array_destroy(ready_queue);
}
TEST(RR, SimpleSequentialArrivalVariableBurst) {
	ProcessControlBlock_t data[] = {
		{ .remaining_burst_time = QUANTUM + 3, .priority = 0, .arrival = 0, .started = false },
		{ .remaining_burst_time = QUANTUM, .priority = 0, .arrival = 1, .started = false },
		{ .remaining_burst_time = QUANTUM - 3, .priority = 0, .arrival = 2, .started = false }
	};
	dyn_array_t *ready_queue = dyn_array_import(data, 3, sizeof(ProcessControlBlock_t), NULL);
	ScheduleResult_t result;

	EXPECT_TRUE(round_robin(ready_queue, &result, QUANTUM));
	EXPECT_NEAR(result.average_waiting_time, 6.33f, 0.1);
	EXPECT_NEAR(result.average_turnaround_time, 11.33f, 0.1);
	EXPECT_EQ(result.total_run_time, 15UL);

	dyn_array_destroy(ready_queue);
}
TEST(RR, LongIdleTime) {
	ProcessControlBlock_t data[] = {
		{ .remaining_burst_time = QUANTUM, .priority = 0, .arrival = 0, .started = false },
		{ .remaining_burst_time = QUANTUM, .priority = 0, .arrival = 20, .started = false }
	};
	dyn_array_t *ready_queue = dyn_array_import(data, 2, sizeof(ProcessControlBlock_t), NULL);
	ScheduleResult_t result;

	EXPECT_TRUE(round_robin(ready_queue, &result, QUANTUM));
	EXPECT_NEAR(result.average_waiting_time, 0.0f, 0.1);
	EXPECT_NEAR(result.average_turnaround_time, 5.0f, 0.1);
	EXPECT_EQ(result.total_run_time, 25UL);

	dyn_array_destroy(ready_queue);
}
TEST(RR, SameTimeArivalEqualQuantum) {
	ProcessControlBlock_t data[] = {
		{ .remaining_burst_time = QUANTUM, .priority = 0, .arrival = 0, .started = false },
		{ .remaining_burst_time = QUANTUM, .priority = 0, .arrival = 0, .started = false },
		{ .remaining_burst_time = QUANTUM, .priority = 0, .arrival = 0, .started = false }
	};
	dyn_array_t *ready_queue = dyn_array_import(data, 3, sizeof(ProcessControlBlock_t), NULL);
	ScheduleResult_t result;

	EXPECT_TRUE(round_robin(ready_queue, &result, QUANTUM));
	EXPECT_NEAR(result.average_waiting_time, 5.0f, 0.1);
	EXPECT_NEAR(result.average_turnaround_time, 10.0f, 0.1);
	EXPECT_EQ(result.total_run_time, 15UL);

	dyn_array_destroy(ready_queue);
}
TEST(RR, SameTimeArivalVariableBurst) {
	ProcessControlBlock_t data[] = {
		{ .remaining_burst_time = QUANTUM + 3, .priority = 0, .arrival = 0, .started = false },
		{ .remaining_burst_time = QUANTUM, .priority = 0, .arrival = 0, .started = false },
		{ .remaining_burst_time = QUANTUM - 3, .priority = 0, .arrival = 0, .started = false }
	};
	dyn_array_t *ready_queue = dyn_array_import(data, 3, sizeof(ProcessControlBlock_t), NULL);
	ScheduleResult_t result;

	EXPECT_TRUE(round_robin(ready_queue, &result, QUANTUM));
	EXPECT_NEAR(result.average_waiting_time, 7.33f, 0.1);
	EXPECT_NEAR(result.average_turnaround_time, 12.33f, 0.1);
	EXPECT_EQ(result.total_run_time, 15UL);

	dyn_array_destroy(ready_queue);
}
TEST(RR, LargeValidQueue) {
	ProcessControlBlock_t data[] = {
		{ .remaining_burst_time = 13, .priority = 0, .arrival = 0, .started = false },
		{ .remaining_burst_time = 13, .priority = 0, .arrival = 4, .started = false },
		{ .remaining_burst_time = 16, .priority = 0, .arrival = 8, .started = false },
		{ .remaining_burst_time = 11, .priority = 0, .arrival = 10, .started = false },
		{ .remaining_burst_time = 12, .priority = 0, .arrival = 10, .started = false },
		{ .remaining_burst_time = 11, .priority = 0, .arrival = 11, .started = false },
		{ .remaining_burst_time = 11, .priority = 0, .arrival = 11, .started = false },
		{ .remaining_burst_time = 13, .priority = 0, .arrival = 13, .started = false },		
		{ .remaining_burst_time = 9, .priority = 0, .arrival = 16, .started = false },
		{ .remaining_burst_time = 11, .priority = 0, .arrival = 18, .started = false }
	};
	dyn_array_t *ready_queue = dyn_array_import(data, 10, sizeof(ProcessControlBlock_t), NULL);
	ScheduleResult_t result;

	EXPECT_TRUE(round_robin(ready_queue, &result, QUANTUM));
	EXPECT_NEAR(result.average_waiting_time, 82.3f, 0.1);
	EXPECT_NEAR(result.average_turnaround_time, 94.3f, 0.1);
	EXPECT_EQ(result.total_run_time, 120UL);

	dyn_array_destroy(ready_queue);
}
TEST(RR, NullReadyQueue) {
	dyn_array_t *ready_queue = NULL;
	ScheduleResult_t result;

	EXPECT_FALSE(round_robin(ready_queue, &result, QUANTUM));
}
TEST(RR, EmptyReadyQueue) {
	dyn_array_t *ready_queue = dyn_array_create(0, sizeof(ProcessControlBlock_t), NULL);
	ScheduleResult_t result;

	EXPECT_FALSE(round_robin(ready_queue, &result, QUANTUM));

	dyn_array_destroy(ready_queue);
}
TEST(RR, NullResult) {
	ProcessControlBlock_t data[] = {
		{ .remaining_burst_time = QUANTUM, .priority = 0, .arrival = 0, .started = false },
	};
	dyn_array_t *ready_queue = dyn_array_import(data, 1, sizeof(ProcessControlBlock_t), NULL);
	ScheduleResult_t *result = NULL;

	EXPECT_FALSE(round_robin(ready_queue, result, QUANTUM));

	dyn_array_destroy(ready_queue);
}
TEST(RR, ZeroQuantum) {
	ProcessControlBlock_t data[] = {
		{ .remaining_burst_time = QUANTUM, .priority = 0, .arrival = 0, .started = false },
	};
	dyn_array_t *ready_queue = dyn_array_import(data, 1, sizeof(ProcessControlBlock_t), NULL);
	ScheduleResult_t result;

	EXPECT_FALSE(round_robin(ready_queue, &result, 0));

	dyn_array_destroy(ready_queue);
}

/*
*  LOAD PROCESS CONTROL BLOCKS UNIT TEST CASES
**/
TEST(load_process_control_blocks, NullFileName) {
	EXPECT_EQ(NULL, load_process_control_blocks(NULL));
}

TEST(load_process_control_blocks, EmptyFileName) {
	EXPECT_EQ(NULL, load_process_control_blocks(""));
}

TEST(load_process_control_blocks, InvalidFileName) {
	EXPECT_EQ(NULL, load_process_control_blocks("\n"));
}

TEST(load_process_control_blocks, NonExistentFileName) {
	EXPECT_EQ(NULL, load_process_control_blocks("../test/fake.bin"));
}

TEST(load_process_control_blocks, InvalidSizeData) {
	EXPECT_EQ(NULL, load_process_control_blocks("../test/invalid_size.bin"));
}

TEST(load_process_control_blocks, InvalidControlBlockData) {
	EXPECT_EQ(NULL, load_process_control_blocks("../test/invalid_control_block.bin"));
}

TEST(load_process_control_blocks, ValidRead) {
	dyn_array_t* data = load_process_control_blocks("../test/valid.bin");
	EXPECT_NE((dyn_array_t*)NULL, data);
	EXPECT_EQ((size_t)4, dyn_array_size(data));
	EXPECT_EQ((size_t)16, dyn_array_capacity(data));

	ProcessControlBlock_t* block1 = (ProcessControlBlock_t*)dyn_array_at(data, 0);
	EXPECT_EQ((uint32_t)15, block1->remaining_burst_time);
	EXPECT_EQ((uint32_t)0, block1->priority);
	EXPECT_EQ((uint32_t)0, block1->arrival);
	EXPECT_EQ(false, block1->started);

	ProcessControlBlock_t* block2 = (ProcessControlBlock_t*)dyn_array_at(data, 1);
	EXPECT_EQ((uint32_t)10, block2->remaining_burst_time);
	EXPECT_EQ((uint32_t)0, block2->priority);
	EXPECT_EQ((uint32_t)1, block2->arrival);
	EXPECT_EQ(false, block2->started);

	ProcessControlBlock_t* block3 = (ProcessControlBlock_t*)dyn_array_at(data, 2);
	EXPECT_EQ((uint32_t)5, block3->remaining_burst_time);
	EXPECT_EQ((uint32_t)0, block3->priority);
	EXPECT_EQ((uint32_t)2, block3->arrival);
	EXPECT_EQ(false, block3->started);

	ProcessControlBlock_t* block4 = (ProcessControlBlock_t*)dyn_array_at(data, 3);
	EXPECT_EQ((uint32_t)20, block4->remaining_burst_time);
	EXPECT_EQ((uint32_t)0, block4->priority);
	EXPECT_EQ((uint32_t)3, block4->arrival);
	EXPECT_EQ(false, block4->started);
}

int main(int argc, char **argv)
{
	::testing::InitGoogleTest(&argc, argv);
	// ::testing::AddGlobalTestEnvironment(new GradeEnvironment);
	return RUN_ALL_TESTS();
}

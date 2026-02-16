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
*  LOAD PROCESS CONTROL BLOCKS UNIT TEST CASES
**/
TEST (load_process_control_blocks, NullFileName) {
	EXPECT_EQ(NULL, load_process_control_blocks(NULL));
}

TEST (load_process_control_blocks, EmptyFileName) {
	EXPECT_EQ(NULL, load_process_control_blocks(""));
}

TEST (load_process_control_blocks, InvalidFileName) {
	EXPECT_EQ(NULL, load_process_control_blocks("\n"));
}

TEST (load_process_control_blocks, NonExistentFileName) {
	EXPECT_EQ(NULL, load_process_control_blocks("../test/fake.bin"));
}

TEST (load_process_control_blocks, InvalidSizeData) {
	EXPECT_EQ(NULL, load_process_control_blocks("../test/invalid_size.bin"));
}

TEST (load_process_control_blocks, InvalidControlBlockData) {
	EXPECT_EQ(NULL, load_process_control_blocks("../test/invalid_control_block.bin"));
}

TEST (load_process_control_blocks, ValidRead) {
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

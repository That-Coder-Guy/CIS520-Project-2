cmake_minimum_required(VERSION 3.5)

# ALL OF THE SPECIFIED NAMES IN THIS CMAKE ARE REQUIRED.
# Set the project name variable to hw2.
# THIS IS REQUIRED
project(hw2)

# Add your CMAKE_C_FLAGS AND CMAKE_CXX_FLAGS here
# Make SURE to use -std=c11 and -std=c++11 where relevant.
# The options specified below are required, but additional options
#  may be used.
set(CMAKE_C_FLAGS "-std=c11 -Wall -Wextra -Wshadow -Werror")
set(CMAKE_CXX_FLAGS "-std=c++11 -Wall -Wextra -Wshadow -Werror")

# Add our include directory to CMake's search paths
# THIS IS REQUIRED
include_directories(include)

# Create libraries from dyn_array and process_scheduling so we can use them later
add_library(dyn_array
	src/dyn_array.c
)

add_library(process_scheduling
    src/process_scheduling.c
)

# process_scheduling depends on dyn_array
target_link_libraries(process_scheduling
    PRIVATE
        dyn_array
)

# Compile the analysis executable
add_executable(analysis
	src/analysis.c
)

# Link the dyn_array library we compiled against our analysis executable
target_link_libraries(analysis
    PRIVATE
        process_scheduling
)

# Compile the tester executable
add_executable(${PROJECT_NAME}_test test/tests.cpp)

target_compile_definitions(${PROJECT_NAME}_test PRIVATE)

# Link ${PROJECT_NAME}_test with dyn_array and gtest and pthread libraries
target_link_libraries(${PROJECT_NAME}_test 
	PRIVATE
		gtest
		pthread
		process_scheduling
)

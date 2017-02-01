// Copyright (C) 2013-2016 Altera Corporation, San Jose, California, USA. All rights reserved.
// Permission is hereby granted, free of charge, to any person obtaining a copy of this
// software and associated documentation files (the "Software"), to deal in the Software
// without restriction, including without limitation the rights to use, copy, modify, merge,
// publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to
// whom the Software is furnished to do so, subject to the following conditions:
// The above copyright notice and this permission notice shall be included in all copies or
// substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
// OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
// NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
// HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
// WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
// OTHER DEALINGS IN THE SOFTWARE.
//
// This agreement shall be governed in all respects by the laws of the State of California and
// by the laws of the United States of America.

/*
 * AOCL.h
 *
 *	A collection of utility functions for making the life easier
 *	when coding with the Altera/Intel OpenCL HLS SDK.
 *
 *  Created on: Jan 30, 2017
 *      Author: Simone Casale-Brunet
 */

#ifndef INC_AOCL_H_
#define INC_AOCL_H_

#include "CL/opencl.h"

#define CL_CONFIGURATION_ERROR -8888

//////////////////////////////////////////
// Files and directories helpers
//////////////////////////////////////////

// Sets the current working directory to be the same as the directory containing the running executable.
bool set_cwd_to_execdir();

// Checks if a file exists.
bool file_exists(const char *file_name);




//////////////////////////////////////////
// Host allocation functions for alignment
//////////////////////////////////////////

// Host allocation function
void *aligned_malloc(size_t size);

// Host deallocation function
void aligned_free(void *ptr);



//////////////////////////////////////////
// Platforms and devices information
//////////////////////////////////////////

// Display the available platforms and devices with some information.
// This is a basic implementation of clinfo
void clinfo();

// Display some device information
void display_device_info(cl_device_id device );

// Get the platform name.
// If the operation can not be accomplished a NULL value is returned
char* get_platform_name(cl_platform_id platform);

// Get the platform information according to the given parameter name.
// If the operation can not be accomplished a NULL value is returned
char* get_platform_info(cl_platform_id platform, cl_platform_info param_name);

// Get the device name.
// If the operation can not be accomplished a NULL value is returned
char* get_device_name(cl_device_id device);




//////////////////////////////////////////
// Platforms and devices helpers
//////////////////////////////////////////

// Get all the available platforms
cl_platform_id* get_platforms(cl_uint *num_platforms, cl_int *status);

// Find a platform that contains the search string in its name (case-insensitive match).
// Returns NULL if no match is found.
cl_platform_id find_platform(const char *platform_name_search, cl_int *status);

// Get all the devices of the given type available on the given platform
cl_device_id* get_devices(cl_platform_id pid, cl_device_type dev_type, cl_uint *num_devices, cl_int *status);




//////////////////////////////////////////
// Kernel binary helpers
//////////////////////////////////////////

// Loads a file in binary form.
unsigned char *load_binary_file(const char *file_name, size_t *size);

// Create a program for all devices associated with the context.
cl_program create_program_from_binary(cl_context context, const char *binary_file_name, const cl_device_id *devices, unsigned num_devices, cl_int *status);



//////////////////////////////////////////
// Profiling helpers
//////////////////////////////////////////

// Returns the time from a high-resolution timer in seconds. This value
// can be used with a value returned previously to measure a high-resolution
// time difference.
double get_current_timestamp();

// Returns the difference between the CL_PROFILING_COMMAND_END and
// CL_PROFILING_COMMAND_START values of a cl_event object.
// This requires that the command queue associated with the event be created
// with the CL_QUEUE_PROFILING_ENABLE property.
//
// The return value is in nanoseconds.
cl_ulong get_start_end_time(cl_event event, cl_int *status);

// Returns the maximum time span for the given set of events.
// The time span starts at the earliest event start time.
// The time span ends at the latest event end time.
cl_ulong get_start_end_time(cl_event *events, unsigned num_events, cl_int *status);

// Wait for the specified number of milliseconds.
void wait_ms(unsigned ms);




//////////////////////////////////////////
// Error and message helpers
//////////////////////////////////////////

// OpenCL context callback function that simply prints the error information
// to stdout (via printf).
void ocl_context_callback_message(const char *errinfo, const void *, size_t, void *);

// Print the error code as a string
void print_error(cl_int error);

// Print the error message with the given error code
void print_error(cl_int error, const char *error_message);

// Test if the status is CL_SUCCESS.
// If it is different print the error message call the clean_function and exit the execution
void test_error(cl_int error, const char *error_message, void (*clean_function)());

#endif /* INC_AOCL_H_ */

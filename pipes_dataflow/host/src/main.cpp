/*
 * main.cpp
 *
 *  Created on: Jan 30, 2017
 *      Author: scb
 */

#include "AOCL.h"
#include <stdio.h>
#include <string.h>

#define PLATFORM_NAME "Intel(R) FPGA"
#define BINARY_NAME   "pipes_dataflow.aocx"

#define SIZE 100
#define QUEUES_SIZE 2

cl_int status;
cl_context context = NULL;
cl_command_queue queues[QUEUES_SIZE];
cl_program program = NULL;
cl_kernel kernel_A = NULL;
cl_kernel kernel_B = NULL;
cl_kernel kernel_C = NULL;

void cleanup();

int check_result(int* v1, int* v2);

// Entry point.
int main() {
	// create program variables
	int *input = (int *) malloc(sizeof(int) * SIZE);
	int *output = (int *) malloc(sizeof(int) * SIZE);

	memset(output, 0, sizeof(int) * SIZE);
	for (int i = 0; i < SIZE; ++i) {
		input[i] = rand();
	}

	set_cwd_to_execdir();

	// find the platform defined by the PLATFORM_NAME variable
	cl_platform_id platform = find_platform(PLATFORM_NAME, &status);
	test_error(status, "ERROR: Unable to find the OpenCL platform.\n", &cleanup);

	// get all the available devives
	cl_uint num_devices;
	cl_device_id* devices = get_devices(platform, CL_DEVICE_TYPE_ALL, &num_devices, &status);
	test_error(status, "ERROR: Unable to find any device.\n", &cleanup);

	// take the first device
	cl_device_id device = devices[0];

	// create a context
	context = clCreateContext(NULL, 1, &device, &ocl_context_callback_message, NULL, &status);
	test_error(status, "ERROR: Failed to open the context.\n", &cleanup);

	// Create the command queues.
	queues[0] = clCreateCommandQueue(context, device, CL_QUEUE_PROFILING_ENABLE, &status);
	test_error(status, "ERROR: Failed to create command queue 0.\n", &cleanup);
	queues[1] = clCreateCommandQueue(context, device, CL_QUEUE_PROFILING_ENABLE, &status);
	test_error(status, "ERROR: Failed to create command queue 1.\n", &cleanup);

	program = create_program_from_binary(context, BINARY_NAME, &device, 1, &status);
	test_error(status, "ERROR: Failed to create the program.\n", &cleanup);

	// Build the program that was just created.
	status = clBuildProgram(program, 1, &device, "", NULL, NULL);
	test_error(status, "ERROR: Failed to build the program.\n", &cleanup);

	// Create the kernels
	kernel_A = clCreateKernel(program, "A", &status);
	test_error(status, "ERROR: Failed to create the kernel \"A\".\n", &cleanup);

	kernel_B = clCreateKernel(program, "B", &status);
	test_error(status, "ERROR: Failed to create the kernel \"B\".\n", &cleanup);

	kernel_C = clCreateKernel(program, "C", &status);
	test_error(status, "ERROR: Failed to create the kernel \"C\".\n", &cleanup);

	// create buffers for the kernels
	cl_mem in_buffer = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(int) * SIZE, input, &status);
	test_error(status, "ERROR: Failed to create the kernel out_buffer.\n", &cleanup);

	cl_mem out_buffer = clCreateBuffer(context, CL_MEM_WRITE_ONLY, sizeof(int) * SIZE, NULL, &status);
	test_error(status, "ERROR: Failed to create the kernel in_buffer.\n", &cleanup);

	// create the pipe shared among the two kernels
	cl_mem pipe_AB = clCreatePipe(context, 0, sizeof(cl_int), SIZE, NULL, &status);
	test_error(status, "ERROR: Failed to create the pipe AB.\n", &cleanup);

	cl_mem pipe_BC = clCreatePipe(context, 0, sizeof(cl_int), SIZE, NULL, &status);
	test_error(status, "ERROR: Failed to create the pipe BC.\n", &cleanup);

	// Set the kernel arguments
	status = clSetKernelArg(kernel_A, 0, sizeof(cl_mem), &in_buffer);
	test_error(status, "ERROR: Failed to set kernel A arg 0.\n", &cleanup);
	status = clSetKernelArg(kernel_A, 1, sizeof(cl_mem), &pipe_AB);
	test_error(status, "ERROR: Failed to set kernel A arg 1.\n", &cleanup);

	status = clSetKernelArg(kernel_B, 0, sizeof(cl_mem), &pipe_AB);
	test_error(status, "ERROR: Failed to set kernel B arg 0.\n", &cleanup);
	status = clSetKernelArg(kernel_B, 1, sizeof(cl_mem), &pipe_BC);
	test_error(status, "ERROR: Failed to set kernel B arg 1.\n", &cleanup);

	status = clSetKernelArg(kernel_C, 0, sizeof(cl_mem), &out_buffer);
	test_error(status, "ERROR: Failed to set kernel C arg 0.\n", &cleanup);
	status = clSetKernelArg(kernel_C, 1, sizeof(cl_mem), &pipe_BC);
	test_error(status, "ERROR: Failed to set kernel C arg 1.\n", &cleanup);

	printf("\nKernels initialization is complete.\n");
	printf("Launching the kernels...\n");

	size_t size = 1;
	cl_event sync;

	status = clEnqueueTask(queues[1], kernel_B, 0, NULL, NULL);
	test_error(status, "ERROR: Failed to launch \"B\".\n", &cleanup);

	// Launch the kernels
	status = clEnqueueNDRangeKernel(queues[0], kernel_A, 1, NULL, &size, &size, 0, NULL, &sync);
	test_error(status, "ERROR: Failed to launch \"A\".\n", &cleanup);

	status = clEnqueueNDRangeKernel(queues[0], kernel_C, 1, NULL, &size, &size, 0, NULL, &sync);
	test_error(status, "ERROR: Failed to launch \"C\".\n", &cleanup);

	// Wait for command queue to complete pending events
	status = clFinish(queues[0]);
	test_error(status, "ERROR: Failed to finish.\n", &cleanup);

	// get device buffer
	printf("\nKernel execution is completed, get results....\n");
	status = clEnqueueReadBuffer(queues[0], out_buffer, CL_TRUE, 0, sizeof(int) * SIZE, output, 0, NULL, NULL);
	test_error(status, "ERROR: Failed to enqueue output read buffer.\n", &cleanup);

	// compare results
	int golden = 0, result = 0;
	for (int i = 0; i < SIZE; ++i) {
		golden += input[i];
		result += output[i];
	}

	int ret = check_result(input, output);

	cleanup();

	return ret;
}

int check_result(int* v1, int* v2) {
	// compare results
	int golden = 0, result = 0;
	for (int i = 0; i < SIZE; ++i) {
		golden += v1[i];
		result += v2[i];
	}

	int ret = 0;
	if (golden != result) {
		ret = -1;
		printf("FAILED %d (instead of %d)!", result, golden);
	} else {
		printf("PASSED!");
	}

	for (int i = 0; i < SIZE; ++i) {
		if (v1[i] != v2[i]) {
			printf("UNSORTED!");
			ret = -1;
			break;
		}
	}

	return ret;
}

void cleanup() {

	if (kernel_A) {
		clReleaseKernel(kernel_A);
	}
	if (kernel_B) {
		clReleaseKernel(kernel_B);
	}
	if (kernel_C) {
		clReleaseKernel(kernel_C);
	}
	if (program) {
		clReleaseProgram(program);
	}
	for (int i = 0; i < QUEUES_SIZE; ++i) {
		cl_command_queue queue = queues[i];
		if (queue) {
			clReleaseCommandQueue(queue);
		}
	}

	if (context) {
		clReleaseContext(context);
	}
}


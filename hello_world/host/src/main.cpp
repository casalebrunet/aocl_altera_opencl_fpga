/*
 * main.cpp
 *
 *  Created on: Jan 30, 2017
 *      Author: scb
 */

#include "AOCL.h"
#include <stdio.h>

#define PLATFORM_NAME "Intel(R) FPGA"
#define BINARY_NAME   "hello_world.aocx"

cl_int status;
cl_context context = NULL;
cl_command_queue queue = NULL;
cl_program program = NULL;
cl_kernel kernel = NULL;

// Runtime constants
// Used to define the work set over which this kernel will execute.
static const size_t work_group_size = 8;  // 8 threads in the demo workgroup
// Defines kernel argument value, which is the workitem ID that will
// execute a printf call
static const int thread_id_to_output = 2;

void cleanup();

// Entry point.
int main() {
	set_cwd_to_execdir();

	// find the platform defined by the PLATFORM_NAME variable
	cl_platform_id platform = find_platform(PLATFORM_NAME, &status);
	test_error(status, "ERROR: Unable to find the OpenCL platform.\n", &cleanup);

	// print some info on the selected platform
	{
		char* platform_info = get_platform_info(platform, CL_PLATFORM_NAME);
		printf("%-20s = %s\n", "CL_PLATFORM_NAME", platform_info);
		platform_info = get_platform_info(platform, CL_PLATFORM_VENDOR);
		printf("%-20s = %s\n", "CL_PLATFORM_VENDOR", platform_info);
		platform_info = get_platform_info(platform, CL_PLATFORM_VERSION);
		printf("%-20s = %s\n", "CL_PLATFORM_VERSION", platform_info);
	}

	// get all the available devives
	cl_uint num_devices;
	cl_device_id* devices = get_devices(platform, CL_DEVICE_TYPE_ALL, &num_devices, &status);
	test_error(status, "ERROR: Unable to find any device.\n", &cleanup);

	// take the first device
	cl_device_id device = devices[0];

	// create a context
	context = clCreateContext(NULL, 1, &device, &ocl_context_callback_message, NULL, &status);
	test_error(status, "ERROR: Failed to open the context.\n", &cleanup);

	// Create the command queue.
	queue = clCreateCommandQueue(context, device, CL_QUEUE_PROFILING_ENABLE, &status);
	test_error(status, "ERROR: Failed to create command queue.\n", &cleanup);

	program = create_program_from_binary(context, BINARY_NAME, &device, 1, &status);
	test_error(status, "ERROR: Failed to create the program.\n", &cleanup);

	// Build the program that was just created.
	status = clBuildProgram(program, 1, &device, "", NULL, NULL);
	test_error(status, "ERROR: Failed to build the program.\n", &cleanup);

	// Create the kernel - name passed in here must match kernel name in the
	// original CL file, that was compiled into an AOCX file using the AOC tool
	const char *kernel_name = "hello_world";  // Kernel name, as defined in the CL file
	kernel = clCreateKernel(program, kernel_name, &status);
	test_error(status, "ERROR: Failed to create the kernel.\n", &cleanup);

	// Set the kernel argument (argument 0)
	status = clSetKernelArg(kernel, 0, sizeof(cl_int), (void*) &thread_id_to_output);
	test_error(status, "ERROR: Failed to set kernel arg 0.\n", &cleanup);

	printf("\nKernel initialization is complete.\n");
	printf("Launching the kernel...\n\n");

	// Configure work set over which the kernel will execute
	size_t wgSize[3] = { work_group_size, 1, 1 };
	size_t gSize[3] = { work_group_size, 1, 1 };

	// Launch the kernel
	status = clEnqueueNDRangeKernel(queue, kernel, 1, NULL, gSize, wgSize, 0, NULL, NULL);
	test_error(status, "ERROR: Failed to launch the kernel.\n", &cleanup);

	// Wait for command queue to complete pending events
	status = clFinish(queue);
	test_error(status, "ERROR: Failed to finish.\n", &cleanup);

	printf("\nKernel execution is complete.\n");

	cleanup();

	return 0;
}

void cleanup() {

	if (kernel) {
		clReleaseKernel(kernel);
	}
	if (program) {
		clReleaseProgram(program);
	}
	if (queue) {
		clReleaseCommandQueue(queue);
	}
	if (context) {
		clReleaseContext(context);
	}
}


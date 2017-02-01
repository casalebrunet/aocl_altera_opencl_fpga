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
 * AOCL.cpp
 *
 *  Created on: Jan 30, 2017
 *      Author: Simone Casale-Brunet
 */

#include "AOCL.h"
//#include "CL/opencl.h"

#include <algorithm>
#include <string.h>
#include <stdio.h>
#include <string>

#ifdef _WIN32 // Windows
#include <windows.h>
#else        // Linux
#include <stdlib.h>
#include <unistd.h>// readlink, chdir
#endif

// This is the minimum alignment requirement to ensure DMA can be used.
const unsigned AOCL_ALIGNMENT = 64;

#ifdef _WIN32 // Windows
void *aligned_malloc(size_t size) {
	return _aligned_malloc (size, AOCL_ALIGNMENT);
}

void aligned_free(void * ptr) {
	_aligned_free(ptr);
}
#else          // Linux
void *aligned_malloc(size_t size) {
	void *result = NULL;
	int rc;
	rc = posix_memalign(&result, AOCL_ALIGNMENT, size);
	return result;
}

void aligned_free(void * ptr) {
	free(ptr);
}
#endif

// Helper functions to display parameters returned by OpenCL queries
static void device_info_ulong(cl_device_id device, cl_device_info param, const char* name) {
	cl_ulong a;
	clGetDeviceInfo(device, param, sizeof(cl_ulong), &a, NULL);
	printf("%-40s = %lu\n", name, a);
}

static void device_info_uint(cl_device_id device, cl_device_info param, const char* name) {
	cl_uint a;
	clGetDeviceInfo(device, param, sizeof(cl_uint), &a, NULL);
	printf("%-40s = %u\n", name, a);
}

static void device_info_bool(cl_device_id device, cl_device_info param, const char* name) {
	cl_bool a;
	clGetDeviceInfo(device, param, sizeof(cl_bool), &a, NULL);
	printf("%-40s = %s\n", name, (a ? "true" : "false"));
}

static void device_info_string(cl_device_id device, cl_device_info param, const char* name) {
	cl_ulong infoSize;
	clGetDeviceInfo(device, param, 0, NULL, &infoSize);
	char* a = (char*) malloc(infoSize);
	clGetDeviceInfo(device, param, infoSize, a, NULL);
	printf("%-40s = %s\n", name, a);
}

void display_device_info(cl_device_id device) {
	device_info_string(device, CL_DEVICE_NAME, "CL_DEVICE_NAME");
	device_info_string(device, CL_DEVICE_VENDOR, "CL_DEVICE_VENDOR");
	device_info_uint(device, CL_DEVICE_VENDOR_ID, "CL_DEVICE_VENDOR_ID");
	device_info_string(device, CL_DEVICE_VERSION, "CL_DEVICE_VERSION");
	device_info_string(device, CL_DRIVER_VERSION, "CL_DRIVER_VERSION");
	device_info_uint(device, CL_DEVICE_ADDRESS_BITS, "CL_DEVICE_ADDRESS_BITS");
	device_info_bool(device, CL_DEVICE_AVAILABLE, "CL_DEVICE_AVAILABLE");
	device_info_bool(device, CL_DEVICE_ENDIAN_LITTLE, "CL_DEVICE_ENDIAN_LITTLE");
	device_info_ulong(device, CL_DEVICE_GLOBAL_MEM_CACHE_SIZE, "CL_DEVICE_GLOBAL_MEM_CACHE_SIZE");
	device_info_ulong(device, CL_DEVICE_GLOBAL_MEM_CACHELINE_SIZE, "CL_DEVICE_GLOBAL_MEM_CACHELINE_SIZE");
	device_info_ulong(device, CL_DEVICE_GLOBAL_MEM_SIZE, "CL_DEVICE_GLOBAL_MEM_SIZE");
	device_info_bool(device, CL_DEVICE_IMAGE_SUPPORT, "CL_DEVICE_IMAGE_SUPPORT");
	device_info_ulong(device, CL_DEVICE_LOCAL_MEM_SIZE, "CL_DEVICE_LOCAL_MEM_SIZE");
	device_info_ulong(device, CL_DEVICE_MAX_CLOCK_FREQUENCY, "CL_DEVICE_MAX_CLOCK_FREQUENCY");
	device_info_ulong(device, CL_DEVICE_MAX_COMPUTE_UNITS, "CL_DEVICE_MAX_COMPUTE_UNITS");
	device_info_ulong(device, CL_DEVICE_MAX_CONSTANT_ARGS, "CL_DEVICE_MAX_CONSTANT_ARGS");
	device_info_ulong(device, CL_DEVICE_MAX_CONSTANT_BUFFER_SIZE, "CL_DEVICE_MAX_CONSTANT_BUFFER_SIZE");
	device_info_uint(device, CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS, "CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS");
	device_info_uint(device, CL_DEVICE_MEM_BASE_ADDR_ALIGN, "CL_DEVICE_MEM_BASE_ADDR_ALIGN");
	device_info_uint(device, CL_DEVICE_MIN_DATA_TYPE_ALIGN_SIZE, "CL_DEVICE_MIN_DATA_TYPE_ALIGN_SIZE");
	device_info_uint(device, CL_DEVICE_PREFERRED_VECTOR_WIDTH_CHAR, "CL_DEVICE_PREFERRED_VECTOR_WIDTH_CHAR");
	device_info_uint(device, CL_DEVICE_PREFERRED_VECTOR_WIDTH_SHORT, "CL_DEVICE_PREFERRED_VECTOR_WIDTH_SHORT");
	device_info_uint(device, CL_DEVICE_PREFERRED_VECTOR_WIDTH_INT, "CL_DEVICE_PREFERRED_VECTOR_WIDTH_INT");
	device_info_uint(device, CL_DEVICE_PREFERRED_VECTOR_WIDTH_LONG, "CL_DEVICE_PREFERRED_VECTOR_WIDTH_LONG");
	device_info_uint(device, CL_DEVICE_PREFERRED_VECTOR_WIDTH_FLOAT, "CL_DEVICE_PREFERRED_VECTOR_WIDTH_FLOAT");
	device_info_uint(device, CL_DEVICE_PREFERRED_VECTOR_WIDTH_DOUBLE, "CL_DEVICE_PREFERRED_VECTOR_WIDTH_DOUBLE");

	{
		cl_command_queue_properties ccp;
		clGetDeviceInfo(device, CL_DEVICE_QUEUE_PROPERTIES, sizeof(cl_command_queue_properties), &ccp, NULL);
		printf("%-40s = %s\n", "Command queue out of order? ", ((ccp & CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE) ? "true" : "false"));
		printf("%-40s = %s\n", "Command queue profiling enabled? ", ((ccp & CL_QUEUE_PROFILING_ENABLE) ? "true" : "false"));
	}
}

bool set_cwd_to_execdir() {
#ifdef _WIN32 // Windows
	HMODULE hMod = GetModuleHandle(NULL);
	char path[MAX_PATH];
	GetModuleFileNameA(hMod, path, MAX_PATH);

#else         // Linux
	// Get path of executable.
	char path[300];
	ssize_t n = readlink("/proc/self/exe", path, sizeof(path) / sizeof(path[0]) - 1);
	if (n == -1) {
		return false;
	}
	path[n] = 0;
#endif

	// Find the last '\' or '/' and terminate the path there; it is now
	// the directory containing the executable.
	size_t i;
	for (i = strlen(path) - 1; i > 0 && path[i] != '/' && path[i] != '\\'; --i)
		;
	path[i] = '\0';

	// Change the current directory.
#ifdef _WIN32 // Windows
	SetCurrentDirectoryA(path);
#else         // Linux
	int rc;
	rc = chdir(path);
#endif

	return true;
}

void clinfo() {
	// Get number of platforms.
	cl_int status;
	cl_uint num_platforms;

	cl_platform_id* platform_ids = get_platforms(&num_platforms, &status);

	printf("%d platform(s) found:\n", num_platforms);
	printf("=============================================================\n");

	for (unsigned i = 0; i < num_platforms; ++i) {
		char* platform_name = get_platform_name(platform_ids[i]);
		printf("[%d] %s\n", (i + 1), platform_name);
		cl_uint num_devices;
		cl_device_id *devices_ids = get_devices(platform_ids[i], CL_DEVICE_TYPE_ALL, &num_devices, &status);
		printf("%d device(s) found:\n", num_devices);
		for (unsigned j = 0; j < num_devices; ++j) {
			printf("-------------------------------------------------------------\n");
			char* device_name = get_device_name(devices_ids[j]);
			printf("[%d.%d] %s\n", i + 1, j + 1, device_name);
			display_device_info(devices_ids[j]);
		}
		printf("=============================================================\n");
	}
}

char* get_platform_info(cl_platform_id platform, cl_platform_info param_name) {
	cl_ulong infoSize;
	clGetPlatformInfo(platform, param_name, 0, NULL, &infoSize);
	char* a = (char*) malloc(infoSize);
	clGetPlatformInfo(platform, param_name, infoSize, a, NULL);
	return a;
}

cl_platform_id* get_platforms(cl_uint *num_platforms, cl_int *status) {
	// get the number of platforms
	*status = clGetPlatformIDs(0, NULL, num_platforms);
	if (*status != CL_SUCCESS) {
		return NULL;
	}

	// get the platform ids
	cl_platform_id* platform_ids = (cl_platform_id *) malloc(sizeof(cl_platform_id) * (*num_platforms));
	*status = clGetPlatformIDs((*num_platforms), platform_ids, NULL);
	if (*status != CL_SUCCESS) {
		return NULL;
	}

	return platform_ids;
}

// Returns the platform name.
char* get_platform_name(cl_platform_id pid) {
	cl_int status;

	size_t size;
	status = clGetPlatformInfo(pid, CL_PLATFORM_NAME, 0, NULL, &size);
	if (status != CL_SUCCESS) {
		return NULL;
	}

	char *name = (char*) malloc(size);

	status = clGetPlatformInfo(pid, CL_PLATFORM_NAME, size, name, NULL);
	if (status != CL_SUCCESS) {
		return NULL;
	}

	return name;
}

// Returns the list of all devices.
cl_device_id* get_devices(cl_platform_id pid, cl_device_type dev_type, cl_uint *num_devices, cl_int *status) {

	*status = clGetDeviceIDs(pid, dev_type, 0, NULL, num_devices);
	if (*status != CL_SUCCESS) {
		return NULL;
	}

	cl_device_id *dids = new cl_device_id[*num_devices];
	*status = clGetDeviceIDs(pid, dev_type, *num_devices, dids, NULL);
	if (*status != CL_SUCCESS) {
		return NULL;
	}

	return dids;
}

// Returns the device name.
char* get_device_name(cl_device_id did) {
	cl_int status;

	size_t size;
	status = clGetDeviceInfo(did, CL_DEVICE_NAME, 0, NULL, &size);
	if (status != CL_SUCCESS) {
		return NULL;
	}

	char *name = (char*) malloc(size);
	status = clGetDeviceInfo(did, CL_DEVICE_NAME, size, name, NULL);
	if (status != CL_SUCCESS) {
		return NULL;
	}

	return name;
}

// Checks if a file exists.
bool file_exists(const char *file_name) {
#ifdef _WIN32 // Windows
	DWORD attrib = GetFileAttributesA(file_name);
	return (attrib != INVALID_FILE_ATTRIBUTES && !(attrib & FILE_ATTRIBUTE_DIRECTORY));
#else         // Linux
	return access(file_name, R_OK) != -1;
#endif
}

// Loads a file in binary form.
unsigned char *load_binary_file(const char *file_name, size_t *size) {
	if (!file_exists(file_name)) {
		return NULL;
	}
	// Open the File
	FILE* fp;
#ifdef _WIN32
	if(fopen_s(&fp, file_name, "rb") != 0) {
		return NULL;
	}
#else
	fp = fopen(file_name, "rb");
	if (fp == 0) {
		return NULL;
	}
#endif

	// Get the size of the file
	fseek(fp, 0, SEEK_END);
	*size = ftell(fp);

	// Allocate space for the binary
	unsigned char *binary = new unsigned char[*size];

	// Go back to the file start
	rewind(fp);

	// Read the file into the binary
	if (fread((void*) binary, *size, 1, fp) == 0) {
		delete[] binary;
		fclose(fp);
		return NULL;
	}

	return binary;
}

cl_program create_program_from_binary(cl_context context, const char *binary_file_name, const cl_device_id *devices, unsigned num_devices,
		cl_int *status) {
	// Early exit for potentially the most common way to fail: AOCX does not exist.
	if (!file_exists(binary_file_name)) {
		printf("AOCX file '%s' does not exist.\n", binary_file_name);
		*status = CL_INVALID_PROGRAM;
		return NULL;
	}

	// Load the binary.
	size_t binary_size;
	unsigned char *binary = load_binary_file(binary_file_name, &binary_size);
	if (binary == NULL) {
		printf("The binary file '%s' can not be loaded.\n", binary_file_name);
		*status = CL_INVALID_PROGRAM;
		return NULL;
	}

	size_t *binary_lengths = (size_t *) malloc(sizeof(size_t) * num_devices);
	unsigned char *binaries[num_devices];

	for (unsigned i = 0; i < num_devices; ++i) {
		binary_lengths[i] = binary_size;
		binaries[i] = binary;
	}

	cl_int binary_status[num_devices];
	cl_program program = clCreateProgramWithBinary(context, num_devices, devices, binary_lengths, (const unsigned char **) binaries, binary_status,
			status);

	if (program == NULL || *status != CL_SUCCESS) {
		printf("Failed to create program with binary");
		*status = CL_CONFIGURATION_ERROR;
		return NULL;
	}

	for (unsigned i = 0; i < num_devices; ++i) {
		if (binary_status[i] != CL_SUCCESS) {
			printf("Failed to load binary for device");
			*status = CL_CONFIGURATION_ERROR;
			return NULL;
		}
	}

	return program;
}

double get_current_timestamp() {
#ifdef _WIN32 // Windows
	// Use the high-resolution performance counter.

	static LARGE_INTEGER ticks_per_second = {};
	if(ticks_per_second.QuadPart == 0) {
		// First call - get the frequency.
		QueryPerformanceFrequency(&ticks_per_second);
	}

	LARGE_INTEGER counter;
	QueryPerformanceCounter(&counter);

	double seconds = double(counter.QuadPart) / double(ticks_per_second.QuadPart);
	return seconds;
#else         // Linux
	timespec a;
	clock_gettime(CLOCK_MONOTONIC, &a);
	return (double(a.tv_nsec) * 1.0e-9) + double(a.tv_sec);
#endif
}

cl_ulong get_start_end_time(cl_event event, cl_int *status) {
	cl_ulong start, end;
	*status = clGetEventProfilingInfo(event, CL_PROFILING_COMMAND_START, sizeof(start), &start, NULL);
	if (*status != CL_SUCCESS) {
		return 0;
	}

	*status = clGetEventProfilingInfo(event, CL_PROFILING_COMMAND_END, sizeof(end), &end, NULL);
	if (*status != CL_SUCCESS) {
		return 0;
	}

	return end - start;
}

cl_ulong get_start_end_time(cl_event *events, unsigned num_events, cl_int *status) {

	cl_ulong min_start = 0;
	cl_ulong max_end = 0;
	for (unsigned i = 0; i < num_events; ++i) {
		cl_ulong start, end;
		*status = clGetEventProfilingInfo(events[i], CL_PROFILING_COMMAND_START, sizeof(start), &start, NULL);
		if (*status != CL_SUCCESS) {
			return 0;
		}

		*status = clGetEventProfilingInfo(events[i], CL_PROFILING_COMMAND_END, sizeof(end), &end, NULL);
		if (*status != CL_SUCCESS) {
			return 0;
		}

		if (i == 0) {
			min_start = start;
			max_end = end;
		} else {
			if (start < min_start) {
				min_start = start;
			}
			if (end > max_end) {
				max_end = end;
			}
		}
	}

	return max_end - min_start;
}

void wait_ms(unsigned ms) {
#ifdef _WIN32 // Windows
	Sleep(ms);
#else         // Linux
	timespec sleeptime = { 0, 0 };
	sleeptime.tv_sec = ms / 1000;
	sleeptime.tv_nsec = long(ms % 1000) * 1000000L;  // convert to nanoseconds
	nanosleep(&sleeptime, NULL);
#endif
}

cl_platform_id find_platform(const char *platform_name_search, cl_int *status) {
	cl_uint num_platforms;
	cl_platform_id* platform_ids = get_platforms(&num_platforms, status);

	if (*status == CL_SUCCESS && platform_ids && num_platforms) {
		std::string search = platform_name_search;
		std::transform(search.begin(), search.end(), search.begin(), tolower);
		for (unsigned i = 0; i < num_platforms; ++i) {
			std::string name = get_platform_name(platform_ids[i]);

			// Convert to lower case.
			std::transform(name.begin(), name.end(), name.begin(), tolower);

			if (name.find(search) != std::string::npos) {
				// Found!
				return platform_ids[i];
			}
		}
	}

	return NULL;
}

void ocl_context_callback_message(const char *errinfo, const void *, size_t, void *) {
	printf("Context callback: %s\n", errinfo);
}

void print_error(cl_int error, const char *error_message) {
	printf("%s ", error_message);
	print_error(error);
	printf("\n");
}

// Print the error associciated with an error code
void print_error(cl_int error) {
	// Print error message
	switch (error) {
	case -1:
		printf("CL_DEVICE_NOT_FOUND ");
		break;
	case -2:
		printf("CL_DEVICE_NOT_AVAILABLE ");
		break;
	case -3:
		printf("CL_COMPILER_NOT_AVAILABLE ");
		break;
	case -4:
		printf("CL_MEM_OBJECT_ALLOCATION_FAILURE ");
		break;
	case -5:
		printf("CL_OUT_OF_RESOURCES ");
		break;
	case -6:
		printf("CL_OUT_OF_HOST_MEMORY ");
		break;
	case -7:
		printf("CL_PROFILING_INFO_NOT_AVAILABLE ");
		break;
	case -8:
		printf("CL_MEM_COPY_OVERLAP ");
		break;
	case -9:
		printf("CL_IMAGE_FORMAT_MISMATCH ");
		break;
	case -10:
		printf("CL_IMAGE_FORMAT_NOT_SUPPORTED ");
		break;
	case -11:
		printf("CL_BUILD_PROGRAM_FAILURE ");
		break;
	case -12:
		printf("CL_MAP_FAILURE ");
		break;
	case -13:
		printf("CL_MISALIGNED_SUB_BUFFER_OFFSET ");
		break;
	case -14:
		printf("CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST ");
		break;

	case -30:
		printf("CL_INVALID_VALUE ");
		break;
	case -31:
		printf("CL_INVALID_DEVICE_TYPE ");
		break;
	case -32:
		printf("CL_INVALID_PLATFORM ");
		break;
	case -33:
		printf("CL_INVALID_DEVICE ");
		break;
	case -34:
		printf("CL_INVALID_CONTEXT ");
		break;
	case -35:
		printf("CL_INVALID_QUEUE_PROPERTIES ");
		break;
	case -36:
		printf("CL_INVALID_COMMAND_QUEUE ");
		break;
	case -37:
		printf("CL_INVALID_HOST_PTR ");
		break;
	case -38:
		printf("CL_INVALID_MEM_OBJECT ");
		break;
	case -39:
		printf("CL_INVALID_IMAGE_FORMAT_DESCRIPTOR ");
		break;
	case -40:
		printf("CL_INVALID_IMAGE_SIZE ");
		break;
	case -41:
		printf("CL_INVALID_SAMPLER ");
		break;
	case -42:
		printf("CL_INVALID_BINARY ");
		break;
	case -43:
		printf("CL_INVALID_BUILD_OPTIONS ");
		break;
	case -44:
		printf("CL_INVALID_PROGRAM ");
		break;
	case -45:
		printf("CL_INVALID_PROGRAM_EXECUTABLE ");
		break;
	case -46:
		printf("CL_INVALID_KERNEL_NAME ");
		break;
	case -47:
		printf("CL_INVALID_KERNEL_DEFINITION ");
		break;
	case -48:
		printf("CL_INVALID_KERNEL ");
		break;
	case -49:
		printf("CL_INVALID_ARG_INDEX ");
		break;
	case -50:
		printf("CL_INVALID_ARG_VALUE ");
		break;
	case -51:
		printf("CL_INVALID_ARG_SIZE ");
		break;
	case -52:
		printf("CL_INVALID_KERNEL_ARGS ");
		break;
	case -53:
		printf("CL_INVALID_WORK_DIMENSION ");
		break;
	case -54:
		printf("CL_INVALID_WORK_GROUP_SIZE ");
		break;
	case -55:
		printf("CL_INVALID_WORK_ITEM_SIZE ");
		break;
	case -56:
		printf("CL_INVALID_GLOBAL_OFFSET ");
		break;
	case -57:
		printf("CL_INVALID_EVENT_WAIT_LIST ");
		break;
	case -58:
		printf("CL_INVALID_EVENT ");
		break;
	case -59:
		printf("CL_INVALID_OPERATION ");
		break;
	case -60:
		printf("CL_INVALID_GL_OBJECT ");
		break;
	case -61:
		printf("CL_INVALID_BUFFER_SIZE ");
		break;
	case -62:
		printf("CL_INVALID_MIP_LEVEL ");
		break;
	case -63:
		printf("CL_INVALID_GLOBAL_WORK_SIZE ");
		break;
	case CL_CONFIGURATION_ERROR:
		printf("CL_CONFIGURATION_ERROR ");
		break;
	default:
		printf("UNRECOGNIZED ERROR CODE (%d)", error);
	}
}

void test_error(cl_int status, const char *error_message, void (*clean_function)()) {
	if (status != CL_SUCCESS) {
		print_error(status, error_message);
		clean_function();
		exit(status);
	}
}


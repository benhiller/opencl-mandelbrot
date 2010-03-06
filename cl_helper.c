#include <sys/sysctl.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>

#include "cl_helper.h"
#include <assert.h>

char* load_program_source(const char *filename) {
	struct stat statbuf;
	FILE *fh;
	char *source;

	fh = fopen(filename, "r");
	if (fh == 0)
		return 0;

	stat(filename, &statbuf);
	source = (char *) malloc(statbuf.st_size + 1);
	fread(source, statbuf.st_size, 1, fh);
	source[statbuf.st_size] = '\0';

	return source;
}

cl_kernel load_kernel_from_file(cl_context context, const char *filename) {
	cl_program program[1];
	cl_kernel  kernel[1];
  int err;
	char *program_source = load_program_source(filename);
	program[0] = clCreateProgramWithSource(context, 1, (const char**)&program_source,
                                         NULL, &err);
	assert(err == CL_SUCCESS);

	err = clBuildProgram(program[0], 0, NULL, NULL, NULL, NULL);
	assert(err == CL_SUCCESS);

	// Now create the kernel "objects" that we want to use in the example file
  // render is the name of the function we are going to run
	kernel[0] = clCreateKernel(program[0], "render", &err);
  return kernel[0];
}

cl_context create_context(int* num_devices) {
  cl_int err;
	cl_device_id cpu = NULL, device = NULL;
  cl_context context;

  // First get the CPU device, as a fallback
	err = clGetDeviceIDs(NULL, CL_DEVICE_TYPE_CPU, 1, &cpu, NULL);
	assert(err == CL_SUCCESS);

  // Find the GPU CL device, this is what we really want
	// If there is no GPU device is CL capable, fall back to CPU
	err = clGetDeviceIDs(NULL, CL_DEVICE_TYPE_GPU, 1, &device, NULL);
	if (err != CL_SUCCESS) device = cpu;
	assert(device);

  context = clCreateContext(0, 1, &device, NULL, NULL, &err);
  assert(err == CL_SUCCESS);

  // TODO
  *num_devices = 1;

  return context;
}

void print_debug_info(cl_context context) {
  cl_device_id device;
  cl_int err;

  err = clGetContextInfo(context, CL_CONTEXT_DEVICES, sizeof(cl_device_id), &device, NULL);
	assert(err == CL_SUCCESS);

  cl_char vendor_name[1024] = {0};
  cl_char device_name[1024] = {0};
  err = clGetDeviceInfo(device, CL_DEVICE_VENDOR, sizeof(vendor_name),
                        vendor_name, NULL);
  err |= clGetDeviceInfo(device, CL_DEVICE_NAME, sizeof(device_name),
                         device_name, NULL);
  assert(err == CL_SUCCESS);

  printf("Connected to %s %s\n", vendor_name, device_name);
}

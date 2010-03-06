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
	program[0] = clCreateProgramWithSource(context, 1,
                                         (const char**)&program_source,
                                         NULL, &err);
	assert(err == CL_SUCCESS);

	err = clBuildProgram(program[0], 0, NULL, NULL, NULL, NULL);
	assert(err == CL_SUCCESS);

	// Now create the kernel "objects" that we want to use in the example file
  // render is the name of the function we are going to run
	kernel[0] = clCreateKernel(program[0], "render", &err);
  return kernel[0];
}

cl_context create_context(cl_uint* num_devices) {
  cl_int err;
  cl_device_id *devices[16], cpus[16];
  cl_uint num_cpus;
  cl_context context;

  // First get the CPU device, as a fallback
	err = clGetDeviceIDs(NULL, CL_DEVICE_TYPE_CPU, 16, cpus, &num_cpus);
	assert(err == CL_SUCCESS);

  // Find the GPU CL device, this is what we really want
	// If there is no GPU device is CL capable, fall back to CPU
	err = clGetDeviceIDs(NULL, CL_DEVICE_TYPE_GPU, 16, *devices, num_devices);
	if (err != CL_SUCCESS || *num_devices == 0) {
    *devices = cpus;
    *num_devices = num_cpus;
  }
	assert(devices);

  context = clCreateContext(0, *num_devices, *devices, NULL, NULL, &err);
  assert(err == CL_SUCCESS);

  return context;
}

void print_debug_info(cl_context context) {
  cl_device_id devices[16];
  size_t size;
  int elements;
  cl_int err;

  err = clGetContextInfo(context, CL_CONTEXT_DEVICES, sizeof(cl_device_id) * 16,
                         &devices, &size);
	assert(err == CL_SUCCESS);

  elements = size/sizeof(cl_device_id);

  int i;
  for(i = 0; i < elements; i++) {
    cl_char vendor_name[1024] = {0};
    cl_char device_name[1024] = {0};
    err = clGetDeviceInfo(devices[i], CL_DEVICE_VENDOR, sizeof(vendor_name),
                          vendor_name, NULL);
    err |= clGetDeviceInfo(devices[i], CL_DEVICE_NAME, sizeof(device_name),
                           device_name, NULL);
    assert(err == CL_SUCCESS);

    printf("Device %d: %s %s\n", i, vendor_name, device_name);
  }
}

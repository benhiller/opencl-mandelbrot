#include <sys/sysctl.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "cl_helper.h"

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
	check_succeeded(err);

	err = clBuildProgram(program[0], 0, NULL, NULL, NULL, NULL);
	check_succeeded(err);

	// Now create the kernel "objects" that we want to use in the example file
  // render is the name of the function we are going to run
	kernel[0] = clCreateKernel(program[0], "render", &err);
  return kernel[0];
}

cl_context create_context(cl_uint* num_devices) {
  cl_int err;
  cl_device_id **devices, cpus[16];
  *devices = malloc(16 * sizeof(cl_device_id));
  cl_uint num_cpus;
  cl_context context;

  // First get the CPU device, as a fallback
	err = clGetDeviceIDs(NULL, CL_DEVICE_TYPE_CPU, 16, cpus, &num_cpus);
	check_succeeded(err);

  // Find the GPU CL device, this is what we really want
	// If there is no GPU device is CL capable, fall back to CPU
	err = clGetDeviceIDs(NULL, CL_DEVICE_TYPE_GPU, 16, *devices, num_devices);
	if (err != CL_SUCCESS || *num_devices == 0) {
    *devices = cpus;
    *num_devices = num_cpus;
  }
	assert(*devices);

  context = clCreateContext(0, *num_devices, *devices, NULL, NULL, &err);
  check_succeeded(err);

  return context;
}

void print_debug_info(cl_context context) {
  cl_device_id devices[16];
  size_t size;
  int elements;
  cl_int err;

  err = clGetContextInfo(context, CL_CONTEXT_DEVICES, sizeof(cl_device_id) * 16,
                         &devices, &size);
  check_succeeded(err);

  elements = size/sizeof(cl_device_id);

  int i;
  for(i = 0; i < elements; i++) {
    cl_char vendor_name[1024] = {0};
    cl_char device_name[1024] = {0};
    err = clGetDeviceInfo(devices[i], CL_DEVICE_VENDOR, sizeof(vendor_name),
                          vendor_name, NULL);
    err |= clGetDeviceInfo(devices[i], CL_DEVICE_NAME, sizeof(device_name),
                           device_name, NULL);
    check_succeeded(err);

    printf("Device %d: %s %s\n", i, vendor_name, device_name);
  }
}

void check_succeeded(cl_int err) {
  if(err != CL_SUCCESS) {
    // Abort and print debugging info
    switch(err) {
      case -1: printf("Device not found"); break;
      case -2: printf("Device not available"); break;
      case -3: printf("Compiler not available"); break;
      case -4: printf("Memory object allocation failure"); break;
      case -5: printf("Out of resources"); break;
      case -6: printf("Out of host memory"); break;
      case -7: printf("Profiling info not available"); break;
      case -8: printf("Memory copy overlap"); break;
      case -9: printf("Image format mismatch"); break;
      case -10: printf("Image format not supported"); break;
      case -11: printf("Build program failure"); break;
      case -12: printf("Map failure"); break;
      case -30: printf("Invalid value"); break;
      case -31: printf("Invaid device type"); break;
      case -32: printf("Invalid platform"); break;
      case -33: printf("Invalid device"); break;
      case -34: printf("Invalid context"); break;
      case -35: printf("Invalid queue properties"); break;
      case -36: printf("Invalid command queue"); break;
      case -37: printf("Invalid host pointer"); break;
      case -38: printf("Invalid memory object"); break;
      case -39: printf("Invalid image format descriptor"); break;
      case -40: printf("Invalid image size"); break;
      case -41: printf("Invalid sampler"); break;
      case -42: printf("Invalid binary"); break;
      case -43: printf("Invalid build options"); break;
      case -44: printf("Invalid program"); break;
      case -45: printf("Invalid program executable"); break;
      case -46: printf("Invalid kernel name"); break;
      case -47: printf("Invalid kernel defintion"); break;
      case -48: printf("Invalid kernel"); break;
      case -49: printf("Invalid argument index"); break;
      case -50: printf("Invalid argument value"); break;
      case -51: printf("Invalid argument size"); break;
      case -52: printf("Invalid kernel arguments"); break;
      case -53: printf("Invalid work dimension"); break;
      case -54: printf("Invalid work group size"); break;
      case -55: printf("Invalid work item size"); break;
      case -56: printf("Invalid global offset"); break;
      case -57: printf("Invalid event wait list"); break;
      case -58: printf("Invalid event"); break;
      case -59: printf("Invalid operation"); break;
      case -60: printf("Invalid GL object"); break;
      case -61: printf("Invalid buffer size"); break;
      case -62: printf("Invalid mip level"); break;
      case -63: printf("Invalid global work size"); break;
    }
    assert(0);
  }
}

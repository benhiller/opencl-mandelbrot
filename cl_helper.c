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
	check_succeeded("Loading kernel", err);

	err = clBuildProgram(program[0], 0, NULL, NULL, NULL, NULL);
	check_succeeded("Building program", err);

	// Now create the kernel "objects" that we want to use in the example file
  // render is the name of the function we are going to run
	kernel[0] = clCreateKernel(program[0], "render", &err);
  return kernel[0];
}

cl_context create_context(cl_uint* num_devices) {
  cl_int err;
  cl_device_id *devices, cpus[16];
  devices = malloc(16 * sizeof(cl_device_id));
  cl_uint num_cpus;
  cl_context context;

  // First get the CPU device, as a fallback
	err = clGetDeviceIDs(NULL, CL_DEVICE_TYPE_CPU, 16, cpus, &num_cpus);
	check_succeeded("Getting device IDs", err);

  // Find the GPU CL device, this is what we really want
	// If there is no GPU device is CL capable, fall back to CPU
	err = clGetDeviceIDs(NULL, CL_DEVICE_TYPE_GPU, 16, devices, num_devices);
	if (err != CL_SUCCESS || *num_devices == 0) {
    devices = cpus;
    *num_devices = num_cpus;
  }
	assert(*devices);

  context = clCreateContext(0, *num_devices, devices, NULL, NULL, &err);
  check_succeeded("Creating context", err);

  return context;
}

void print_debug_info(cl_context context) {
  cl_device_id devices[16];
  size_t size;
  int elements;
  cl_int err;

  err = clGetContextInfo(context, CL_CONTEXT_DEVICES, sizeof(cl_device_id) * 16,
                         &devices, &size);
  check_succeeded("Getting context info", err);

  elements = size/sizeof(cl_device_id);

  int i;
  for(i = 0; i < elements; i++) {
    cl_char vendor_name[1024] = {0};
    cl_char device_name[1024] = {0};
    err = clGetDeviceInfo(devices[i], CL_DEVICE_VENDOR, sizeof(vendor_name),
                          vendor_name, NULL);
    err |= clGetDeviceInfo(devices[i], CL_DEVICE_NAME, sizeof(device_name),
                           device_name, NULL);
    check_succeeded("Getting device info", err);

    printf("Device %d: %s %s\n", i, vendor_name, device_name);
  }
}

void check_succeeded(char* message, cl_int err) {
  if(err != CL_SUCCESS) {
    // Abort and print debugging info
    printf("%s: ", message);
    switch(err) {
      case -1: printf("Device not found\n"); break;
      case -2: printf("Device not available\n"); break;
      case -3: printf("Compiler not available\n"); break;
      case -4: printf("Memory object allocation failure\n"); break;
      case -5: printf("Out of resources\n"); break;
      case -6: printf("Out of host memory\n"); break;
      case -7: printf("Profiling info not available\n"); break;
      case -8: printf("Memory copy overlap\n"); break;
      case -9: printf("Image format mismatch\n"); break;
      case -10: printf("Image format not supported\n"); break;
      case -11: printf("Build program failure\n"); break;
      case -12: printf("Map failure\n"); break;
      case -30: printf("Invalid value\n"); break;
      case -31: printf("Invaid device type\n"); break;
      case -32: printf("Invalid platform\n"); break;
      case -33: printf("Invalid device\n"); break;
      case -34: printf("Invalid context\n"); break;
      case -35: printf("Invalid queue properties\n"); break;
      case -36: printf("Invalid command queue\n"); break;
      case -37: printf("Invalid host pointer\n"); break;
      case -38: printf("Invalid memory object\n"); break;
      case -39: printf("Invalid image format descriptor\n"); break;
      case -40: printf("Invalid image size\n"); break;
      case -41: printf("Invalid sampler\n"); break;
      case -42: printf("Invalid binary\n"); break;
      case -43: printf("Invalid build options\n"); break;
      case -44: printf("Invalid program\n"); break;
      case -45: printf("Invalid program executable\n"); break;
      case -46: printf("Invalid kernel name\n"); break;
      case -47: printf("Invalid kernel defintion\n"); break;
      case -48: printf("Invalid kernel\n"); break;
      case -49: printf("Invalid argument index\n"); break;
      case -50: printf("Invalid argument value\n"); break;
      case -51: printf("Invalid argument size\n"); break;
      case -52: printf("Invalid kernel arguments\n"); break;
      case -53: printf("Invalid work dimension\n"); break;
      case -54: printf("Invalid work group size\n"); break;
      case -55: printf("Invalid work item size\n"); break;
      case -56: printf("Invalid global offset\n"); break;
      case -57: printf("Invalid event wait list\n"); break;
      case -58: printf("Invalid event\n"); break;
      case -59: printf("Invalid operation\n"); break;
      case -60: printf("Invalid GL object\n"); break;
      case -61: printf("Invalid buffer size\n"); break;
      case -62: printf("Invalid mip level\n"); break;
      case -63: printf("Invalid global work size\n"); break;
    }
    assert(0);
  }
}

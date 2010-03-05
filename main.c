// Note: Most of the code comes from the MacResearch OpenCL podcast

#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <sys/sysctl.h>
#include <sys/stat.h>

#include <OpenCL/OpenCL.h>

#include "bmp.h"

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

int runCL(int width, int height)
{
	cl_program program[1];
	cl_kernel  kernel[2];

	cl_command_queue cmd_queue;
	cl_context       context;

	cl_device_id cpu = NULL, device = NULL;

	cl_int err = 0;
	size_t returned_size = 0;
	
	// Multiply by 3 here, since we need red, green and blue for each pixel
  size_t buffer_size = sizeof(char) * width * height * 3;

	cl_mem image;

  char *host_image = (char *) malloc(buffer_size);

  // First get the CPU device, as a fallback
	err = clGetDeviceIDs(NULL, CL_DEVICE_TYPE_CPU, 1, &cpu, NULL);
	assert(err == CL_SUCCESS);
  
	// Find the GPU CL device, this is what we really want
	// If there is no GPU device is CL capable, fall back to CPU
	err = clGetDeviceIDs(NULL, CL_DEVICE_TYPE_GPU, 1, &device, NULL);
	if (err != CL_SUCCESS) device = cpu;
	assert(device);
  
	// Get some information about the returned device
	cl_char vendor_name[1024] = {0};
	cl_char device_name[1024] = {0};
	err = clGetDeviceInfo(device, CL_DEVICE_VENDOR, sizeof(vendor_name),
                        vendor_name, &returned_size);
	err |= clGetDeviceInfo(device, CL_DEVICE_NAME, sizeof(device_name),
                         device_name, &returned_size);
	assert(err == CL_SUCCESS);
	printf("Connecting to %s %s...\n", vendor_name, device_name);
	
	context = clCreateContext(0, 1, &device, NULL, NULL, &err);
	assert(err == CL_SUCCESS);

	cmd_queue = clCreateCommandQueue(context, device, 0, NULL);

  // Mark this write only, since the kernel does not have to read the image it 
  // is writing. I am not sure if this has any performance benefit.
	image	= clCreateBuffer(context, CL_MEM_WRITE_ONLY, buffer_size, NULL, NULL);

	// Load the program source from disk
	const char * filename = "mandelbrot.cl";
	char *program_source = load_program_source(filename);
	program[0] = clCreateProgramWithSource(context, 1, (const char**)&program_source,
                                         NULL, &err);
	assert(err == CL_SUCCESS);

	err = clBuildProgram(program[0], 0, NULL, NULL, NULL, NULL);
	assert(err == CL_SUCCESS);

	// Now create the kernel "objects" that we want to use in the example file
  // render is the name of the function we are going to run
	kernel[0] = clCreateKernel(program[0], "render", &err);

  // Now setup the arguments to our kernel
  // In our case, we just need to give it a pointer to the image
  err  = clSetKernelArg(kernel[0],  0, sizeof(cl_mem), &image);
  assert(err == CL_SUCCESS);

  // Run the calculation by enqueuing it and forcing the
  // command queue to complete the task
  size_t global_work_size[2] = {width, height};
  err = clEnqueueNDRangeKernel(cmd_queue, kernel[0], 2, NULL,
                             global_work_size, NULL, 0, NULL, NULL);
  assert(err == CL_SUCCESS);
  
  // Wait for the kernel to finish before reading the buffer
  // I am not sure this is necessary, since it is a queue, should double check
  clFinish(cmd_queue);

  err = clEnqueueReadBuffer(cmd_queue, image, CL_TRUE, 0, buffer_size, 
                            host_image, 0, NULL, NULL);
  assert(err == CL_SUCCESS);

  // Now write the file
  write_bmp("output.bmp", width, height, host_image);

	// Release OpenCL objects
	clReleaseMemObject(image);
	clReleaseCommandQueue(cmd_queue);
	clReleaseContext(context);
  free(host_image);
  
	return CL_SUCCESS;
}

int main(int argc, const char * argv[]) {
  runCL(1024, 1024);
  return 0;
}

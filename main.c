#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <sys/sysctl.h>
#include <sys/stat.h>

#include <OpenCL/OpenCL.h>

#include "bmp.h"

// Taken from macresearch
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
	cl_kernel kernel[2];

	cl_command_queue cmd_queue;
	cl_context   context;

	cl_device_id cpu = NULL, device = NULL;

	cl_int err = 0;
	size_t returned_size = 0;
  size_t buffer_size = sizeof(char) * width * height * 3;

	cl_mem image;

  char *host_image = (char *) malloc(buffer_size);

#pragma mark Device Information
	{
		// Find the CPU CL device, as a fallback
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
	}

#pragma mark Context and Command Queue
	{
		// Now create a context to perform our calculation with the
		// specified device
		context = clCreateContext(0, 1, &device, NULL, NULL, &err);
		assert(err == CL_SUCCESS);

		// And also a command queue for the context
		cmd_queue = clCreateCommandQueue(context, device, 0, NULL);
	}

#pragma mark Create Buffer
  {

		image	= clCreateBuffer(context, CL_MEM_WRITE_ONLY, buffer_size, NULL, NULL);
  }

#pragma mark Program and Kernel Creation
	{
		// Load the program source from disk
		// The kernel/program is the project directory and in Xcode the executable
		// is set to launch from that directory hence we use a relative path
		const char * filename = "mandelbrot.cl";
		char *program_source = load_program_source(filename);
		program[0] = clCreateProgramWithSource(context, 1, (const char**)&program_source,
                                           NULL, &err);
		assert(err == CL_SUCCESS);

		err = clBuildProgram(program[0], 0, NULL, NULL, NULL, NULL);
		assert(err == CL_SUCCESS);

		// Now create the kernel "objects" that we want to use in the example file
		kernel[0] = clCreateKernel(program[0], "render", &err);
	}

#pragma mark Kernel Arguments
	{
		// Now setup the arguments to our kernel
		err  = clSetKernelArg(kernel[0],  0, sizeof(cl_mem), &image);
		assert(err == CL_SUCCESS);
	}

#pragma mark Execution and Read
	{
		// Run the calculation by enqueuing it and forcing the
		// command queue to complete the task
		size_t global_work_size[2] = {width, height};
		err = clEnqueueNDRangeKernel(cmd_queue, kernel[0], 2, NULL,
                                 global_work_size, NULL, 0, NULL, NULL);
		assert(err == CL_SUCCESS);
		clFinish(cmd_queue);

    err = clEnqueueReadBuffer(cmd_queue, image, CL_TRUE, 0, buffer_size, host_image, 0, NULL, NULL);
		assert(err == CL_SUCCESS);
	}

#pragma mark File output
  {
    /* for(int i = 0; i < width; i++) {
      for(int j = 0; j < height; j++) {
        if(host_image[width*j + i*3] != 0) printf("X");
        else printf("0");
      }
      printf("\n");
    } */
    write_bmp("output.bmp", width, height, host_image);
  }


#pragma mark Teardown
	{
		clReleaseMemObject(image);

		clReleaseCommandQueue(cmd_queue);
		clReleaseContext(context);
	}
	return CL_SUCCESS;
}

int main(int argc, const char * argv[]) {
  runCL(1024, 1024);
  return 0;
}

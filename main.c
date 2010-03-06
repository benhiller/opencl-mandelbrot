// Note: Most of the code comes from the MacResearch OpenCL podcast

#include <stdio.h>
#include <assert.h>
#include <stdlib.h>

#include <OpenCL/OpenCL.h>

#include "bmp.h"
#include "cl_helper.h"

int runCL(int width, int height)
{
	cl_kernel  kernel;

	cl_command_queue cmd_queue;
	cl_context       context;

	cl_int err = 0;
  int devices = 0;
  cl_device_id device;

	size_t returned_size = 0;

	// Multiply by 3 here, since we need red, green and blue for each pixel
  size_t buffer_size = sizeof(char) * width * height * 3;

	cl_mem image;

  char *host_image = (char *) malloc(buffer_size);

	context = create_context(&devices);
  print_debug_info(context);

  err = clGetContextInfo(context, CL_CONTEXT_DEVICES, sizeof(cl_device_id), &device, NULL);
	assert(err == CL_SUCCESS);

	cmd_queue = clCreateCommandQueue(context, device, 0, NULL);

  // Mark this write only, since the kernel does not have to read the image it
  // is writing. I am not sure if this has any performance benefit.
	image	= clCreateBuffer(context, CL_MEM_WRITE_ONLY, buffer_size, NULL, NULL);

	// Load the program source from disk
	const char *filename = "mandelbrot.cl";
  kernel = load_kernel_from_file(context, filename);

  // Now setup the arguments to our kernel
  // In our case, we just need to give it a pointer to the image
  err  = clSetKernelArg(kernel,  0, sizeof(cl_mem), &image);
  assert(err == CL_SUCCESS);

  // Run the calculation by enqueuing it and forcing the
  // command queue to complete the task
  size_t global_work_size[2] = {width, height};
  err = clEnqueueNDRangeKernel(cmd_queue, kernel, 2, NULL,
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

// Note: Most of the code comes from the MacResearch OpenCL podcast

#include <stdio.h>
#include <stdlib.h>

#include <OpenCL/OpenCL.h>

#include "bmp.h"
#include "cl_helper.h"

int runCL(int width, int height)
{
	cl_kernel  kernel;

	cl_command_queue cmd_queue[16];
	cl_context       context;

	cl_int err = 0;
  cl_uint num_devices = 0;
  cl_device_id devices[16];

	size_t returned_size = 0;

	// Multiply by 3 here, since we need red, green and blue for each pixel
  size_t buffer_size = sizeof(char) * width * height * 3;

	cl_mem image;

  char *host_image = (char *) malloc(buffer_size);

	context = create_context(&num_devices);
  if(num_devices == 0) {
    printf("No compute devices found\n");
    return -1;
  }
  print_debug_info(context);

  err = clGetContextInfo(context, CL_CONTEXT_DEVICES, sizeof(cl_device_id) * 16,
                         &devices, NULL);
	check_succeeded("Getting context info", err);

  int i;
  for(i = 0; i < num_devices; i++) {
    cmd_queue[i] = clCreateCommandQueue(context, devices[i], 0, &err);
    check_succeeded("Creating command queue", err);
  }

  // Mark this write only, since the kernel does not have to read the image it
  // is writing. I am not sure if this has any performance benefit.
	image	= clCreateBuffer(context, CL_MEM_WRITE_ONLY, buffer_size, NULL, &err);
  check_succeeded("Creating buffer", err);

	// Load the program source from disk
	const char *filename = "mandelbrot.cl";
  kernel = load_kernel_from_file(context, filename);

  // Now setup the arguments to our kernel
  // In our case, we just need to give it a pointer to the image
  err  = clSetKernelArg(kernel, 0, sizeof(cl_mem), &image);
  check_succeeded("Setting kernel arg", err);

  // Run the calculation by enqueuing it and forcing the
  // command queue to complete the task
  // To support multiple compute devices, need to split this up
  // among all of them. Easiest way to split up is block alloc.
  //
  // Assuming that num_devices divides width and height evenly
  size_t device_work_size[2] = {width, height/num_devices};
  for(i = 0; i < num_devices; i++) {
    size_t device_work_offset[2] = {0, device_work_size[1]*i};
    size_t offset = device_work_offset[1]*3*width;
    err = clEnqueueNDRangeKernel(cmd_queue[i], kernel, 2, device_work_offset,
                                 device_work_size, NULL, 0, NULL, NULL);
    check_succeeded("Running kernel", err);

    // Non-blocking read, so we can continue queuing up more kernels
    err = clEnqueueReadBuffer(cmd_queue[i], image, CL_FALSE, offset,
                              buffer_size/num_devices,
                              host_image, 0, NULL, NULL);
    check_succeeded("Reading buffer", err);
  }
  for(i = 0; i < num_devices; i++) {
    clFinish(cmd_queue[i]);
  }

  // Now write the file
  write_bmp("output.bmp", width, height, host_image);

	// Release OpenCL objects
	clReleaseMemObject(image);
  for(i = 0; i < num_devices; i++) {
    clReleaseCommandQueue(cmd_queue[i]);
  }
	clReleaseContext(context);
  free(host_image);

	return CL_SUCCESS;
}

int main(int argc, const char * argv[]) {
  runCL(1024, 1024);
  return 0;
}

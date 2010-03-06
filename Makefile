C = gcc
OPENCL = -framework OpenCL

all: mandelbrot

mandelbrot: main.c bmp.o cl_helper.o
	$(C) $(OPENCL) -o mandelbrot main.c bmp.o cl_helper.o

bmp.o: bmp.c bmp.h
	$(C) -c bmp.c

cl_helper.o: cl_helper.c cl_helper.h
	$(C) -c cl_helper.c


clean:
	rm -f mandelbrot output.bmp *.o

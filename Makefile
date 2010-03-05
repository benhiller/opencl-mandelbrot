C = gcc
OPENCL = -framework OpenCL

all: mandelbrot

mandelbrot: main.c bmp
	$(C) $(OPENCL) -o mandelbrot main.c bmp.o

bmp: bmp.c bmp.h
	$(C) -c bmp.c

clean:
	rm -f mandelbrot output.bmp *.o

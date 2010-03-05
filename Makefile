C = gcc
OPENCL = -framework OpenCL

all: mandelbrot

mandelbrot: main.c bmp.o
	$(C) $(OPENCL) -o mandelbrot main.c bmp.o

bmp.o: bmp.c bmp.h
	$(C) -c bmp.c

clean:
	rm -f mandelbrot output.bmp *.o

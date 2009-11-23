default: all

all: libresample

libresample: filterkit.o resample.o resamplesubs.o
	ar -r libresample.a *.o

%.o: %.c
	gcc -c $< -o $@
libfooid_test: libfooid main.o
	gcc main.o -L. -L./libresample -lfooid -lsndfile -lresample -o test

libfooid: common.o fooid.o harmonics.o regress.o s_fft.o spectrum.o
	ar -r libfooid.a *.o

%.o : %.c
	gcc -c $< -o $@ -DSLOWROUND
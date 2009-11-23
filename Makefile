libfooid_test: libfooid main.o
	gcc main.o -L. -L./libresample -lfooid -lsndfile -lresample -o test

$OBJS = common.o \
	fooid.o \
	harmonics.o \
	regress.o \
	s_fft.o \
	spectrum.o

libfooid: $(OBJS)
	ar -r libfooid.a $(OBJS)

%.o : %.c
	gcc -c $< -o $@ -Wall
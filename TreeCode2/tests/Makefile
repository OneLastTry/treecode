LIBDIR = ../
INCDIR = ../

CFLAGS   = -Wall -g -I/usr/include/eigen3 -D_GLIBCXX_PARALLEL -I$(INCDIR)
OPTFLAGS = -O3 -msse -msse2 -msse3 -msse4a -ffast-math -march=barcelona -m64 -fopenmp
LDFLAGS  = -g -lboost_unit_test_framework -fopenmp -L$(LIBDIR)
CC = gfilt

NAME = tests 

test:
	$(CC) $(LDFLAGS) $(CFLAGS) TreeTest.cpp -o $(NAME)
	
clean:
	rm -f $(NAME)
 

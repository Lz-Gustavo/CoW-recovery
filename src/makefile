CC = g++
LIBS = -lpthread -std=c++11

cow: CoW-library.o
	$(CC) CoW-library.o -o cow $(LIBS)

Cow-library.o: libcow.h config.txt
	$(CC) -c CoW-library.cpp

clean:
	rm *.o cow
CC = g++
CFLAGS = -Wall
LIBS = -lsndfile
all: normaliserdoethet
normaliserdoethet: normaliserdoethet.cpp
	g++ -c normaliserdoethet.cpp  ${CFLAGS} ${LIBS}
	g++ -o $@ $@.o ${LIBS}
clean:
	rm -rf *.o

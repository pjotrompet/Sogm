CC = g++
CFLAGS = -Wall

all: guitarmain

guitarmain: modulodistortion.o guitar_main.o amp.o
	$(CC) -o $@ $(CFLAGS) amp.o modulodistortion.o guitar_main.o

.cpp.o:
	$(CC) -c $< $(CFLAGS)

clean:
	rm -f *.o
	rm -f `find . -perm +111 -type f`


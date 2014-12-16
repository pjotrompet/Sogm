CC = g++
CFLAGS = -Wall

all: song

song: song.o song_main.o
	$(CC) -o $@ $(CFLAGS) song.o song_main.o

.cpp.o:
	$(CC) -c $< $(CFLAGS)

clean:
	rm -f *.o
	rm -f `find . -perm +111 -type f`


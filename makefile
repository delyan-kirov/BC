CC = g++
CFLAGS = -std=c++23 -Wall

main.exe: main.cpp
	$(CC) $(CFLAGS) -o main.exe main.cpp

.PHONY: clean bear

clean:
	rm main

bear:
	bear -- make main.exe

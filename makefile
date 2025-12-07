CC = clang++
CFLAGS = -Wall -g -o0

main.exe: main.cpp
	$(CC) $(CFLAGS) -o main.exe main.cpp

.PHONY: clean bear

clean:
	rm main.exe

bear:
	bear -- make main.exe

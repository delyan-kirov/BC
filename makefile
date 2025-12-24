CC = clang++
CFLAGS = -Wall -g -o0

main: main.cpp
	$(CC) $(CFLAGS) -o main main.cpp

.PHONY: clean bear

clean:
	rm main.exe

bear:
	bear -- make main.exe

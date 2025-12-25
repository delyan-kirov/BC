SRC = src
INC = inc
BIN = bin

CFLAGS = -Wall -Wextra -g -O0
CC = clang++ $(CFLAGS) -I$(INC)

OBJS = \
	$(BIN)/main.o \
	$(BIN)/LX.o \
	$(BIN)/EX.o

$(BIN)/main: $(OBJS)
	$(CC) -o $@ $^

$(BIN)/main.o: $(SRC)/main.cpp $(BIN)/EX.o
	$(CC) -c $< -o $@

$(BIN)/EX.o: $(SRC)/EX.cpp $(INC)/EX.hpp $(BIN)/LX.o
	$(CC) -c $< -o $@

$(BIN)/LX.o: $(SRC)/LX.cpp $(INC)/LX.hpp
	$(CC) -c $< -o $@

.PHONY: clean bear

clean:
	rm -f $(BIN)/*

bear:
	make clean
	bear -- make $(BIN)/main

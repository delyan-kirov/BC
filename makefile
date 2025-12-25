CC = clang++
CFLAGS = -Wall -g -O0
SRC = src
INC = inc
BIN = bin

OBJS = \
	$(BIN)/main.o \
	$(BIN)/parser.o \
	$(BIN)/tokenizer.o

$(BIN)/main: $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

$(BIN)/main.o: $(SRC)/main.cpp $(INC)/parser.hpp
	$(CC) $(CFLAGS) -I$(INC) -c $< -o $@

$(BIN)/parser.o: $(SRC)/parser.cpp $(INC)/parser.hpp $(INC)/tokenizer.hpp
	$(CC) $(CFLAGS) -I$(INC) -c $< -o $@

$(BIN)/tokenizer.o: $(SRC)/tokenizer.cpp $(INC)/tokenizer.hpp
	$(CC) $(CFLAGS) -I$(INC) -c $< -o $@

clean:
	rm -f $(BIN)/*

.PHONY: clean

bear:
	make clean
	bear -- make $(BIN)/main

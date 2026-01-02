SRC = src
INC = inc
BIN = bin
TST = tst

CFLAGS = -Wall -Wextra -Wimplicit-fallthrough -Werror -g -O0
CFLAGS += -DTRACE_ENABLED
CC = g++ $(CFLAGS) -I$(INC)

#-----------------------------OBJCS-----------------------------

OBJS = \
	$(BIN)/main.o \
	$(BIN)/LX.o \
	$(BIN)/AR.o

$(BIN)/main: $(OBJS)
	$(CC) -o $@ $^

$(BIN)/main.o: $(SRC)/main.cpp $(BIN)/LX.o
	$(CC) -c $< -o $@

$(BIN)/AR.o: $(SRC)/AR.cpp $(INC)/AR.hpp
	$(CC) -c $< -o $@

$(BIN)/LX.o: $(SRC)/LX.cpp $(INC)/LX.hpp $(BIN)/AR.o
	$(CC) -c $< -o $@

#-----------------------------TESTS-----------------------------
tests = \
	tst_mult

TOBJS = \
	$(BIN)/LX.o \
	$(BIN)/AR.o

$(BIN)/tst_mult: $(TST)/tst_mult.cpp $(TOBJS)
	$(CC) -o $@ $^

test: $(BIN)/tst_mult $(BIN)/main
	@$(BIN)/tst_mult

.PHONY: clean bear test init

init:
	mkdir -p $(BIN)
	make clean
	make test

clean:
	rm -f $(BIN)/*

bear:
	mkdir -p $(BIN)
	make clean
	bear -- make $(BIN)/main test

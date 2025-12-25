SRC = src
INC = inc
BIN = bin
TST = tst

CFLAGS = -Wall -Wextra -g -O0
CC = clang++ $(CFLAGS) -I$(INC)

#-----------------------------OBJCS-----------------------------

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

#-----------------------------TESTS-----------------------------
tests = tst_addition_n_subtraction

TOBJS = \
	$(BIN)/LX.o \
	$(BIN)/EX.o

$(BIN)/tst_addition_n_subtraction: $(TST)/tst_addition_n_subtraction.cpp $(TOBJS)
	$(CC) -o $@ $^

test: $(BIN)/tst_addition_n_subtraction
	@$(BIN)/tst_addition_n_subtraction

.PHONY: clean bear test

clean:
	rm -f $(BIN)/*

bear:
	make clean
	bear -- make $(BIN)/main test

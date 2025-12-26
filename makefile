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
	$(BIN)/EX.o \
	$(BIN)/AR.o

$(BIN)/main: $(OBJS)
	$(CC) -o $@ $^

$(BIN)/main.o: $(SRC)/main.cpp $(BIN)/EX.o
	$(CC) -c $< -o $@

$(BIN)/EX.o: $(SRC)/EX.cpp $(INC)/EX.hpp $(BIN)/LX.o
	$(CC) -c $< -o $@

$(BIN)/LX.o: $(SRC)/LX.cpp $(INC)/LX.hpp
	$(CC) -c $< -o $@

#-----------------------------SLIBS-----------------------------
# 
$(BIN)/AR.o: $(SRC)/AR.cpp $(INC)/AR.hpp
	$(CC) -c $< -o $@

#-----------------------------TESTS-----------------------------
tests = \
	tst_addition_n_subtraction \
	tst_arena

TOBJS = \
	$(BIN)/LX.o \
	$(BIN)/EX.o \
	$(BIN)/AR.o \

$(BIN)/tst_addition_n_subtraction: $(TST)/tst_addition_n_subtraction.cpp $(TOBJS)
	$(CC) -o $@ $^

$(BIN)/tst_arena: $(TST)/tst_arena.cpp $(TOBJS)
	$(CC) -o $@ $^

test: $(BIN)/tst_addition_n_subtraction $(BIN)/tst_arena
	@$(BIN)/tst_addition_n_subtraction
	@$(BIN)/tst_arena

.PHONY: clean bear test

clean:
	rm -f $(BIN)/*

bear:
	make clean
	bear -- make $(BIN)/main test

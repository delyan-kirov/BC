SRC = src
INC = inc
BIN = bin
TST = tst

CFLAGS = -Wall -Wextra -Wimplicit-fallthrough -g -O0
CC = clang++ $(CFLAGS) -I$(INC)

#-----------------------------OBJCS-----------------------------

OBJS = \
	$(BIN)/main.o \
	$(BIN)/LX.o \
	$(BIN)/EX.o \
	$(BIN)/AR.o \
	$(BIN)/TL.o 

$(BIN)/main: $(OBJS)
	$(CC) -o $@ $^

$(BIN)/main.o: $(SRC)/main.cpp $(BIN)/EX.o
	$(CC) -c $< -o $@

$(BIN)/TL.o: $(SRC)/TL.cpp $(BIN)/EX.o
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
	$(BIN)/TL.o

$(BIN)/tst_addition_n_subtraction: $(TST)/tst_addition_n_subtraction.cpp $(TOBJS)
	$(CC) -o $@ $^

$(BIN)/tst_mult: $(TST)/tst_mult.cpp $(TOBJS)
	$(CC) -o $@ $^

$(BIN)/tst_arena: $(TST)/tst_arena.cpp $(TOBJS)
	$(CC) -o $@ $^

test: $(BIN)/tst_addition_n_subtraction $(BIN)/tst_arena $(BIN)/tst_mult
	# @$(BIN)/tst_addition_n_subtraction
	# @$(BIN)/tst_arena
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

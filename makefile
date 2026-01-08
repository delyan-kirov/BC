.PHONY: clean bear test init

SRC = src
INC = inc
BIN = bin
TST = tst

CFLAGS = -Wall -Wextra -Wimplicit-fallthrough -Werror -g -O0
# CFLAGS += -DTRACE_ENABLED
CC = g++ $(CFLAGS) -I$(INC)
CFSO = -fPIC -shared

#-----------------------------OBJCS-----------------------------

FrontEndSrc = \
	$(SRC)/AR.cpp \
	$(SRC)/LX.cpp \
	$(SRC)/EX.cpp

FrontEndInc = \
	$(INC)/AR.hpp \
	$(INC)/LX.hpp \
	$(INC)/UT.hpp \
	$(INC)/ER.hpp \
	$(INC)/EX.hpp

FrontEnd = $(BIN)/frontEnd.so

$(BIN)/main: $(FrontEnd)
	$(CC) $(SRC)/main.cpp -o $@ $^

$(FrontEnd): $(FrontEndInc) $(FrontEndSrc)
	$(CC) $(CFSO) $(FrontEndSrc) -o $@

#-----------------------------TESTS-----------------------------

$(BIN)/tst_mult: $(TST)/tst_mult.cpp $(BIN)/main
	$(CC) $(FrontEnd) $(TST)/tst_mult.cpp -o $@

test: $(BIN)/tst_mult
	@$(BIN)/tst_mult

all:
	make
	make test

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

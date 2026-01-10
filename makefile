#------------------------------DIRS-----------------------------

SRC = src/
INC = inc/
BIN = bin/
TST = tst/

CFLAGS = -Wall -Wextra -Wimplicit-fallthrough -Werror -g -O0
CFLAGS += -DTRACE_ENABLED
CC = g++ $(CFLAGS) -I$(INC)
CFSO = -fPIC -shared

#------------------------------MAIN-----------------------------
# 
test: $(BIN)tst_mult
	@$(BIN)tst_mult

#------------------------------OBJC-----------------------------
BCsrc = \
	$(SRC)AR.cpp \
	$(SRC)LX.cpp \
	$(SRC)EX.cpp

BCinc = \
	$(INC)AR.hpp \
	$(INC)LX.hpp \
	$(INC)UT.hpp \
	$(INC)ER.hpp \
	$(INC)EX.hpp

BC = $(BIN)bc.so

# $(BIN)main: $(BC)
# 	$(CC) $(SRC)main.cpp -o $@ $^

$(BC): $(BCinc) $(BCsrc)
	$(CC) $(CFSO) $(BCsrc) -o $@

#-----------------------------TEST------------------------------

$(BIN)tst_mult: $(TST)tst_mult.cpp $(BC)
	$(CC) $(BC) $(TST)tst_mult.cpp -o $@

#-----------------------------CMND------------------------------
COMMANDS = clean bear test init list
.PHONY: COMMANDS

list:
	@true
	$(foreach command, $(COMMANDS), $(info $(command)))

all:
	make
	make test

init:
	mkdir -p $(BIN)
	make clean
	make test

clean:
	rm -f $(BIN)*

bear:
	mkdir -p $(BIN)
	make clean
	bear -- make test

#------------------------------DIRS-----------------------------
BIN = bin/
TST = tst/
INC = inc/

CFLAGS = -Wall -Wextra -Wimplicit-fallthrough -Werror -g -O0
CC = g++ $(CFLAGS) -I$(INC)

#-----------------------------TEST------------------------------

BC = $(BIN)bc.so

$(BIN)tst_mult: $(BC) $(TST)tst_mult.cpp
	$(CC) $(BC) $(TST)tst_mult.cpp -o $@

#-----------------------------CMND------------------------------
COMMANDS = clean bear test init list
.PHONY: COMMANDS

test: $(BIN)tst_mult
	@$(BIN)tst_mult

list:
	@true
	$(foreach command, $(COMMANDS), $(info $(command)))

$(BC):
	$(MAKE) -C src

all:
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

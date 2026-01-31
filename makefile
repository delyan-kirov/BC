#------------------------------DIRS-----------------------------

SRC = src/
INC = inc/
BIN = bin/
TST = tst/

CFLAGS = -Wall -Wextra -Wimplicit-fallthrough -Werror -g -O1
CC = clang++ $(CFLAGS) -I$(INC)
CFSO = -fPIC -shared

#------------------------------MAIN-----------------------------
# 
test: $(BIN)tst_mult $(BIN)tst_addition_n_subtraction
	@$(BIN)tst_mult
	@$(BIN)tst_addition_n_subtraction

#------------------------------OBJC-----------------------------
BCsrc = \
	$(SRC)LX.cpp \
	$(SRC)EX.cpp \
	$(SRC)TL.cpp

BCinc = \
	$(INC)LX.hpp \
	$(INC)UT.hpp \
	$(INC)EX.hpp \
	$(INC)TL.hpp

BC = $(BIN)bc.so

# $(BIN)main: $(BC)
# 	$(CC) $(SRC)main.cpp -o $@ $^

$(BC): $(BCinc) $(BCsrc)
	$(CC) $(CFSO) $(BCsrc) -o $@

#-----------------------------TEST------------------------------

$(BIN)tst_mult: $(TST)tst_mult.cpp $(BC)
	$(CC) $(BC) $(TST)tst_mult.cpp -o $@

$(BIN)tst_addition_n_subtraction: $(TST)tst_addition_n_subtraction.cpp $(BC)
	$(CC) $(BC) $(TST)tst_addition_n_subtraction.cpp -o $@

#-----------------------------CMND------------------------------
COMMANDS = clean bear test init list format valgrind gf2 trace executables
.PHONY: COMMANDS

executables: $(BC)
	@true

trace: CFLAGS += -DTRACE_ENABLED
trace: clean $(BC)
	make

debug:
	gf2 $(BIN)tst_mult &

list:
	@true
	$(foreach command, $(COMMANDS), $(info $(command)))

valgrind:
	valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes --verbose ./bin/tst_mult

format:
	find . -regex '.*\.\(cpp\|hpp\|c\|h\)$\' -exec clang-format -i {} + 

all:
	make
	make test

init:
	mkdir -p $(BIN)
	make clean
	make test
	make valgrind

clean:
	rm -f tmp.*
	rm -f vgcore*
	rm -f $(BIN)*

bear:
	mkdir -p $(BIN)
	make clean
	bear -- make test

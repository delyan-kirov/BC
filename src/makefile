INC = ../inc/
BIN = ../bin/

CFLAGS = -Wall -Wextra -Wimplicit-fallthrough -Werror -g -O0
CFLAGS += -DTRACE_ENABLED
CC = g++ $(CFLAGS) -I$(INC)
CFSO = -fPIC -shared

BCsrc = \
	AR.cpp \
	LX.cpp \
	EX.cpp

BCinc = \
	$(INC)AR.hpp \
	$(INC)LX.hpp \
	$(INC)UT.hpp \
	$(INC)ER.hpp \
	$(INC)EX.hpp

BC = $(BIN)bc.so

$(BC): $(BCinc) $(BCsrc)
	$(CC) $(CFSO) $(BCsrc) -o $@

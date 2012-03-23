CCOMP = gcc

LOADER = gcc

LIB = -lcgi

SRC = src

BIN = cgi-bin

all: vscpc.cgi

vscpc.cgi: vscpc.o
	$(LOADER) $(LIB) vscpc.o -o $(BIN)/vscpc.cgi

vscpc.o: $(SRC)/vscpc.c
	$(CCOMP) $(LIB) -c $(SRC)/vscpc.c

clean:
	rm -f *.o
	rm -f $(BIN)/*

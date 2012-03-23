CCOMP = gcc

LOADER = gcc

LIB = -lpthread

SRC = src

BIN = bin

all: vscpd

vscpd: vscpd.o
	$(LOADER) $(LIB) vscpd.o -o $(BIN)/vscpd 
	
vscpd.o: $(SRC)/vscpd.c
	$(CCOMP) $(LIB) -c $(SRC)/vscpd.c

clean:
	rm -f *.o
	rm -f $(BIN)/*



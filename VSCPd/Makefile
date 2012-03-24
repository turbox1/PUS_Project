CCOMP = gcc

LOADER = gcc

LIB = -lvscp_fun -lpthread

SRC = src

BIN = bin

INC_P = ../VSCP/src
LIB_P = ../VSCP/lib


all: vscpd

vscpd: vscpd.o
	$(LOADER) vscpd.o -o $(BIN)/vscpd -I $(INC_P) -L $(LIB_P) $(LIB)
	
vscpd.o: $(SRC)/vscpd.c 
	$(CCOMP) -I $(INC_P) -L $(LIB_P) $(LIB) -c $(SRC)/vscpd.c

clean:
	rm -f *.o
	rm -f $(BIN)/*



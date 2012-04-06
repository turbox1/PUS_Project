CCOMP = gcc

LOADER = gcc

SRC = src

LIB = lib


all: libvscp_fun.a


libvscp_fun.a: vscp_fun.o
	ar rs $(LIB)/libvscp_fun.a vscp_fun.o

vscp_fun.o: $(SRC)/vscp_fun.c $(SRC)/vscp_fun.h $(SRC)/vscp_frame.h
	$(CCOMP) -c $(SRC)/vscp_fun.c

clean:
	rm -f *.o
	rm -f $(LIB)/*.a

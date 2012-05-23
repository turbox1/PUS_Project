all: vscpc.cgi

OFLAGS = -O3

vscpc.cgi: vscpc.o
	$(CC) $(OFLAGS) $(LFLAGS) $(CFLAGS) vscpc.o -o vscpc.cgi

vscpc.o: vscpc.c vscpc_cfg.h
	$(CC) $(CFLAGS) -c vscpc.c 

clean:
	rm -f *.c

all: vscpc.cgi

OFLAGS = -O3

vscpc.cgi: vscpc.o
	$(CC) $(OFLAGS) $(LFLAGS) $(CFLAGS) vscpc.o -o vscpc.cgi

vscpc.o: vscpc.c vscp_frame.h
	$(CC) $(CFLAGS) vscpc.c -c 

clean:
	rm -f *.c

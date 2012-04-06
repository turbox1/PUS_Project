all: vscpd

vscpd: vscpd.o uart.o
	$(CC) vscpd.o uart.o -o vscpd $(CFLAGS)

vscpd.o: vscpd.c vscp_frame.h
	$(CC) $(CFLAGS) vscpd.c -c

uart.o: uart.h uart.c
	$(CC) uart.c -c

clean:
	rm -f *.o

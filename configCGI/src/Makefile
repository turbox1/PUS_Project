all: confc.cgi

OFLAGS = -O3

confc.cgi: confc.o
	$(CC) $(OFLAGS) $(LFLAGS) $(CFLAGS) confc.o -o confc.cgi

confc.o: confc.c
	$(CC) $(CFLAGS) -c confc.c 

clean:
	rm -f *.cgi
	rm -f *.o
	rm -f *.conf

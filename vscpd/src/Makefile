all: vscpd

OPT= -O3

vscpd: main.o uart.o vscp_firmware.o vscp_log.o
	$(CC) $(OPT) -lpthread main.o vscp_firmware.o vscp_log.o uart.o -o vscpd

main.o: main.c src/vscp_config.h
	$(CC) $(CFLAGS) $(OPT) -c main.c

uart.o:	src/uart.c src/uart.h
	$(CC) $(OPT) -c src/uart.c

vscp_firmware.o: src/vscp_firmware.c src/vscp_firmware.h src/vscp_config.h
	$(CC) $(CFLAGS) $(OPT) -c src/vscp_firmware.c

vscp_log.o: src/vscp_log.c src/vscp_log.h
	$(CC) $(CFLAGS) $(OPT) -c src/vscp_log.c

clean:
	rm -f *.o

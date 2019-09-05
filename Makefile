CC=gcc

CFLAGS= -Wall

TARGET= hcraft

all: $(TARGET)
	@echo
	@echo "Sources Compiled!"
	@echo

clean:
	rm -f $(TARGET) core *.o *~ 

hcraft: main.o usage.o modes.o hex.o 
	@echo "Compiling hcraft..."
	$(CC) $(CFLAGS) -o hcraft main.o usage.o modes.o hex.o
	@echo

main.o: main.c hcraft.h
	@echo "Compiling main object..."
	$(CC) $(CFLAGS) -c main.c

usage.o: usage.c hcraft.h
	@echo "Compiling usage object..."
	$(CC) $(CFLAGS) -c usage.c

modes.o: modes.c hcraft.h
	@echo "Compiling modes object..."
	$(CC) $(CFLAGS) -c modes.c

hex.o: hex.c hcraft.h
	@echo "Compiling hex object..."
	$(CC) $(CFLAGS) -c hex.c

install:
	@echo "Installing Package Components..."
	strip hcraft
	@echo "Installing hcraft..."
	install -m 755 hcraft /usr/local/bin
	install -m 644 hcraft.modes /etc

uninstall:
	rm -f /usr/local/bin/hcraft


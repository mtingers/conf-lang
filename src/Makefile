BIN=conflang.bin
CFLAGS=-Wall -O2

all: conflang

conflang: util.o parse.o main.o def.o
	cc $(CFLAGS) -o $(BIN) util.o main.o parse.o def.o

debug: CFLAGS = -Wall -g -DDEBUG

debug: clean all

profile: CFLAGS = -Wall -g -pg

profile: clean all

clean:
	rm -f *.o
	rm -f $(BIN)

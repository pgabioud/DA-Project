$(shell mkdir bin -p)
INCLUDES = include
SRC = src
BIN = bin
SAN = -fsanitize=address

CFLAGS=-Wall -Wextra -g -pthread $(SAN) -std=gnu11 -I $(INCLUDES)/ 

$(BINDIR):
	mkdir -p $(BINDIR)

da_proc: $(BIN)/Protocol.o $(BIN)/Utils.o $(BIN)/da_proc.o
	g++ -std=c++0x -Wall $(CFLAGS) $(BIN)/Protocol.o $(BIN)/Utils.o $(BIN)/da_proc.o -o da_proc

$(BIN)/Protocol.o : $(SRC)/Protocol.cpp $(INCLUDES)/Protocol.h
	g++ -c $(CFLAGS) $(SRC)/Protocol.cpp -o $@

$(BIN)/Utils.o : $(SRC)/Utils.cpp $(INCLUDES)/Utils.h
	g++ -c $(CFLAGS) $(SRC)/Utils.cpp -o $@

$(BIN)/da_proc.o : $(SRC)/da_proc.cpp
	g++ -c $(CFLAGS) $(SRC)/da_proc.cpp -lpthread -o $@

clean:
	rm $(BIN)/*.o da_proc

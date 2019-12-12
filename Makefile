$(shell mkdir bin -p)
INCLUDES = include
SRC = src
BIN = bin
SAN = -fsanitize=address

CFLAGS=-Wall -g -pthread -std=c++11 -I $(INCLUDES)/ 

$(BINDIR):
	mkdir -p $(BINDIR)																																																																																																																																																																																																																																																																																																																																																		

da_proc: $(BIN)/Protocol.o $(BIN)/UDP.o $(BIN)/StubbornLinks.o $(BIN)/PerfectLinks.o $(BIN)/Urb.o $(BIN)/LCB.o $(BIN)/Fifo.o $(BIN)/Utils.o $(BIN)/da_proc.o
	g++ $(CFLAGS) $(BIN)/Protocol.o $(BIN)/UDP.o $(BIN)/StubbornLinks.o $(BIN)/PerfectLinks.o $(BIN)/Urb.o $(BIN)/LCB.o $(BIN)/Fifo.o $(BIN)/Utils.o $(BIN)/da_proc.o -o da_proc

$(BIN)/Protocol.o : $(SRC)/Protocol.cpp $(INCLUDES)/Protocol.h
	g++ -c $(CFLAGS) $(SRC)/Protocol.cpp -o $@

$(BIN)/UDP.o : $(SRC)/UDP.cpp $(INCLUDES)/UDP.h $(INCLUDES)/Protocol.h
	g++ -c $(CFLAGS) $(SRC)/UDP.cpp -o $@

$(BIN)/StubbornLinks.o : $(SRC)/StubbornLinks.cpp $(INCLUDES)/StubbornLinks.h $(INCLUDES)/UDP.h
	g++ -c $(CFLAGS) $(SRC)/StubbornLinks.cpp -o $@

$(BIN)/PerfectLinks.o : $(SRC)/PerfectLinks.cpp $(INCLUDES)/PerfectLinks.h $(INCLUDES)/StubbornLinks.h
	g++ -c $(CFLAGS) $(SRC)/PerfectLinks.cpp -o $@

$(BIN)/Urb.o : $(SRC)/Urb.cpp $(INCLUDES)/Urb.h $(INCLUDES)/PerfectLinks.h
	g++ -c $(CFLAGS) $(SRC)/Urb.cpp -o $@

$(BIN)/LCB.o : $(SRC)/LCB.cpp $(INCLUDES)/LCB.h $(INCLUDES)/Urb.h
	g++ -c $(CFLAGS) $(SRC)/LCB.cpp -o $@

$(BIN)/Fifo.o : $(SRC)/Fifo.cpp $(INCLUDES)/Fifo.h $(INCLUDES)/Urb.h
	g++ -c $(CFLAGS) $(SRC)/Fifo.cpp -o $@

$(BIN)/Utils.o : $(SRC)/Utils.cpp $(INCLUDES)/Utils.h
	g++ -c $(CFLAGS) $(SRC)/Utils.cpp -o $@

$(BIN)/da_proc.o : $(SRC)/da_proc.cpp
	g++ -c $(CFLAGS) $(SRC)/da_proc.cpp -lpthread -o $@

clean:
	rm $(BIN)/*.o da_proc

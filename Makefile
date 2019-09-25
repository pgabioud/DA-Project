$(shell   mkdir -p bin)

SRCDIR   = src
BINDIR   = bin
INCLUDES = include
CC=g++
CFLAGS=-Wall

all: $(BINDIR)/da_proc
	
$(BINDIR)/da_proc : $(SRCDIR)/da_proc.cpp
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -f $(BINDIR)/da_proc $(BINDIR)/*.o 

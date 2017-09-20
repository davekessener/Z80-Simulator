CC=g++
SRC=$(wildcard *.cc)
OBJ=$(SRC:.cc=.o)
DEP=$(wildcard *.h)
CFLAGS=-ggdb -Wall -I..
TARGET=asm.exe

.PHONY: all clean

%.o: %.cc $(DEP)
	$(CC) $(CFLAGS) -c $< -o $@

all: $(TARGET)
	$(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(LINKFLAGS) $(OBJ) -o $(TARGET) $(LIBS)

clean:
	rm -f *.o $(TARGET)


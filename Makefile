LIBDIR=C:\MinGW\msys\1.0\home\dave\libs
CC=g++
SRC=$(wildcard *.cc)
OBJ=$(SRC:.cc=.o)
DEP=$(wildcard *.h)
CFLAGS=-Wall -ggdb -Wl,-subsystem,windows -O0 -I$(LIBDIR)\include\SDL2
LINKFLAGS=-L$(LIBDIR)\lib
LIBS=-lmingw32 -lSDL2main -lSDL2
TARGET=Z80.exe

.PHONY: all clean

%.o: %.cc $(DEP)
	$(CC) $(CFLAGS) -c $< -o $@

all: $(TARGET)
	$(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(LINKFLAGS) $(OBJ) -o $(TARGET) $(LIBS)

clean:
	rm -f *.o $(TARGET)


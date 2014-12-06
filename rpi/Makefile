# Makefile for rpi-pwrbtn

CFLAGS=-c -Wall
LDFLAGS=-L.
SOURCES=main.cpp cprocid.cpp crpi_gpio.cpp
OBJECTS=$(SOURCES:.cpp=.o)
EXECUTABLE=rpi-pwrbtn.bin

all: $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CXX) $(OBJECTS) -o $@ $(LDFLAGS)

.cpp.o:
	$(CXX) $(INCLUDES) $(CFLAGS) $< -o $@

clean:
	rm -rf *.o $(EXECUTABLE)


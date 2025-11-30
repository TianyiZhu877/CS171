CC = g++
FLAGS = -g -std=c++11 -I./include -I../
SOURCES = *.cpp

all: utils.o

utils.o: $(SOURCES)
	$(CC) $(FLAGS) -c $(SOURCES) -o utils.o

clean:
	rm -f *.o

.PHONY: clean all

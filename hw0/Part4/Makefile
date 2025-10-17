###############################################################################
# CS/CNS 171 Fall 2015
#
# This is a template Makefile for the assignment. Edit it however you find
# convenient.
###############################################################################
CC = g++
FLAGS = -g -std=c++11 -I../utils/include -I../
UTILS_DIR = ../utils
UTILS_OBJS = $(UTILS_DIR)/utils.o
SOURCES = *.cpp

EXENAME = ppm_test

all: $(UTILS_OBJS) $(SOURCES)
	$(CC) $(FLAGS) -o $(EXENAME) $(SOURCES) $(UTILS_OBJS)

$(UTILS_OBJS):
	$(MAKE) -C $(UTILS_DIR)

clean:
	rm -f *.o $(EXENAME)
	$(MAKE) -C $(UTILS_DIR) clean

.PHONY: all clean


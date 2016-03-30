# The Makefile for the C++ implementation of atm

COMPILER = g++
OBJS = utils.o topic.o document.o corpus.o gibbs.o  author.o
SOURCE = $(OBJS:.o=.cc)

FLAGS = -g -Wall  -I/usr/local/Cellar/gsl/1.16/include -std=c++11

# GSL library
LIBS = -lgsl -lgslcblas -L/usr/local/Cellar/gsl/1.16/lib

default: atm infer

atm: $(OBJS) 
	$(COMPILER) $(FLAGS) $(OBJS) atm_main.cc -o atm  $(LIBS)

infer: $(OBJS) 
	$(COMPILER) $(FLAGS) $(OBJS) infer_main.cc -o infer  $(LIBS)

%.o: %.cc
	$(COMPILER) -c $(FLAGS) -o $@  $< 

.PHONY: clean
clean: 
	rm -f *.o
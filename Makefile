#!/usr/bin/env make
# Makefile compiles MachineCode
# jlettvin 20151005
#_____________________________________
__=echo \
"____________________________________"
OS=$(shell uname)
MODULE=$(shell basename `pwd`)
export

.PHONY: all
.PHONY: clean

all:	$(MODULE)

clean:	Makefile
	@rm -f *.o *.out *.txt

$(MODULE): $(MODULE).cpp Makefile
	@echo "Compiling $(MODULE)"
	@g++ -o $@ $<
	@echo "Run ./$(MODULE)"

#_____________________________________

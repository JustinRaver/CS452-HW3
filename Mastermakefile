prog?=$(basename $(notdir $(PWD)))
objs+=$(addsuffix .o,$(basename $(wildcard *.c *.cc)))

defines+=-D_GNU_SOURCE
ccflags+=-g -Wall -MMD $(defines)
ldflags+=-g

.SUFFIXES:

%.o: %.c  ; gcc -o $@ -c $< $(ccflags)
%.i: %.c  ; gcc -o $@ -E $< $(defines)
%.o: %.cc ; g++ -o $@ -c $< $(ccflags)
%.i: %.cc ; g++ -o $@ -E $< $(defines)

$(prog): $(objs) ; g++ -o $@ $^ $(ldflags)

.PHONY: clean run valgrind

clean: ; rm -f $(prog) *.o *.d *.i

run:      $(prog) ; ./$< $(args)
valgrind: $(prog) ; $@ --leak-check=full --show-leak-kinds=all ./$< $(args)

sinclude *.d

# Tutorial
# http://www.cs.colby.edu/maxwell/courses/tutorials/maketutor/

# Global variables definition.

IDIR=include
CDIR=src
ODIR=obj
CC=mpicc
CFLAGS=-I $(IDIR)

# Source files compilation rules.

_DEPS = input_reader.h pointer_stack.h minor_tools.h
DEPS = $(patsubst %,$(IDIR)/%,$(_DEPS))

_OBJ = input_reader.o pointer_stack.o minor_tools.o
OBJ = $(patsubst %,$(CDIR)/$(ODIR)/%,$(_OBJ))

$(CDIR)/$(ODIR)/%.o: $(CDIR)/%.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

# Main file compilation rule.

$(ODIR)/%.o: %.c
	$(CC) -c -o $@ $< $(CFLAGS)

_MOBJ= main.o
MOBJ = $(patsubst %,$(ODIR)/%,$(_MOBJ))

main: $(OBJ) $(MOBJ)
	$(CC) $^ -o $@ $(CFLAGS)

# Test files compilation rules.

TDIR=test

_TDEPS = test_tools.h pointer_stack_test.h minor_tools_test.h
TDEPS = $(patsubst %,$(TDIR)/$(IDIR)/%,$(_TDEPS))

_TOBJ = test_tools.o pointer_stack_test.o minor_tools_test.o
TOBJ = $(patsubst %,$(TDIR)/$(CDIR)/$(ODIR)/%,$(_TOBJ))

$(TDIR)/$(CDIR)/$(ODIR)/%.o: $(TDIR)/$(CDIR)/%.c $(TDEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

_TMOBJ= test.o
TMOBJ = $(patsubst %,$(TDIR)/$(ODIR)/%,$(_TMOBJ))

$(TDIR)/$(ODIR)/%.o: $(TDIR)/%.c
	$(CC) -c -o $@ $< $(CFLAGS)

checktest: $(OBJ) $(TOBJ) $(TMOBJ)
	$(CC) $^ -o $@ $(CFLAGS)

# Clean definition

.PHONY: clean

clean:
	rm -f $(CDIR)/$(ODIR)/*.o *~ core $(INCDIR)/*~
# Tutorial
# http://www.cs.colby.edu/maxwell/courses/tutorials/maketutor/

# Global variables definition.

IDIR=include
CDIR=src
ODIR=obj
CC=mpicc
CFLAGS=-I $(IDIR)

# Source files compilation rules.

_DEPS = input_reader.h pointer_stack.h
DEPS = $(patsubst %,$(IDIR)/%,$(_DEPS))

_OBJ = input_reader.o pointer_stack.o
OBJ = $(patsubst %,$(CDIR)/$(ODIR)/%,$(_OBJ))

$(ODIR)/%.o: %.c
	$(CC) -c -o $@ $< $(CFLAGS)

$(CDIR)/$(ODIR)/%.o: $(CDIR)/%.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

# Main file compilation rule.

_MOBJ= main.o
MOBJ = $(patsubst %,$(ODIR)/%,$(_MOBJ))

main: $(OBJ) $(MOBJ)
	$(CC) $^ -o $@ $(CFLAGS)

# Test files compilation rules.

TDIR=test

_TDEPS = pointer_stack_test.h
TDEPS = $(patsubst %,$(TDIR)/%,$(_TDEPS))

_TOBJ = test.o pointer_stack_test.o
TOBJ = $(patsubst %,$(TDIR)/$(ODIR)/%,$(_TOBJ))

$(TDIR)/$(ODIR)/%.o: $(TDIR)/%.c $(TDEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

checktest: $(OBJ) $(TOBJ)
	$(CC) $^ -o $@ $(CFLAGS)

# Clean definition

.PHONY: clean

clean:
	rm -f $(CDIR)/$(ODIR)/*.o *~ core $(INCDIR)/*~
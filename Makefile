IDIR =../include
CC=gcc
CFLAGS=-I$(IDIR) -g -Wall

ODIR=obj
LDIR =../lib

LIBS=-mglibc -llinphone -lmediastreamer_base

_DEPS = 
DEPS = $(patsubst %,$(IDIR)/%,$(_DEPS))

_OBJ = main.o doorphone.o
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))

doorphone: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

$(ODIR)/%.o: %.c $(DEPS) $(ODIR)
	$(CC) -c -o $@ $< $(CFLAGS)

$(ODIR):
	mkdir $@

.PHONY: clean

clean:
	rm -rf $(ODIR) *~ core $(INCDIR)/*~ 
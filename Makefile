IDIR =../include
CC=gcc
CFLAGS=-I$(IDIR) -g -Wall -O0

ODIR=obj
LDIR =../lib

LIBS=-mglibc -llinphone -lmediastreamer_base

_DEPS = 
DEPS = $(patsubst %,$(IDIR)/%,$(_DEPS))

_OBJ = main.o linphone_helper.o call_strategy_seq.o
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))


$(ODIR)/%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

doorphone: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

.PHONY: clean

clean:
	rm -f $(ODIR)/*.o *~ core $(INCDIR)/*~ 
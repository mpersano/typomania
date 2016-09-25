CC      = gcc
LD      = gcc
CFLAGS  = -g -Wall -I/usr/include/freetype2

CFILES  = dumpglyphs.c
OBJS    = $(CFILES:.c=.o)
LIBS    = -lfreetype -lpng
LFLAGS  =
BIN     = dumpglyphs

$(BIN): $(OBJS)
	$(LD) $(LFLAGS) $(OBJS) -o $@ $(LIBS)

.c.o:
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f *.o *~ core* *.stackdump $(BIN)

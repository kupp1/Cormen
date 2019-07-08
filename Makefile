P=Cormen
OBJECTS=irc.o pcre_wrpr.o strlib.o
CFLAGS=-g -Iinclude -Wall `pkg-config --cflags libpcre libssl libcrypto` 
LDLIBS=`pkg-config --libs libpcre libssl libcrypto`
CC=cc

$(P): $(OBJECTS)

.PHONY: clean
clean:
	-rm $(OBJECTS) $(P)
P=Cormen
OBJECTS=irc.o pcre_wrpr.o strlib.o
CFLAGS = -g -Iinclude -Wall -o $(P) `pkg-config --cflags libpcre`
LDLIBS= `pkg-config --libs libpcre`
CC=cc

$(P): $(OBJECTS)

clean:
	-rm $(OBJECTS) $(P)
P=Cormen
OBJECTS=irc.o pcre_l.o strlib.o
CFLAGS = -g -Iinclude -Wall -o $(P) `pkg-config --cflags libpcre`
LDLIBS= `pkg-config --libs libpcre`
CC=c99

$(P): $(OBJECTS)

clean:
	-rm $(OBJECTS) $(P)
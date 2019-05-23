P=Cormen
OBJECTS=irc.o regex.o
CFLAGS = -Wall -o $(P) `pkg-config --cflags libpcre`
LDLIBS= `pkg-config --libs libpcre`
CC=c99

$(P): $(OBJECTS)

clean:
	-rm $(OBJECTS) $(P)
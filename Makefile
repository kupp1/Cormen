P=Cormen
OBJECTS=
CFLAGS = -Wall -o $@ `pkg-config --cflags libpcre`
LDLIBS= `pkg-config --libs libpcre`
CC=c99

$(P): $(OBJECTS)
CC = gcc
OBJECTS = bkglight.o
LIBS = -I/usr/local/include/ -L/usr/local/lib -lraspicam -lwiringPi
CFLAGS = -Wall -O2
BINDIR = $(DESTDIR)/usr/bin
NAME = bkglight

bkglight: $(OBJECTS)
        $(CC) -o $(NAME) $(OBJECTS) $(LIBS)

%.o: %.cpp
        $(CC) -c $(CFLAGS) $<

install:
        install --mode=755 $(NAME) $(BINDIR)/

clean:
        rm *.o $(NAME)

uninstall:
        rm $(BINDIR)/$(NAME)

CC = g++
OBJECTS = bkglight.cpp
LIBS = -I/usr/local/include/ -L/usr/local/lib -lraspicam -lwiringPi -lpthread
BINDIR = $(DESTDIR)/usr/bin
NAME = bkglight

bkglight: $(OBJECTS)
	$(CC) -o $(NAME) $(OBJECTS) $(LIBS)

install:
	install --mode=755 $(NAME) $(BINDIR)/

clean:
	rm *.o $(NAME)

uninstall:
	rm $(BINDIR)/$(NAME)

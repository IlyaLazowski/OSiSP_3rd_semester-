CFLAGS = -W -Wall -Wextra -std=c11 -pedantic `pkg-config --cflags gtk+-3.0` -lX11
LDFLAGS = `pkg-config --libs gtk+-3.0` -lX11

.PHONY: clean

all: main

main: LayoutController.c makefile
	gcc $(CFLAGS) LayoutController.c -o main $(LDFLAGS)

clean:
	rm -f main

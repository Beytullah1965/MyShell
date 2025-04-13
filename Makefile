CC = gcc
CFLAGS = -Wall -g `pkg-config --cflags gtk+-3.0`
LDFLAGS = `pkg-config --libs gtk+-3.0`

# Derleme ve bağlantı komutları
all: myshell

myshell: main.o model.o controller.o view.o
	$(CC) -o myshell main.o model.o controller.o view.o $(LDFLAGS)

main.o: main.c
	$(CC) -c main.c $(CFLAGS)

model.o: model.c
	$(CC) -c model.c $(CFLAGS)

controller.o: controller.c
	$(CC) -c controller.c $(CFLAGS)

view.o: view.c
	$(CC) -c view.c $(CFLAGS)

clean:
	rm -f *.o myshell


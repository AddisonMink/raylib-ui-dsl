.PHONY: game

game:
	$(CC) main.c \
	ui.c \
	$(shell pkg-config --libs --cflags raylib) -o ui-test
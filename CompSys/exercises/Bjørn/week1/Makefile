# GNU C compiler
CC=gcc
CFLAGS=-Werror -Wall -Wextra -g
TARGET=$(basename $(FILE))

.PHONY: clean

compile: $(FILE)
	$(CC) $(CFLAGS) -o $(TARGET) $(FILE)


clean: $(FILE)
	rm $(TARGET)

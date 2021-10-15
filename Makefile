CFLAGS = "-g"

test/main: test/main.c get_addr_info.c printable_sock.c
	$(CC) $(CFLAGS) -Wall -o $@ $^

.PHONY: clean
clean:
	rm test/main

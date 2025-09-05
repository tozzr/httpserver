build:
	gcc -std=c89 -pthread -o httpserver main.c

clean:
	rm -f httpserver
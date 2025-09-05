build:
	gcc -std=c89 -pthread -o httpserver main.c src/*.c

clean:
	rm -f httpserver
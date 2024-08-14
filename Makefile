client:
	gcc client.c -o client
server:
	gcc server.c -o server -lpthread
run-server:
	./server 7777 5
clean:
	rm -f client server
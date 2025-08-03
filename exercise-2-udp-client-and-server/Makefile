all:
	gcc -o server server.c
	gcc -o client client.c

test:
	./server > server.log & echo $$! > server.pid && \
	./client ipv4 127.0.0.1 > client.log && \
	./client msg 192.168.0.70 >> client.log && \
	sleep 1 && \
	kill -9 $$(cat server.pid) && \
	rm server.pid

log:
	echo "server log"
	cat server.log
	echo "client log"
	cat client.log

clean:
	rm -f server client server.log client.log server.pid

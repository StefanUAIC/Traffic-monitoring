all:
	g++ server.cpp -lsqlite3 -o server
	g++ client.cpp -o client
clean:
	rm -f client server
	gcc $( pkg-config --cflags gtk4 ) -o client client.cpp $( pkg-config --libs gtk4 ) -lglib-2.0

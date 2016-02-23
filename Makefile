all: app

app: application.o server.o client.o
	g++ -g application.o server.o client.o -o app -lssl -lcrypto

application.o: application.cpp
	g++ -g -c application.cpp

server.o: server.cpp
	g++ -g -c server.cpp 

client.o: client.cpp
	g++ -g -c client.cpp 

clean:
	rm *.o app

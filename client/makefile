CFLAGS:=-c -Wall -Weffc++ -g -std=c++11 -Iinclude
LDFLAGS:=-lboost_system -lpthread
# Updated makefile for StompEMIClient
all: StompEMIClient

StompEMIClient: bin/ConnectionHandler.o bin/StompClient.o bin/StompProtocol.o bin/event.o bin/FileUtils.o
	g++ -o bin/StompEMIClient bin/ConnectionHandler.o bin/StompClient.o bin/StompProtocol.o bin/event.o bin/FileUtils.o $(LDFLAGS)

bin/ConnectionHandler.o: src/ConnectionHandler.cpp
	g++ $(CFLAGS) -o bin/ConnectionHandler.o src/ConnectionHandler.cpp

bin/event.o: src/event.cpp
	g++ $(CFLAGS) -o bin/event.o src/event.cpp

bin/StompClient.o: src/StompClient.cpp
	g++ $(CFLAGS) -o bin/StompClient.o src/StompClient.cpp

bin/StompProtocol.o: src/StompProtocol.cpp
	g++ $(CFLAGS) -o bin/StompProtocol.o src/StompProtocol.cpp

bin/FileUtils.o: src/FileUtils.cpp
	g++ $(CFLAGS) -o bin/FileUtils.o src/FileUtils.cpp

.PHONY: clean

run: 
	bin/StompEMIClient
clean:
	rm -f bin/*

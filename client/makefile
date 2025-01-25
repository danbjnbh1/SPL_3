# Compiler and flags
CFLAGS := -c -Wall -Weffc++ -g -std=c++17 -Iinclude
CXXFLAGS := -std=c++17 -Wall -Wextra -O2
LDFLAGS := -lboost_system -lpthread

# Target executable
TARGET := bin/StompEMIClient

# Object files
OBJECTS := bin/ConnectionHandler.o bin/StompClient.o bin/StompProtocol.o bin/event.o bin/FileUtils.o

# Default target
all: $(TARGET)

# Link the final executable
$(TARGET): $(OBJECTS)
	g++ -o $(TARGET) $(OBJECTS) $(LDFLAGS)

# Compile individual source files
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

# Clean target to remove all generated files
.PHONY: clean
clean:
	rm -f bin/*

# Run the executable
.PHONY: run
run: $(TARGET)
	$(TARGET)

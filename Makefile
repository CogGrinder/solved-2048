EXECUTABLE=2048main

all: build

run: $(EXECUTABLE)
	./$(EXECUTABLE)
$(EXECUTABLE):
	g++ main.cpp -o $(EXECUTABLE)
build:$(EXECUTABLE)
debug:
	g++ main.cpp -D DEBUG

clean:
	rm -f $(EXECUTABLE)
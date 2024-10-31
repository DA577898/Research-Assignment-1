CC = g++

all: compile run

compile:
	$(CC) main.cpp -o main.exe

run:
	./main.exe

clean:
	rm -f *o main.exe
CC = g++

all: compile run clean

compile:
	$(CC) main.cpp -o main.exe

run:
	./main.exe

clean:
	rm -f *o main.exe
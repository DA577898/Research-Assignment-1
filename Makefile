CC = g++

all: compile run

compile:
	$(CC) -std=c++17 .\main.cpp -o .\main.exe

run:
	./main.exe

clean:
	rm -f *o main.exe
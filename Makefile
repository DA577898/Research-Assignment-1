all: main.exe

main.exe: main.cpp
	g++ main.cpp -o main.exe

run: main.exe
	./main.exe

clean:
	rm -f main.exe
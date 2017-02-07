all: pre main

pre:
	rm -f main.out

main:
	g++ -o main.out main.cpp -lglut -lGL -lGLEW -lGLU

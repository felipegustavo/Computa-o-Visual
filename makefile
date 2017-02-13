all: main.out clear

main.out: main.o
	g++ main.o -o main.out -lGLEW -lglfw -lGL -lSOIL -lX11 -lXi -lXrandr -lXxf86vm -lXinerama -lXcursor -lrt -lm -pthread

main.o:
	g++ -std=c++11 -c ../main.cpp
	
clear:
	rm -f main.o
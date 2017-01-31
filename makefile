all: main.o main.out clean

main.out: main.o
	g++ main.o -o main.out -lGLEW -lglfw -lGL -lX11 -lXi -lXrandr -lXxf86vm -lXinerama -lXcursor -lrt -lm -pthread

main.o:
	g++ -std=c++11 -c main.cpp

clean:
	rm -f *.o main.o
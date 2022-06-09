all: player.o
	g++ -std=c++14 -Ofast -Wall -Wextra -o PeLDama player.o main.cpp
	
player.o: player.cpp
	g++ -c -std=c++14 -Ofast -Wall -Wextra -o player.o player.cpp

clean:
	rm *.o *.txt

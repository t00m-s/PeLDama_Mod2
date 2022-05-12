
all: player.o
	g++ -std=c++14 -Wall -Wextra -o PeLDama player.o testMain.cpp

player.o: player.cpp
	g++ -c -std=c++14 -Wall -Wextra -o player.o player.cpp

clean:
	rm player.o *.txt

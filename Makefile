all: player.o
	g++ -std=c++14 -Ofast -Wall -Wextra -o Dama player.o main.cpp
	
player.o: player.cpp
	g++ -c -std=c++14 -Ofast -Wall -Wextra -o player.o player.cpp

playerDebug.o: player.cpp
	g++ -c -std=c++14 -Wall -Wextra -g -o playerDebug.o player.cpp

debug: playerDebug.o
	g++ -std=c++14 -Wall -Wextra -g -o DamaDebug playerDebug.o main.cpp

clean:
	rm *.o *.txt

ex:
	rm Dama && rm DamaDebug

all: player.o
	g++ -std=c++14 -Ofast -Wall -Wextra -o Dama player.o main.cpp

debug: debug.o
	g++ -std=c++14 -Wall -Wextra -o Dama debug.o main.cpp

player.o: player.cpp
	g++ -c -std=c++14 -Ofast -Wall -Wextra -o player.o player.cpp

debug.o: player.cpp
	g++ -c -std=c++14 -g -Wall -Wextra -o debug.o player.cpp

clean:
	rm *.o board_*

cleanExecutable:
	rm Dama

valgrind:
	make debug && valgrind --leak-check=full \
	 --show-leak-kinds=all \
	 --track-origins=yes \
	 --verbose \
	 --log-file=valgrind-out.txt \
	 ./Dama

#include "player.hpp"
#include <string>
using std::string;
int main()
{
    Player p1(1);
    Player p2(2);
    int i = 1;
    p1.init_board("board_1.txt");
    while(i < 100)
    {
        if(p1.wins())
        {
            std::cout << "P1 WIN" << std::endl;
            return 1;
        }   
        
        p1.load_board("board_" + std::to_string(i++) + ".txt");
        p1.move();
        p1.store_board("board_" + std::to_string(i) + ".txt");

        p2.load_board("board_" + std::to_string(i++) + ".txt");
        p2.move();
        p2.store_board("board_" + std::to_string(i) + ".txt");

        if(p2.wins())
        {
            std::cout << "P2 WIN" << std::endl;
            return 2;
        }
    }
    return 0;
}
#include "player.hpp"
#include <string>
using std::string;
int main()
{
    Player p1(1);
    Player p2(2);
    int i = 1;
    p1.init_board("board_1.txt");
    p1.load_board("board_1.txt");
    /*while(i < 100)
    {
        try
        {
            std::cout << "PLAYER 1: " << std::endl;
            std::cout << "CARICATA BOARD: " << i << std::endl;
            p1.load_board("board_" + std::to_string(i++) + ".txt");
            p1.move();
            std::cout << "SALVATA BOARD: " << i << std::endl;
            p1.store_board("board_" + std::to_string(i) + ".txt");
            
            std::cout << "PLAYER 2: " << std::endl;
            std::cout << "CARICATA BOARD: " << i << std::endl;
            p2.load_board("board_" + std::to_string(i++) + ".txt");
            p2.move();
            std::cout << "SALVATA BOARD: " << i << std::endl;
            p2.store_board("board_" + std::to_string(i) + ".txt");
        }
        catch(player_exception e)
        {
            std::cerr << e.msg << std::endl;
        }
    }*/
    p1.printAsInMemory();

    return 0;
}
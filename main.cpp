#include "player.hpp"
#include <string>
using std::string;
int main()
{
    Player p1(1);
    Player p2(2);
    int i = 1;
    p1.init_board("board_1.txt");
    while(i < 300)
    {
        try
        {
            p1.load_board("board_" + std::to_string(i++) + ".txt");
            p1.move();
            p1.store_board("board_" + std::to_string(i) + ".txt");
            
            p2.load_board("board_" + std::to_string(i++) + ".txt");
            p2.move();
            p2.store_board("board_" + std::to_string(i) + ".txt");
        }
        catch(player_exception& err)
        {
            std::cerr << err.msg << std::endl;
            return 0;
        }
    }
    p1.printHistory();
    Player p3(p1);
    Player p4 = p1;
    std::cout << "P3 HISTORY:" << std::endl;
    p3.printHistory();

    std::cout << "P4 HISTORY:" << std::endl;
    p4.printHistory();

    return 0;
}
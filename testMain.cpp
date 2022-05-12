#include "player.hpp"

int main()
{
    Player p1;
    p1.init_board("board1.txt");
    p1.print();
    /*
    try
    {
        p1(4, 2 ,0);
    }
    catch(player_exception err)
    {
        std::cout << err.msg << std::endl;
    }
    */
    return 0;
}
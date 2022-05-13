#include "player.hpp"

int main()
{
    Player p1;
    //p1.init_board("board1.txt");
    p1.print();
    try
    {
        p1.load_board("board1.txt");
    }
    catch(player_exception err)
    {
        std::cout << err.msg << std::endl;
    }

    try
    {
        std::cout << p1(0, 1, 0) << std::endl; 
    }
    catch(player_exception err)
    {
        std::cout << err.msg << std::endl;
    }
    
    return 0;
}
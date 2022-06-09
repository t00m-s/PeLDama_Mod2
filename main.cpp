#include "player.hpp"

int main()
{
    Player p1;
    p1.init_board("xd.txt");
    try
    {
        p1.load_board("xd.txt");
        p1.printHistory();
    }
    catch(player_exception err)
    {
        std::cout << err.msg << std::endl;
    }

    return 0;
}
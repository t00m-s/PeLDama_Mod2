#include "player.hpp"
#include <string>
using std::string;
int main()
{
    Player p1(1);
    p1.init_board("boardTemp.txt");
    p1.load_board("boardTemp.txt");
    return 0;
}

#include "player.hpp"
#include <iostream>
#include <string>
/*
  TODO:
  Controllare valid_move(), copyConstructor e copyAssign
*/
int main()
{
    Player p1(1);
    Player p2(2);
    int i = 1;
    p1.init_board("board_1.txt");
    
    while(true)
    {
        try
        {
            
            p1.load_board("board_" + std::to_string(i++) + ".txt");
            p1.move();
            if(p1.valid_move())
                std::cout << "Player1 Mossa valida." << std::endl;
            else
                std::cout << "Player1 Mossa non valida. " << std::endl;
            p1.store_board("board_" + std::to_string(i) + ".txt");

            p2.load_board("board_" + std::to_string(i++) + ".txt");
            p2.move();
            if(p2.valid_move())
                std::cout << "Player2 Mossa valida." << std::endl;
            else
                std::cout << "Player2 Mossa non valida. " << std::endl;
            p2.store_board("board_" + std::to_string(i) + ".txt");

            if(p1.wins())
            {    std::cout << "1 HA VINTO.";
                break;
            }
            if(p2.wins())
            {
                std::cout << "2 HA VINTO.";
                break;
            }
        }
        catch(player_exception e)
        {
            std::cerr << e.msg << std::endl;
        }
    }

    Player p3(p1);
    p3 = p2;
    Player p4(1);
    Player p5(2);
    p5 = p4;
    return 0;
}

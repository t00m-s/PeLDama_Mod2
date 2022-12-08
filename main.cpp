#include "player.hpp"
#include <iostream>
#include <string>
/*
  TODO:
  Controllare copyConstructor e copyAssign
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
            {    std::cout << "1 HA VINTO." << std::endl;
                break;
            }
            if(p2.wins())
            {
                std::cout << "2 HA VINTO." << std::endl;
                break;
            }

        }
        catch(player_exception e)
        {
            std::cerr << e.msg << std::endl;
        }
    }


    //Wrong parameter on player
    try
    {
        std::cout << "1) ";
        Player ppppp(4);
        std::cout << "Player value 4 caricato" << std::endl;
    }
    catch(player_exception e)
    {
        std::cerr << e.msg << std::endl;
    }
    //Loading wrong boards
    try
    {
        std::cout << "2) ";
        p1.load_board("goodBoard.txt");
        std::cout << "Board goodBoard caricata. " << std::endl;
    }
    catch(player_exception e)
    {
        std::cerr << e.msg << std::endl;
    }

    try //Da errore ma non quello che volevo, devo farla meglio
    {
        std::cout << "3) ";
        p1.load_board("pieceOnWhite.txt");
        std::cout << "Board pieceOnWhite caricata. " << std::endl;
    }
    catch(player_exception e)
    {
        std::cerr << e.msg << std::endl;
    }

    try //Uguale punto 3
    {
        std::cout <<"4) ";
        p1.load_board("oPieceOnWhiteCell.txt");
        std::cout << "Board oPieceOnWhiteCell caricata. " << std::endl;
    }
    catch(player_exception e)
    {
        std::cerr << e.msg << std::endl;
    }

    try
    {
        std::cout << "5) ";
        p1.load_board("wrongCharacter.txt");
        std::cout << "Board wrongCharacter caricata. " << std::endl;
    }
    catch(player_exception e)
    {
        std::cerr << e.msg << std::endl;
    }
    Player p3(p1);
    p3 = p2;
    p3 = p1;
    try
    {
        p3(3, 5, 2);
        std::cout << "1" << "\n";
        p3(3, 5 , 22);
        std::cout << "2" << "\n";
        p3(3, 5 , 222);
        std::cout << "3" << "\n";
    }
    catch(player_exception e)
        {
            std::cerr << e.msg << std::endl;
        }

    Player p4(1);
    Player p5(2);
    p5 = p4;
    return 0;
}

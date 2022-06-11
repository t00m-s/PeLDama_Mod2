#include "player.hpp"
#include <iostream>
#include <fstream>
#include <string>
#include <thread>
#include <time.h>
using std::cout;
using std::cin;
using std::endl;
using std::ifstream;
using std::string;

void game(Player a, Player b, string board_name) 
{
    int i = 0;
    while (i < 300) // 100 sono le mosse massime
    {
        std::cout << "ha mosso Player_1 (x): " << std::endl;
        a.load_board(board_name);
        if (i > 0) 
        { //controllo cio che ha fatto b
            if (b.valid_move()) 
            {
                if (b.wins()) 
                {
                    cout << "Game over. Player " << 2 << " wins." << endl;
                    exit(0);
                }
            } 
            else 
            {
                std::cout << " Player 2 ha fatto una mossa che lo ha portato alla sconfitta" << std::endl;
                exit(0);
            }
        }
        a.move();
        std::cout << "numero di ricorrenze di questo campo: " << a.recurrence() << std::endl;
        a.store_board(board_name);
        std::cout << "ha mosso Player_ 2 (o)" << std::endl;
        b.load_board(board_name);
        if (i > 0) 
        { //controllo cio che ha fatto a
            if (a.valid_move()) 
            {
                if (a.wins()) 
                {
                    cout << "Game over. Player " << 1 << " wins." << endl;
                    exit(0);
                }
            } 
            else 
            {
                std::cout << " Player 1 ha fatto una mossa che lo ha portato alla sconfitta" << std::endl;
                exit(0);
            }
        }
        b.move();        
        std::cout << "numero di ricorrenze di questo campo: " << b.recurrence() << std::endl;
        b.store_board(board_name);
        ++i;
    }
}
int main() {

    /* PARTITA CPU VS CPU */
    try 
    {
        Player a(1);
        Player b(2);
        string board_name = "board_1.txt";
        a.init_board(board_name);
        game(a, b, board_name);
    } 
    catch (player_exception & t) 
    {
        cout << t.msg << endl;
    }

    /*TEST FUNZIONI */

    // POP: V
    // STORE: ? eccezione con throw non printa l'msg
    // RECURRENCE: V
    // VALID MOVE: V, funziona anche se provo a mangiarmi le mie stesse pedine X non contolla se muovo le pedine dell'avversario
    // WINS: X2  V V
    // LOSES : X2 V V
}
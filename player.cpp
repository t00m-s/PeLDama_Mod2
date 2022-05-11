#include "player.hpp"

typedef struct History
{
    Player::piece board[8][8];
    History* prev;
} History_t ;

struct Player::Impl
{
    Player::piece lastBoard[8][8]; //Board di gioco
    History_t* boardOffset;
};

Player::Player(int player_nr)
{
    if(!(player_nr == 1 || player_nr == 2))
        std::cout << "mammt";

    this->pimpl = new Impl;

}
Player::~Player()
{
    delete pimpl;
}

Player& Player::operator=(const Player &rhs)
{
    if(this != &rhs)
    {}
    return *this;
}

Player::piece Player::operator()(int r, int c, int history_offset) const
{
    //operator() per fare board[r][c] -> board(r, c);
    //history_offset di quanto devo spostare il pointer della history
    if(r < 0 || r > 7)
        throw player_exception{player_exception::index_out_of_bounds, "Row not valid."};
    if(c < 0 || c > 7)
        throw player_exception{player_exception::index_out_of_bounds, "Column not valid."};
    if(history_offset < 0)
        throw player_exception{player_exception::index_out_of_bounds, "Negative offsets are not valid."};

    History* temp = this->pimpl->boardOffset;
    while(history_offset > 0 && temp)
    {
        --history_offset;
        temp = temp->prev;
    }

    if(history_offset)
        throw player_exception{player_exception::index_out_of_bounds, "Board with the given offset does not exist."};

    return temp->board[r][c];
}

void Player::load_board(const std::string &filename)
{
    // std::getline(stream, doveSalvare)
}

void Player::init_board(const std::string &filename) const
{
    std::ofstream writeBoard;
    writeBoard.open(filename);   
    // 1 3 5 7
    // 0 2 4 6
    // 1 3 5 7

    //Player::piece::x
    for(size_t i = 0; i < 3; ++i)
    {
        if(i % 2)
            for(size_t j = 0; j < 8; ++j)
                this->pimpl->lastBoard[i][j] = j % 2 ? Player::piece::x : Player::piece::e;
        else
            for(size_t j = 0; j < 8; ++j)
                this->pimpl->lastBoard[i][j] = j % 2 ? Player::piece::e : Player::piece::x;
    }

    for(size_t i = 3; i < 5; ++i)
        for(size_t j = 0; j < 8; ++j)
            this->pimpl->lastBoard[i][j] = Player::piece::e;

    //Player::piece::o
    for(size_t i = 5; i < 8; ++i)
    {
        if(i % 2)
            for(size_t j = 0; j < 8; ++j)
                this->pimpl->lastBoard[i][j] = j % 2 ? Player::piece::o : Player::piece::e;
        else
            for(size_t j = 0; j < 8; ++j)
                this->pimpl->lastBoard[i][j] = j % 2 ? Player::piece::e : Player::piece::o;
    }

    for(size_t i = 0; i < 8; ++i)
    {
        std::string row = "";
        for(size_t j = 0; j < 8; ++j)
        {
            switch(this->pimpl->lastBoard[i][j])
            {
                case Player::piece::x:
                    row.append("x");
                    break;
                case Player::piece::X:
                    row.append("X");
                    break;
                case Player::piece::o:
                    row.append("o");
                    break;
                case Player::piece::O:
                    row.append("O");
                    break;
                default:
                    row.append(" ");
                    break;
            }
        }
        if(i != 7)
            row.append("\n");
        writeBoard << row;
    }
    writeBoard.close(); // Il distruttore si arrangia da solo teoricamente

}

void Player::move()
{

}

bool Player::valid_move() const
{
    return false;
}

void Player::pop()
{

}

bool Player::wins(int player_nr) const
{
    return false;
}

bool Player::wins() const
{
    return false;
}

bool Player::loses(int player_nr) const
{
    return false;
}

bool Player::loses() const
{
    return false;
}

int Player::recurrence() const
{
    return 0;
}

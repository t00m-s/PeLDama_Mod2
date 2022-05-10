#include "player.hpp"

struct Player::Impl
{
    Player::piece board[8][8];
};

Player::Player(int player_nr)
{
    if(!(player_nr == 1 || player_nr == 2))
        throw player_exception{player_exception::index_out_of_bounds, "Player value not valid."};

    this->pimpl = new Impl;
}
Player::~Player()
{
    delete pimpl;
}

Player &Player::operator=(const Player &rhs)
{
    if(this != &rhs)
    {}
    return *this;
}

Player::piece Player::operator()(int r, int c, int history_offset) const
{
    //operator() per fare board[r][c] -> board(r, c);
    return Player::piece::O;
}

void Player::load_board(const std::string &filename)
{

}

void Player::init_board(const std::string &filename) const
{
    std::ofstream board;
    board.open(filename);
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

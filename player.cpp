#include "player.hpp"

struct History
{
    Player::piece board[8][8];
    History* prev;
};

struct Player::Impl
{
    Player::piece lastBoard[8][8]; //Board di gioco
    History* boardOffset;
    int player_nr;
};

Player::Player(int player_nr)
{
    if(!(player_nr == 1 || player_nr == 2))
        throw player_exception{player_exception::index_out_of_bounds, "Player value not valid."};
        
    this->pimpl = new Impl;
    this->pimpl->player_nr = player_nr;

}

void deleteHistory(History* hist)
{
    if(hist->prev)
        deleteHistory(hist->prev);
    
    delete hist;
}

Player::~Player()
{
    deleteHistory(this->pimpl->boardOffset);
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

    if(history_offset || temp == nullptr)
        throw player_exception{player_exception::index_out_of_bounds, "Board with the given offset does not exist."};

    return temp->board[r][c];
}

void Player::load_board(const std::string &filename)
{
    std::fstream loader;
    loader.open(filename);
    if(!loader)
        throw player_exception{player_exception::missing_file, "Wrong file format."};

    size_t oPieces = 0;
    size_t xPieces = 0;
    History* newHead = new History;
    newHead->prev = this->pimpl->boardOffset; //Attacco in testa
    this->pimpl->boardOffset = newHead;
    for(size_t i = 0; i < 8; ++i)
    {
        std::string temp;
        std::getline(loader, temp);
        for(size_t j = 0; j < 8; ++j)
        {   
            if(i % 2 == 0 && j % 2 == 0 && temp[j] != ' ')
                throw player_exception{player_exception::invalid_board, "Pieces on while cells are not allowed."};

            if(i % 2 != 0 && j % 2 != 0 && temp[j] != ' ')
                throw player_exception{player_exception::invalid_board, "Pieces on while cells are not allowed."};

            switch(temp[j])
            {
                case 'x':
                    this->pimpl->boardOffset->board[i][j] = Player::piece::x;
                    ++xPieces;
                    break;
                case 'X':
                    this->pimpl->boardOffset->board[i][j] = Player::piece::X;
                    ++xPieces;
                    break;
                case 'o':
                    this->pimpl->boardOffset->board[i][j] = Player::piece::o;
                    ++oPieces;
                    break;
                case 'O':
                    this->pimpl->boardOffset->board[i][j] = Player::piece::O;
                    ++oPieces;
                    break;
                default:
                    this->pimpl->boardOffset->board[i][j] = Player::piece::e;
                    break;
            }
        }
    }
    loader.close();
    if(oPieces > 12 || xPieces > 12)
        throw player_exception{player_exception::invalid_board, "Too many pieces."};
    
}

void Player::init_board(const std::string &filename) const //done
{
    std::ofstream writeBoard;
    writeBoard.open(filename);   
    
    //Player::piece::x
    for(size_t i = 0; i < 3; ++i)
    {
        if(i % 2)
            for(size_t j = 0; j < 8; ++j)
                this->pimpl->lastBoard[i][j] = j % 2 ? Player::piece::e : Player::piece::x;
        else
            for(size_t j = 0; j < 8; ++j)
                this->pimpl->lastBoard[i][j] = j % 2 ? Player::piece::x : Player::piece::e;
    }

    for(size_t i = 3; i < 5; ++i)
        for(size_t j = 0; j < 8; ++j)
            this->pimpl->lastBoard[i][j] = Player::piece::e;

    //Player::piece::o
    for(size_t i = 5; i < 8; ++i)
    {
        if(i % 2)
            for(size_t j = 0; j < 8; ++j)
                this->pimpl->lastBoard[i][j] = j % 2 ? Player::piece::e : Player::piece::o;
        else
            for(size_t j = 0; j < 8; ++j)
                this->pimpl->lastBoard[i][j] = j % 2 ? Player::piece::o : Player::piece::e;
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
        if(i < 7)
            row.append("\n");
        writeBoard << row;
    }
    writeBoard.close(); // Il distruttore si arrangia da solo teoricamente

}

void Player::store_board(const std::string& filename, int history_offset = 0) const
{
    int cnt = 0;
    std::ofstream writer(filename, std::ios::trunc);
    if(writer)
    {
        History* temp = this->pimpl->boardOffset;
        while(temp && cnt != history_offset)
        {
            ++cnt;
            temp = temp->prev;
        }
        
        if(!temp || cnt != history_offset)
            throw player_exception{player_exception::index_out_of_bounds, "Board with chosen offset does not exist."};

        for(size_t i = 0; i < 8; ++i)
        {
            for(size_t j = 0; j < 8; ++j)
            {
                switch(temp->board[i][j])
                {
                    case Player::piece::x:
                        break;
                    default:
                        break;
                }
            }
        }
        
    }
    writer.close();
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

void Player::print() const
{
    for(int i = 7; i >= 0; --i)
    {
        for(size_t j = 0; j < 8; ++j)
        {
            switch(this->pimpl->lastBoard[i][j])
            {
                case Player::piece::o:
                    std::cout << "o";
                    break;
                case Player::piece::O:
                    std::cout << "O";
                    break;
                case Player::piece::x:
                    std::cout << "x";
                    break;
                case Player::piece::X:
                    std::cout << "X";
                    break;
                default:
                    std::cout << " ";
                    break;
            }
        }
        std::cout << std::endl;
    }
}

void Player::printHistory() const
{

}

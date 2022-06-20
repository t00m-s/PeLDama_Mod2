#include "player.hpp"
#define POS_INF  ((unsigned) ~0)
//TODO:
//Create functions to reuse code(eg. move legality)
struct History
{
    Player::piece board[8][8];
    History* prev;
};

struct Player::Impl
{
    History* boardOffset;
    int player_nr;
};

Player::Player(int player_nr)
{
    if(!(player_nr == 1 || player_nr == 2))
        throw player_exception{player_exception::index_out_of_bounds, "Player value not valid."};
        
    this->pimpl = new Impl;
    this->pimpl->player_nr = player_nr;
    this->pimpl->boardOffset = nullptr;
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

Player::Player(const Player& rhs)
{
    this->pimpl = new Impl;
    this->pimpl->player_nr = rhs.pimpl->player_nr;
    
    if(rhs.pimpl->boardOffset != nullptr)
    {
        this->pimpl->boardOffset = new History;
    
        History* temp = rhs.pimpl->boardOffset;
        History* t  = this->pimpl->boardOffset;
        while(temp)
        {
            for(int i = 0; i < 8; ++i)
                for(int j = 0; j < 8; ++j)
                    t->board[i][j] = temp->board[i][j];

            if(temp->prev)
            {
                t->prev = new History;
                t = t ->prev;
            }
            temp = temp->prev;
        }
    }
    else
        this->pimpl->boardOffset = nullptr;
}

Player& Player::operator=(const Player &rhs)
{
    if(this != &rhs)
    {
        deleteHistory(this->pimpl->boardOffset);
        this->pimpl->player_nr = rhs.pimpl->player_nr;
        if(rhs.pimpl->boardOffset != nullptr)
        {
            this->pimpl->boardOffset = new History;
            History* temp = rhs.pimpl->boardOffset;
            History* t  = this->pimpl->boardOffset;
            while(temp)
            {
                for(int i = 0; i < 8; ++i)
                    for(int j = 0; j < 8; ++j)
                        t->board[i][j] = temp->board[i][j];

                if(temp->prev)
                {
                    t->prev = new History;
                    t = t ->prev;
                }
                temp = temp->prev;
            }
        }
        else
            this->pimpl->boardOffset = nullptr;
    }
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

    if(history_offset || !temp)
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
                throw player_exception{player_exception::invalid_board, "Pieces on white cells are not allowed."};

            if(i % 2 != 0 && j % 2 != 0 && temp[j] != ' ')
                throw player_exception{player_exception::invalid_board, "Pieces on white cells are not allowed."};

            if(oPieces > 12 || xPieces > 12)
                throw player_exception{player_exception::invalid_board, "Too many pieces."};

            switch(temp[j])
            {
                case 'x':
                    newHead->board[i][j] = Player::piece::x;
                    ++xPieces;
                    break;
                case 'X':
                    newHead->board[i][j] = Player::piece::X;
                    ++xPieces;
                    break;
                case 'o':
                    newHead->board[i][j] = Player::piece::o;
                    ++oPieces;
                    break;
                case 'O':
                    newHead->board[i][j] = Player::piece::O;
                    ++oPieces;
                    break;
                default:
                    newHead->board[i][j] = Player::piece::e;
                    break;
            }
        }
    }
    loader.close();
}

void Player::init_board(const std::string &filename) const
{
    std::ofstream writeBoard;
    writeBoard.open(filename, std::ios::trunc);
    Player::piece board[8][8];
    //x
    for(size_t i = 0; i < 3; ++i)
    {
        if(i % 2)
            for(size_t j = 0; j < 8; ++j)
                board[i][j] = j % 2 ? Player::piece::e : Player::piece::x;
        else
            for(size_t j = 0; j < 8; ++j)
                board[i][j] = j % 2 ? Player::piece::x : Player::piece::e;
    }

    //empty
    for(size_t i = 3; i < 5; ++i)
        for(size_t j = 0; j < 8; ++j)
            board[i][j] = Player::piece::e;

    //o
    for(size_t i = 5; i < 8; ++i)
    {
        if(i % 2)
            for(size_t j = 0; j < 8; ++j)
                board[i][j] = j % 2 ? Player::piece::e : Player::piece::o;
        else
            for(size_t j = 0; j < 8; ++j)
                board[i][j] = j % 2 ? Player::piece::o : Player::piece::e;
    }

    //Salva su file
    for(size_t i = 0; i < 8; ++i)
    {
        std::string row;
        for(size_t j = 0; j < 8; ++j)
        {
            switch(board[i][j])
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

void Player::store_board(const std::string& filename, int history_offset) const
{
    int cnt = 0;
    std::ofstream writer(filename, std::ios::trunc);
    if(writer)
    {
        History* temp = this->pimpl->boardOffset;
        while(temp && cnt < history_offset)
        {
            ++cnt;
            temp = temp->prev;
        }
        
        if(!temp || cnt != history_offset)
            throw player_exception{player_exception::index_out_of_bounds, "Board with chosen offset does not exist."};

        for(size_t i = 0; i < 8; ++i)
        {
            std::string row = "";
            for(size_t j = 0; j < 8; ++j)
            {
                switch(temp->board[i][j])
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
            
            writer << row;
        }   
    }
    writer.close();
}

double evaluateBoard(Player::piece board[8][8])
{
    /*
        *Pezzi normali: 1
        *Dame: 10
    */
    double eval = 0;
    for(size_t i = 0; i < 8; ++i)
        for(size_t j = 0; j < 8; ++j)
            switch(board[i][j])
            {
                case Player::piece::x:
                    ++eval;
                    break;
                case Player::piece::X:
                    eval += 10;
                    break;
                case Player::piece::o:
                    --eval;
                    break;
                case Player::piece::O:
                    eval -= 10;
                    break;
                default:
                    break;

            }
    
    return eval;
}

bool noMoves(Player::piece board[8][8], int player_nr)
{
    bool moves = false;
    if(player_nr == 1)
    {
        int i = 0, j = 0;
        while(!moves && i < 8)
        {
            while(!moves && j < 8)
            {
                if(board[i][j] == Player::piece::x)
                {
                    if(i + 1 < 8 && j - 1 >= 0 && board[i + 1][j - 1] == Player::piece::e) //casella vuota SX
                        moves = true;

                    if(!moves && i + 1 < 8 && j + 1 < 8 && board[i + 1][j + 1] == Player::piece::e) //casella vuota DX
                        moves = true;

                    //Pezzo nemico alto sx e posso mangiarlo
                    if(!moves && i + 1 < 8 && j - 1 >= 0 && board[i + 1][j - 1] == Player::piece::o)
                        if(i + 2 < 8 && j - 2 >= 0 && board[i + 2][j - 2] == Player::piece::e)
                            moves = true;

                    //Pezzo nemico alto dx e posso mangiarlo
                    if(!moves && i + 1 < 8 && j + 1 < 8 && board[i + 1][j + 1] == Player::piece::o)
                        if(i + 2 < 8 && j + 2 < 8 && board[i + 2][j + 2] == Player::piece::e)
                            moves = true;
                }
                else if(board[i][j] == Player::piece::X)
                {
                    //Casella vuota alto SX
                    if(i + 1 < 8 && j - 1 >= 0 && board[i + 1][j - 1] == Player::piece::e)
                        moves = true;
                    //Casella vuota alto DX
                    if(!moves && i + 1 < 8 && j + 1 < 8 && board[i + 1][j + 1] == Player::piece::e)
                        moves = true;

                    //Casella vuota basso SX
                    if(!moves && i - 1 >= 0 && j - 1 >= 0 && board[i - 1][j - 1] == Player::piece::e)
                        moves = true;

                    //Casella vuota basso DX
                    if(!moves && i - 1 >= 0 && j + 1 < 8 && board[i - 1][j + 1] == Player::piece::e)
                        moves = true;

                    //Pezzo nemico alto sx e posso mangiarlo
                    if(!moves && i + 1 < 8 && j - 1 >= 0 && (board[i + 1][j - 1] == Player::piece::o
                                                         || board[i + 1][j - 1] == Player::piece::O))
                        if(i + 2 < 8 && j - 2 >= 0 && board[i + 2][j - 2] == Player::piece::e)
                            moves = true;

                    //Pezzo nemico alto dx e posso mangiarlo
                    if(!moves && i + 1 < 8 && j + 1 < 8 && (board[i + 1][j + 1] == Player::piece::o
                                                        || board[i + 1][j + 1] == Player::piece::O))
                        if(i + 2 < 8 && j + 2 < 8 && board[i + 2][j + 2] == Player::piece::e)
                            moves = true;

                    //Pezzo nemico basso sx e posso mangiarlo
                    if(!moves && i - 1 >= 0 && j - 1 >= 0 && (board[i - 1][j - 1] == Player::piece::o
                                                          || board[i - 1][j - 1] == Player::piece::O))
                        if(i - 2 >= 0 && j - 2 >= 0 && board[i - 2][j - 2] == Player::piece::e)
                            moves = true;

                    //Pezzo nemico basso dx e posso mangiarlo
                    if(!moves && i - 1 >= 0 && j + 1 < 8 && (board[i - 1][j + 1] == Player::piece::o
                                                         || board[i - 1][j + 1] == Player::piece::O))
                        if(i - 2 >= 0 && j + 2 < 8 && board[i - 2][j + 2] == Player::piece::e)
                            moves = true;
                }
                ++j;
            }
            j = 0;
            ++i;
        }
    }

    if(player_nr == 2)
    {
        int i = 0, j = 0;
        while(!moves && i < 8)
        {
            while(!moves && j < 8)
            {
                if(board[i][j] == Player::piece::o)
                {
                    //Casella vuota basso SX
                    if(i - 1 >= 0 && j - 1 >= 0 && board[i - 1][j - 1] == Player::piece::e)
                        moves = true;

                    //Casella vuota basso DX
                    if(!moves && i - 1 >= 0 && j + 1 < 8 && board[i - 1][j + 1] == Player::piece::e)
                        moves = true;

                    //Pezzo nemico sx e posso mangiarlo
                    if(!moves && i - 1 >= 0 && j - 1 >= 0 && board[i - 1][j - 1] == Player::piece::x)
                        if(i - 2 >= 0 && j - 2 >= 0 && board[i - 2][j - 2] == Player::piece::e)
                            moves = true;

                    //Pezzo nemico dx e posso mangiarlo
                    if(!moves && i - 1 >= 0 && j + 1 < 8 && board[i - 1][j + 1] == Player::piece::x)
                        if(i - 2 >= 0 && j + 2 < 8 && board[i - 2][j + 2] == Player::piece::e)
                            moves = true;
                }
                else if(board[i][j] == Player::piece::O)
                {
                    //Casella vuota alto SX
                    if(i + 1 < 8 && j - 1 >= 0 && board[i + 1][j - 1] == Player::piece::e)
                        moves = true;

                    //Casella vuota alto DX
                    if(!moves && i + 1 < 8 && j + 1 < 8 && board[i + 1][j + 1] == Player::piece::e)
                        moves = true;

                    //Pezzo nemico alto SX e posso mangiarlo
                    if(!moves && i + 1 < 8 && j - 1 >= 0 && (board[i + 1][j - 1] == Player::piece::x
                                                          || board[i + 1][j - 1] == Player::piece::X))
                        if(i + 2 < 8 && j - 2 >= 0 && board[i + 2][j - 2] == Player::piece::e)
                            moves = true;

                    //Pezzo nemico alto DX e posso mangiarlo
                    if(!moves && i + 1 < 8 && j + 1 < 8 && (board[i + 1][j + 1] == Player::piece::x
                                                         || board[i + 1][j + 1] == Player::piece::X))
                        if(i + 2 < 8 && j + 2 < 8 && board[i + 2][j + 2] == Player::piece::e)
                            moves = true;

                    //Pezzo nemico basso SX e posso mangiarlo
                    if(!moves && i - 1 >= 0 && j - 1 >= 0 && (board[i - 1][j - 1] == Player::piece::x
                                                           || board[i - 1][j - 1] == Player::piece::X))
                        if(i - 2 >= 0 && j - 2 >= 0 && board[i - 2][j - 2] == Player::piece::e)
                            moves = true;

                    //Pezzo nemico basso dx e posso mangiarlo
                    if(!moves && i - 1 >= 0 && j + 1 < 8 && (board[i - 1][j + 1] == Player::piece::x
                                                          || board[i - 1][j - 1] == Player::piece::X))
                    if(i - 2 >= 0 && j + 2 < 8 && board[i - 2][j + 2] == Player::piece::e)
                        moves = true;
                }
                ++j;
            }
            j = 0;
            ++i;
        }
    }
    return !moves;
}

// + -
bool move_topLeft(Player::piece board[8][8], int player_nr, int row, int col)
{
    bool res = false;
    Player::piece original = board[row][col];
    if(player_nr == 1)
    {
        if(row + 1 < 8 && col - 1 >= 0)
        {
            if(board[row + 1][col - 1] == Player::piece::e) //TopLeft vuoto
            {
                board[row][col] = Player::piece::e;
                board[row + 1][col - 1] = row + 1 == 7 ? Player::piece::X : original;
                res = true;
            }
            else if(board[row + 1][col - 1] == Player::piece::o) //TopLeft pedina nemica
            {
                if (row + 2 < 8 && col - 2 >= 0
                    && board[row + 2][col - 2] == Player::piece::e)
                {
                    board[row][col] = Player::piece::e;
                    board[row + 1][col - 1] = Player::piece::e;
                    board[row + 2][col - 2] = row + 2 == 7 ? Player::piece::X : original;
                    res = true;
                }
            }
            else if(board[row + 1][col - 1] == Player::piece::O)
            {

                if (row + 2 < 8 && col - 2 >= 0
                    && board[row + 2][col - 2] == Player::piece::e
                    && original == Player::piece::X)
                {
                    board[row][col] = Player::piece::e;
                    board[row + 1][col - 1] = Player::piece::e;
                    board[row + 2][col - 2] = original;
                    res = true;
                }
            }
        }
    }
    else
    {
        //Player2 può muoversi solo se il pezzo è una dama
        if(original == Player::piece::O)
        {
            if(board[row + 1][col - 1] == Player::piece::e)
            {
                board[row][col] = Player::piece::e;
                board[row + 1][col - 1] = original;
                res = true;
            }
            else if(board[row + 1][col - 1] == Player::piece::x
                    || board[row + 1][col - 1] == Player::piece::X)
            {
                board[row][col] = Player::piece::e;
                board[row + 1][col - 1] = Player::piece::e;
                board[row + 2][col - 2] = original;
                res = true;
            }
        }
    }

    return res;
}

// + +
bool move_topRight(Player::piece board[8][8], int player_nr, int row, int col)
{
    bool res = false;
    Player::piece original = board[row][col];
    if(player_nr == 1)
    {
        if(row + 1 < 8 && col + 1 < 8)
        {
            if(board[row + 1][col + 1] == Player::piece::e) //TopRight vuoto
            {
                board[row][col] = Player::piece::e;
                board[row + 1][col + 1] = row + 1 == 7 ? Player::piece::X : original; //Se il pezzo è 'X' è come avere due branch identici
                res = true;
            }
            else if(board[row + 1][col + 1] == Player::piece::o) //TopRight pedina nemica
            {
                if (row + 2 < 8 && col + 2 < 8
                    && board[row + 2][col + 2] == Player::piece::e)
                {
                    board[row][col] = Player::piece::e;
                    board[row + 1][col + 1] = Player::piece::e;
                    board[row + 2][col + 2] = row + 2 == 7 ? Player::piece::X : original;
                    res = true;
                }
            }
            else if(board[row + 1][col + 1] == Player::piece::O)
            {

                if (row + 2 < 8 && col + 2 < 8
                    && board[row + 2][col + 2] == Player::piece::e
                    && original == Player::piece::X)
                {
                    board[row][col] = Player::piece::e;
                    board[row + 1][col + 1] = Player::piece::e;
                    board[row + 2][col + 2] = original;
                    res = true;
                }
            }
        }
    }
    else
    {
        //Player2 può muoversi solo se il pezzo è una dama
        if(original == Player::piece::O)
        {
            if(board[row + 1][col + 1] == Player::piece::e)
            {
                board[row][col] = Player::piece::e;
                board[row + 1][col + 1] = original;
                res = true;
            }
            else if(board[row + 1][col + 1] == Player::piece::x
                || board[row + 1][col + 1] == Player::piece::X)
            {
                if(row + 2 < 8 && col + 2 < 8
                    && board[row + 2][col + 2] == Player::piece::e)
                {
                    board[row][col] = Player::piece::e;
                    board[row + 1][col + 1] = Player::piece::e;
                    board[row + 2][col + 2] = original;
                    res = true;
                }
            }
        }
    }

    return res;
}

// - -
bool move_downLeft(Player::piece board[8][8], int player_nr, int row, int col)
{
    bool res = false;
    Player::piece original = board[row][col];
    if(player_nr == 1)
    {
        //Solo dama
        if(original == Player::piece::X)
        {
            if(row - 1 >= 0 && col - 1 >= 0)
            {
                if(board[row - 1][col - 1] == Player::piece::e)
                {
                    board[row][col] = Player::piece::e;
                    board[row - 1][col - 1] = original;
                    res = true;
                }
                else if(board[row - 1][col - 1] == Player::piece::o
                        || board[row - 1][col - 1] == Player::piece::O)
                {
                    if (row - 2 >= 0 && col - 2 >= 0
                        && board[row - 2][col - 2] == Player::piece::e)
                    {
                        board[row][col] = Player::piece::e;
                        board[row - 1][col - 1] = Player::piece::e;
                        board[row - 2][col - 2] = original;
                        res = true;
                    }
                }
            }
        }
    }
    else
    {
        if(row - 1 >= 0 && col - 1 >= 0)
        {
            if(board[row - 1][col - 1] == Player::piece::e)
            {
                board[row][col] = Player::piece::e;
                board[row - 1][col - 1] = row - 1 == 0 ? Player::piece::O : original;
                res = true;
            }
            else if(board[row - 1][col - 1] == Player::piece::x)
            {
                if(row - 2 >= 0 && col - 2 >= 0
                    && board[row  - 2][col - 2] == Player::piece::e)
                {
                    board[row][col] = Player::piece::e;
                    board[row - 1][col - 1] = Player::piece::e;
                    board[row - 2][col - 2] = row - 2 == 0 ? Player::piece::O : original;
                    res = true;
                }
            }
            else if(board[row - 1][col - 1] == Player::piece::X)
            {
                if(original == Player::piece::O)
                {
                    if(row - 2 >= 0 && col - 2 >= 0
                        && board[row - 2][col - 2] == Player::piece::e)
                    {
                        board[row][col] = Player::piece::e;
                        board[row - 1][col - 1] = Player::piece::e;
                        board[row - 2][col - 2] = original;
                        res = true;
                    }
                }
            }

        }
    }

    return res;
}


// - +
bool move_downRight(Player::piece board[8][8], int player_nr, int row, int col)
{
    bool res = false;
    Player::piece original = board[row][col];
    if(player_nr == 1)
    {
        //Solo dama
        if(original == Player::piece::X)
        {
            if(row - 1 >= 0 && col + 1 < 8)
            {
                if(board[row - 1][col + 1] == Player::piece::e)
                {
                    board[row][col] = Player::piece::e;
                    board[row - 1][col + 1] = original;
                    res = true;
                }
                else if(board[row - 1][col + 1] == Player::piece::o
                    || board[row - 1][col + 1] == Player::piece::O)
                {
                    if (row - 2 >= 0 && col + 2 < 8
                        && board[row - 2][col + 2] == Player::piece::e)
                    {
                        board[row][col] = Player::piece::e;
                        board[row - 1][col + 1] = Player::piece::e;
                        board[row - 2][col + 2] = original;
                        res = true;
                    }
                }
            }
        }
    }
    else
    {
        if(row - 1 >= 0 && col + 1 < 8)
        {
            if(board[row - 1][col + 1] == Player::piece::e)
            {
                board[row][col] = Player::piece::e;
                board[row - 1][col + 1] = row - 1 == 0 ? Player::piece::O : original;
                res = true;
            }
            else if(board[row - 1][col + 1] == Player::piece::x)
            {
                if(row - 2 >= 0 && col + 2 < 8
                    && board[row  - 2][col + 2] == Player::piece::e)
                {
                    board[row][col] = Player::piece::e;
                    board[row - 1][col + 1] = Player::piece::e;
                    board[row - 2][col + 2] = row - 2 == 0 ? Player::piece::O : original;
                    res = true;
                }
            }
            else if(board[row - 1][col + 1] == Player::piece::X)
            {
                if(original == Player::piece::O)
                {
                    if(row - 2 >= 0 && col + 2 < 8
                        && board[row - 2][col + 2] == Player::piece::e)
                    {
                        board[row][col] = Player::piece::e;
                        board[row - 1][col + 1] = Player::piece::e;
                        board[row - 2][col + 2] = original;
                        res = true;
                    }
                }
            }

        }
    }

    return res;
}

double minimax(Player::piece board[8][8], int depth, int player_nr, double alpha, double beta) 
{
    if(noMoves(board, player_nr))  //Settare un valore per indicare perdita per il giocatore
        return player_nr == 1 ? -400000 : POS_INF;

    if(depth == 0)
        return evaluateBoard(board);
    
    Player::piece tempBoard[8][8];
    for(int i = 0; i < 8; ++i)
        for(int j = 0; j < 8; ++j)
            tempBoard[i][j] = board[i][j];

    if(player_nr == 1)
    {
        double maxResult = -400000;
        for(int i = 0; i < 8; ++i)
        {
            for(int j = 0; j < 8; ++j)
            {
                if(board[i][j] == Player::piece::x)
                {
                    if(move_topLeft(tempBoard, 1, i, j))
                    {
                        double eval = minimax(tempBoard, depth - 1, 2, alpha, beta);
                        maxResult = eval > maxResult ? eval : maxResult;
                        alpha = eval > alpha ? eval : alpha;
                        //Undo move_topLeft
                        for(int i = 0; i < 8; ++i)
                            for(int j = 0; j < 8; ++j)
                                tempBoard[i][j] = board[i][j];

                        if(alpha >= beta)
                            return alpha;
                    }
                    if(move_topRight(tempBoard, 1, i, j))
                    {
                        double eval = minimax(tempBoard, depth - 1, 2, alpha, beta);
                        maxResult = eval > maxResult ? eval : maxResult;
                        alpha = eval > alpha ? eval : alpha;

                        for(int i = 0; i < 8; ++i)
                            for(int j = 0; j < 8; ++j)
                                tempBoard[i][j] = board[i][j];

                        if(alpha >= beta)
                            return alpha;
                    }
                }
                else if(board[i][j] == Player::piece::X)
                {
                    if(move_topLeft(tempBoard, 1, i, j))
                    {
                        double eval = minimax(tempBoard, depth - 1, 2, alpha, beta);
                        maxResult = eval > maxResult ? eval : maxResult;
                        alpha = eval > alpha ? eval : alpha;
                        //Undo move_topLeft
                        for(int i = 0; i < 8; ++i)
                            for(int j = 0; j < 8; ++j)
                                tempBoard[i][j] = board[i][j];

                        if(alpha >= beta)
                            return alpha;
                    }

                    if(move_topRight(tempBoard, 1, i, j))
                    {
                        double eval = minimax(tempBoard, depth - 1, 2, alpha, beta);
                        maxResult = eval > maxResult ? eval : maxResult;
                        alpha = eval > alpha ? eval : alpha;

                        for(int i = 0; i < 8; ++i)
                            for(int j = 0; j < 8; ++j)
                                tempBoard[i][j] = board[i][j];

                        if(alpha >= beta)
                            return alpha;
                    }

                    if(move_downLeft(tempBoard, 1, i, j))
                    {
                        double eval = minimax(tempBoard, depth - 1, 2, alpha, beta);
                        maxResult = eval > maxResult ? eval : maxResult;
                        alpha = eval > alpha ? eval : alpha;
                        //Undo move_downLeft
                        for(int i = 0; i < 8; ++i)
                            for(int j = 0; j < 8; ++j)
                                tempBoard[i][j] = board[i][j];

                        if(alpha >= beta)
                            return alpha;
                    }

                    if(move_downRight(tempBoard, 1, i, j))
                    {
                        double eval = minimax(tempBoard, depth - 1, 2, alpha, beta);
                        maxResult = eval > maxResult ? eval : maxResult;
                        alpha = eval > alpha ? eval : alpha;
                        //Undo move_downRight
                        for(int i = 0; i < 8; ++i)
                            for(int j = 0; j < 8; ++j)
                                tempBoard[i][j] = board[i][j];

                        if(alpha >= beta)
                            return alpha;
                    }
                }
            }
        }
        return maxResult;
    }
    else
    {
        double minResult = POS_INF;
        for(int i = 7; i >= 0; --i)
        {
            for(int j = 0; j < 8; ++j)
            {
                if(board[i][j] == Player::piece::o)
                {
                    if(move_downLeft(tempBoard, 2, i, j))
                    {
                        double eval = minimax(board, depth - 1, 1, alpha, beta);
                        minResult = eval < minResult ? eval : minResult;
                        beta = eval < beta ? eval : beta;

                        //Undo move_downLeft
                        for(int i = 0; i < 8; ++i)
                            for(int j = 0; j < 8; ++j)
                                tempBoard[i][j] = board[i][j];
                        
                        if(alpha >= beta)
                            return beta;
                    }

                    if(move_downRight(tempBoard, 2, i, j))
                    {
                        double eval = minimax(board, depth - 1, 1, alpha, beta);
                        minResult = eval < minResult ? eval : minResult;
                        beta = eval < beta ? eval : beta;

                        //Undo move_downRight
                        for(int i = 0; i < 8; ++i)
                            for(int j = 0; j < 8; ++j)
                                tempBoard[i][j] = board[i][j];
                        
                        if(alpha >= beta)
                            return beta;
                    }
                }
                else if(board[i][j] == Player::piece::O)
                {
                    if(move_downLeft(tempBoard, 2, i, j))
                    {
                        double eval = minimax(board, depth - 1, 1, alpha, beta);
                        minResult = eval < minResult ? eval : minResult;
                        beta = eval < beta ? eval : beta;

                        //Undo move_downLeft
                        for(int i = 0; i < 8; ++i)
                            for(int j = 0; j < 8; ++j)
                                tempBoard[i][j] = board[i][j];
                        
                        if(alpha >= beta)
                            return beta;
                    }

                    if(move_downRight(tempBoard, 2, i, j))
                    {
                        double eval = minimax(board, depth - 1, 1, alpha, beta);
                        minResult = eval < minResult ? eval : minResult;
                        beta = eval < beta ? eval : beta;

                        //Undo move_downRight
                        for(int i = 0; i < 8; ++i)
                            for(int j = 0; j < 8; ++j)
                                tempBoard[i][j] = board[i][j];
                        
                        if(alpha >= beta)
                            return beta;
                    }

                    if(move_topLeft(tempBoard, 2, i, j))
                    {
                        double eval = minimax(board, depth - 1, 1, alpha, beta);
                        minResult = eval < minResult ? eval : minResult;
                        beta = eval < beta ? eval : beta;

                        //Undo move_downLeft
                        for(int i = 0; i < 8; ++i)
                            for(int j = 0; j < 8; ++j)
                                tempBoard[i][j] = board[i][j];
                        
                        if(alpha >= beta)
                            return beta;
                    }

                    if(move_topRight(tempBoard, 2, i, j))
                    {
                        double eval = minimax(board, depth - 1, 1, alpha, beta);
                        minResult = eval < minResult ? eval : minResult;
                        beta = eval < beta ? eval : beta;

                        //Undo move_downRight
                        for(int i = 0; i < 8; ++i)
                            for(int j = 0; j < 8; ++j)
                                tempBoard[i][j] = board[i][j];
                        
                        if(alpha >= beta)
                            return beta;
                    }
                }
            }
        }
        return minResult;
    }

    return 0; //Suppress Warning
}

void Player::move()
{
    //https://www.youtube.com/watch?v=l-hh51ncgDI
    //Spiegazione minimax
    int p2Depth = 5;
    int p1Depth = 5;

    if(!this->pimpl->boardOffset)
        throw player_exception{player_exception::invalid_board, "Board history does not exist."};

    Player::piece temporaryBoard[8][8];
    for(int i = 0; i < 8; ++i)
        for(int j = 0; j < 8; ++j)
            temporaryBoard[i][j] = this->pimpl->boardOffset->board[i][j];
    
    if(noMoves(temporaryBoard, this->pimpl->player_nr)) //Salva una board uguale
    {
        History* t = new History;
        for(int i = 0; i < 8; ++i)
            for(int j = 0; j < 8; ++j)
                t->board[i][j] = temporaryBoard[i][j];

        t->prev = this->pimpl->boardOffset;
        this->pimpl->boardOffset = t;
        return;
    }

    //Parte mossa

    //Salva nella history
    History* t = new History;
    t->prev = this->pimpl->boardOffset;
    this->pimpl->boardOffset = t;
    for(int i = 0; i < 8; ++i)
        for(int j = 0; j < 8; ++j)
            t->board[i][j] = temporaryBoard[i][j];
    
}

bool Player::valid_move() const
{
    bool flag = true;
    //Controllo se la board è uguale a quella precedente   
    if(!this->pimpl->boardOffset || !this->pimpl->boardOffset->prev)
        throw player_exception{player_exception::index_out_of_bounds, "Less than two boards in history."};
    
    for(int i = 0; i < 8; ++i)
        for(int j = 0; j < 8; ++j)
            flag = flag && 
                (this->pimpl->boardOffset->board[i][j] == this->pimpl->boardOffset->prev->board[i][j]);
    
    if(flag)
        return false;

    //Guardo eventuali celle bianche con pezzi
    for(int i = 0; i < 8; ++i)
        for(int j = 0; j < 8; ++j)
        {
            if(i % 2 == 0 && j % 2 && this->pimpl->boardOffset->board[i][j] != Player::piece::e)
                flag = false;
            if(i % 2 && j % 2 == 0 && this->pimpl->boardOffset->board[i][j] != Player::piece::e)
                flag = false;
        }
    
    //Pezzi non promossi
    for(int j = 0; j < 8; ++j)
        if(this->pimpl->boardOffset->board[0][j] == Player::piece::o 
            || this->pimpl->boardOffset->board[7][j] == Player::piece::x)
                flag = false;
    
    return flag;
}

void Player::pop()
{
    if(!this->pimpl->boardOffset)
        throw player_exception{player_exception::index_out_of_bounds, "History does not exist."};
    
    History* temp = this->pimpl->boardOffset;
    this->pimpl->boardOffset = this->pimpl->boardOffset->prev;
    delete temp;
}

bool Player::wins(int player_nr) const
{
    if(!(player_nr == 1 || player_nr == 2))
        throw player_exception{player_exception::index_out_of_bounds, "Player number not valid."};

    bool win = false;

    if(player_nr == 1) //'x'
    {
        size_t otherPieces = 0;
        
        for(size_t i = 0; i < 8; ++i)
            for(size_t j = 0; j < 8; ++j)
                switch(this->pimpl->boardOffset->board[i][j])
                {
                    case Player::piece::o:
                        ++otherPieces;
                        break;
                    case Player::piece::O:
                        ++otherPieces;
                        break;
                    default:
                        break;
                }

        if(otherPieces == 0)
            win = true;
    }
    else    // 'o'
    {
        size_t otherPieces = 0;
        
        for(size_t i = 0; i < 8; ++i)
            for(size_t j = 0; j < 8; ++j)
                switch(this->pimpl->boardOffset->board[i][j])
                {
                    case Player::piece::x:
                        ++otherPieces;
                        break;
                    case Player::piece::X:
                        ++otherPieces;
                        break;
                    default:
                        break;
                }

        if(otherPieces == 0)
            win = true;
    }

    return win;
}

bool Player::wins() const
{
    return wins(this->pimpl->player_nr);
}

bool Player::loses(int player_nr) const
{
    if(!(player_nr == 1 || player_nr == 2))
        throw player_exception{player_exception::index_out_of_bounds, "Player number not valid."};
    
    bool lost = false;
    if(player_nr == 1) //'x'
    {
        size_t myPieces = 0;
        
        for(size_t i = 0; i < 8; ++i)
            for(size_t j = 0; j < 8; ++j)
                switch(this->pimpl->boardOffset->board[i][j])
                {
                    case Player::piece::x:
                        ++myPieces;
                        break;
                    case Player::piece::X:
                        ++myPieces;
                        break;
                    default:
                        break;
                }

        if(!myPieces)
            lost = true;
    }

    if(player_nr == 2) // 'o'
    {
        size_t myPieces = 0;
        
        for(size_t i = 0; i < 8; ++i)
            for(size_t j = 0; j < 8; ++j)
                switch(this->pimpl->boardOffset->board[i][j])
                {
                    case Player::piece::o:
                        ++myPieces;
                        break;
                    case Player::piece::O:
                        ++myPieces;
                        break;
                    default:
                        break;
                }

        if(!myPieces)
            lost = true;
    }

    return lost;
}

bool Player::loses() const
{
    return loses(this->pimpl->player_nr);
}

int Player::recurrence() const
{
    if(!this->pimpl->boardOffset)
        throw player_exception{player_exception::index_out_of_bounds, "History does not exist."};
    int times = 1;

    History* temp = this->pimpl->boardOffset->prev;
    if(!temp)
        return times;
    
    while(temp)
    {
        bool equal = true;
        for(size_t i = 0; i < 8; ++i)
            for(size_t j = 0; j < 8; ++j)
                equal = equal && (this->pimpl->boardOffset->board[i][j] == temp->board[i][j]);

        if(equal)
            ++times;
        
        temp = temp->prev;
    }
    return times;
}

void Player::print() const
{
    for(int i = 7; i >= 0; --i)
    {
        for(size_t j = 0; j < 8; ++j)
        {
            switch(this->pimpl->boardOffset->board[i][j])
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
    History* temp = this->pimpl->boardOffset;
    while(temp) //Apparently esiste una seconda board quando creo che è piena di x wtf
    {
        for(int i = 7; i >= 0; --i)
        {
            for(size_t j = 0; j < 8; ++j)
            {
                switch(temp->board[i][j])
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
        temp = temp->prev;
    }
}

#include "player.hpp"
#define POS_INF  ((unsigned) ~0)

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
    if(hist)
    {
        deleteHistory(hist->prev);
        delete hist;
    }
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
    //operator() per fare board[r][c] -> player(r, c, offset);
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
    auto* newHead = new History;
    newHead->prev = this->pimpl->boardOffset; //Attacco in testa
    this->pimpl->boardOffset = newHead;
    for(int i = 7; i >= 0; --i)
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

    //Salva su file
    for(int i = 7; i >= 0; --i)
    {
        std::string row;
        for(size_t j = 0; j < 8; ++j)
        {
            Player::piece actual;

            if(i >= 0 && i < 3)
            {
                if(i % 2)
                    actual = j % 2 ? Player::piece::e : Player::piece::x;
                else
                    actual = j % 2 ? Player::piece::x : Player::piece::e;
            }
            else if(i >= 3 && i < 5)
                actual = Player::piece::e;
            else
            {
                if(i % 2)
                    actual = j % 2 ? Player::piece::e : Player::piece::o;
                else
                    actual = j % 2 ? Player::piece::o : Player::piece::e;
            }

            switch(actual)
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
        if(i != 0)
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

        for(int i = 7; i >= 0; --i)
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
            if(i != 0)
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

// + -
bool move_downLeft(Player::piece board[8][8], int player_nr, int row, int col)
{
    bool done = false;
    if(row + 1 < 8 && col - 1 >= 0)
    {
        if(player_nr == 1 && (board[row][col] == Player::piece::x || board[row][col] == Player::piece::X))
        {
            if(board[row + 1][col - 1] == Player::piece::e)
            {
                Player::piece original = board[row][col];
                board[row][col] = Player::piece::e;
                board[row + 1][col - 1] = row + 1 == 7 ? Player::piece::X : original;
                done = true;
            }
            else if(board[row + 1][col - 1] == Player::piece::o)
            {
                if(row + 2 < 8 && col - 2 >= 0 && board[row + 2][col - 2] == Player::piece::e)
                {
                    Player::piece original = board[row][col];
                    board[row][col] = Player::piece::e;
                    board[row + 1][col - 1] = Player::piece::e;
                    board[row + 2][col - 2] = row + 2 == 7 ? Player::piece::X : original;
                    done = true;
                }
            }
            else if(board[row + 1][col - 1] == Player::piece::O && board[row + 2][col - 2] == Player::piece::e)
            {
                if(row + 2 < 8 && col - 2 >= 0 && board[row][col] == Player::piece::X)
                {
                    board[row][col] = Player::piece::e;
                    board[row + 1][col - 1] = Player::piece::e;
                    board[row + 2][col - 2] = Player::piece::X;
                    done = true;
                }
            }
        }
        else if(player_nr == 2)
        {
            if(board[row][col] == Player::piece::O)
            {
                if(board[row + 1][col - 1] == Player::piece::e)
                {
                    board[row][col] = Player::piece::e;
                    board[row + 1][col - 1] = Player::piece::O;
                    done = true;
                }
                else if(board[row + 1][col - 1] == Player::piece::x
                         || board[row + 1][col - 1] == Player::piece::X)
                {
                    if(row + 2 < 8 && col - 2 >= 0 && board[row + 2][col - 2] == Player::piece::e)
                    {
                        board[row][col] = Player::piece::e;
                        board[row + 1][col - 1] = Player::piece::e;
                        board[row + 2][col - 2] = Player::piece::O;
                        done = true;
                    }
                }
            }
        }
    }
    return done;
}

// + +
bool move_downRight(Player::piece board[8][8], int player_nr, int row, int col)
{
    bool done = false;
    if(row + 1 < 8 && col + 1 < 8)
    {
        if(player_nr == 1 && (board[row][col] == Player::piece::x || board[row][col] == Player::piece::X))
        {
            if(board[row + 1][col + 1] == Player::piece::e)
            {
                Player::piece original = board[row][col];
                board[row][col] = Player::piece::e;
                board[row + 1][col + 1] = row + 1 == 7 ? Player::piece::X : original;
                done = true;
            }
            else if(board[row + 1][col + 1] == Player::piece::o)
            {
                if(row + 2 < 8 && col - 2 >= 0 && board[row + 2][col + 2] == Player::piece::e)
                {
                    Player::piece original = board[row][col];
                    board[row][col] = Player::piece::e;
                    board[row + 1][col + 1] = Player::piece::e;
                    board[row + 2][col + 2] = row + 2 == 7 ? Player::piece::X : original;
                    done = true;
                }
            }
            else if(board[row + 1][col + 1] == Player::piece::O)
            {
                if(row + 2 < 8 && col + 2 < 8 && board[row][col] == Player::piece::X && board[row + 2][col + 2] == Player::piece::e)
                {
                    board[row][col] = Player::piece::e;
                    board[row + 1][col + 1] = Player::piece::e;
                    board[row + 2][col + 2] = Player::piece::X;
                    done = true;
                }
            }
        }
        else if(player_nr == 2)
        {
            if(board[row][col] == Player::piece::O)
            {
                if(board[row + 1][col + 1] == Player::piece::e)
                {
                    board[row][col] = Player::piece::e;
                    board[row + 1][col + 1] = Player::piece::O;
                    done = true;
                }
                else if(board[row + 1][col + 1] == Player::piece::x
                         || board[row + 1][col + 1] == Player::piece::X)
                {
                    if(row + 2 < 8 && col + 2 < 8 && board[row + 2][col + 2] == Player::piece::e)
                    {
                        board[row][col] = Player::piece::e;
                        board[row + 1][col + 1] = Player::piece::e;
                        board[row + 2][col + 2] = Player::piece::O;
                        done = true;
                    }
                }
            }
        }
    }
    return done;
}

// - -
bool move_topLeft(Player::piece board[8][8], int player_nr, int row, int col)
{
    bool done = false;
    if(row - 1 >= 0 && col - 1 >= 0)
    {
        if(player_nr == 2 && (board[row][col] == Player::piece::o || board[row][col] == Player::piece::O))
        {
            if(board[row - 1][col - 1] == Player::piece::e)
            {
                Player::piece original = board[row][col];
                board[row][col] = Player::piece::e;
                board[row - 1][col - 1] = row - 1 == 0 ? Player::piece::O : original;
                done = true;
            }
            else if(board[row - 1][col - 1] == Player::piece::x)
            {
                if(row - 2 >= 0 && col - 2 >= 0 && board[row - 2][col - 2] == Player::piece::e)
                {
                    Player::piece original = board[row][col];
                    board[row][col] = Player::piece::e;
                    board[row - 1][col - 1] = Player::piece::e;
                    board[row - 2][col - 2] = row - 2 == 0 ? Player::piece::O : original;
                    done = true;
                }
            }
            else if(board[row + 1][col - 1] == Player::piece::X)
            {
                if(row - 2 >= 0 && col - 2 >= 0 && board[row][col] == Player::piece::O && board[row - 2][col - 2] == Player::piece::e)
                {
                    board[row][col] = Player::piece::e;
                    board[row - 1][col - 1] = Player::piece::e;
                    board[row - 2][col - 2] = Player::piece::X;
                    done = true;
                }
            }
        }
        else if(player_nr == 1)
        {
            if(board[row][col] == Player::piece::X)
            {
                if(board[row - 1][col - 1] == Player::piece::e)
                {
                    board[row][col] = Player::piece::e;
                    board[row - 1][col - 1] = Player::piece::X;
                    done = true;
                }
                else if(board[row - 1][col - 1] == Player::piece::o
                         || board[row - 1][col - 1] == Player::piece::O)
                {
                    if(row - 2 >= 0 && col - 2 >= 0 && board[row - 2][col - 2] == Player::piece::e)
                    {
                        board[row][col] = Player::piece::e;
                        board[row - 1][col - 1] = Player::piece::e;
                        board[row - 2][col - 2] = Player::piece::X;
                        done = true;
                    }
                }
            }
        }
    }
    return done;
}

// - +
bool move_topRight(Player::piece board[8][8], int player_nr, int row, int col)
{
    bool done = false;
    if(row - 1 >= 0 && col + 1 < 8)
    {
        if(player_nr == 2 && (board[row][col] == Player::piece::o || board[row][col] == Player::piece::O))
        {
            if(board[row - 1][col + 1] == Player::piece::e)
            {
                Player::piece original = board[row][col];
                board[row][col] = Player::piece::e;
                board[row - 1][col + 1] = row - 1 == 0 ? Player::piece::O : original;
                done = true;
            }
            else if(board[row - 1][col + 1] == Player::piece::x)
            {
                if(row - 2 >= 0 && col + 2 < 8 && board[row - 2][col + 2] == Player::piece::e)
                {
                    Player::piece original = board[row][col];
                    board[row][col] = Player::piece::e;
                    board[row - 1][col + 1] = Player::piece::e;
                    board[row - 2][col + 2] = row - 2 == 0 ? Player::piece::O : original;
                    done = true;
                }
            }
            else if(board[row + 1][col - 1] == Player::piece::X)
            {
                if(row - 2 >= 0 && col + 2 < 8 && board[row][col] == Player::piece::O && board[row - 2][col + 2] == Player::piece::e)
                {
                    board[row][col] = Player::piece::e;
                    board[row - 1][col + 1] = Player::piece::e;
                    board[row - 2][col + 2] = Player::piece::X;
                    done = true;
                }
            }
        }
        else if(player_nr == 1) //Might be bug fixed with this line ... YUP
        {
            if(board[row][col] == Player::piece::X)
            {
                if(board[row - 1][col + 1] == Player::piece::e)
                {
                    board[row][col] = Player::piece::e;
                    board[row - 1][col + 1] = Player::piece::X;
                    done = true;
                }
                else if(board[row - 1][col + 1] == Player::piece::o
                         || board[row - 1][col + 1] == Player::piece::O)
                {
                    if(row - 2 >= 0 && col + 2 < 8 && board[row - 2][col + 2] == Player::piece::e)
                    {
                        board[row][col] = Player::piece::e;
                        board[row - 1][col + 1] = Player::piece::e;
                        board[row - 2][col + 2] = Player::piece::X;
                        done = true;
                    }
                }
            }
        }
    }
    return done;
}

bool noMoves(Player::piece board[8][8], int player_nr)
{
    bool moves = false;
    int i = 0, j = 0;
    while(!moves && i < 8)
    {
        while(!moves && j < 8)
        {
            if(move_topLeft(board, player_nr, i, j))
                moves = true;
            else if(move_topRight(board, player_nr, i, j))
                moves = true;
            else if(move_downLeft(board, player_nr, i, j))
                moves = true;
            else if(move_downRight(board, player_nr, i, j))
                moves = true;

            ++j;
        }
        j = 0;
        ++i;
    }
    return !moves;
}

//PLAYER 1 X
//PLAYER 2 O
void Player::move()
{
    if(!this->pimpl->boardOffset)
        throw player_exception{player_exception::invalid_board, "Board history does not exist."};

    Player::piece temporaryBoard[8][8];
    for(int i = 0; i < 8; ++i)
        for(int j = 0; j < 8; ++j)
            temporaryBoard[i][j] = this->pimpl->boardOffset->board[i][j];

    /*
    if(noMoves(otherTemp, this->pimpl->player_nr)) //Appende in testa una board uguale
    {
        History* t = new History;
        for(int i = 0; i < 8; ++i)
            for(int j = 0; j < 8; ++j)
                t->board[i][j] = temporaryBoard[i][j];

        t->prev = this->pimpl->boardOffset;
        this->pimpl->boardOffset = t;
        return;
    }
*/
    //Prova le 4 direzioni per ogni pezzo
    //Simil minimax ma senza depth
    double bestEval; //Bug was there, bestEval non veniva sovrascitto
    if(this->pimpl->player_nr == 1)
        bestEval = -400000;
    else
        bestEval = POS_INF;
    std::pair<int, int> coords;
    char direction = ' ';
    for(int row = 0; row < 8; ++row)
    {
        for(int col = 0; col < 8; ++col)
        {
            if(move_topLeft(temporaryBoard, this->pimpl->player_nr, row, col))
            {
                double currentEval = evaluateBoard(temporaryBoard);
                if(this->pimpl->player_nr == 1)
                {
                    if(currentEval > bestEval)
                    {
                        bestEval = currentEval;
                        coords.first = row;
                        coords.second = col;
                        direction = 'Q';
                    }
                }
                else
                {
                    if(currentEval < bestEval)
                    {
                        bestEval = currentEval;
                        coords.first = row;
                        coords.second = col;
                        direction = 'Q';
                    }
                }
                for(int i = 0; i < 8; ++i)
                    for(int j = 0; j < 8; ++j)
                        temporaryBoard[i][j] = this->pimpl->boardOffset->board[i][j];
            }

            if(move_topRight(temporaryBoard, this->pimpl->player_nr, row, col))
            {
                double currentEval = evaluateBoard(temporaryBoard);
                if(this->pimpl->player_nr == 1)
                {
                    if(currentEval > bestEval)
                    {
                        bestEval = currentEval;
                        coords.first = row;
                        coords.second = col;
                        direction = 'E';
                    }
                }
                else
                {
                    if(currentEval < bestEval)
                    {
                        bestEval = currentEval;
                        coords.first = row;
                        coords.second = col;
                        direction = 'E';
                    }
                }
                for(int i = 0; i < 8; ++i)
                    for(int j = 0; j < 8; ++j)
                        temporaryBoard[i][j] = this->pimpl->boardOffset->board[i][j];
            }

            if(move_downLeft(temporaryBoard, this->pimpl->player_nr, row, col))
            {
                double currentEval = evaluateBoard(temporaryBoard);
                if(this->pimpl->player_nr == 1)
                {
                    if(currentEval > bestEval)
                    {
                        bestEval = currentEval;
                        coords.first = row;
                        coords.second = col;
                        direction = 'A';
                    }
                }
                else
                {
                    if(currentEval < bestEval)
                    {
                        bestEval = currentEval;
                        coords.first = row;
                        coords.second = col;
                        direction = 'A';
                    }
                }
                for(int i = 0; i < 8; ++i)
                    for(int j = 0; j < 8; ++j)
                        temporaryBoard[i][j] = this->pimpl->boardOffset->board[i][j];
            }

            if(move_downRight(temporaryBoard, this->pimpl->player_nr, row, col))
            {
                double currentEval = evaluateBoard(temporaryBoard);
                if(this->pimpl->player_nr == 1)
                {
                    if(currentEval > bestEval)
                    {
                        bestEval = currentEval;
                        coords.first = row;
                        coords.second = col;
                        direction = 'D';
                    }
                }
                else
                {
                    if(currentEval < bestEval)
                    {
                        bestEval = currentEval;
                        coords.first = row;
                        coords.second = col;
                        direction = 'D';
                    }
                }

                for(int i = 0; i < 8; ++i)
                    for(int j = 0; j < 8; ++j)
                        temporaryBoard[i][j] = this->pimpl->boardOffset->board[i][j];
            }
        }
    }

    switch (direction)
    {
        case 'Q':
            move_topLeft(temporaryBoard, this->pimpl->player_nr, coords.first, coords.second);
            break;
        case 'E':
            move_topRight(temporaryBoard, this->pimpl->player_nr, coords.first, coords.second);
            break;
        case 'A':
            move_downLeft(temporaryBoard, this->pimpl->player_nr, coords.first, coords.second);
            break;
        case 'D':
            move_downRight(temporaryBoard, this->pimpl->player_nr, coords.first, coords.second);
            break;
        default:
            break;
    }
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

    if(!this->pimpl->boardOffset || !this->pimpl->boardOffset->prev)
        throw player_exception{player_exception::index_out_of_bounds, "Less than two boards in history."};

    //Controllo se la board è uguale a quella precedente
    for(int i = 0; i < 8; ++i)
        for(int j = 0; j < 8; ++j)
            flag = flag &&
                (this->pimpl->boardOffset->board[i][j] == this->pimpl->boardOffset->prev->board[i][j]);

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

    std::pair<int, int> changes[3];
    int aux = 0;
    for(int i = 0; i < 8; ++i)
    {
        for(int j = 0; j < 8; ++j)
            if(this->pimpl->boardOffset->board[i][j] != this->pimpl->boardOffset->prev->board[i][j])
            {
                if(aux < 3)
                {
                    changes[aux].first = i;
                    changes[aux++].second = j;
                }
            }
    }

    if(aux > 3)
        flag = false;

    //Al massimo ho tre possibilità x 4 direzioni (12 possibilità totali)
    //Controllo se una mossa in una delle 4 direzioni (nelle coordinate cambiate) mi crea la board uguale
    bool foundMove = false;
    Player::piece tempBoard[8][8];
    for(int i = 0; i < 8; ++i)
        for(int j = 0; j < 8; ++j)
            tempBoard[i][j] = this->pimpl->boardOffset->prev->board[i][j];

    int i = 0;
    while(i < 3 && !foundMove)
    {
        if(move_topLeft(tempBoard, this->pimpl->player_nr, changes[i].first, changes[i].second))
        {
            bool eq = true;
            for(int r = 0; r < 8; ++r)
                for(int c = 0; c < 8; ++c)
                    eq = eq &&
                        (this->pimpl->boardOffset->board[r][c] == tempBoard[r][c]);

            if(eq)
                foundMove = true;

            //Resetto la tempBoard allo stato precedente per sicurezza
            for(int r = 0; r < 8; ++r)
                for(int c = 0; c < 8; ++c)
                    tempBoard[r][c] = this->pimpl->boardOffset->prev->board[r][c]
        }

        if(!foundMove && move_topRight(tempBoard, this->pimpl->player_nr, changes[i].first, changes[i].second))
        {
            bool eq = true;
            for(int r = 0; r < 8; ++r)
                for(int c = 0; c < 8; ++c)
                    eq = eq &&
                        (this->pimpl->boardOffset->board[r][c] == tempBoard[r][c]);

            if(eq)
                foundMove = true;
            
            //Resetto la tempBoard allo stato precedente per sicurezza
            for(int r = 0; r < 8; ++r)
                for(int c = 0; c < 8; ++c)
                    tempBoard[r][c] = this->pimpl->boardOffset->prev->board[r][c];
        }

        if(!foundMove && move_downLeft(tempBoard, this->pimpl->player_nr, changes[i].first, changes[i].second))
        {
            bool eq = true;
            for(int r = 0; r < 8; ++r)
                for(int c = 0; c < 8; ++c)
                    eq = eq &&
                        (this->pimpl->boardOffset->board[r][c] == tempBoard[r][c]);

            if(eq)
                foundMove = true;

            for(int r = 0; r < 8; ++r)
                for(int c = 0; c < 8; ++c)
                    tempBoard[r][c] = this->pimpl->boardOffset->prev->board[r][c];           
        }
        if(!foundMove && move_downRight(tempBoard, this->pimpl->player_nr, changes[i].first, changes[i].second))
        {
            bool eq = true;
            for(int r = 0; r < 8; ++r)
                for(int c = 0; c < 8; ++c)
                    eq = eq &&
                        (this->pimpl->boardOffset->board[r][c] == tempBoard[r][c]);

            if(eq)
                foundMove = true;
            
            for(int r = 0; r < 8; ++r)
                for(int c = 0; c < 8; ++c)
                    tempBoard[r][c] = this->pimpl->boardOffset->prev->board[r][c];           
        }
        ++i;
    }

    return foundMove;
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
    if(!this->pimpl->boardOffset)
        throw player_exception{player_exception::invalid_board, "History does not exist."};
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
    while(temp)
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

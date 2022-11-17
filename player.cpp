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

//Definizione funzioni ausiliarie
double evaluateBoard(Player::piece board[8][8]);
bool move_downLeft(Player::piece board[8][8], int player_nr, int row, int col);
bool move_downRight(Player::piece board[8][8], int player_nr, int row, int col);
bool move_topLeft(Player::piece board[8][8], int player_nr, int row, int col);
bool move_topRight(Player::piece board[8][8], int player_nr, int row, int col);
bool noMoves(Player::piece board[8][8], int player_nr);

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
        //hist = nullptr;
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
    this->pimpl->boardOffset = nullptr;
    
    if(rhs.pimpl && rhs.pimpl->boardOffset)
        this->pimpl->boardOffset = new History;
    
    auto rhsHistory = rhs.pimpl->boardOffset;
    auto playerHistory = this->pimpl->boardOffset;
        
    while(rhsHistory != nullptr) //Valgrind flag
    {
        for(int i = 0; i < 8; ++i)
            for(int j = 0; j < 8; ++j)
                playerHistory->board[i][j] = rhsHistory->board[i][j];

        //Ex flag valgrind
        playerHistory->prev = rhsHistory->prev ? new History : nullptr;

        rhsHistory = rhsHistory->prev;
        playerHistory = playerHistory->prev;
    }
        
}


Player& Player::operator=(const Player& rhs)
{
    if(this != &rhs)
    {
        deleteHistory(this->pimpl->boardOffset);
        this->pimpl->player_nr = rhs.pimpl->player_nr;
        if(rhs.pimpl->boardOffset != nullptr)
        {
            this->pimpl->boardOffset = new History;
            //Ex flag valgrind perchè dimenticato
            this->pimpl->boardOffset->prev = nullptr;
            History* rhsHistory = rhs.pimpl->boardOffset;
            History* playerHistory  = this->pimpl->boardOffset;
            while(rhsHistory)
            {
                for(int i = 0; i < 8; ++i)
                    for(int j = 0; j < 8; ++j)
                        playerHistory->board[i][j] = rhsHistory->board[i][j];

                if(rhsHistory->prev)
                {
                    playerHistory->prev = new History;
                    playerHistory = playerHistory ->prev;
                    playerHistory->prev = nullptr;
                }
                rhsHistory = rhsHistory->prev;
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

    History* playerHistory = this->pimpl->boardOffset;
    while(history_offset-- > 0 && playerHistory)
    {
        playerHistory = playerHistory->prev;
    }

    if(history_offset < 0 || !playerHistory)
        throw player_exception{player_exception::index_out_of_bounds, "Board with the given offset does not exist."};

    return playerHistory->board[r][c];
}

void Player::load_board(const std::string &filename)
{

    std::ifstream reader(filename);
    std::string line;
    if(!reader)
        throw player_exception{player_exception::missing_file, "The file does not exist."};
     
    //Se non esiste la history
    if(this->pimpl->boardOffset == nullptr)
    {
        this->pimpl->boardOffset = new History;
        this->pimpl->boardOffset->prev = nullptr; // Mettendo questo fixa tutto what de actual cazzo
    }
    else
    {
        History* nextBoard = new History;
        nextBoard->prev = this->pimpl->boardOffset;
        this->pimpl->boardOffset = nextBoard;
    }
     
    //Praticamente in ogni file va tipo:
    //(CELLA SPAZIO)
    //Ecco perchè non andava quando consegnato
    size_t p1Pieces = 0, p2Pieces = 0;
    for(int i = 7; i >= 0; --i)
    {
        if(getline(reader, line))
        {
            if(line.size() != 15)
                throw player_exception{player_exception::invalid_board, "Wrong line size"};
            size_t col = 0;
            Player::piece curr;
            //Modificato j += 2 con ++j e aggiunto if, vediamo se cambia qualcosa
            for(size_t j = 0; j < line.size(); ++j)
            {
                switch (line[j])
                {
                case 'x':
                    curr = Player::piece::x;
                    ++p1Pieces;
                    break;
                case 'X':
                    curr = Player::piece::X;
                    ++p1Pieces;
                    break;
                case 'o':
                    curr = Player::piece::o;
                    ++p2Pieces;
                    break;
                case 'O':
                    curr = Player::piece::O;
                    ++p2Pieces;
                    break;
                case ' ':
                    curr = Player::piece::e;
                    break;
                default:
                    throw player_exception{player_exception::invalid_board, "Not a valid piece"};
                    break;
                }
                if(j % 2 == 0)
                    this->pimpl->boardOffset->board[i][col++] = curr;
            }
        }
    }

    if(p1Pieces > 12)
        throw player_exception{player_exception::invalid_board, "Player 1 has too many pieces"};

    if(p2Pieces > 12)
        throw player_exception{player_exception::invalid_board, "Player 2 has too many pieces"};

    //Controlla pezzi non promossi
    for(size_t i = 0; i < 8; ++i)
        if(this->pimpl->boardOffset->board[0][i] == Player::piece::o
           || this->pimpl->boardOffset->board[7][i] == Player::piece::x)
            throw player_exception{player_exception::invalid_board, "Pieces were not promoted."};

    //Pezzi nei posti sbagliati
    //se i pari -> controllo j pari
    //se i dispari -> controllo j dispari 
    for(int i = 0; i < 8; ++i)
        for(int j = 0; j < 8; ++j)
        {
            if(i % 2 == 0 && j % 2 == 0 && this->pimpl->boardOffset->board[i][j] != Player::piece::e)
                throw player_exception{player_exception::invalid_board, "Invalid board"};
            if(i % 2 != 0 && j % 2 != 0 && this->pimpl->boardOffset->board[i][j] != Player::piece::e)
                throw player_exception{player_exception::invalid_board, "Invalid board"};
        }
}

void Player::init_board(const std::string &filename) const
{
    std::ofstream writer(filename, std::ios::trunc);
    std::string row1 = "o   o   o   o  \n";
    std::string row2 = "  o   o   o   o\n";
    std::string emptyRow ="               \n";
    std::string row3 = "  x   x   x   x\n";
    std::string row4 = "x   x   x   x  \n";
    std::string row5 = "  x   x   x   x";
    writer << row1;
    writer << row2;
    writer << row1;
    writer << emptyRow;
    writer << emptyRow;
    writer << row3;
    writer << row4;
    writer << row5;
}

void Player::store_board(const std::string &filename, int history_offset) const
{
    std::ofstream writer(filename, std::ios::trunc);
    History* index = this->pimpl->boardOffset;
    while(index && history_offset-- > 0)
    {
	index = index->prev;
    }

    if(!index)
	throw player_exception{player_exception::index_out_of_bounds, "The given offset does not exist"};
    
    for(int i = 7; i >= 0; --i)
    {
	for(int j = 0; j < 8; ++j)
	{
	    if(j != 7)
	    {
		switch (index->board[i][j])
		{
		case Player::piece::x:
		    writer << "x ";
		    break;
		case Player::piece::X:
		    writer << "X ";
		    break;
		case Player::piece::o:
		    writer << "o ";
		    break;
		case Player::piece::O:
		    writer << "O ";
		    break;
		default:
		    writer << "  ";
		    break;
		}
	    }
	    else
	    {
		if(i != 0)
		{
		    switch (index->board[i][j])
		    {
		    case Player::piece::x:
			writer << "x\n";
			break;
		    case Player::piece::X:
			writer << "X\n";
			break;
		    case Player::piece::o:
			writer << "o\n";
			break;
		    case Player::piece::O:
			writer << "O\n";
			break;
		    default:
			writer << " \n";
			break;
		    }
		}
		else //Ultima riga
		{
		    switch (index->board[i][j])
		    {
		    case Player::piece::x:
			writer << "x";
			break;
		    case Player::piece::X:
			writer << "X";
			break;
		    case Player::piece::o:
			writer << "o";
			break;
		    case Player::piece::O:
			writer << "O";
			break;
		    default:
			writer << " ";
			break;
		    }
		}
	    }
	}
    }
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
        
        if(player_nr == 2)
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
        
        if(player_nr == 2)
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
            else if(board[row - 1][col + 1] == Player::piece::X) 
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
        
        if(player_nr == 1)
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
            else if(board[row - 1][col + 1] == Player::piece::X)
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
        
        if(player_nr == 1)
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


    //Prova le 4 direzioni per ogni pezzo
    //Minimax ma senza depth
    double bestEval;
    if(this->pimpl->player_nr == 1)
        bestEval = -400000;
    else
        bestEval = POS_INF;
    
    std::pair<int, int> coords(-2, -2);
    char direction = 'W';
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
                else if(this->pimpl->player_nr == 2)
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
                else if(this->pimpl->player_nr == 2)
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
                else if(this->pimpl->player_nr == 2)
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
                else if(this->pimpl->player_nr == 2)
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
    //Anche se non ha fatto mosse
    History* nextMove = new History;
    nextMove->prev = this->pimpl->boardOffset;
    this->pimpl->boardOffset = nextMove;
    for(int i = 0; i < 8; ++i)
        for(int j = 0; j < 8; ++j)
            nextMove->board[i][j] = temporaryBoard[i][j];

}


bool Player::valid_move() const
{
    if(!this->pimpl->boardOffset || !this->pimpl->boardOffset->prev)
        throw player_exception{player_exception::index_out_of_bounds, "Less than two boards in history."};

    //Controllo se la board è uguale a quella precedente
    int r = 0, c = 0;
    bool equalBoards = true;
    bool legalBoard = true;
    while(r < 8 && equalBoards)
    {
        while(c < 8 && equalBoards)
        {
            equalBoards = equalBoards &&
                (this->pimpl->boardOffset->board[r][c] == this->pimpl->boardOffset->prev->board[r][c]);
            ++c;
        }
        c = 0;
        ++r;
    }
    legalBoard = !equalBoards;
    
    //Guardo eventuali celle bianche con pezzi
    for(int i = 0; i < 8; ++i)
        for(int j = 0; j < 8; ++j)
        {
            if(i % 2 == 0 && j % 2 == 0 && this->pimpl->boardOffset->board[i][j] != Player::piece::e) // Righe pari
                legalBoard = false;
            if(i % 2 != 0 && j % 2 != 0 && this->pimpl->boardOffset->board[i][j] != Player::piece::e) // Righe dispari
                legalBoard = false;
        }
    
    //Pezzi non promossi
    for(int j = 0; j < 8; ++j)
        if(this->pimpl->boardOffset->board[0][j] == Player::piece::o
           || this->pimpl->boardOffset->board[7][j] == Player::piece::x)
            legalBoard = false;

    std::pair<int, int> changes[3];
    int changedPositions = 0;
    for(int i = 0; i < 8; ++i)
    {
        for(int j = 0; j < 8; ++j)
            if(this->pimpl->boardOffset->board[i][j] != this->pimpl->boardOffset->prev->board[i][j])
            {
                if(changedPositions < 3)
                {
                    changes[changedPositions].first = i;
                    changes[changedPositions++].second = j;
                }
            }
    }

    if(changedPositions > 3)
        legalBoard = false;

    //Evito di controllare le mosse se ho già più di tre caselle
    if(!legalBoard)
        return false;
    
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

            //reset allo stato precedente
            for(int r = 0; r < 8; ++r)
                for(int c = 0; c < 8; ++c)
                    tempBoard[r][c] = this->pimpl->boardOffset->prev->board[r][c];
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

            //reset allo stato precedente
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

            //reset allo stato precedente
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

            //reset allo stato precedente
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

    History* currentBoard = this->pimpl->boardOffset;
    this->pimpl->boardOffset = this->pimpl->boardOffset->prev;
    delete currentBoard;
}

bool Player::wins(int player_nr) const
{
    if(!(player_nr == 1 || player_nr == 2))
        throw player_exception{player_exception::index_out_of_bounds, "Player number not valid."};

    if(this->pimpl->boardOffset == nullptr)
        throw player_exception{player_exception::index_out_of_bounds, "History does not exist."};
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
    
    if(player_nr == 2)    // 'o'
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

    if(this->pimpl->boardOffset == nullptr)
        throw player_exception{player_exception::index_out_of_bounds, "History does not exist."};
    
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

    History* playerHistory = this->pimpl->boardOffset->prev;
    while(playerHistory)
    {
        bool equal = true;
        for(size_t i = 0; i < 8; ++i)
            for(size_t j = 0; j < 8; ++j)
                equal = equal && (this->pimpl->boardOffset->board[i][j] == playerHistory->board[i][j]);

        if(equal)
            ++times;

        playerHistory = playerHistory->prev;
    }
    return times;
}


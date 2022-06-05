#include "player.hpp"
#define POS_INF  ((unsigned) ~0)

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
    this->pimpl->boardOffset = new History;
    this->pimpl->boardOffset->prev = nullptr;
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

Player::Player(const Player& rhs)
{
}

Player& Player::operator=(const Player &rhs)
{
    if(this != &rhs)
    {
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
                throw player_exception{player_exception::invalid_board, "Pieces on while cells are not allowed."};

            if(i % 2 != 0 && j % 2 != 0 && temp[j] != ' ')
                throw player_exception{player_exception::invalid_board, "Pieces on while cells are not allowed."};

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

void Player::init_board(const std::string &filename) const //done
{
    std::ofstream writeBoard;
    writeBoard.open(filename);   
    
    //x
    for(size_t i = 0; i < 3; ++i)
    {
        if(i % 2)
            for(size_t j = 0; j < 8; ++j)
                this->pimpl->lastBoard[i][j] = j % 2 ? Player::piece::e : Player::piece::x;
        else
            for(size_t j = 0; j < 8; ++j)
                this->pimpl->lastBoard[i][j] = j % 2 ? Player::piece::x : Player::piece::e;
    }

    //empty
    for(size_t i = 3; i < 5; ++i)
        for(size_t j = 0; j < 8; ++j)
            this->pimpl->lastBoard[i][j] = Player::piece::e;

    //o
    for(size_t i = 5; i < 8; ++i)
    {
        if(i % 2)
            for(size_t j = 0; j < 8; ++j)
                this->pimpl->lastBoard[i][j] = j % 2 ? Player::piece::e : Player::piece::o;
        else
            for(size_t j = 0; j < 8; ++j)
                this->pimpl->lastBoard[i][j] = j % 2 ? Player::piece::o : Player::piece::e;
    }

    //Salva su file
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
        *Pezzi normali: value 1
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
                ++j;
            }
            j = 0;
        }
    }

    if(player_nr == 2)
    {
        for(int i = 8; i >= 0; --i)
            for(int j = 0; j < 8; ++j)
            {

            }
    }
    return moves;
}

double minimax(Player::piece board[8][8], int depth, int player_nr, int alpha, int beta) 
//TODO: Evitare di copiare tutta la board ogni volta
//Alla fine della ricorsione annulla il movimento iniziale
{
    if(noMoves(board, player_nr))  //Settare un valore per indicare perdita per il giocatore
        return player_nr == 1 ? -400000 : POS_INF;

    if(depth == 0)
        return evaluateBoard(board);

    Player::piece cpyBoard[8][8];
    for(int i = 0; i < 8; ++i)
        for(int j = 0; j < 8; ++j)
            cpyBoard[i][j] = board[i][j];

    if(player_nr == 1) //Massimizza per player1 
    {
        auto maxEval = -400000; //Valore a caso negativo 
        for(int i = 0; i < 8; ++i)
            for(int j = 0; j < 8; ++j)
            {
                //Mossa
                if(board[i][j] == Player::piece::x) //Solo basso sx e dx, ricordati che sono invertite nella memoria 
                {
                    if(i + 1 < 8 && j - 1 >= 0)
                    {
                        switch(board[i + 1][j - 1])
                        {
                            case Player::piece::e:
                                cpyBoard[i][j] = Player::piece::e;
                                cpyBoard[i + 1][j - 1] = i + 1 == 7 ? Player::piece::X : Player::piece::x; //Promuove in caso
                                break;
                            case Player::piece::o:
                                if(i + 2 < 8 && j - 2 >= 0)
                                {    
                                    cpyBoard[i][j] = Player::piece::e;
                                    cpyBoard[i + 1][j - 1] = Player::piece::e;
                                    cpyBoard[i + 2][j - 2] = i + 2 == 7 ? Player::piece::X : Player::piece::x; //Promuove in caso
                                }
                                break;
                            default: // pezzo alleato o che non può mangiare
                                break;
                        }
                        auto eval = minimax(cpyBoard, depth - 1, 2, alpha, beta);
                        maxEval = eval > maxEval ? eval : maxEval;
                        alpha = alpha > eval ? alpha : eval;
                        if(alpha >= beta)
                            break;
                        return maxEval;
                    }

                    if(i + 1 < 8 && j + 1 < 8)
                    {
                        for(int i = 0; i < 8; ++i) //Potenzialmente inutile se non modificata prima
                            for(int j = 0; j < 8; ++j)
                                cpyBoard[i][j] = board[i][j];

                        switch(board[i + 1][j + 1])
                        {
                            case Player::piece::e:
                                cpyBoard[i][j] = Player::piece::e;
                                cpyBoard[i + 1][j + 1] = i + 1 == 7 ? Player::piece::X : Player::piece::x; //Promuove in caso
                                break;
                            case Player::piece::o:
                                if(i + 2 < 8 && j + 2 < 8)
                                {    
                                    cpyBoard[i][j] = Player::piece::e;
                                    cpyBoard[i + 1][j + 1] = Player::piece::e;
                                    cpyBoard[i + 2][j + 2] = i + 2 == 7 ? Player::piece::X : Player::piece::x; //Promuove in caso
                                }
                                break;
                            default: // pezzo alleato o che non può mangiare
                                break;
                        }

                        auto eval = minimax(cpyBoard, depth - 1, 2, alpha, beta);
                        maxEval = eval > maxEval ? eval : maxEval;
                        alpha = alpha > eval ? alpha : eval;
                        if(alpha >= beta)
                            break;
                        return maxEval;
                    }
                }

                if(board[i][j] == Player::piece::X) // Tutte e 4 direzioni
                {

                    for(int i = 0; i < 8; ++i)
                        for(int j = 0; j < 8; ++j)
                            cpyBoard[i][j] = board[i][j];

                    if(i + 1 < 8 && j - 1 >= 0) //Alto SX
                    {
                        switch(board[i + 1][j - 1])
                        {
                            case Player::piece::e:
                                cpyBoard[i][j] = Player::piece::e;
                                cpyBoard[i + 1][j - 1] = Player::piece::X;
                                break;
                            case Player::piece::o:
                                if(i + 2 < 8 && j - 2 >= 0)
                                {    
                                    cpyBoard[i][j] = Player::piece::e;
                                    cpyBoard[i + 1][j - 1] = Player::piece::e;
                                    cpyBoard[i + 2][j - 2] = Player::piece::X;
                                }
                                break;
                            case Player::piece::O:
                                if(i + 2 < 8 && j - 2 >= 0)
                                {    
                                    cpyBoard[i][j] = Player::piece::e;
                                    cpyBoard[i + 1][j - 1] = Player::piece::e;
                                    cpyBoard[i + 2][j - 2] = Player::piece::X;
                                }
                                break;
                            default: // pezzo alleato o che non può mangiare
                                break;
                        }
                        auto eval = minimax(cpyBoard, depth - 1, 2, alpha, beta);
                        maxEval = eval > maxEval ? eval : maxEval;
                        alpha = alpha > eval ? alpha : eval;
                        if(alpha >= beta)
                            break;
                        return maxEval;
                    }

                    if(i + 1 < 8 && j + 1 < 8) //Alto DX
                    {
                        switch(board[i + 1][j + 1])
                        {
                            case Player::piece::e:
                                cpyBoard[i][j] = Player::piece::e;
                                cpyBoard[i + 1][j + 1] = Player::piece::X;
                                break;
                            case Player::piece::o:
                                if(i + 2 < 8 && j + 2 < 8)
                                {    
                                    cpyBoard[i][j] = Player::piece::e;
                                    cpyBoard[i + 1][j + 1] = Player::piece::e;
                                    cpyBoard[i + 2][j + 2] = Player::piece::X;
                                }
                                break;
                            case Player::piece::O:
                                if(i + 2 < 8 && j - 2 < 8)
                                {    
                                    cpyBoard[i][j] = Player::piece::e;
                                    cpyBoard[i + 1][j + 1] = Player::piece::e;
                                    cpyBoard[i + 2][j + 2] = Player::piece::X;
                                }
                                break;
                            default: // pezzo alleato
                                break;
                        }
                        auto eval = minimax(cpyBoard, depth - 1, 2, alpha, beta);
                        maxEval = eval > maxEval ? eval : maxEval;
                        alpha = alpha > eval ? alpha : eval;
                        if(alpha >= beta)
                            break;
                        return maxEval;
                    }

                    if(i - 1 >= 0 && j - 1 >= 0) //Basso SX
                    {
                        switch(board[i - 1][j - 1])
                        {
                            case Player::piece::e:
                                cpyBoard[i][j] = Player::piece::e;
                                cpyBoard[i - 1][j - 1] = Player::piece::X;
                                break;
                            case Player::piece::o:
                                if(i - 2 >= 0 && j - 2 >= 0)
                                {    
                                    cpyBoard[i][j] = Player::piece::e;
                                    cpyBoard[i - 1][j - 1] = Player::piece::e;
                                    cpyBoard[i - 2][j - 2] = Player::piece::X;
                                }
                                break;
                            case Player::piece::O:
                                if(i - 2 >= 0 && j - 2 >= 0)
                                {    
                                    cpyBoard[i][j] = Player::piece::e;
                                    cpyBoard[i - 1][j - 1] = Player::piece::e;
                                    cpyBoard[i - 2][j - 2] = Player::piece::X;
                                }
                                break;
                            default: // pezzo alleato
                                break;
                        }
                        auto eval = minimax(cpyBoard, depth - 1, 2, alpha, beta);
                        maxEval = eval > maxEval ? eval : maxEval;
                        alpha = alpha > eval ? alpha : eval;
                        if(alpha >= beta)
                            break;
                        return maxEval;
                    }

                    if(i - 1 >= 0 && j + 1 < 8) //Basso DX
                    {
                        switch(board[i - 1][j + 1])
                        {
                            case Player::piece::e:
                                cpyBoard[i][j] = Player::piece::e;
                                cpyBoard[i - 1][j + 1] = Player::piece::X;
                                break;
                            case Player::piece::o:
                                if(i - 2 >= 0 && j + 2 <= 8)
                                {    
                                    cpyBoard[i][j] = Player::piece::e;
                                    cpyBoard[i - 1][j + 1] = Player::piece::e;
                                    cpyBoard[i - 2][j + 2] = Player::piece::X;
                                }
                                break;
                            case Player::piece::O:
                                if(i - 2 >= 0 && j + 2 <= 8)
                                {    
                                    cpyBoard[i][j] = Player::piece::e;
                                    cpyBoard[i - 1][j + 1] = Player::piece::e;
                                    cpyBoard[i - 2][j + 2] = Player::piece::X;
                                }
                                break;
                            default: // pezzo alleato o che non può mangiare
                                break;
                        }
                        auto eval = minimax(cpyBoard, depth - 1, 2, alpha, beta);
                        maxEval = eval > maxEval ? eval : maxEval;
                        alpha = alpha > eval ? alpha : eval;
                        if(alpha >= beta)
                            break;
                        return maxEval;
                    }
                }
            }
    }

    if(player_nr == 2) //Massimizza per player2 (minimizza player1)
    {
        auto minEval = POS_INF;
        for(int i = 7; i >= 0; --i)
            for(int j = 0; j < 8; ++j)
            {
                //Mossa
                auto eval = minimax(cpyBoard, depth - 1, 1, alpha, beta);
                minEval = eval < minEval ? eval : minEval;
                beta = beta < eval ? beta : eval;
                if(alpha >= beta)
                    break;
                return minEval;
            }         
    }    
    return 0; // Giusto per i warning
}

void Player::move()
{
    //https://www.youtube.com/watch?v=l-hh51ncgDI
    //Spiegazione 
    if(noMoves(this->pimpl->lastBoard, this->pimpl->player_nr))
        return; //Salva una board uguale

}

bool Player::valid_move() const
{
    //Trovo dov'è il cambio 
    return false;
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
    if(player_nr != 1 || player_nr != 2)
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

        if(!otherPieces)
            win = true;
    }

    if(player_nr == 2) // 'o'
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

        if(!otherPieces)
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
    if(player_nr != 1 || player_nr != 2)
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
    History* temp = this->pimpl->boardOffset;
    while(temp->prev) //Apparently esiste una seconda board quando creo che è piena di x wtf
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

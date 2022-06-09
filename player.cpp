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
    this->pimpl = new Impl;
    this->pimpl->boardOffset = new History;
    this->pimpl->player_nr = rhs.pimpl->player_nr;

    History* temp = rhs.pimpl->boardOffset;
    History* t  = this->pimpl->boardOffset;
    while(temp)
    {
        for(int i = 0; i < 8; ++i)
            for(int j = 0; j < 8; ++j)
                t->board[i][j] = temp->board[i][j];

        if(temp->prev)
        {
            temp = temp->prev;
            t->prev = new History;
            t = t ->prev;
        }
    }
}

Player& Player::operator=(const Player &rhs)
{
    if(this != &rhs)
    {
        deleteHistory(this->pimpl->boardOffset);
        this->pimpl->player_nr = rhs.pimpl->player_nr;
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
                temp = temp->prev;
                t->prev = new History;
                t = t ->prev;
            }
        }
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

void Player::init_board(const std::string &filename) const //done
{
    std::ofstream writeBoard;
    writeBoard.open(filename);   
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
        std::string row = "";
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
                if(board[i][j] == Player::piece::x)
                {
                    if(i + 1 < 8 && j - 1 >= 0 && board[i + 1][j - 1] == Player::piece::e) //casella vuota SX
                        moves = true;

                    if(!moves && i + 1 < 8 && j + 1 < 8 && board[i + 1][j + 1] == Player::piece::e) //casella vuota DX
                        moves = true;

                    //Pezzo nemico sx e posso mangiarlo
                    if(!moves && i + 1 < 8 && j - 1 >= 0 && board[i + 1][j - 1] == Player::piece::o)
                        if(i + 2 < 8 && j - 2 >= 0 && board[i + 2][j - 2] == Player::piece::e)
                            moves = true;

                    //Pezzo nemico dx e posso mangiarlo
                    if(!moves && i + 1 < 8 && j + 1 < 8 && board[i + 1][j + 1] == Player::piece::o)
                        if(i + 2 < 8 && j + 2 < 8 && board[i + 2][j + 2] == Player::piece::e)
                            moves = true;
                }

                if(board[i][j] == Player::piece::X)
                {
                    if(i + 1 < 8 && j - 1 >= 0 && board[i + 1][j - 1] == Player::piece::e) //casella vuota SX
                        moves = true;

                    if(!moves && i + 1 < 8 && j + 1 < 8 && board[i + 1][j + 1] == Player::piece::e) //casella vuota DX
                        moves = true;

                    //Pezzo nemico alto sx e posso mangiarlo
                    if(!moves && i + 1 < 8 && j - 1 >= 0 && (board[i + 1][j - 1] == Player::piece::o)
                                                        || board[i + 1][j - 1] == Player::piece::O)
                        if(i + 2 < 8 && j - 2 >= 0 && board[i + 2][j - 2] == Player::piece::e)
                            moves = true;

                    //Pezzo nemico alto dx e posso mangiarlo
                    if(!moves && i + 1 < 8 && j + 1 < 8 && (board[i + 1][j + 1] == Player::piece::o
                                                        || board[i + 1][j + 1] == Player::piece::O))
                        if(i + 2 < 8 && j + 2 < 8 && board[i + 2][j + 2] == Player::piece::e)
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
                    if(i - 1 >= 0 && j - 1 >= 0 && board[i - 1][j - 1] == Player::piece::e) //casella vuota SX
                        moves = true;

                    if(!moves && i - 1 >= 0 && j + 1 < 8 && board[i - 1][j + 1] == Player::piece::e) //casella vuota DX
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

                if(board[i][j] == Player::piece::X)
                {
                    if(i + 1 < 8 && j - 1 >= 0 && board[i + 1][j - 1] == Player::piece::e) //casella vuota SX
                        moves = true;

                    if(!moves && i + 1 < 8 && j + 1 < 8 && board[i + 1][j + 1] == Player::piece::e) //casella vuota DX
                        moves = true;

                    //Pezzo 'o' nemico sx e posso mangiarlo
                    if(!moves && i + 1 < 8 && j - 1 >= 0 && board[i + 1][j - 1] == Player::piece::o)
                        if(i + 2 < 8 && j - 2 >= 0 && board[i + 2][j - 2] == Player::piece::e)
                            moves = true;

                    //Pezzo 'o' nemico dx e posso mangiarlo
                    if(!moves && i + 1 < 8 && j + 1 < 8 && board[i + 1][j + 1] == Player::piece::o)
                        if(i + 2 < 8 && j + 2 < 8 && board[i + 2][j + 2] == Player::piece::e)
                            moves = true;

                    //Pezzo 'O' nemico sx e posso mangiarlo
                    if(!moves && i + 1 < 8 && j - 1 >= 0 && board[i + 1][j - 1] == Player::piece::O)
                        if(i + 2 < 8 && j - 2 >= 0 && board[i + 2][j - 2] == Player::piece::e)
                            moves = true;

                    //Pezzo 'O' nemico dx e posso mangiarlo
                    if(!moves && i + 1 < 8 && j + 1 < 8 && board[i + 1][j + 1] == Player::piece::O)
                        if(i + 2 < 8 && j + 2 < 8 && board[i + 2][j + 2] == Player::piece::e)
                            moves = true;
                }
                ++j;
            }
            j = 0;
            ++i;
        }
    }
    return moves;
}

double minimax(Player::piece board[8][8], int depth, int player_nr, double alpha, double beta) 
//TODO: Evitare di copiare tutta la board ogni volta
//Alla fine della ricorsione annulla il movimento iniziale
{
    if(noMoves(board, player_nr))  //Settare un valore per indicare perdita per il giocatore
        return player_nr == 1 ? -400000 : POS_INF;

    if(depth == 0)
        return evaluateBoard(board);
/*
    Player::piece cpyBoard[8][8];
    for(int i = 0; i < 8; ++i)
        for(int j = 0; j < 8; ++j)
            cpyBoard[i][j] = board[i][j];
*/
    if(player_nr == 1) //Massimizza per player1 
    {
        auto maxEval = -400000; //Valore a caso negativo 
        for(int i = 0; i < 8; ++i)
        {
            for(int j = 0; j < 8; ++j)
            {
                bool modified = false;
                Player::piece first, second;
                //Mossa
                if(board[i][j] == Player::piece::x) //Solo basso sx e dx, ricordati che sono invertite nella memoria 
                {
                    if(i + 1 < 8 && j - 1 >= 0)
                    {
                        
                        switch(board[i + 1][j - 1])
                        {
                            case Player::piece::e:
                                first = board[i + 1][j - 1];
                                board[i][j] = Player::piece::e;
                                board[i + 1][j - 1] = i + 1 == 7 ? Player::piece::X : Player::piece::x; //Promuove
                                modified = true;
                                break;
                            case Player::piece::o:
                                if(i + 2 < 8 && j - 2 >= 0)
                                {   
                                    first = board[i + 1][j - 1];
                                    second = board[i + 2][j - 2]; 
                                    board[i][j] = Player::piece::e;
                                    board[i + 1][j - 1] = Player::piece::e;
                                    board[i + 2][j - 2] = i + 2 == 7 ? Player::piece::X : Player::piece::x; //Promuove
                                    modified = true;
                                }
                                break;
                            default: // pezzo alleato o che non può mangiare
                                break;
                        }
                        auto eval = minimax(board, depth - 1, 2, alpha, beta);
                        //Undo mossa precedente se fatta
                        if(modified)
                        {
                            board[i][j] = Player::piece::x;
                            board[i + 1][j - 1] = first;
                            if(i + 2 < 8 && j - 2 >= 0)
                                board[i + 2][j - 2] = second;
                            
                            modified = false;
                        }
                        maxEval = eval > maxEval ? eval : maxEval;
                        alpha = alpha > maxEval ? alpha : maxEval;
                        if(alpha >= beta) // Da ritornare un valore magari e non break
                            break;
                        return maxEval;
                    }

                    if(i + 1 < 8 && j + 1 < 8)
                    {
                        switch(board[i + 1][j + 1])
                        {
                            case Player::piece::e:
                                first = board[i + 1][j + 1];
                                board[i][j] = Player::piece::e;
                                board[i + 1][j + 1] = i + 1 == 7 ? Player::piece::X : Player::piece::x; //Promuove
                                modified = true;
                                break;
                            case Player::piece::o:
                                if(i + 2 < 8 && j + 2 < 8)
                                {    
                                    first = board[i + 1][j + 1];
                                    second = board[i + 2][j + 2];
                                    board[i][j] = Player::piece::e;
                                    board[i + 1][j + 1] = Player::piece::e;
                                    board[i + 2][j + 2] = i + 2 == 7 ? Player::piece::X : Player::piece::x; //Promuove
                                    modified = true;
                                }
                                break;
                            default: // pezzo alleato o che non può mangiare
                                break;
                        }

                        auto eval = minimax(board, depth - 1, 2, alpha, beta);
                        if(modified)
                        {
                            board[i][j] = Player::piece::x;
                            board[i + 1][j + 1] = first;
                            if(i + 2 < 8 && j + 2 < 8)
                                board[i + 2][j + 2] = second;

                            modified = false;
                        }
                        maxEval = eval > maxEval ? eval : maxEval;
                        alpha = alpha > maxEval ? alpha : maxEval;
                        if(alpha >= beta)
                            break;
                        return maxEval;
                    }
                }

                if(board[i][j] == Player::piece::X) // Tutte e 4 direzioni
                {
                    if(i + 1 < 8 && j - 1 >= 0) //Alto SX
                    {
                        switch(board[i + 1][j - 1])
                        {
                            case Player::piece::e:
                                modified = true;
                                first = board[i + 1][j - 1];
                                board[i][j] = Player::piece::e;
                                board[i + 1][j - 1] = Player::piece::X;
                                break;
                            case Player::piece::o:
                                if(i + 2 < 8 && j - 2 >= 0)
                                {    
                                    modified = true;
                                    first = board[i + 1][j - 1];
                                    second = board[i + 2][j - 2];
                                    board[i][j] = Player::piece::e;
                                    board[i + 1][j - 1] = Player::piece::e;
                                    board[i + 2][j - 2] = Player::piece::X;
                                }
                                break;
                            case Player::piece::O:
                                if(i + 2 < 8 && j - 2 >= 0)
                                {    
                                    modified = true;
                                    first = board[i + 1][j - 1];
                                    second = board[i + 2][j - 2];
                                    board[i][j] = Player::piece::e;
                                    board[i + 1][j - 1] = Player::piece::e;
                                    board[i + 2][j - 2] = Player::piece::X;
                                }
                                break;
                            default: // pezzo alleato o che non può mangiare
                                break;
                        }
                        auto eval = minimax(board, depth - 1, 2, alpha, beta);
                        if(modified)
                        {
                            board[i][j] = Player::piece::X;
                            board[i + 1][j - 1] = first;
                            if(i + 2 < 8 && j - 2 >= 0)
                                board[i + 2][j - 2] = second;
                            
                            modified = false;
                        }
                        maxEval = eval > maxEval ? eval : maxEval;
                        alpha = alpha > maxEval ? alpha : maxEval;
                        if(alpha >= beta)
                            break;
                        return maxEval;
                    }

                    if(i + 1 < 8 && j + 1 < 8) //Alto DX
                    {
                        switch(board[i + 1][j + 1])
                        {
                            case Player::piece::e:
                                modified = true;
                                first = board[i + 1][j + 1];
                                board[i][j] = Player::piece::e;
                                board[i + 1][j + 1] = Player::piece::X;
                                break;
                            case Player::piece::o:
                                if(i + 2 < 8 && j + 2 < 8)
                                {   
                                    modified = true;
                                    first = board[i + 1][j + 1];
                                    second = board[i + 2][j + 2]; 
                                    board[i][j] = Player::piece::e;
                                    board[i + 1][j + 1] = Player::piece::e;
                                    board[i + 2][j + 2] = Player::piece::X;
                                }
                                break;
                            case Player::piece::O:
                                if(i + 2 < 8 && j - 2 < 8)
                                {    
                                    modified = true;
                                    first = board[i + 1][j + 1];
                                    second = board[i + 2][j + 2]; 
                                    board[i][j] = Player::piece::e;
                                    board[i + 1][j + 1] = Player::piece::e;
                                    board[i + 2][j + 2] = Player::piece::X;
                                }
                                break;
                            default: // pezzo alleato 
                                break;
                        }
                        auto eval = minimax(board, depth - 1, 2, alpha, beta);
                        if(modified)
                        {
                            board[i][j] = Player::piece::X;
                            board[i + 1][j + 1] = first;
                            if(i +  2 < 8 && j + 2 < 8)
                                board[i + 2][j + 2] = second;

                            modified = false;
                        }
                        maxEval = eval > maxEval ? eval : maxEval;
                        alpha = alpha > maxEval ? alpha : maxEval;
                        if(alpha >= beta)
                            break;
                        return maxEval;
                    }

                    if(i - 1 >= 0 && j - 1 >= 0) //Basso SX
                    {
                        switch(board[i - 1][j - 1])
                        {
                            case Player::piece::e:
                                modified = true;
                                first = board[i - 1][j - 1];
                                board[i][j] = Player::piece::e;
                                board[i - 1][j - 1] = Player::piece::X;
                                break;
                            case Player::piece::o:
                                if(i - 2 >= 0 && j - 2 >= 0)
                                {    
                                    modified = true;
                                    first = board[i - 1][j - 1];
                                    second = board[i - 2][j - 2];
                                    board[i][j] = Player::piece::e;
                                    board[i - 1][j - 1] = Player::piece::e;
                                    board[i - 2][j - 2] = Player::piece::X;
                                }
                                break;
                            case Player::piece::O:
                                if(i - 2 >= 0 && j - 2 >= 0)
                                {    
                                    modified = true;
                                    first = board[i - 1][j - 1];
                                    second = board[i - 2][j - 2];
                                    board[i][j] = Player::piece::e;
                                    board[i - 1][j - 1] = Player::piece::e;
                                    board[i - 2][j - 2] = Player::piece::X;
                                }
                                break;
                            default: // pezzo alleato
                                break;
                        }
                        auto eval = minimax(board, depth - 1, 2, alpha, beta);
                        if(modified)
                        {
                            board[i][j] = Player::piece::X;
                            board[i - 1][j - 1] = first;
                            if(i - 2 >= 0 && j - 2 >= 0)
                                board[i - 2][j - 2] = second;

                            modified = false;
                        }
                        maxEval = eval > maxEval ? eval : maxEval;
                        alpha = alpha > maxEval ? alpha : maxEval;
                        if(alpha >= beta)
                            break;
                        return maxEval;
                    }

                    if(i - 1 >= 0 && j + 1 < 8) //Basso DX
                    {
                        switch(board[i - 1][j + 1])
                        {
                            case Player::piece::e:
                                modified = true;
                                first = board[i - 1][j + 1];
                                board[i][j] = Player::piece::e;
                                board[i - 1][j + 1] = Player::piece::X;
                                break;
                            case Player::piece::o:
                                if(i - 2 >= 0 && j + 2 <= 8)
                                {    
                                    modified = true;
                                    first = board[i - 1][j - 1];
                                    second = board[i - 2][j - 2];
                                    board[i][j] = Player::piece::e;
                                    board[i - 1][j + 1] = Player::piece::e;
                                    board[i - 2][j + 2] = Player::piece::X;
                                }
                                break;
                            case Player::piece::O:
                                if(i - 2 >= 0 && j + 2 <= 8)
                                {   
                                    modified = true;
                                    first = board[i - 1][j - 1];
                                    second = board[i - 2][j - 2]; 
                                    board[i][j] = Player::piece::e;
                                    board[i - 1][j + 1] = Player::piece::e;
                                    board[i - 2][j + 2] = Player::piece::X;
                                }
                                break;
                            default: // pezzo alleato o che non può mangiare
                                break;
                        }
                        auto eval = minimax(board, depth - 1, 2, alpha, beta);
                        if(modified)
                        {
                            board[i][j] = Player::piece::X;
                            board[i - 1][j + 1] = first;
                            if(i - 2 >= 0 && j + 2 < 8)
                                board[i - 2][j + 2] = second;

                            modified = false;
                        }
                        maxEval = eval > maxEval ? eval : maxEval;
                        alpha = alpha > maxEval ? alpha : maxEval;
                        if(alpha >= beta)
                            break;
                        return maxEval;
                    }
                }
            }
        }
    }

    if(player_nr == 2) //Massimizza per player2 (minimizza player1)
    {
        auto minEval = POS_INF;
        for(int i = 7; i >= 0; --i)
        {
            for(int j = 0; j < 8; ++j)
            {
                bool modified = false;
                Player::piece first, second;
                //Mossa
                if(board[i][j] == Player::piece::o) //Solo basso sx e dx, ricordati che sono invertite nella memoria 
                {
                    if(i - 1 >= 0 && j - 1 >= 0) //Basso SX
                    {
                        switch(board[i - 1][j - 1])
                        {
                            case Player::piece::e:
                                modified = true;
                                first = board[i - 1][j - 1];
                                board[i][j] = Player::piece::e;
                                board[i - 1][j - 1] = i - 1 == 0 ? Player::piece::O : Player::piece::o; //Promuove in caso
                                break;
                            case Player::piece::x:
                                if(i - 2 >= 0 && j - 2 >= 0)
                                {    
                                    modified = true;
                                    first = board[i - 1][j - 1];
                                    second = board[i - 2][j - 2];
                                    board[i][j] = Player::piece::e;
                                    board[i - 1][j - 1] = Player::piece::e;
                                    board[i - 2][j - 2] = i - 2 == 0 ? Player::piece::O : Player::piece::o; //Promuove in caso
                                }
                                break;
                            default: // pezzo alleato o che non può mangiare
                                break;
                        }
                        auto eval = minimax(board, depth - 1, 1, alpha, beta);
                        if(modified)
                        {
                            board[i][j] = Player::piece::o;
                            board[i - 1][j - 1] = first;
                            if(i - 2 >= 0 && j - 2  >= 0)
                                board[i - 2][j - 2] = second;

                            modified = false;
                        }
                        minEval = eval < minEval ? eval : minEval;
                        alpha = alpha < minEval ? alpha : minEval;
                        if(alpha >= beta)
                            break;
                        return minEval;
                    }

                    if(i - 1 >= 0 && j + 1 < 8) //Basso DX
                    {
                        switch(board[i - 1][j + 1])
                        {
                            case Player::piece::e:
                                modified = true;
                                first = board[i - 1][j + 1];
                                board[i][j] = Player::piece::e;
                                board[i - 1][j + 1] = i - 1 == 0 ? Player::piece::O : Player::piece::o; //Promuove in caso
                                break;
                            case Player::piece::x:
                                if(i + 2 < 8 && j + 2 < 8)
                                {    
                                    modified = true;
                                    first = board[i + 1][j + 1];
                                    second = board[i + 2][j + 2];
                                    board[i][j] = Player::piece::e;
                                    board[i - 1][j + 1] = Player::piece::e;
                                    board[i - 2][j + 2] = i - 2 == 0 ? Player::piece::O : Player::piece::o; //Promuove in caso
                                }
                                break;
                            default: // pezzo alleato o che non può mangiare
                                break;
                        }

                        auto eval = minimax(board, depth - 1, 1, alpha, beta);
                        if(modified)
                        {
                            board[i][j] = Player::piece::o;
                            board[i - 1][j + 1] = first;
                            if(i - 2 >= 0 && j + 2 < 8)
                                board[i - 2][j + 2] = second;

                            modified = false;
                        }
                        minEval = eval < minEval ? eval : minEval;
                        alpha = alpha < minEval ? alpha : minEval;
                        if(alpha >= beta)
                            break;
                        return minEval;
                    }
                }

                if(board[i][j] == Player::piece::O) // Tutte e 4 direzioni
                {
                    if(i + 1 < 8 && j - 1 >= 0) //Alto SX
                    {
                        switch(board[i + 1][j - 1])
                        {
                            case Player::piece::e:
                                modified = true;
                                first = board[i + 1][j - 1];
                                board[i][j] = Player::piece::e;
                                board[i + 1][j - 1] = Player::piece::O;
                                break;
                            case Player::piece::x:
                                if(i + 2 < 8 && j - 2 >= 0)
                                {    
                                    modified = true;
                                    first = board[i + 1][j - 1];
                                    second = board[i + 2][j - 2];
                                    board[i][j] = Player::piece::e;
                                    board[i + 1][j - 1] = Player::piece::e;
                                    board[i + 2][j - 2] = Player::piece::O;
                                }
                                break;
                            case Player::piece::X:
                                if(i + 2 < 8 && j - 2 >= 0)
                                {    
                                    modified = true;
                                    first = board[i + 1][j - 1];
                                    second = board[i + 2][j - 2];
                                    board[i][j] = Player::piece::e;
                                    board[i + 1][j - 1] = Player::piece::e;
                                    board[i + 2][j - 2] = Player::piece::O;
                                }
                                break;
                            default: // pezzo alleato o che non può mangiare
                                break;
                        }

                        auto eval = minimax(board, depth - 1, 1, alpha, beta);
                        if(modified)
                        {
                            board[i][j] = Player::piece::O;
                            board[i + 1][j - 1] = first;
                            if(i + 2 < 8 && j - 2 >= 0)
                                board[i + 2][j - 2] = second;

                            modified = false;
                        }
                        minEval = eval < minEval ? eval : minEval;
                        alpha = alpha < minEval ? alpha : minEval;
                        if(alpha >= beta)
                            break;
                        return minEval;
                    }

                    if(i + 1 < 8 && j + 1 < 8) //Alto DX
                    {
                        switch(board[i + 1][j + 1])
                        {
                            case Player::piece::e:
                                modified = true;
                                first = board[i + 1][j + 1];
                                board[i][j] = Player::piece::e;
                                board[i + 1][j + 1] = Player::piece::O;
                                break;
                            case Player::piece::x:
                                if(i + 2 < 8 && j + 2 < 8)
                                {   
                                    modified = true;
                                    first = board[i + 1][j + 1];
                                    second = board[i + 2][j + 2]; 
                                    board[i][j] = Player::piece::e;
                                    board[i + 1][j + 1] = Player::piece::e;
                                    board[i + 2][j + 2] = Player::piece::O;
                                }
                                break;
                            case Player::piece::X:
                                if(i + 2 < 8 && j - 2 < 8)
                                {    
                                    modified = true;
                                    first = board[i + 1][j + 1];
                                    second = board[i + 2][j + 2];
                                    board[i][j] = Player::piece::e;
                                    board[i + 1][j + 1] = Player::piece::e;
                                    board[i + 2][j + 2] = Player::piece::O;
                                }
                                break;
                            default: // pezzo alleato
                                break;
                        }

                        auto eval = minimax(board, depth - 1, 1, alpha, beta);
                        if(modified)
                        {
                            board[i][j] = Player::piece::O;
                            board[i + 1][j + 1] = first;
                            if(i + 2 < 8 && j + 2 < 8)
                                board[i + 2][j + 2] = second;

                            modified = false;
                        }
                        minEval = eval < minEval ? eval : minEval;
                        alpha = alpha < minEval ? alpha : minEval;
                        if(alpha >= beta)
                            break;
                        return minEval;
                    }

                    if(i - 1 >= 0 && j - 1 >= 0) //Basso SX
                    {
                        switch(board[i - 1][j - 1])
                        {
                            case Player::piece::e:
                                modified = true;
                                first = board[i - 1][j - 1];
                                board[i][j] = Player::piece::e;
                                board[i - 1][j - 1] = Player::piece::O;
                                break;
                            case Player::piece::x:
                                if(i - 2 >= 0 && j - 2 >= 0)
                                {   
                                    modified = true;
                                    first = board[i - 1][j - 1];
                                    second = board[i - 1][j - 2]; 
                                    board[i][j] = Player::piece::e;
                                    board[i - 1][j - 1] = Player::piece::e;
                                    board[i - 2][j - 2] = Player::piece::O;
                                }
                                break;
                            case Player::piece::X:
                                if(i - 2 >= 0 && j - 2 >= 0)
                                {    
                                    modified = true;
                                    first = board[i - 1][j - 1];
                                    second = board[i - 1][j - 2]; 
                                    board[i][j] = Player::piece::e;
                                    board[i - 1][j - 1] = Player::piece::e;
                                    board[i - 2][j - 2] = Player::piece::O;
                                }
                                break;
                            default: // pezzo alleato
                                break;
                        }
                        
                        auto eval = minimax(board, depth - 1, 1, alpha, beta);
                        if(modified)
                        {
                            board[i][j] = Player::piece::O;
                            board[i - 1][j - 1] = first;
                            if(i - 2 >= 0 && j - 2 >= 0)
                                board[i - 2][j - 2] = second;

                            modified = false;
                        }
                        minEval = eval < minEval ? eval : minEval;
                        alpha = alpha < minEval ? alpha : minEval;
                        if(alpha >= beta)
                            break;
                        return minEval;
                    }

                    if(i - 1 >= 0 && j + 1 < 8) //Basso DX
                    {
                        switch(board[i - 1][j + 1])
                        {
                            case Player::piece::e:
                                modified = true;
                                first = board[i - 1][j + 1];
                                board[i][j] = Player::piece::e;
                                board[i - 1][j + 1] = Player::piece::O;
                                break;
                            case Player::piece::x:
                                if(i - 2 >= 0 && j + 2 <= 8)
                                {    
                                    modified = true;
                                    first = board[i - 1][j + 1];
                                    second = board[i - 2][j + 2];
                                    board[i][j] = Player::piece::e;
                                    board[i - 1][j + 1] = Player::piece::e;
                                    board[i - 2][j + 2] = Player::piece::O;
                                }
                                break;
                            case Player::piece::X:
                                if(i - 2 >= 0 && j + 2 <= 8)
                                {    
                                    modified = true;
                                    first = board[i - 1][j + 1];
                                    second = board[i - 2][j + 2];
                                    board[i][j] = Player::piece::e;
                                    board[i - 1][j + 1] = Player::piece::e;
                                    board[i - 2][j + 2] = Player::piece::O;
                                }
                                break;
                            default: // pezzo alleato
                                break;
                        }
                        
                        auto eval = minimax(board, depth - 1, 1, alpha, beta);
                        if(modified)
                        {
                            board[i][j] = Player::piece::O;
                            board[i - 1][j + 1] = first;
                            if(i - 2 >= 0 && j + 2 < 8)
                                board[i - 2][j + 2] = second;

                            modified = false;
                        }
                        minEval = eval < minEval ? eval : minEval;
                        alpha = alpha < minEval ? alpha : minEval;
                        if(alpha >= beta)
                            break;
                        return minEval;
                    }
                }
            }
        }
    }    
    
    return 0; //Suppress warning
}

void Player::move()
{
    //https://www.youtube.com/watch?v=l-hh51ncgDI
    //Spiegazione minimax 
    if(!this->pimpl->boardOffset)
        throw player_exception{player_exception::invalid_board, "Board history does not exist."};

    //Creo una board aggiuntiva dove minimax fa quello che vuole
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

    //Prova tutte le mosse -> Sceglie quella con minimax maggiore/minore in base al player e salva
    if(this->pimpl->player_nr == 1)
    {
        double value = -400000;
        std::pair<int, int> cellPosition; //row, column
        char direction; // 'Q' alto sx 'E' alto dx 'A' basso sx 'D' basso dx

        for(int i = 0; i < 8; ++i)
        {
            for(int j = 0; j < 8; ++j)
            {
                if(temporaryBoard[i][j] == Player::piece::x)
                {
                    //Alto SX
                    if(i + 1 < 8 && j - 1 >= 0)
                    {
                        //Pezzo vuoto
                        if(temporaryBoard[i + 1][j - 1] == Player::piece::e)
                        {
                            temporaryBoard[i][j] = Player::piece::e;
                            temporaryBoard[i + 1][j - 1] = i + 1 == 7 ? Player::piece::X : Player::piece::x;
                            double tempValue = minimax(temporaryBoard, 3, this->pimpl->player_nr, -400000, POS_INF);
                            if(tempValue > value)
                            {
                                value = tempValue;
                                cellPosition.first = i;
                                cellPosition.second = j;
                            }
                            //Undo della mossa
                        }
                    }
                }
                
                if(temporaryBoard[i][j] == Player::piece::X)
                {

                }
            }
        }
    }


    if(this->pimpl->player_nr == 2)
    {

    }
}

bool Player::valid_move() const
{
    //Controllo se la board è uguale a quella precedente   
    if(!this->pimpl->boardOffset || !this->pimpl->boardOffset->prev)
        throw player_exception{player_exception::index_out_of_bounds, "Less than two boards in history."};
    
    bool equalBoards = true;
    for(int i = 0; i < 8; ++i)
        for(int j = 0; j < 8; ++j)
            equalBoards = equalBoards && 
                (this->pimpl->boardOffset->board[i][j] == this->pimpl->boardOffset->prev->board[i][j]);
    
    if(equalBoards)
        return false;

    //Trovo dov'è il cambio
    size_t changed = 0;
    std::pair<int, int> positions[3] = {std::make_pair(-1, -1), std::make_pair(-1, -1), std::make_pair(-1, -1)};
    for(int i = 0; i < 8 && changed < 3; ++i)
        for(int j = 0; j < 8 && changed < 3; ++j)
            if(this->pimpl->boardOffset->board[i][j] != this->pimpl->boardOffset->prev->board[i][j])
                positions[changed++] = std::make_pair(i, j);

    if(changed > 3)
        return false;

    
 
    return true;
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

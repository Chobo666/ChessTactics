#include <string>
#include <vector>
#include <unordered_map>
#include <cmath>
#include "move.h"
#include "position.h"


/**
 * @brief uses isupper from C/C++ standard library, but returns bool instead of int.
 * 
 */
bool is_upper(char c){
    return (isupper(c) != 0);
}


/**
 * @brief checks if the given coordinates are valid (0 <= col, row < 8)
 * 
 */
bool are_valid_coords(int col, int row){
    return col >= 0 && col < 8 && row >= 0 && row < 8;
}


/**
 * @brief Converts column, row to internal square representation index
 * 
 * @throws const char* if the input is invalid
 */
int get_square(int col, int row){
    if(!are_valid_coords(col, row)){
        throw "invalid square coordinates";
    }
    return col + 8 * row;
}


/**
 * @brief Converts string from standard chess notation (e.g. "a5") to index representing the square in internal implementation of board.
 * 
 * @throws const char* if the input is invalid
 */
int get_square(std::string str){
    return get_square(tolower(str[0]) - 'a', (8 - (tolower(str[1]) - '0')));
}


// returns printable string representing the current board state
std::string Position::to_string(){
    std::string txt = "#  a b c d e f g h  #\n\n";
    for(int i = 0; i < 8; i++){
        txt += std::to_string(8-i) + "  ";
        for(int j = 0; j < 8; j++){
            char piece = m_board[get_square(j, i)];
            txt = txt + piece + " ";
        }
        txt += " " + std::to_string(8-i) + "\n";
    }
    txt += "\n#  a b c d e f g h  #\n";
    if(m_to_move == 'w'){
        txt += "white to move\n";
    } else {
        txt += "black to move\n";
    }
    return txt;
}


/**
 * @brief eturns 64-bit hash representing the board state (the hash changes if a move is played on the board).
 * A position instance thus cannot be used as a key in std::unordered_map. The hash is used in evaluation in lookup table.
 * 
 * Hash collisions may appear, but i think it should be too rare to handle.
 */
size_t Position::get_hash(){
    std::size_t h1 = std::hash<std::string>{}(m_board);
    std::size_t h2 = std::hash<int>{}(m_en_passant);
    std::size_t h3 = std::hash<char>{}(m_to_move);
    return h1 ^ (h2 << 1) ^ (h3 << 2);
}


/**
 * @brief Returns new Position represented by given FEN string (see https://en.wikipedia.org/wiki/Forsyth%E2%80%93Edwards_Notation)
 * FEN notation is commonly used across chess software making it possible to easily import the position to other program
 * 
 * @throws std::exception May throw stdlib C/C++ exceptions / SEGFAULT if the FEN is invalid
 */
Position::Position(std::string FEN){
    m_board[64] = '\0';
    m_prev_moves = std::vector<Move>();
    m_pieces = std::unordered_multimap<char, int>();
    int i = 0;
    int j = 0;
    while(i < 64){
        if(FEN[j] == '/'){
            j++;
        } else if('0' <= FEN[j] && FEN[j] <= '9'){
            for(int k = 0; k < FEN[j] - '0'; k++){
                m_board[i] = '.';
                i++;
            }
            j++;
        } else {
            m_board[i] = FEN[j];
            m_pieces.insert({FEN[j], i});
            i++;
            j++;
        }
    }
    j++;
    m_to_move = FEN[j];
    j+=2;
    while(FEN[j] != ' '){
        // skip castles (unused)
        j++;
    }
    j++;
    if(FEN[j] == '-'){
        m_en_passant = -1;
        j++;
    } else {
        m_en_passant = get_square(FEN.substr(j, 2));
        j += 2;
    }
    // skip halfmove count since last pawn move and capture as well as total move count (unused)
}


/**
 * FEN notation is commonly used across chess software making it possible to easily import the position to other program
 * (see https://en.wikipedia.org/wiki/Forsyth%E2%80%93Edwards_Notation)
 * 
 * @return std::string FEN representing the current position
 */
std::string Position::get_fen(){
    std::string result = std::string();
    // Board and pieces
    int empty_buffer = 0;
    for(int i = 0; i < 64; i++){
        if(m_board[i] == '.'){
            empty_buffer++;
        } else {
            if(empty_buffer > 0){
                result += std::to_string(empty_buffer);
                empty_buffer = 0;
            }
            result += m_board[i];
        }
        if(i % 8 == 7){
            if(empty_buffer > 0){
                result += std::to_string(empty_buffer);
                empty_buffer = 0;
            }
            if(i != 63){
                result += "/";
            }
        }
    }
    result += " ";
    // to move
    result += m_to_move;
    // castles (unsupported)
    result += " - ";
    // en-passant
    if(m_en_passant == -1){
        result += "-";
    } else {
        result += square_string(m_en_passant);
    }
    // half moves since last capture/pawn move, total moves (unsupported)
    result += " 0 1";

    return result;
}


/**
 * @brief Returns new Position representing starting position
 */
Position::Position() : Position("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"){}


/**
 * @brief Constructs a new Position object from lists of pieces (e.g. {"Re7", "Kf5"}, {"Kh8"}),
 * side to move and possibly en-passant square
 * 
 * @throws const char* if any of given squares is invalid
 */
Position::Position(std::vector<std::string> white_pieces, std::vector<std::string> black_pieces, char to_move, std::string en_passant_square){
    m_board[64] = '\0';
    m_prev_moves = std::vector<Move>();
    m_pieces = std::unordered_multimap<char, int>();
    for(int i = 0; i < 64; i++){
        m_board[i] = '.';
    }
    for(auto s : white_pieces){
        if(s.length() == 2){ //pawn
            int sq = get_square(s);
            m_pieces.insert({'P', sq});
            m_board[sq] = 'P';
        } else {
            int sq = get_square(s.substr(1, 2));
            m_pieces.insert({toupper(s[0]), sq});
            m_board[sq] = toupper(s[0]);
        }
    }
    for(auto s : black_pieces){
        if(s.length() == 2){ //pawn
            int sq = get_square(s);
            m_pieces.insert({'p', sq});
            m_board[sq] = 'p';
        } else {
            int sq = get_square(s.substr(1, 2));
            m_pieces.insert({tolower(s[0]), sq});
            m_board[sq] = tolower(s[0]);
        }
    }
    m_to_move = to_move;
    if(en_passant_square.length() == 2){
        m_en_passant = get_square(en_passant_square);
    } else {
        m_en_passant = -1;
    }
}


/**
 * @brief returns true if a given square is hit by given player
 */
bool Position::square_hit(int square, bool by_white){
    int col = square % 8;
    int row = square / 8;
    for(std::pair<int,int> cds : (std::pair<int,int>[]) {{1,2},{2,1},{-1,2},{-2,1},{-1,-2},{-2,-1},{1,-2},{2,-1}}){
        // Knight hits
        if(!are_valid_coords(col+cds.first, row+cds.second)){
            continue;
        }
        int sq = get_square(col+cds.first, row+cds.second);
        if(tolower(m_board[sq]) == 'n' && by_white == is_upper(m_board[sq])){
            return true;
        }
    }
    std::vector<std::pair<int,int>> pawn_dirs;
    if(by_white){
        pawn_dirs = {{1,1},{-1,1}};
    } else {
        pawn_dirs = {{1,-1},{-1,-1}};
    }
    for(std::pair<int,int> cds : pawn_dirs){
        // Pawn hits
        if(!are_valid_coords(col+cds.first, row+cds.second)){
            continue;
        }
        int sq = get_square(col+cds.first, row+cds.second);
        if(tolower(m_board[sq]) == 'p' && by_white == is_upper(m_board[sq])){
            return true;
        }
    }
    for(std::pair<int,int> cds : (std::pair<int,int>[]) {{1,-1},{-1,1},{1,1},{-1,-1},{1,0},{-1,0},{0,1},{0,-1}}){
        // King hits
        if(!are_valid_coords(col+cds.first, row+cds.second)){
            continue;
        }
        int sq = get_square(col+cds.first, row+cds.second);
        if(tolower(m_board[sq]) == 'k' && by_white == is_upper(m_board[sq])){
            return true;
        }
    }
    // sliding pieces
    for(std::pair<int,int> cds : (std::pair<int,int>[]) {{1,0},{-1,0},{0,1},{0,-1}}){
        // Rook (Queen)
        std::pair<int,int> curr = {col+cds.first, row+cds.second};
        while(are_valid_coords(curr.first, curr.second)){
            int sq = get_square(curr.first, curr.second);
            if(m_board[sq] != '.'){
                // a piece was found in the direction
                if(
                    by_white == is_upper(m_board[sq]) && // the piece is of the color we check
                    ((tolower(m_board[sq]) == 'r') || (tolower(m_board[sq]) == 'q'))
                ){
                    return true;
                } else {
                    break;
                }
            }
            curr.first += cds.first;
            curr.second += cds.second;
        }
    }
    for(std::pair<int,int> cds : (std::pair<int,int>[]) {{1,1},{-1,1},{1,-1},{-1,-1}}){
        // Bishop (Queen)
        std::pair<int,int> curr = {col+cds.first, row+cds.second};
        while(are_valid_coords(curr.first, curr.second)){
            int sq = get_square(curr.first, curr.second);
            if(m_board[sq] != '.'){
                // a piece was found in the direction
                if(
                    by_white == is_upper(m_board[sq]) && // the piece is of the color we check
                    ((tolower(m_board[sq]) == 'b') || (tolower(m_board[sq]) == 'q'))
                ){
                    return true;
                } else {
                    break;
                }
            }
            curr.first += cds.first;
            curr.second += cds.second;
        }
    }
    return false;
}


/**
 * @brief finds all legal moves in the current position for the player to move
 * 
 * @return std::vector<Move> possible moves
 */
std::vector<Move> Position::get_possible_moves(){
    auto pseudo_legal = std::vector<Move>();
    auto king = m_pieces.find(m_to_move == 'w' ? 'K' : 'k'); // Store pointer to king to validate moves
    for(auto piece : m_pieces){
        if(is_upper(piece.first) != (m_to_move == 'w')){
            // The piece belongs to player which is not on the move
            continue;
        }
        for(auto move : find_pseudo_legal_moves(piece.first, piece.second)){
            // get all pseudo-legal moves for the piece and add them to the list
            pseudo_legal.push_back(move);
        }
    }
    auto legal_moves = std::vector<Move>();
    for(auto move : pseudo_legal){
        perform_move(move);
        // do the move on the board
        if(!square_hit(king->second, !is_upper(king->first))){
            // look if plyer's king would be hit by opponent's piece
            legal_moves.push_back(move);
        }
        undo_move();
    }
    return legal_moves;
}


/**
 * @brief Get iterator to given piece at given square in m_pieces
 * 
 * @return std::unordered_multimap<char, int>::iterator iterator to the piece or m_pieces.end() if not found
 */
std::unordered_multimap<char, int>::iterator Position::get_piece(char piece, int square){
    auto range = m_pieces.equal_range(piece);
    for(auto ptr = range.first; ptr != range.second; ptr++){
        if(ptr->second == square){
            return ptr;
        }
    }
    return m_pieces.end();
}


/**
 * @brief Performs the move and updates all necessary states (e.g. updating m_board, m_pieces, pushing the move to m_prev_moves etc.)
 * 
 * Does not check for move validity. Can be reverted by Position::undo_move() (only for valid moves, otherwise no guarantees)
 * 
 * @param move should be from Position::get_possible_moves()
 * 
 * @throws const char* in some cases of invalid moves to prevent SEGFAULT
 */
void Position::perform_move(Move move){
    auto moved_piece = get_piece(move.m_piece, move.m_from);
    if(moved_piece == m_pieces.end()){
        throw "moving piece not found";
    }
    if(move.m_captured != '.'){
        // remove taken piece from the piece list
        auto taken_piece = get_piece(move.m_captured, move.m_to);
        if(taken_piece == m_pieces.end()){
            throw "taken piece not found";
        }
        m_pieces.erase(taken_piece);
    }
    moved_piece->second = move.m_to;
    m_board[move.m_from] = '.';
    m_board[move.m_to] = move.m_piece;
    m_prev_moves.push_back(move);
    if(move.m_special){
        if(is_upper(move.m_piece) != is_upper(move.m_special)){
            throw("invalid special side");
        }
        switch (move.m_special){
            case 'e': // black en-passant
                {
                auto taken = get_piece('P', move.m_to-8);
                if(taken == m_pieces.end()){
                    throw "couldn't find target of en passant";
                }
                m_pieces.erase(taken);
                m_board[move.m_to - 8] = '.';
                }
                break;
            case 'E': // white en-passant
                {
                auto taken = get_piece('p', move.m_to+8);
                if(taken == m_pieces.end()){
                    throw "couldn't find target of en passant";
                }
                m_pieces.erase(taken);
                m_board[move.m_to + 8] = '.';
                }
                break;
            default: // piece promotion
                m_pieces.erase(moved_piece);
                m_pieces.insert({move.m_special, move.m_to});
                m_board[move.m_to] = move.m_special;
        }
    }
    // Update m_en_passant
    if(tolower(move.m_piece) == 'p' && abs(move.m_from - move.m_to) == 16){
        m_en_passant = (move.m_to + move.m_from) / 2;
    } else {
        m_en_passant = -1;
    }
    // Swap m_to_move
    if(m_to_move == 'w'){
        m_to_move = 'b';
    } else {
        m_to_move = 'w';
    }
}


/**
 * @brief Un-does the last move from m_prev_moves and updates all necessary states (e.g. updating m_board, m_pieces, pushing the move to m_prev_moves etc.)
 * 
 * @throws const char* in some cases of invalid moves (corrupted states) to prevent SEGFAULT
 */
void Position::undo_move(){
    Move move = m_prev_moves.back();
    m_prev_moves.pop_back();
    auto moved_piece = get_piece(m_board[move.m_to], move.m_to);
    if(moved_piece == m_pieces.end()){
        throw "moving piece not found";
    }
    if(move.m_captured != '.'){
        // place back taken piece to the piece list
        m_pieces.insert({move.m_captured, move.m_to});
    }
    moved_piece->second = move.m_from;
    m_board[move.m_from] = move.m_piece;
    m_board[move.m_to] = move.m_captured;
    if(move.m_special){
        if(is_upper(move.m_piece) != is_upper(move.m_special)){
            throw("invalid special side");
        }
        switch (move.m_special){
            case 'e': // black en-passant
                m_pieces.insert({'P', move.m_to-8});
                m_board[move.m_to - 8] = 'P';
                break;
            case 'E': // white en-passant
                m_pieces.insert({'p', move.m_to+8});
                m_board[move.m_to + 8] = 'p';
                break;
            default: // piece promotion
                m_pieces.erase(moved_piece);
                m_pieces.insert({move.m_piece, move.m_from});
                m_board[move.m_from] = move.m_piece;
        }
    }
    // Swap m_to_move
    if(m_to_move == 'w'){
        m_to_move = 'b';
    } else {
        m_to_move = 'w';
    }
    // Update m_en_passant
    m_en_passant = move.m_last_enpassant;
}


/**
 * @brief generates all pseudo-legal moves for the piece at given square. Pseudo-legal moves are moves that follow piece movement,
 * but may be illegal due to player exposing his king to opponent's pieces 
 * 
 * @return std::vector<Move> pseudo-legal moves
 */
std::vector<Move> Position::find_pseudo_legal_moves(char piece, int square){
    auto result = std::vector<Move>();
    int col = square % 8;
    int row = square / 8;

    switch(tolower(piece)){ // switch based on piece type (not by color)
        case 'p': // pawn
            int dir;
            // setting direction of movement from color
            if(is_upper(piece)){
                dir = -1;
            } else {
                dir = 1;
            }
            for(int c_dir : {1, -1}){
                // diagonal movement - taking opponent's piece
                if(!are_valid_coords(col+c_dir, row+dir)){
                    continue;
                }
                int sq = get_square(col+c_dir, row+dir);
                if(m_board[sq] != '.' && is_upper(m_board[sq]) != is_upper(piece)){
                    // opponent's piece is there and can be taken
                    if(row+dir == 0){
                        // white promotion
                        for(char p : {'Q', 'R', 'N', 'B'}){
                            result.push_back(Move(square, sq, piece, m_board[sq], p, m_en_passant));
                        }
                    } else if(row+dir == 7){
                        // black promotion
                        for(char p : {'q', 'r', 'n', 'b'}){
                            result.push_back(Move(square, sq, piece, m_board[sq], p, m_en_passant));
                        }
                    } else {
                        // no promotion
                        result.push_back(Move(square, sq, piece, m_board[sq], 0, m_en_passant));
                    }
                } else if(sq == m_en_passant){
                    // en-passant
                    result.push_back(Move(square, sq, piece, m_board[sq], is_upper(piece) ? 'E' : 'e', m_en_passant));
                }
            }
            if((m_board[get_square(col, row+dir)] == '.')){
                // since unpromoted pawn cannot exist on first/last rank, the square is always valid
                // moving pawn one square forward
                int sq = get_square(col, row+dir);
                if(row+dir == 0){
                    // white promotion
                    for(char p : {'Q', 'R', 'N', 'B'}){
                        result.push_back(Move(square, sq, piece, '.', p, m_en_passant));
                    }
                } else if(row+dir == 7){
                    // black promotion
                    for(char p : {'q', 'r', 'n', 'b'}){
                        result.push_back(Move(square, sq, piece, '.', p, m_en_passant));
                    }
                } else {
                    result.push_back(Move(square, sq, piece, '.', 0, m_en_passant));
                }
            }
            if(
                ((is_upper(piece) && row == 6) || (!is_upper(piece) && row == 1)) && // is on 2rd rank if white or 7th rank if black
                m_board[get_square(col,row+dir)] == '.' && m_board[get_square(col, row+2*dir)] == '.' // both the squares in front of the pawn are empty
            ){
                // this move cannot be a promotion
                result.push_back(Move(square, get_square(col, row+2*dir), piece, '.', 0, m_en_passant));
            }
            break;
        case 'n': // knight
            for(std::pair<int,int> cds : (std::pair<int,int>[]) {{1,2},{2,1},{-1,2},{-2,1},{-1,-2},{-2,-1},{1,-2},{2,-1}}){
                if(!are_valid_coords(col+cds.first, row+cds.second)){
                    continue;
                }
                int sq = get_square(col+cds.first, row+cds.second);
                if(m_board[sq] == '.' || is_upper(piece) != is_upper(m_board[sq])){
                    // the square is empty or contains opponent's piece
                    result.push_back(Move(square, get_square(col+cds.first, row+cds.second), piece, m_board[sq], 0, m_en_passant));
                }
            }
            break;
        case 'k': // king (castles are not implemented)
            for(std::pair<int,int> cds : (std::pair<int,int>[]) {{1,-1},{-1,1},{1,1},{-1,-1},{1,0},{-1,0},{0,1},{0,-1}}){
                if(!are_valid_coords(col+cds.first, row+cds.second)){
                    continue;
                }
                int sq = get_square(col+cds.first, row+cds.second);
                if(m_board[sq] == '.' || is_upper(piece) != is_upper(m_board[sq])){
                    // the square is empty or contains opponent's piece
                    result.push_back(Move(square, get_square(col+cds.first, row+cds.second), piece, m_board[sq], 0, m_en_passant));
                }
            }
            break;
        case 'q': // queen
            for(std::pair<int,int> cds : (std::pair<int,int>[]) {{1,-1},{-1,1},{1,1},{-1,-1},{1,0},{-1,0},{0,1},{0,-1}}){
                std::pair<int,int> curr = {col+cds.first, row+cds.second};
                while(are_valid_coords(curr.first, curr.second)){
                    int sq = get_square(curr.first, curr.second);
                    if(m_board[sq] != '.' && is_upper(piece) == is_upper(m_board[sq])){
                        // our piece, we cannot move to this square nor any further
                        break;
                    }
                    // if the square is empty or contain opponent's piece, the move is possible
                    result.push_back(Move(square, get_square(curr.first, curr.second), piece, m_board[sq], 0, m_en_passant));
                    if(m_board[sq] != '.'){
                        // any piece, we cannot move any further 
                        break;
                    }
                    curr.first += cds.first;
                    curr.second += cds.second;
                }
            }
            break;
        case 'b': // bishop
            for(std::pair<int,int> cds : (std::pair<int,int>[]) {{1,-1},{-1,1},{1,1},{-1,-1}}){
                std::pair<int,int> curr = {col+cds.first, row+cds.second};
                while(are_valid_coords(curr.first, curr.second)){
                    int sq = get_square(curr.first, curr.second);
                    if(m_board[sq] != '.' && is_upper(piece) == is_upper(m_board[sq])){
                        // our piece, we cannot move to this square nor any further
                        break;
                    }
                    // if the square is empty or contain opponent's piece, the move is possible
                    result.push_back(Move(square, get_square(curr.first, curr.second), piece, m_board[sq], 0, m_en_passant));
                    if(m_board[sq] != '.'){
                        // any piece, we cannot move any further 
                        break;
                    }
                    curr.first += cds.first;
                    curr.second += cds.second;
                }
            }
            break;
        case 'r': //rook
            for(std::pair<int,int> cds : (std::pair<int,int>[]) {{1,0},{-1,0},{0,1},{0,-1}}){
                std::pair<int,int> curr = {col+cds.first, row+cds.second};
                while(are_valid_coords(curr.first, curr.second)){
                    int sq = get_square(curr.first, curr.second);
                    if(m_board[sq] != '.' && is_upper(piece) == is_upper(m_board[sq])){
                        // our piece, we cannot move to this square nor any further
                        break;
                    }
                    // if the square is empty or contain opponent's piece, the move is possible
                    result.push_back(Move(square, get_square(curr.first, curr.second), piece, m_board[sq], 0, m_en_passant));
                    if(m_board[sq] != '.'){
                        // any piece, we cannot move any further 
                        break;
                    }
                    curr.first += cds.first;
                    curr.second += cds.second;
                }
            }
            break;
    }
    return result;
}
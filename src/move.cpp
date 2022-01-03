#include <string>
#include "move.h"

/**
 * @brief converts square index from inner representation of board to classic notation
 * 
 * @param square int 0-63 representing index of square (to see the specifications, go to position.cpp Position.m_board)
 * @return std::string square representation in classic notation (a1 - h8)
 */
std::string square_string(int square){
    char col = (char)(square%8)+'a';
    char row = (char)(8-square/8)+'0';
    return std::string({col, row});
}


// constructor
Move::Move(int from, int to, char piece, char captured, char special, int last_enpassant){
    m_from = from;
    m_to = to;
    m_piece = piece;
    m_captured = captured;
    m_special = special;
    m_last_enpassant = last_enpassant;
}


/**
 * @return the classic representation of the move as text using english caption of the pieces (R,N,B,Q,K) and no caption for pawn moves
 * 
 * (e.g. Rxc6, e7, dxe6, g8=D)
 * 
 * Does not resolve move collisions (e.g. that there are two moves Ne2 possible, as Ng1-e2 and Nc3-e2 in long classical notation) 
 */
std::string Move::to_string(){
    
    if(tolower(m_piece) == 'p'){
        // pawns do not have caption, are a special case
        std::string result = "";
        if(m_captured != '.' || tolower(m_special) == 'e'){
            // if the move is a capture, it is necessary to add file from which the pawn moved
            result += std::string({square_string(m_from)[0], 'x'});
        }
        result += std::string(square_string(m_to));
        if(m_special && tolower(m_special) != 'e'){
            // if the move is a promotion, add chosen promotion piece
            result += std::string({'=', m_special});
        }
        return result;
    }
    if(m_captured != '.'){
        return std::string({(char)toupper(m_piece), 'x', square_string(m_to)[0], square_string(m_to)[1]});
    }
    return std::string({(char)toupper(m_piece), square_string(m_to)[0], square_string(m_to)[1]});
}


/**
 * @return the classic long (full) representation of the move as text using english caption of the pieces (R,N,B,Q,K) and no caption for pawn moves
 * 
 * (e.g. Rc2xc6, e6-e7, d5xe6, g7-g8=D)
 */
std::string Move::to_full_string(){
    if(tolower(m_piece) == 'p'){
        // pawns do not have caption, are a special case
        std::string result = "";
        result += std::string(square_string(m_from));
        if(m_captured != '.'){
            result += "x";
        } else {
            result += "-";
        }
        result += std::string(square_string(m_to));
        if(m_special && tolower(m_special) != 'e'){
            // if the move is a promotion, add chosen promotion piece
            result += std::string({'=', m_special});
        }
        return result;
    }
    if(m_captured != '.'){
        return std::string({(char)toupper(m_piece), square_string(m_from)[0], square_string(m_from)[1], 'x', square_string(m_to)[0], square_string(m_to)[1]});
    }
    return std::string({(char)toupper(m_piece), square_string(m_from)[0], square_string(m_from)[1], '-', square_string(m_to)[0], square_string(m_to)[1]});
}
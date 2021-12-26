#pragma once

#include <string>

std::string square_string(int square);

/**
 * @brief data class for representing moves
 * 
 * Implements storing all needed information as well as methods to convert the move to standard text representation
 * using english caption of the pieces (R,N,B,Q,K) and no caption for pawn moves
 * 
 */
class Move{

    public:

        // square from which the piece moved
        int m_from;

        // square to where the piece moved
        int m_to;

        // moving piece
        char m_piece;

        // piece that was previously on m_to (or '.' if the square was empty). Note that for en-passant the value is '.' even though a pawn was captured
        char m_captured;

        /**
         * @brief stores all additional infformation for special pawn moves (en-passant and promotion)
         * 
         * For most cases the value is (char) 0.
         * 
         * If the move is a promotion, the value is the target of promotion ('Q','R','B','N','q','r','b','n')
         * 
         * If the move is en-passant, the value is 'E' for white and 'e' for black
         */
        char m_special;

        // en-passant from previous board state (used for correctly undoing moves)
        int m_last_enpassant;

        // constructor
        Move(int from, int to, char piece, char captured, char special = (char)0, int last_enpassant=-1);
        

        /**
         * @return the classic representation of the move as text using english caption of the pieces (R,N,B,Q,K) and no caption for pawn moves
         * 
         * (e.g. Rxc6, e7, dxe6, g8=D)
         * 
         * Does not resolve move collisions (e.g. that there are two moves Ne2 possible, as Ng1-e2 and Nc3-e2 in long classical notation) 
         */
        std::string to_string();


        /**
         * @return the classic long (full) representation of the move as text using english caption of the pieces (R,N,B,Q,K) and no caption for pawn moves
         * 
         * (e.g. Rc2xc6, e6-e7, d5xe6, g7-g8=D)
         */
        std::string to_full_string();
        
};

#include "move.cpp"


#pragma once

#include "move.h"
#include <string>
#include <vector>
#include <unordered_map>

/**
 * @brief uses isupper from C/C++ standard library, but returns bool instead of int.
 * 
 */
bool is_upper(char c);

/**
 * @brief checks if the given coordinates are valid (0 <= col, row < 8)
 * 
 */
bool are_valid_coords(int col, int row);

/**
 * @brief Converts column, row to internal square representation index
 * 
 * @throws const char* if the input is invalid
 */
int get_square(int col, int row);

/**
 * @brief Converts string from standard chess notation (e.g. "a5") to index representing the square in internal implementation of board.
 * 
 * @throws const char* if the input is invalid
 */
int get_square(std::string str);

/**
 * @brief Represents a board state.
 * Does not support castles.
 */
class Position{

    public:
        /*
        Stores board information for lookup square -> piece. Is terminated by 'null character' as it is used as std::string for hashing

        index:
        00 01 02 03 04 05 06 07
        08 09 10 11 12 13 14 15
        16 17 18 19 20 21 22 23
        24 25 26 27 28 29 30 31
        32 33 34 35 36 37 38 39
        40 41 42 43 44 45 46 47
        48 49 50 51 52 53 54 55
        56 57 58 59 60 61 62 63

        classic:
        a8 b8 c8 d8 e8 f8 g8 h8
        a7 b7 c7 d7 e7 f7 g7 h7
        a6 b6 c6 d6 e6 f6 g6 h6
        a5 b5 c5 d5 e5 f5 g5 h5
        a4 b4 c4 d4 e4 f4 g4 h4
        a3 b3 c3 d3 e3 f3 g3 h3
        a2 b2 c2 d2 e2 f2 g2 h2
        a1 b1 c1 d1 e1 f1 g1 h1

        row, col = index / 8, index % 8:
        00 10 20 30 40 50 60 70
        01 11 21 31 41 51 61 71
        02 12 22 32 42 52 62 72
        03 13 23 33 43 53 63 73
        04 14 24 34 44 54 64 74
        05 15 25 35 45 55 65 75
        06 16 26 36 46 56 66 76
        07 17 27 37 47 57 67 77 
        */
        char m_board[65];

        // 'w' or 'b'
        char m_to_move;

        // square available for en-passant or -1 if such square does not exist
        int m_en_passant;

        // stores board information for lookup piece -> square. equal_range(char piece) contains all the squares which contain given piece
        std::unordered_multimap<char, int> m_pieces;

        // stores all previous moves, that were played on the board. Last played move is m_prev_moves.back(). Is used to un-do moves correctly.
        std::vector<Move> m_prev_moves;

        // returns printable string representing the current board state
        std::string to_string();


        /**
         * @brief eturns 64-bit hash representing the board state (the hash changes if a move is played on the board).
         * A position instance thus cannot be used as a key in std::unordered_map. The hash is used in evaluation in lookup table.
         * 
         * Hash collisions may appear, but i think it should be too rare to handle.
         */
        size_t get_hash();


        /**
         * @brief Returns new Position represented by given FEN string (see https://en.wikipedia.org/wiki/Forsyth%E2%80%93Edwards_Notation)
         * FEN notation is commonly used across chess software making it possible to easily import the position to other program
         * 
         * @throws std::exception May throw stdlib C/C++ exceptions / SEGFAULT if the FEN is invalid
         */
        Position(std::string FEN);


        /**
         * FEN notation is commonly used across chess software making it possible to easily import the position to other program
         * (see https://en.wikipedia.org/wiki/Forsyth%E2%80%93Edwards_Notation)
         * 
         * @return std::string FEN representing the current position
         */
        std::string get_fen();


        /**
         * @brief Returns new Position representing starting position
         */
        Position();


        /**
         * @brief Constructs a new Position object from lists of pieces (e.g. {"Re7", "Kf5"}, {"Kh8"}),
         * side to move and possibly en-passant square
         * 
         * @throws const char* if any of given squares is invalid
         */
        Position(std::vector<std::string> white_pieces, std::vector<std::string> black_pieces, char to_move, std::string en_passant_square="");


        /**
         * @brief returns true if a given square is hit by given player
         */
        bool square_hit(int square, bool by_white);


        /**
         * @brief finds all legal moves in the current position for the player to move
         * 
         * @return std::vector<Move> possible moves
         */
        std::vector<Move> get_possible_moves();


        /**
         * @brief Get iterator to given piece at given square in m_pieces
         * 
         * @return std::unordered_multimap<char, int>::iterator iterator to the piece or m_pieces.end() if not found
         */
        std::unordered_multimap<char, int>::iterator get_piece(char piece, int square);


        /**
         * @brief Performs the move and updates all necessary states (e.g. updating m_board, m_pieces, pushing the move to m_prev_moves etc.)
         * 
         * Does not check for move validity. Can be reverted by Position::undo_move() (only for valid moves, otherwise no guarantees)
         * 
         * @param move should be from Position::get_possible_moves()
         * 
         * @throws const char* in some cases of invalid moves to prevent SEGFAULT
         */
        void perform_move(Move move);


        /**
         * @brief Un-does the last move from m_prev_moves and updates all necessary states (e.g. updating m_board, m_pieces, pushing the move to m_prev_moves etc.)
         * 
         * @throws const char* in some cases of invalid moves (corrupted states) to prevent SEGFAULT
         */
        void undo_move();


    private:

        /**
         * @brief generates all pseudo-legal moves for the piece at given square. Pseudo-legal moves are moves that follow piece movement,
         * but may be illegal due to player exposing his king to opponent's pieces 
         * 
         * @return std::vector<Move> pseudo-legal moves
         */
        std::vector<Move> find_pseudo_legal_moves(char piece, int square);

};

#include "position.cpp"
#pragma once

#include "position.h"
#include <map>
#include <algorithm>
#include <iostream>

#define Cache std::unordered_map<size_t, std::pair<int, int>>

namespace Engine{

    // Piece values used in evaluation
    std::unordered_map<char, int> piece_values = {
        {'K', 1000},
        {'Q', 9},
        {'R', 5},
        {'N', 3},
        {'B', 3},
        {'P', 1},
        {'k', -1000},
        {'q', -9},
        {'r', -5},
        {'n', -3},
        {'b', -3},
        {'p', -1},
    };

    // Evaluation value for mate in 0
    int MATE = 1000000;

    // Minimal evaluation value which is considered as mate (should be more than sum of piece values)
    int MATE_THRESHOLD = 2000;

    // Smallest default depth used in calculations. Changing this value will have huge performance impact
    int MIN_DEPTH = 2;

    // Highest default depth used in calculations. Changing this value will allow generation of harder puzzles, but thta will impact performance
    int MAX_DEPTH = 5;


    /**
     * @brief Worsen eval by 1 every turn so that engine chooses fastest mate
     * 
     */
    int process_eval(int num);

    /**
     * @brief Get the evaluation guess, used for ordering search in alfa/beta search
     * 
     * @return int previous evaluation of the position of longest depth or 0 if there is no information in the cache
     */
    int get_eval_guess(size_t hash, Cache* cache);


    /**
     * @brief Comparator used to sort std::vector<std::pair<int, Move>> in descending order
     * 
     */
    bool sort_moves(const std::pair<int,Move> a, std::pair<int,Move> b);

    /**
     * @brief Searches the position for all possible continuations
     * @return (MATE - halfmoves_to_mate) if +-, -(MATE - halfmoves_to_mate) if -+, material count otherwise
     * 
     * Implementation: as DFS due to nature of Position. BFS would need to copy the positions. 
     * 
     * @param maxdepth maximal depth in halfmoves to search the position
     * 
     * @param cache previous evaluations stored in hashmap. The elements are stored ad std::pair<int, int>.
     * Where element.first is evaluation depth and element.second is the evaluation
     */
    int evaluate(Position* position, int maxdepth, Cache* cache, int alfa=-MATE, int beta=MATE);

    /**
     * @brief Evaluate the position iteratively, gradually increasing the depth of search. Due to the nature of search,
     * as we can use the evaluation from previous iteration to guess the order of search, it usually tends to be
     * faster than direct aprroach
     */
    int iter_evaluate(Position* position, int maxdepth, Cache* cache);

    /**
     * @brief search for the fastest mate.
     *  
     * @return std::string representing the evaluation (e.g. "White mates in 3" or "Unknown result")
     */
    std::string find_fastest_mate(Position* position, int max_moves, Cache* cache);

    /**
     * @brief plays a move with the best evaluation with specified depth.
     * If there are more moves with the best evaluation, choose one at random
     * 
     * Uses std::random_shuffle, thus can be seeded by srand()
     * 
     * If the position is mate or stalemate, return without perfoming any changes to the position
     * 
     * Throws const char* if internal logic error is encountered
     */
    void play_random_best(Position* position, int max_depth, Cache* cache);

    /**
     * @brief generates a puzzle by letting the engine play itself.
     * 
     * @param max_moves max moves complexity of the puzzle to be generated (usually the puzzles are 2 or 3 moves long at max, exceptionally 4). 
     * This is due to setting the Engine::MAX_DEPTH to 5. Changing this setting may yield harder puzzles, but exponential performance change.
     * 
     * @param verbose if true, the process reports the current state of generation into std::cout
     * @param seed value used to generate the puzzles. Same seeds will return same puzzles.
     * If there is any seed given, the cache gets reseted (cleared) before generating the puzzle to ensure deterministic result
     * 
     * @return Position the puzzle
     */
    Position generate_puzzle_by_playing(Cache* cache, int max_moves, bool verbose=true, std::string seed = "");

    /**
     * @brief return true if the given move is the best move in the position (there can be more best moves)
     * 
     */
    bool is_solution(Position* puzzle, Move move, Cache* cache);

}

#include "engine.cpp"

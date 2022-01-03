#include <map>
#include <algorithm>
#include <iostream>
#include "position.h"
#include "engine.h"

#define Cache std::unordered_map<size_t, std::pair<int, int>>

namespace Engine{

    /**
     * @brief Worsen eval by 1 every turn so that engine chooses fastest mate
     * 
     */
    int process_eval(int num){
        if(abs(num) < MATE_THRESHOLD){
            // No mate is coming, eval is piece count
            return num;
        } else if(num >= MATE_THRESHOLD){
            // Worsen eval by 1 every turn so that engine chooses fastest mate
            return num - 1;
        } else {
            return num + 1;
        }

    }

    /**
     * @brief Get the evaluation guess, used for ordering search in alfa/beta search
     * 
     * @return int previous evaluation of the position of longest depth or 0 if there is no information in the cache
     */
    int get_eval_guess(size_t hash, Cache* cache){
        auto cached_result = cache->find(hash);
        if(cached_result != cache->end()){
            return cached_result->second.second;
        } else {
            return 0;
        }
    }


    /**
     * @brief Comparator used to sort std::vector<std::pair<int, Move>> in descending order
     * 
     */
    bool sort_moves(const std::pair<int,Move> a, std::pair<int,Move> b){
        return (a.first > b.first);
    }

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
    int evaluate(Position* position, int maxdepth, Cache* cache, int alfa, int beta){
        size_t hash = position->get_hash();
        // Look if the position has been already evaluated
        auto cached_result = cache->find(hash);
        if(cached_result != cache->end() && cached_result->second.first >= maxdepth){
            // If the depth of evaluation is sufficient, return the stored value
            return cached_result->second.second;
        }

        auto possible_moves = position->get_possible_moves();
        int side = position->m_to_move == 'w' ? 1 : -1;
        if(possible_moves.size() == 0){
            // The position is either a mate or stalemate
            auto king = position->m_pieces.find(position->m_to_move == 'w' ? 'K' : 'k');
            if(position->square_hit(king->second, position->m_to_move != 'w')){
                //mate
                cache->insert({hash, {__INT_MAX__, -side * MATE}});
                //for(auto m : position->m_prev_moves){std::cout << m.print() << " ";}; std::cout << "mate" << std::endl;
                return -side * MATE;
            } else { //stalemate
                cache->insert({hash, {__INT_MAX__, 0}});
                //for(auto m : position->m_prev_moves){std::cout << m.print() << " ";}; std::cout << "stalemate" << std::endl;
                return 0;
            }
        } else if(position->m_pieces.size() <= 2){
            // only kings on board (to be precise, there should be also a case (K+N vs k) and (K+B vs k))
            cache->insert({hash, {__INT_MAX__, 0}});
            return 0;
        }
        if(maxdepth <= 0){
            // No deeper evaluation, count the material
            int result = 0;
            for (auto p : position->m_pieces){
                result += piece_values.find(p.first)->second;
            }
            if(cached_result != cache->end()){
                cached_result->second = {maxdepth, result};
            } else {
                cache->insert({hash, {maxdepth, result}});
            }
            return result;
        }
        int eval = -MATE;

        // Search order is important in alfa/beta pruned search. Guess the order by previous evaluation
        auto ordered_moves = std::vector<std::pair<int, Move>>();
        for(auto m : possible_moves){
            position->perform_move(m);
            int guess = get_eval_guess(position->get_hash(), cache);
            position->undo_move();
            ordered_moves.push_back({guess, m});
        }
        std::sort(ordered_moves.begin(), ordered_moves.end(), sort_moves);

        // Recursively evaluate positions with lower depth
        for(auto pair : ordered_moves){
            // try a move, evaluate and redo
            position->perform_move(pair.second);
            int new_eval = evaluate(position, maxdepth-1, cache, -beta, -alfa) * side;
            if(new_eval > eval){
                eval = new_eval;
                if(eval > alfa){
                    alfa = eval;
                }
            }
            position->undo_move();
            if(process_eval(eval) >= beta){
                // alfa-beta cutoff, we didn't investigate full position => no caching
                return process_eval(eval) * side; 
            }
        }
        // save result into cache
        if(cached_result != cache->end()){
            cached_result->second = {abs(eval) > MATE_THRESHOLD ? __INT_MAX__ : maxdepth, process_eval(eval) * side};
        } else {
            cache->insert({hash, {abs(eval) > MATE_THRESHOLD ? __INT_MAX__: maxdepth, process_eval(eval) * side}});
        }
        return process_eval(eval) * side;
    }

    /**
     * @brief Evaluate the position iteratively, gradually increasing the depth of search. Due to the nature of search,
     * as we can use the evaluation from previous iteration to guess the order of search, it usually tends to be
     * faster than direct aprroach
     */
    int iter_evaluate(Position* position, int maxdepth, Cache* cache){
        for(int depth = 1; depth <= maxdepth; depth++){
            evaluate(position, depth, cache);
        }
        return evaluate(position, maxdepth, cache);
    }

    /**
     * @brief search for the fastest mate.
     *  
     * @return std::string representing the evaluation (e.g. "White mates in 3" or "Unknown result")
     */
    std::string find_fastest_mate(Position* position, int max_moves, Cache* cache){
        for(int depth = 0; depth < max_moves; depth++){
            int eval = evaluate(position, 2*depth, cache);
            if(abs(eval) > MATE_THRESHOLD){
                auto s = eval > 0 ? std::string("White ") : std::string("Black ");
                s += "mates in ";
                s += std::to_string((MATE - abs(eval) + 1)/2);
                return s;
            }
        }
        return "Unknown result";
    }

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
    void play_random_best(Position* position, int max_depth, Cache* cache){;
        auto moves = position->get_possible_moves();
        std::random_shuffle(moves.begin(), moves.end());

        if(moves.size() == 0){
            //cannot move any further
            return;
        }

        // Evaluation may have went deeper and changed, reevaluate the position to make sure the evaluation is actual
        cache->erase(position->get_hash());

        int target = iter_evaluate(position, max_depth, cache);
        for(auto m : moves){
            position->perform_move(m);
            if(process_eval(iter_evaluate(position, max_depth-1, cache)) == target){
                return;
            }
            position->undo_move();
        }
        
        // Assert (unreachable) - this shouldn't ever happen
        throw "Could't play random best, internal logic error";
    }

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
    Position generate_puzzle_by_playing(Cache* cache, int max_moves, bool verbose, std::string seed){
        if(seed.length() > 0){
            // To get deterministic result from seed we need to clear the cache
            // The program will (in some cases) need the cache after generating the puzzle to solve it
            cache->clear();
            srand(std::hash<std::string>{}(seed));
        }   
        if(verbose){
            std::cout << "Generating puzzle...";
        }
        Position pos = Position();
        while(abs(evaluate(&pos, MIN_DEPTH, cache)) < MATE_THRESHOLD){
            if(pos.m_prev_moves.size() > 150 || pos.get_possible_moves().size() == 0){
                // the enigne was sometimes getting stuck inside positions (K+R vs K), which didn't lead to puzzles
                // or in stalemate, which cannot be played any longer, but doesn't yield a puzzle
                pos = Position();
            }
            play_random_best(&pos, MIN_DEPTH, cache);
            if(verbose){
                std::cout << "#" << std::flush;
            }
        }
        if(verbose){
            std::cout << "...done!" << std::endl << "Reinforcing the puzzle...";
        }
        // save the information of longest puzzle, to know where to return
        int longest_mate = 0;

        // save the undone moves in case of need to re-do some of them to return to the position with longest puzzle
        auto undone_moves = std::vector<Move>();

        for(int depth = 2; abs(iter_evaluate(&pos, depth, cache)) > MATE_THRESHOLD;){
            int moves_to_mate = (MATE - abs(evaluate(&pos, MIN_DEPTH, cache)) + 1)/2;
            if(moves_to_mate > longest_mate){
                longest_mate = moves_to_mate;
            }
            if(moves_to_mate == max_moves){
                // We reached the required max moves, no need for further search
                break;
            }
            undone_moves.push_back(pos.m_prev_moves.back());
            pos.undo_move();
            if(depth < MAX_DEPTH){
                depth++;
            }
            if(verbose){
                std::cout << "#" << std::flush;
            }
        }
        if(abs(evaluate(&pos, MIN_DEPTH, cache)) < MATE_THRESHOLD){
            // prev evaluation of any depth did not end as forced mate -> we have undone too many moves
            pos.perform_move(undone_moves.back());
            undone_moves.pop_back();
        }

        // at this point, the position should lead to forced mate.

        // mate in max_moves or longest mate whichever is lower
        int target = max_moves < longest_mate ? max_moves : longest_mate;
        
        while((MATE - abs(evaluate(&pos, MIN_DEPTH, cache)) + 1) / 2 < target){
            // redo undone moves until we reach position with target moves to mate
            pos.perform_move(undone_moves.back());
            undone_moves.pop_back();
        }

        // at this point, the position should be longest found mate or of requested moves
        if(abs(evaluate(&pos, MIN_DEPTH, cache)) % 2 == 0){
            // losing side is on move, play best move (not shortening the puzzle)
            play_random_best(&pos, 2, cache);
        }
        if(verbose){
            std::cout << "...done!" << std::endl;
        }
        return pos;
    }

    /**
     * @brief return true if the given move is the best move in the position (there can be more best moves)
     * 
     */
    bool is_solution(Position* puzzle, Move move, Cache* cache){
        int eval = evaluate(puzzle, MIN_DEPTH, cache);
        puzzle->perform_move(move);
        if (process_eval(iter_evaluate(puzzle, MATE - abs(eval) - 1, cache)) == eval){
            puzzle->undo_move();
            return true;
        } else {
            puzzle->undo_move();
            return false;
        }
    }

}
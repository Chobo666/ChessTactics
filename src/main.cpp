#include "engine.h"
#include <iostream>
#include <string>

std::string STARTUP_MSG = 
"Welcome to the Chess puzzle generator! An iteractive chess puzzle tool.\n"
"Instead of trying to solve the puzzle from the console view, feel free to copy paste\n"
"the puzzle FEN to any application that can show you the board better\n"
"(for example http://www.ee.unb.ca/cgi-bin/tervo/fen.pl). While solving, please enter the\n"
"moves in standard Long algebraic notation (e.g. Ra1-d1, Re7xe8, e2-e4, d7xe8=D)\n";

int get_number_of_moves_from_user();
Move get_move_from_user(std::vector<Move> possible_moves);

int main(){

    // Provide basic infromation and get parameters from user

    std::cout << STARTUP_MSG << std::endl;
    std::string seed;
    std::cout << "Enter seed: ";
    std::getline(std::cin, seed);
    std::cout << "Seed is: " << seed << std::endl;

    int max_moves = get_number_of_moves_from_user();

    // generate puzzles and let user solve them interactively

    auto cache = Cache();
    for(int puzzle_number = 0;; puzzle_number++){

        Position puzzle = Engine::generate_puzzle_by_playing(&cache, max_moves, true, seed + "_" + std::to_string(puzzle_number));
        std::cout << std::endl;
        std::cout << "puzzle No. " << puzzle_number << "  with seed: " <<  seed + "_" + std::to_string(puzzle_number) << std::endl;
        std::cout << "FEN: " << puzzle.get_fen() << std::endl;

        // How many times user can be wrong in each puzzle before we show them a solution
        int corrections_left = 3;

        while (abs(Engine::evaluate(&puzzle, Engine::MAX_DEPTH, &cache)) != Engine::MATE){
            // Until the puzzle is solved (to mate)

            std::cout << puzzle.to_string() << std::endl;
            std::cout << Engine::find_fastest_mate(&puzzle, Engine::MAX_DEPTH, &cache) << std::endl;

            auto possible_moves = puzzle.get_possible_moves();

            // User has to enter next move of their solution
            Move selected_move = get_move_from_user(possible_moves);

            if(Engine::is_solution(&puzzle, selected_move, &cache)){
                // User's chosen move leads to fastest mate
                std::cout << "Correct! " << std::endl;
                puzzle.perform_move(selected_move);

                if(abs(Engine::evaluate(&puzzle, Engine::MAX_DEPTH, &cache)) != Engine::MATE){
                    // If the puzzle has a continuation, play move for defending side
                    Engine::play_random_best(&puzzle, Engine::MAX_DEPTH, &cache);
                    std::cout << "Opponent played: " << puzzle.m_prev_moves.back().to_full_string() << std::endl;
                }
            } else {
                // User's chosen move doesn't lead to fastest mate
                if(corrections_left > 0){
                    // We won't show user the solution, until he has some corrections left
                    std::cout << "Wrong! Try again. " << --corrections_left  << " corrections left" << std::endl;
                } else {
                    // Show the user next solution move
                    Engine::play_random_best(&puzzle, Engine::MAX_DEPTH, &cache);
                    std::cout << "The solution was: " << puzzle.m_prev_moves.back().to_full_string() << std::endl;

                    if(abs(Engine::evaluate(&puzzle, Engine::MAX_DEPTH, &cache)) != Engine::MATE){
                        // If the puzzle has a continuation, play move for defending side
                        Engine::play_random_best(&puzzle, Engine::MAX_DEPTH, &cache);
                        std::cout << "Opponent played: " << puzzle.m_prev_moves.back().to_full_string() << std::endl;
                    }
                }
            }
        }
    }
}

int get_number_of_moves_from_user(){
    std::string input; // temporary buffer to store input
    int max_moves = -1;
    while(max_moves == -1){
        try{
            std::cout << "Enter max moves for puzzles: ";
            std::getline(std::cin, input);
            max_moves = std::stoi(input);
            if(max_moves <= 0){
                max_moves = -1;
                throw std::invalid_argument("Negative input");
            }
        } catch (std::invalid_argument& ex){
            std::cout << "Enter positive integer" << std::endl;
        } catch (std::out_of_range& ex){
            std::cout << "Enter a reasonable number" << std::endl;
        }
    }
    return max_moves;
}

Move get_move_from_user(std::vector<Move> possible_moves){
    std::string input; // temporary buffer to store input
    while(true){
        std::cout << "Enter next move of Your solution: ";
        std::getline(std::cin, input);
        for(auto m : possible_moves){
            if(m.to_full_string() == input){
                return m;
            }
        }
        // remind user of what moves are possible and what is the format
        std::cout << "Invalid input, possible moves are:" << std::endl;
        for(auto m : possible_moves){
            std::cout << m.to_full_string() << " ";
        }
        std::cout << std::endl;
    }
}
#include "types.hpp"

#include <iostream>
#include <iomanip>
#include <string>
#include <bitset>

#include <cassert>
#include <chrono>

#include "state.hpp"
#include "utils.hpp"
#include "test_state.hpp"
#include "interrupt_handler.hpp"

// 2048 lite
/******************/
/* IN THIS MODEL: */
/******************/
// statespace is a grid of 2x3 of integers 0 and 1 to 5 representing 0 and 2^1 to 2^5

// action is either
// Up: swipe up ie "compact" blocks upward until impact on top edge while prioritising top 2 blocks in case of 3 similar blocks
// Down: same but down
// Left: same but right
// Right: same but left
// Note: action is illegal if no "movement" or "fusion" occurs

// Offhand idea 1: maybe illegal moves are actually legal but give a penalty and use a finite turn ?
// Or just uses a finite turn, but wouldn't that be bad for optimisation with duplicate states...
// Conclusion: I don't think this is an actual concern, just make the move do nothing.

// Complications: if action is illegal probability of transition is 0? - note: this seems irrelevant
// Solution: instead of enumerating all unlikely scenarios and then noticing their transition is 0, enumerate "neighbour" situations using
// a function that enumerates all empty squares after a move.

// Other note: the game can be seen as guaranteed actions followed by probabilistic Nature's turns
// Idea: introduce Nature as a "player" - ie list all empty squares after move

// Idea 2: use pointers and a structure that can be traced back ? starting from a state,
// eg empty board, list all possible nature moves, each corresponding to a state with an optimal move.
// BUT: to find this optimal move, you need to come from the "ending"

// Define endings function: all states with no more player moves (no "movement" or "fusion" possible in all 4 directions, ie kind of like no)
// Good ending: ending that contains one (2 is not possible in 2x3) 32=2^5 square
// Idea 3: start from ending states, and point to all possible Nature moves by finding 2=2^1 and 4=2^2 squares

// Offhand idea 2: If we want a simpler test, make the goal variable and start less far from the end.


// State class sketchout


int main(int argc, char *argv[]) {
    util::setup_signal_handlers();

    // #ifdef DEBUG
    // test();
    // #endif

    int rows = State::ROWS;
    int cols = State::COLS;
    int8_t winning_objective = 5; // power of winning objective

    // TODO: remove Deprecated
    /*
    // user entered winning_objective
    if (argc>1) {
        winning_objective = atoi(argv[1]);
    }

    // user entered rows
    if (argc>2) {
        rows = atoi(argv[2]);
    }

    // user entered cols
    if (argc>3) {
        rows = atoi(argv[3]);
    }
    */

    // if, on the turn T-1:
    // - half of the grid + 1 is filled with winning_objective - 1
    // - the other half was first filled with winning_objective - 2
    // then game is winnable in no more than T turns
    // in the worst case, this happens with only Nature moves of 2^1,
    // so T = grid total / 2 + 1 suffices


    int halfgrid = rows*cols /2;
    int worse_case_total = pow(2,winning_objective-1)*(halfgrid + 1) + pow(2,winning_objective-2)* (rows*cols - halfgrid - 1);
    int T = ( worse_case_total )/2 + 1;


    // user entered T
    if (argc>4) {
        T = atoi(argv[4]);
    }


    std::cout << "solved-2048 by Vincent Meduski" << std::endl;
    std::cout << "Rows= " << rows << std::endl;
    std::cout << "Columns= " << cols << std::endl;
    std::cout << "Time horizon= " << std::setw(2) << T << std::endl;
    std::cout << "Objective= " << std::setw(2) << ( 2 << (winning_objective-1) )<< std::endl;
    std::cout << "Executing backwards induction for optimal policy..." << std::endl;

    // empty policy that will be filled with policy_t
    int total_combinations = pow((winning_objective+1), rows*cols);
    std::vector<action_type> policy(total_combinations);
    std::vector<reward_type> value(total_combinations);
    // used for storing newly calculated values
    std::vector<reward_type> new_value(total_combinations);
    
    auto start = std::chrono::high_resolution_clock::now();
    optimal_policy(policy, value, new_value, winning_objective, T);
    auto stop = std::chrono::high_resolution_clock::now();

    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);

    std::cout << "Execution time= " << duration.count()*pow(10,-6) << "s" << std::endl;

    // a game simulation with Nature player
    // it can be played by user or by optimal player, computed above

    bool interactive_game = true;
    if (interactive_game) {

        while (true){ //play until user quits
        std::cout << "To play, use w, a, s and d as directions Up, Left, Down, Right\n";
        std::cout << "Enter to start: ";
        std::getchar();

        // initialise to empty game
        
        State gamestate = State();
        
        action_type optimal = Up;
        
        // at each iteration make Nature move

        do {
            auto nature_move = gamestate.random_nature_move();
            if (nature_move.has_value()) {
                gamestate = nature_move.value();
            } else {
                // Note: this case is impossible to reach
                std::cout << "No more nature moves possible, game ends." << std::endl;
                break; // no more nature moves possible, game ends
            }

            print_gamestate(gamestate);
            int64_t hash = gamestate_to_hash(winning_objective,gamestate, rows, cols);
            std::cout << "Value= " << value[hash] << std::endl;
            optimal = policy[hash];
            std::cout << "Optimal policy= ";
            print_move(optimal);

            action_type a = None;
            std::optional<State> next_state = std::nullopt;
            // player_move returns true if move was successful and false if move is invalid
            if (optimal!=None) {
                
                do
                {
                    char input;
                    std::cin >> input;
                    switch (input)
                    {
                    case 'w':
                        a=Up;
                        break;
                    case 'a':
                        a=Left;
                        break;
                    case 's':
                        a=Down;
                        break;
                    case 'd':
                        a=Right;
                        break;
                    
                    default:
                        a=None;
                        break;
                    }
                    next_state = gamestate.player_move(a);
                }
                while (next_state.has_value()==false); // repeat until valid move is entered
            }
            if (next_state.has_value()) {
                gamestate = next_state.value();
            } else {
                std::cout << "No more player moves possible, game ends." << std::endl;
                break; // no more player moves possible, game ends
            }
        }
        while (optimal!=None); // optimal policy is None when no move is possible
        
        int64_t hash = gamestate_to_hash(winning_objective,gamestate, rows, cols);
        std::cout << "\nGame End.\nReward= " <<value[hash] << "\n" << std::endl;

        // while (random_nature_move(gamestate) && optimal!=None); // DEBUG: uncomment for testing gamestates
        }
    }

    std::cout << "Hello World" << std::endl;
    return 0;
}

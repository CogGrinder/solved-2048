#include "utils.hpp"

#include "types.hpp"
#include "state.hpp"
#include "debug.hpp"
#include "interrupt_handler.hpp"

#include <iostream>
#include <iomanip>

/*
 * new policy at fixed time
 */
// action_type* new_policy(state_size_type state_size) {
//     action_type* out = new action_type[state_size];
//     return out;
// }

/*
 * Reward is simply an indicator of when goal has been attained.
 * Reward can only increase, tiles can only move, stay still or increase,
 * even if game has no more valid moves. 
 */
reward_type final_reward(int8_t goal, const State& gamestate) {
    for (int i = 0; i < State::ROWS; i++)
    {
        for (int j = 0; j < State::COLS; j++)
        {
            if (gamestate(i, j)>=goal)
            {
                // certificate of winning found
                return 1.0;
            }        
        }
    }
    // default, was not won
    return 0.0;
}



/*
 * t: time
 * s: state
 * a: action
 */
reward_type r([[maybe_unused]] int t, [[maybe_unused]] State s, [[maybe_unused]] action_type a) {
    return 0;
}

// Legacy debug printing functions
// TODO: remove with better logging system
void print_gamestate(const State& gamestate) {
    std::cout << gamestate;
}

void print_move(action_type a) {
    std::cout << a << std::endl;
}


void optimal_policy(std::vector<action_type> &policy, std::vector<reward_type> &value, std::vector<reward_type> &new_value, int winning_objective, int T) {
    int rows = State::ROWS;
    int cols = State::COLS;
    int total_combinations = pow((winning_objective+1), rows*cols);
    PRINT(total_combinations);

    /* INITIALISE VALUE */
    // go through all possible positions for tiles
    // TODO: adapt int size to total combinations
    int64_t hashed_state = 0;

    // allocated temporary game state
    State temp;

    // initialising value to final time reward
    while (hashed_state < total_combinations) {
        hash_to_gamestate(winning_objective, hashed_state, temp);
        value[hashed_state] = final_reward(winning_objective, temp);

        // go to the next hash
        hashed_state++;
    }
    
    //sum of rewards over all actions - average gain
    // reward_type* value_at_previous_time = final_time_reward(state_size); //initialise to final gain



    for (int time = T-1; time >= 0 ; time--)
    {
        if (util::global_stop_requested.load()) {
            std::cout << "\n[User Interrupt] MDP backwards induction stopped at time " << time+1 << std::endl;
            util::global_stop_requested.store(false);
            break;
        }

        std::cout << "Time: " << time << std::endl;

        // policy will be rewritten
        
        State temp;

        // hashed_state 0 is an empty board. It does not have any valid moves for player therefore game ends
        policy[0] = Action::None;
        new_value[0] = 0;


        // go through all possible positions for tiles, except 0 because you get Up as optimal move
        for (int64_t hashed_state = 1; hashed_state < total_combinations; hashed_state++) {
            // generate the gamestate, with only the decided empty tiles, all others empty
            hash_to_gamestate(winning_objective, hashed_state, temp);
            if (time <= T-5) {PRINT_GAMESTATE(temp);}

            //default
            policy[hashed_state] = Action::None;
                        
            reward_type max_bellman_expression = -1; //initialise max to -1
            action_type argmax = Action::None;

            //find max_bellman_expression and argmax over all actions (action_set)
            for (auto a : Actions::All)
            {
                //Bellman expression: r + average value with action a
                reward_type bellman_expression = r(time,temp,action_type(a));

                if (a==Action::None) {
                    // None skips the turn
                    // so bellman_expression is previous value of the same state
                    bellman_expression = value[hashed_state];
                } else {
                    // generate all Nature moves from Player move
                    State temp_player_move(temp);
                    
                    // bool valid_move = player_move(temp_player_move,action_type(a));
                    std::optional<State> next_state = State(temp_player_move).player_move(action_type(a));
                    
                    if (next_state.has_value()) {
                        if (time <= T-5) {PRINT(action_type(a));}      
                        if (time <= T-5) {PRINT_GAMESTATE(next_state.value());}
                        // We must consider all Nature moves
                        std::vector<Coord> nature = next_state.value().all_nature_moves();
                        if (time <= T-5) {PRINT(nature.size());}
                        for (std::size_t k = 0; k < nature.size(); k++)
                        {
                            if (time <= T-5) {PRINT(nature[k].i);}
                            if (time <= T-5) {PRINT(nature[k].j);}

                            State nature_move(next_state.value());
                                                        
                            // Nature generates a 2=2^1 tile
                            // TODO: clean up this explicit access (by not using state_type directly)
                            nature_move(nature[k].i, nature[k].j) = 1;
                            int64_t hashed_state_prime_2 = gamestate_to_hash(winning_objective,nature_move);                            
                            // if (time <= T-5) {PRINT_GAMESTATE(nature_move);}

                            // Nature generates a 4=2^2 tile
                            nature_move(nature[k].i, nature[k].j) = 2;
                            int64_t hashed_state_prime_4 = gamestate_to_hash(winning_objective,nature_move);
                            // if (time <= T-5) {PRINT_GAMESTATE(nature_move);}

                            // transition_probability is actually just :
                            // 1 - look at player move
                            // 2 - look at nature move
                            bellman_expression += value[hashed_state_prime_2] * 1.0/(nature.size()*2);
                            bellman_expression += value[hashed_state_prime_4] * 1.0/(nature.size()*2);
                            if (time <= T-5) {PRINT(value[hashed_state_prime_2]);}
                            if (time <= T-5) {PRINT(value[hashed_state_prime_4]);}

                        }
                    } else {
                        // ignore this move with sentinel penalty value
                        bellman_expression = -1;
                    }
                }

                if (bellman_expression > max_bellman_expression ) {
                    argmax = action_type(a);
                    max_bellman_expression = bellman_expression;
                }
            }
            // std::cout << std::endl;
            
            // max_bellman_expression is done, update value and policy
            new_value[hashed_state] = max_bellman_expression;
            policy[hashed_state] = argmax;

            if (time <= T-5) {
                PRINT(max_bellman_expression);
                PRINT(hashed_state);
                PRINT_MOVE(argmax);
                PRINT_GAMESTATE(temp);
                #ifdef DEBUG
                // pause after each state for debugging
                std::cout << "Press Enter to continue..." << std::endl;
                std::cin.get();
                #endif
            }

        }

        // exchange pointers to value and new_value
        value.swap(new_value);
    }
}
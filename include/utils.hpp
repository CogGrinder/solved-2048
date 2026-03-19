#pragma once

#include "types.hpp"
#include "state.hpp"

#include <cstdint>
#include <vector>

reward_type final_reward(int8_t goal, const state_type& gamestate);
reward_type r(int t, state_type s, action_type a);

void print_gamestate(const State& gamestate);
void print_move(action_type a);

inline void hash_to_gamestate(int winning_objective, int64_t hash, State& gamestate, int rows, int cols) {
    int64_t hash_copy = hash;
    for (int i = 0; i < rows; i++)
    {
        for (int j = 0; j < cols; j++)
        {
            // checks 0 to winning_objective int that corresponds to i, j
            gamestate(i, j) = hash_copy%(winning_objective+1);
            // shifts the "bits"
            hash_copy = hash_copy / (winning_objective+1) ;
        }
    }
}

/// @brief gamestate_to_hash
/// @param winning_objective 
/// @param gamestate 
/// @return hash if gamestate has all tiles no greater than winning_objective, 0 else
inline int64_t gamestate_to_hash(int winning_objective, State& gamestate, int rows, int cols) {
    // TODO: clean up rows and cols
    int64_t hash = 0;
    for (int i = rows-1; i >= 0; i--)
    {
        for (int j = cols-1; j >= 0; j--)
        {
            hash *= (winning_objective+1);
            if (gamestate(i, j)>winning_objective) {
                return 0;
            }
            hash += gamestate(i, j);
            // shifts the bits of the bitmask
        }
    }
    return hash;
}

void optimal_policy(std::vector<action_type>& policy,
				   std::vector<reward_type>& value,
				   std::vector<reward_type>& new_value,
				   int winning_objective,
				   int T);

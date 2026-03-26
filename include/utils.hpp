#pragma once

#include "types.hpp"
#include "state.hpp"

#include <cstdint>
#include <vector>

reward_type final_reward(int8_t goal, const state_type& gamestate);
reward_type r(int t, state_type s, action_type a);

void print_gamestate(const State& gamestate);
void print_move(action_type a);

/// @brief hash_to_gamestate
/// @param winning_objective 
/// @param hash 
/// @param gamestate gamestate is modified in place to match the hash 
inline void hash_to_gamestate(int winning_objective, const int64_t hash, State& gamestate) {
    int64_t hash_copy = hash;
    for (int i = 0; i < State::SIZE; i++)
    {
        // checks 0 to winning_objective int that corresponds to i, j
        gamestate.data_[i] = hash_copy%(winning_objective+1);
        // shifts the "bits"
        hash_copy = hash_copy / (winning_objective+1) ;
    }
}

/**
 * @brief Compresses a gamestate into a unique 64-bit integer hash.
 * Uses a base (winning_objective + 1) encoding to map the board state
 * to a unique value with a small memory footprint.
 * This is used for indexing into the MDP policy table.
 * @param winning_objective The target tile value (as an exponent).
 * @param gamestate The current board configuration to hash.
 * @return int64_t The unique hash, or 0 if any tile exceeds the winning_objective.
 * * @note This assumes the total state space fits within a 64-bit integer.
 */
inline int64_t gamestate_to_hash(int winning_objective, const State& gamestate) {
    // TODO: FUTURE: when state_type will be a flat array,
    // we can directly iterate over it instead of using gamestate(i, j)
    // TODO: FUTURE: support larger state spaces by using a larger integer type
    int64_t hash = 0;
    for (int i = State::SIZE-1; i >= 0; i--)
    {
        // shifts the bits of the bitmask, does nothing for hash=0
        hash *= (winning_objective+1);
        if (gamestate.data_[i] > winning_objective) {
            // We treat all exceeding values as winning objectives
            // Note: this is an edge case but helps reduce risk of
            // evaluating overacheiving as invalid
            // This should not affect policy calculation because we do not iterate over
            // these states and only visit them as next states of winning states
            // Depending on the implementation of the final reward function, this may become relevant.
            // This is also beneficial for logic and heuristics that use the sum of all tiles
            hash += winning_objective;
        } else {
            hash += gamestate.data_[i];
        }
    }
    return hash;
}

void optimal_policy(std::vector<action_type>& policy,
				   std::vector<reward_type>& value,
				   std::vector<reward_type>& new_value,
				   int winning_objective,
				   int T);

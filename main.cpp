#include <vector>
#include <cmath>

#include <iostream>
#include <iomanip>
#include <string>
#include <bitset>

#include <cassert>

// DEBUG macros

#ifdef DEBUG

#define PRINT_TILE(x) std::cout << #x << "= " << std::setw(2) << ( 2 << x-1 )<< std::endl
#define PRINT(x) std::cout << #x << "= " << x << std::endl
#define OK() std::cout << " OK at " << __LINE__ << std::endl
#define PRINT_GAMESTATE(x) print_gamestate(x)
#define PRINT_MOVE(a) print_move(a)

#else
#define PRINT_TILE(x)
#define PRINT(x)
#define OK()
#define PRINT_GAMESTATE(x)
#define PRINT_MOVE(a)

#endif

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


enum action_type { Up, Down, Left, Right, None};
typedef std::vector<std::vector<int8_t>> state_type;
// typedef std::vector<int8_t> state_size_type;
typedef double reward_type;

struct Coord {
    int i;
    int j;
};

// compactifying function:
// takes Move (Up, Down, Left, Right) // do i add None? 
// eg Up: swipe up ie "compact" blocks upward until impact on top edge while prioritising top 2 blocks in case of 3 similar blocks

// Question: return legal moves ? Or just modified gamestate ?
// Since gamestate is small, just copy the gamestate and modify it


/* shifting functions:
 * for use in algorithms
 * i and j are coordinates for tile to shift to (overwrites tile)
 * in other words: removes element in row or column and shifts all others,
 * adding 0 on the end
 * 
 * @returns:
 *  bool: indicates wether non-zero values were shifted, used to test if move was legal
 */

inline bool shift_up(int i, int j, int rows, int cols, state_type& gamestate){
    bool moved_non_zero_tile = false;
    int8_t new_value;
    for (int i_rewrite = i; i_rewrite < rows-1; i_rewrite++) {
        new_value = gamestate[i_rewrite+1][j];
        gamestate[i_rewrite][j] = new_value;
        if (new_value!=0) moved_non_zero_tile = true;
    }
    // insert a 0 at the end
    gamestate[rows-1][j] = 0;
    return moved_non_zero_tile;
}

inline bool shift_down(int i, int j, int rows, int cols, state_type& gamestate){
    bool moved_non_zero_tile = false;
    int8_t new_value;
    for (int i_rewrite = i; i_rewrite > 0; i_rewrite--) {
        new_value = gamestate[i_rewrite-1][j];
        gamestate[i_rewrite][j] = new_value;
        if (new_value!=0) moved_non_zero_tile = true;
    }
    // insert a 0 at the end
    gamestate[0][j] = 0;
    return moved_non_zero_tile;
}

inline bool shift_left(int i, int j, int rows, int cols, state_type& gamestate){
    bool moved_non_zero_tile = false;
    int8_t new_value;
    for (int j_rewrite = j; j_rewrite < cols-1; j_rewrite++) {
        new_value = gamestate[i][j_rewrite+1];
        gamestate[i][j_rewrite] = new_value;
        if (new_value!=0) moved_non_zero_tile = true;
    }
    // insert a 0 at the end
    gamestate[i][cols-1] = 0;
    return moved_non_zero_tile;
}

inline bool shift_right(int i, int j, int rows, int cols, state_type& gamestate){
    bool moved_non_zero_tile = false;
    int8_t new_value;
    for (int j_rewrite = j; j_rewrite > 0; j_rewrite--) {
        new_value = gamestate[i][j_rewrite-1];
        gamestate[i][j_rewrite] = new_value;
        if (new_value!=0) moved_non_zero_tile = true;
    }
    // insert a 0 at the end
    gamestate[i][0] = 0;
    return moved_non_zero_tile;
}

// this part of a move is deterministic and is independent of Nature move
// warning, modifies gamestate
bool player_move(state_type& gamestate, action_type a){
    int rows = gamestate.size();
    int cols = gamestate[0].size();

    bool is_valid_move = false;
    switch (a)
    {
    case Up:
        for (int j = 0; j < cols; j++)
        {
            // on each column, first check for holes
            int i = 0;
            int number_of_zeros = 0;
            int8_t previous_non_zero_tile = 0;
            int previous_non_zero_tile_index = -1;

            // note: consecutive zeros may slow things down
            while (i < rows && number_of_zeros < rows)
            {
                // shifts due to adding tiles and "gravity"
                if (gamestate[i][j]!=0) {
                    // check that previous tile was the same
                    if (previous_non_zero_tile==gamestate[i][j]) {
                        // add tiles together
                        gamestate[previous_non_zero_tile_index][j]++;
                        shift_up(i, j, rows, cols, gamestate);
                        // reset previous_non_zero_tile
                        // (cannot add tiles to the same tile in the same turn)
                        previous_non_zero_tile = 0;
                        is_valid_move=true;
                    } else {
                        previous_non_zero_tile=gamestate[i][j];
                        previous_non_zero_tile_index = i;
                        // move to next tile
                        i++;
                    }
                } else {
                    // "gravity" shift on top of blank space
                    if (shift_up(i, j, rows, cols, gamestate)) is_valid_move=true;
                    number_of_zeros++;
                }
            }
        }
        break;
    case Down:
        for (int j = 0; j < cols; j++)
        {
            // on each column, first check for holes
            int i = rows-1;
            int number_of_zeros = 0;
            int8_t previous_non_zero_tile = 0;
            int previous_non_zero_tile_index = -1;

            // note: consecutive zeros may slow things down
            while (i >= 0 && number_of_zeros < rows)
            {
                // shifts due to adding tiles and "gravity"
                if (gamestate[i][j]!=0) {
                    // check that previous tile was the same
                    if (previous_non_zero_tile==gamestate[i][j]) {
                        // add tiles together
                        gamestate[previous_non_zero_tile_index][j]++;
                        shift_down(i, j, rows, cols, gamestate);
                        previous_non_zero_tile = 0;
                        is_valid_move=true;
                    } else {
                        previous_non_zero_tile=gamestate[i][j];
                        previous_non_zero_tile_index = i;
                        // move to next tile
                        i--;
                    }
                } else {
                    // "gravity" shift on top of blank space
                    if (shift_down(i, j, rows, cols, gamestate)) is_valid_move=true;
                    number_of_zeros++;
                }
            }
        }
        break;
    case Left:
        for (int i = 0; i < rows; i++)
        {
            // on each column, first check for holes
            int j = 0;
            int number_of_zeros = 0;
            int8_t previous_non_zero_tile = 0;
            int previous_non_zero_tile_index = -1;

            // note: consecutive zeros may slow things down
            while (j < cols && number_of_zeros < cols)
            {
                // shifts due to adding tiles and "gravity"
                if (gamestate[i][j]!=0) {
                    // check that previous tile was the same
                    if (previous_non_zero_tile==gamestate[i][j]) {
                        // add tiles together
                        gamestate[i][previous_non_zero_tile_index]++;
                        shift_left(i, j, rows, cols, gamestate);
                        previous_non_zero_tile = 0;
                        is_valid_move=true;
                    } else {
                        previous_non_zero_tile=gamestate[i][j];
                        previous_non_zero_tile_index = j;
                        // move to next tile
                        j++;
                    }
                } else {
                    // "gravity" shift on top of blank space
                    if (shift_left(i, j, rows, cols, gamestate)) is_valid_move=true;
                    number_of_zeros++;
                }
            }
        }
        break;
    case Right:
        for (int i = 0; i < rows; i++)
        {
            // on each column, first check for holes
            int j = cols-1;
            int number_of_zeros = 0;
            int8_t previous_non_zero_tile = 0;
            int previous_non_zero_tile_index = -1;

            // note: consecutive zeros may slow things down
            while (j >= 0 && number_of_zeros < cols)
            {
                // shifts due to adding tiles and "gravity"
                if (gamestate[i][j]!=0) {
                    // check that previous tile was the same
                    if (previous_non_zero_tile==gamestate[i][j]) {
                        // add tiles together
                        gamestate[i][previous_non_zero_tile_index]++;
                        shift_right(i, j, rows, cols, gamestate);
                        previous_non_zero_tile = 0;
                        is_valid_move=true;
                    } else {
                        previous_non_zero_tile=gamestate[i][j];
                        previous_non_zero_tile_index = j;
                        // move to next tile
                        j--;
                    }
                } else {
                    // "gravity" shift on top of blank space
                    if (shift_right(i, j, rows, cols, gamestate)) is_valid_move=true;
                    number_of_zeros++;
                }
            }
        }
        break;
    
    default:
        break;
    }

    return is_valid_move;
    
}

std::vector<Coord> all_nature_moves(const state_type& gamestate) {
    int rows = gamestate.size();
    int cols = gamestate[0].size();
    std::vector<Coord> list_of_empty_tiles;
    for (int i = 0; i < rows; i++)
    {
        for (int j = 0; j < cols; j++)
        {
        if (gamestate[i][j]==0)
        {
            list_of_empty_tiles.push_back({i,j});
        }        
        }
    }

    return list_of_empty_tiles;
}

bool random_nature_move(int rows, int cols, state_type& gamestate){
    std::vector<Coord> list_of_empty_tiles = all_nature_moves(gamestate);
    // Nature move is valid if there are still available tiles
    bool is_valid_move = list_of_empty_tiles.size()>0;
    

    if (is_valid_move)
    {
        srand((unsigned) time(NULL) + list_of_empty_tiles.size()); 
        Coord chosen_square = list_of_empty_tiles[rand()%list_of_empty_tiles.size()];
        int8_t new_value = rand()%2 + 1;

        gamestate[chosen_square.i][chosen_square.j] = new_value;
    }

    return is_valid_move;
}


// struct State
// {
//     state_type data;
//     auto pointers_to_predecessors
//     /* data */
// };



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
reward_type final_reward(int8_t goal, const state_type& gamestate) {
    int rows = gamestate.size();
    int cols = gamestate[0].size();
    for (int i = 0; i < rows; i++)
    {
        for (int j = 0; j < cols; j++)
        {
            if (gamestate[i][j]>=goal)
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
reward_type r(int t, state_type s, action_type a) {
    return 0;
}

void print_gamestate(state_type& gamestate) {
    int rows = gamestate.size();
    int cols = gamestate[0].size();
    // set this to true for easy test generation for copy and paste
    bool display_as_cpp_vectors = false;

    std::cout << std::string(3*cols,'_') << std::endl;
    for (int i = 0; i < rows; i++)
    {
        if (display_as_cpp_vectors) {
            std::cout << "\n{";
        } else {
            std::cout << "\n";
        }
        for (int j = 0; j < cols; j++)
        {
            if (display_as_cpp_vectors) {
                std::cout << std::setw(2) << gamestate[i][j]*1;
                if (j < cols-1) std::cout << ",";
            } else {
                if (gamestate[i][j] != 0) {
                    std::cout << std::setw(2) << ( 2 << gamestate[i][j]-1 ) << " ";
                } else {
                    std::cout << "   ";
                }
            }
        }
        if (display_as_cpp_vectors) {
            std::cout << "}";
            if (i < rows-1) std::cout << ",";
        }
    }
    std::cout << std::endl;
    std::cout << std::string(3*cols,'-') << std::endl;

}

void print_move(action_type a) {
    switch (a) {
        case Up: std::cout << "Up" << std::endl; break;
        case Down: std::cout << "Down" << std::endl; break;
        case Left: std::cout << "Left" << std::endl; break;
        case Right: std::cout << "Right" << std::endl; break;
        default: std::cout << "None" << std::endl; break;
    }
}

inline void hash_to_gamestate(int winning_objective, int64_t hash, state_type& gamestate, int rows, int cols) {
    int64_t hash_copy = hash;
    for (int i = 0; i < rows; i++)
    {
        for (int j = 0; j < cols; j++)
        {
            // checks 0 to winning_objective int that corresponds to i, j
            gamestate[i][j] = hash_copy%(winning_objective+1);
            // shifts the "bits"
            hash_copy = hash_copy / (winning_objective+1) ;
        }
    }
}

/// @brief gamestate_to_hash
/// @param winning_objective 
/// @param gamestate 
/// @return hash if gamestate has all tiles no greater than winning_objective, 0 else
inline int64_t gamestate_to_hash(int winning_objective, state_type& gamestate, int rows, int cols) {
    int64_t hash = 0;
    for (int i = rows-1; i >= 0; i--)
    {
        for (int j = cols-1; j >= 0; j--)
        {
            hash *= (winning_objective+1);
            if (gamestate[i][j]>winning_objective) {
                return 0;
            }
            hash += gamestate[i][j];
            // shifts the bits of the bitmask
        }
    }
    return hash;
}

int main(int argc, char *argv[]) {

    /* TESTING : game playing for rule testing */

    // Testing player_move:
    // Testing fusion and movement
    {
        {
            int rows = 1;
            int cols = 4;

            state_type gamestate = 
            {
                { 0, 0, 3, 2}
            };

            // PRINT_GAMESTATE(gamestate);
            player_move(gamestate,Up);
            // PRINT_GAMESTATE(gamestate);

            state_type changed_gamestate_assert =
            {
                { 0, 0, 3, 2}
            };

            assert((gamestate==changed_gamestate_assert));
        }

        {
            int rows = 3;
            int cols = 4;

            state_type gamestate = 
            {
                { 0, 0, 3, 2},
                { 0, 1, 1, 2},
                { 3, 1, 1, 4}
            };

            // PRINT_GAMESTATE(gamestate);
            player_move(gamestate,Up);
            // PRINT_GAMESTATE(gamestate);

            state_type changed_gamestate_assert =
            {
                { 3, 2, 3, 3},
                { 0, 0, 2, 4},
                { 0, 0, 0, 0}
            };
            
            assert((gamestate==changed_gamestate_assert));
        }

        {
            int rows = 5;
            int cols = 5;

            state_type gamestate = 
            {
                { 0, 0, 3, 3, 2},
                { 0, 1, 1, 1, 2},
                { 3, 1, 1, 1, 4},
                { 4, 0, 0, 3, 5},
                { 3, 4, 4, 4, 4}
            };

            // PRINT_GAMESTATE(gamestate);
            player_move(gamestate,Up);
            // PRINT_GAMESTATE(gamestate);
            
            state_type changed_gamestate_assert =
            {
                { 3, 2, 3, 3, 3},
                { 4, 4, 2, 2, 4},
                { 3, 0, 4, 3, 5},
                { 0, 0, 0, 4, 4},
                { 0, 0, 0, 0, 0}
            };

            assert((gamestate==changed_gamestate_assert));
        }
    }
    // Testing move validation
    {
        {
            state_type gamestate = 
            {
                { 3, 4, 4, 4 },
                { 1, 3, 2, 1 },
                { 2, 0, 1, 0 },
                { 0, 0, 0, 0 }
            };

            // PRINT_GAMESTATE(gamestate);
            bool is_valid_move = player_move(gamestate,Up);
            // PRINT_GAMESTATE(gamestate);

            assert ((is_valid_move==false));
        }
        {
            state_type gamestate = 
            {
                { 2, 1, 3 },
                { 2, 3, 1 }
            };

            // PRINT_GAMESTATE(gamestate);
            bool is_valid_move = player_move(gamestate,Left);
            // PRINT_GAMESTATE(gamestate);

            assert ((is_valid_move==false));
        }
        {
            state_type gamestate = 
            {
                { 2, 1, 3 },
                { 3, 3, 1 }
            };

            // PRINT_GAMESTATE(gamestate);
            bool is_valid_move = player_move(gamestate,Down);
            // PRINT_GAMESTATE(gamestate);

            assert ((is_valid_move==false));
        }
    }

    // Testing move hashing
    {
        {
            int rows = 4;
            int cols = 4;

            state_type gamestate = 
            {
                { 3, 4, 4, 4 },
                { 1, 3, 2, 1 },
                { 2, 0, 1, 0 },
                { 0, 0, 0, 1 }
            };

            // PRINT_GAMESTATE(gamestate);
            auto hash = gamestate_to_hash(5,gamestate, rows, cols);

            state_type game_from_hash(rows, std::vector<int8_t>(cols));
            hash_to_gamestate(5,hash,game_from_hash, rows, cols);

            // PRINT_GAMESTATE(gamestate);
            for (int i = 0; i < rows; i++)
            {
                for (int j = 0; j < cols; j++)
                {
                    assert ((game_from_hash[i][j]==gamestate[i][j]));
                }
            }
            
        }
        
                    
    }

    // Testing random_nature_move
    if (false) {
        {
            int rows = 5;
            int cols = 5;

            state_type gamestate = 
            {
                { 0, 0, 3, 3, 2},
                { 0, 1, 1, 1, 2},
                { 3, 1, 1, 1, 4},
                { 4, 0, 0, 3, 5},
                { 3, 4, 4, 4, 4}
            };

            PRINT_GAMESTATE(gamestate);
            while (random_nature_move(rows,cols,gamestate)) {
                PRINT_GAMESTATE(gamestate);
            }
        }
    }



    int rows = 2;
    int cols = 3;
    int8_t winning_objective = 5; // power of winning objective
    int total_combinations = pow((winning_objective+1), rows*cols);

    int T = pow(2,winning_objective-1)/2*rows*cols; // TODO fix this

    if (argc>1) {
        T = atoi(argv[1]);
    }

    PRINT(total_combinations);
    std::vector<reward_type> value(total_combinations);

    /* INITIALISE VALUE */
    // go through all possible positions for tiles
    int64_t hashed_state = 0;

    // allocated temporary game state
    state_type temp(rows, std::vector<int8_t>(cols));

    // initialising value to final time reward
    while (hashed_state < total_combinations) {
        hash_to_gamestate(winning_objective, hashed_state, temp, rows, cols);
        value[hashed_state] = final_reward(winning_objective, temp);

        // go to the next hash
        hashed_state++;
    }

    // empty policy that will be filled with policy_t
    std::vector<std::vector<action_type>> policy;
    
    //sum of rewards over all actions - average gain
    // reward_type* value_at_previous_time = final_time_reward(state_size); //initialise to final gain
    
    // used for storing newly calculated values
    std::vector<reward_type> new_value(total_combinations);


    for (int time = T-1; time >= 0 ; time--)
    {
        std::cout << "Time: " << time << std::endl;

        // Write new fixed time t=time policy in policy
        // Policy will be inserted at index 0 of list 
        std::vector<action_type> policy_t(total_combinations);
        
        state_type temp(rows, std::vector<int8_t>(cols));
        state_type temp_prime(rows, std::vector<int8_t>(cols));

        // hashed_state 0 is an empty board. It does not have any valid moves for player therefore game ends
        policy_t[0] = None;
        new_value[0] = 0;


        // go through all possible positions for tiles, except 0 because you get Up as optimal move
        for (int64_t hashed_state = 1; hashed_state < total_combinations; hashed_state++) {
            // generate the gamestate, with only the decided empty tiles, all others empty
            hash_to_gamestate(winning_objective, hashed_state, temp, rows, cols);
            PRINT_GAMESTATE(temp); // DEBUG        

            //pointer to best action
            policy_t[hashed_state] = None;
                        
            reward_type max_bellman_expression = -1; //initialise max to -1
            action_type argmax = None;

            //find max_bellman_expression and argmax over all actions (action_set)
            for (int a = Up; a <= None; a++)
            {
                //Bellman expression: r + average value with action a
                reward_type bellman_expression = r(time,temp,action_type(a));

                if (a==None) {
                    // None skips the turn
                    // so bellman_expression is previous value of the same state
                    bellman_expression = value[hashed_state];
                } else {
                    // generate all Nature moves from Player move
                    state_type temp_player_move(temp);
                    
                    bool valid_move = player_move(temp_player_move,action_type(a));
                    
                    if (valid_move) {
                        std::vector<Coord> nature = all_nature_moves(temp_player_move);
                        for (int i = 0; i < nature.size(); i++)
                        {
                            state_type nature_move(temp_player_move);
                            
                            // Nature generates a 2=2^1 tile
                            nature_move[nature[i].i][nature[i].j] = 1;
                            int64_t hashed_state_prime_2 = gamestate_to_hash(winning_objective,nature_move, rows, cols);                            

                            // Nature generates a 4=2^2 tile
                            nature_move[nature[i].i][nature[i].j] = 2;
                            int64_t hashed_state_prime_4 = gamestate_to_hash(winning_objective,nature_move, rows, cols);

                            // transition_probability is actually just :
                            // 1 - look at player move
                            // 2 - look at nature move
                            bellman_expression += value[hashed_state_prime_2] * 1.0/(nature.size()*2);
                            bellman_expression += value[hashed_state_prime_4] * 1.0/(nature.size()*2);
                        }
                    } else {
                        // ignore this move
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
            PRINT(max_bellman_expression);
            policy_t[hashed_state] = argmax;
            // PRINT(argmax);
            PRINT_MOVE(argmax);

        }
        
        // add policy_t to policy
        policy.insert(policy.begin(), policy_t);

        // exchange pointers to value and new_value
        value.swap(new_value);
    }
    


    // a game simulation with Nature player
    // it can be played by user or by optimal player, computed above

    bool interactive_game = true;
    if (interactive_game) {

        while (true){ //play until user quits
        std::cout << "To play, use w, a, s and d as directions Up, Left, Down, Right\n";
        std::cout << "Enter to start: ";
        std::getchar();

        // initialise to empty game
        state_type gamestate(rows, std::vector<int8_t>(cols)); // DEBUG: comment this for testing gamestates

        action_type optimal = Up;
        
        // at each iteration make Nature move
        while (random_nature_move(rows,cols,gamestate) && optimal!=None) { // DEBUG: comment this for testing gamestates

        // do { // DEBUG: uncomment for testing gamestates

            print_gamestate(gamestate);
            int64_t hash = gamestate_to_hash(winning_objective,gamestate, rows, cols);
            std::cout << "Value= " << value[hash] << std::endl;
            optimal = policy[0][hash];
            std::cout << "Optimal policy= ";
            print_move(optimal);

            action_type a = None;
            // player_move returns true if move was successful and false if move is invalid
            while (!player_move(gamestate,a) && optimal!=None)
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
            }
        }
        
        // while (random_nature_move(rows,cols,gamestate) && optimal!=None); // DEBUG: uncomment for testing gamestates
        }
    }

    std::cout << "Hello World" << std::endl;
    return 0;
}

#include <vector>
#include <iostream>
#include <iomanip>
#include <string>

#include <cassert>

// DEBUG macros
#define PRINT_TILE(x) std::cout << #x << "= " << std::setw(2) << ( 2 << x-1 )<< std::endl
#define PRINT(x) std::cout << #x << "= " << x << std::endl

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

bool player_move(int rows, int cols, state_type& gamestate, action_type a){
    // this part of a move is deterministic and is independent of Nature move
    // TODO: warning, check copying of gamestate
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

std::vector<Coord> all_nature_moves(int rows, int cols, const state_type& gamestate) {

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
    std::vector<Coord> list_of_empty_tiles = all_nature_moves(rows, cols, gamestate);
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

// reward_type* final_time_reward(state_size_type state_size) {
//     reward_type* out = new reward_type[state_size];
// }



/*
 * t: time
 * s: state
 * a: action
 */
reward_type r(int t, state_type s, action_type a) {
    return 0;
}

double transition_probability(state_type s, state_type s_prime, action_type a) {
    // TODO
    return 0;
}

void display_gamestate(int rows, int cols, state_type& gamestate) {
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

            // display_gamestate(rows,cols,gamestate);
            player_move(rows,cols,gamestate,Up);
            // display_gamestate(rows,cols,gamestate);

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

            // display_gamestate(rows,cols,gamestate);
            player_move(rows,cols,gamestate,Up);
            // display_gamestate(rows,cols,gamestate);

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

            // display_gamestate(rows,cols,gamestate);
            player_move(rows,cols,gamestate,Up);
            // display_gamestate(rows,cols,gamestate);
            
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
            int rows = 4;
            int cols = 4;

            state_type gamestate = 
            {
                { 3, 4, 4, 4 },
                { 1, 3, 2, 1 },
                { 2, 0, 1, 0 },
                { 0, 0, 0, 0 }
            };

            // display_gamestate(rows,cols,gamestate);
            bool is_valid_move = player_move(rows,cols,gamestate,Up);
            // display_gamestate(rows,cols,gamestate);

            assert ((is_valid_move==false));
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

            display_gamestate(rows,cols,gamestate);
            while (random_nature_move(rows,cols,gamestate)) {
                display_gamestate(rows,cols,gamestate);
            }
        }
    }


    // a game simulation with Nature player
    // it can be played by user or by optimal player, computed above

    int rows = 4;
    int cols = 4;

    state_type gamestate = 
    {
        { 0, 0, 0, 0},
        { 0, 0, 0, 0},
        { 0, 0, 0, 0},
        { 0, 0, 0, 0}
    };

    while (random_nature_move(rows,cols,gamestate)) {
        display_gamestate(rows,cols,gamestate);
        
        action_type a = None;
        // player_move returns true if move was successful and false if move is invalid
        while (!player_move(rows,cols,gamestate,a))
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

    std::cout << "Hello World" << std::endl;
    return 0;
}

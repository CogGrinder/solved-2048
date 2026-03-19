#include "types.hpp"
#include "state.hpp"
#include "test_state.hpp"
// TODO: legacy debug utilities
#include "debug.hpp"
#include <iostream>

#ifdef DEBUG
  #define ASSERT_DEBUG(expr, result, original, text) \
    do { \
      if (!(expr)) { \
        PRINT_GAMESTATE(original); \
        std::cout << "Assertion failed: " << text << std::endl; \
        PRINT_GAMESTATE(result); \
      } \
      assert(expr); \
    } while(0)
#else
  #define ASSERT_DEBUG(expr, result, original, text) assert(expr)
#endif

// old testing suite
void test() {

    if (State::ROWS == 2 && State::COLS == 3) {
        std::cout << "Testing on 2x3 board" << std::endl;
    } else {
        std::cout << "Testing not implemented on non-2x3 board" << std::endl;
        return;
        // TODO: no longer tests for niche 4-tile fusion case
    }

    // TESTING : game playing for rule testing //

    // Testing player_move:
    // Testing fusion and movement
    {
        {
            State gamestate({
                { 0, 3, 2},
                { 0, 2, 0},
            });

            auto result = gamestate.player_move(Up);

            ASSERT_DEBUG(!result.has_value(), result.value_or(State()), gamestate, "Move should be invalid");
        }

        {
            State gamestate({
                { 2, 2, 4},
                { 3, 1, 1},
            });

            auto result = gamestate.player_move(Left);

            State gamestate_2({
                { 3, 4, 0},
                { 3, 2, 0},
            });
            
            ASSERT_DEBUG((result.value_or(State())==gamestate_2), result.value_or(State()), gamestate_2, "Move should result in the following gamestate");

            result = gamestate_2.player_move(Down);

            State gamestate_3({
                { 0, 4, 0},
                { 4, 2, 0},
            });

            ASSERT_DEBUG((result.value_or(State())==gamestate_3), result.value_or(State()), gamestate_3, "Move should result in the following gamestate");
            
        }


        // TODO: find a way to test game logic for other sizes, 4x4 and 5x5 no longer available for testing
        /*
        {
            // int rows = 3;
            // int cols = 4;

            state_type gamestate = 
            {
                { 0, 0, 3, 2},
                { 0, 1, 1, 2},
                { 3, 1, 1, 4}
            };

            // PRINT_GAMESTATE(gamestate);
            player_move(gamestate,Up);
            // PRINT_GAMESTATE(gamestate);

            state_type gamestate_2 =
            {
                { 3, 2, 3, 3},
                { 0, 0, 2, 4},
                { 0, 0, 0, 0}
            };
            
            assert((gamestate==gamestate_2));
        }
        {
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
            
            state_type gamestate_2 =
            {
                { 3, 2, 3, 3, 3},
                { 4, 4, 2, 2, 4},
                { 3, 0, 4, 3, 5},
                { 0, 0, 0, 4, 4},
                { 0, 0, 0, 0, 0}
            };

            assert((gamestate==gamestate_2));
        }
        */
    }

    // Testing move validation
    {
        {
            State gamestate({
                { 1, 4, 4 },
                { 2, 0, 1 },
            });

            auto result = gamestate.player_move(Up);

            assert ((result.has_value()==false));
        }
        {
            State gamestate({
                { 2, 1, 3 },
                { 2, 3, 1 }
            });

            auto result = gamestate.player_move(Left);

            assert ((result.has_value()==false));
        }
        {
            State gamestate({
                { 2, 1, 3 },
                { 3, 3, 1 }
            });

            auto result = gamestate.player_move(Down);

            assert ((result.has_value()==false));
        }
    }
    
    // Testing move hashing
    {
        {
            int rows = 2;
            int cols = 3;

            State gamestate({
                { 3, 4, 0},
                { 1, 3, 5},
            });

            // PRINT_GAMESTATE(gamestate);
            auto hash = gamestate_to_hash(5,gamestate, rows, cols);

            State game_from_hash;
            hash_to_gamestate(5,hash,game_from_hash, rows, cols);

            // PRINT_GAMESTATE(gamestate);
            assert ((game_from_hash==gamestate));
        }
        
    }
    
    /*
    // Testing random_nature_move
    {
        // int rows = 5;
        // int cols = 5;

        state_type gamestate = 
        {
            { 0, 0, 3, 3, 2},
            { 0, 1, 1, 1, 2},
            { 3, 1, 1, 1, 4},
            { 4, 0, 0, 3, 5},
            { 3, 4, 4, 4, 4}
        };

        PRINT_GAMESTATE(gamestate);
        while (random_nature_move(gamestate)) {
            PRINT_GAMESTATE(gamestate);
        }
    }
    */
}
// end of old testing suite

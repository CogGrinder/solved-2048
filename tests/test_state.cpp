#include "state.hpp"
#include "utils.hpp"

#include <gtest/gtest.h>
#include <vector>

namespace {

#define SKIP_UNLESS_BOARD_2X3()                                            \
    do {                                                                    \
        if (State::ROWS != 2 || State::COLS != 3) {                        \
            GTEST_SKIP() << "Tests currently target only 2x3 boards.";    \
        }                                                                   \
    } while (0)

}  // namespace

/* Test for constructor for hotswapping tests to flat array implementation
TEST(StateConstructorTest, CopiesFromVector2D) {
    SKIP_UNLESS_BOARD_2X3();

    const std::vector<std::vector<int8_t>> data = {
        {0, 3, 2},
        {0, 2, 0},
    };

    const State from_vector(data);
    const State from_flat(std::array<int8_t, 6>{0, 3, 2, 0, 2, 0});

    EXPECT_EQ(from_vector, from_flat);
}
*/

TEST(StateMoveTest, InvalidUpMovesReturnNullopt) {
    SKIP_UNLESS_BOARD_2X3();

    const State up_invalid({
        {0, 3, 2},
        {0, 2, 0},
    });
    EXPECT_FALSE(up_invalid.player_move(Action::Up).has_value());

    const State up_invalid_2({
        {1, 4, 4},
        {2, 0, 1},
    });
    EXPECT_FALSE(up_invalid_2.player_move(Action::Up).has_value());

    const State left_invalid({
        {2, 1, 3},
        {2, 3, 1},
    });
    EXPECT_FALSE(left_invalid.player_move(Action::Left).has_value());

    const State down_invalid({
        {2, 1, 3},
        {3, 3, 1},
    });
    EXPECT_FALSE(down_invalid.player_move(Action::Down).has_value());
}

TEST(StateMoveTest, DoubleMergePrevention) {
    SKIP_UNLESS_BOARD_2X3();
    // [1, 1, 1] should become [2, 1, 0] and should NOT become [1, 2, 0] or any other line
    const State triple({
        {1, 1, 1},
        {0, 0, 0},
    });
    const auto result = triple.player_move(Action::Left);
    const State expected({
        {2, 1, 0},
        {0, 0, 0},
    });
    EXPECT_EQ(*result, expected);
}

TEST(StateMoveTest, FusionAndMovementExpectedSequence) {
    SKIP_UNLESS_BOARD_2X3();

    const State gamestate({
        {2, 2, 4},
        {3, 1, 1},
    });

    const auto after_left = gamestate.player_move(Action::Left);
    ASSERT_TRUE(after_left.has_value());

    const State expected_after_left({
        {3, 4, 0},
        {3, 2, 0},
    });
    EXPECT_EQ(*after_left, expected_after_left);

    const auto after_down = expected_after_left.player_move(Action::Down);
    ASSERT_TRUE(after_down.has_value());

    const State expected_after_down({
        {0, 4, 0},
        {4, 2, 0},
    });
    EXPECT_EQ(*after_down, expected_after_down);
}

TEST(StateHashTest, HashRoundTripPreservesState) {
    SKIP_UNLESS_BOARD_2X3();

    State gamestate({
        {3, 4, 0},
        {1, 3, 5},
    });

    const auto hash = gamestate_to_hash(5, gamestate);

    State from_hash;
    hash_to_gamestate(5, hash, from_hash);

    EXPECT_EQ(from_hash, gamestate);
}

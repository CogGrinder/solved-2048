#pragma once
#include "types.hpp"
#include <cstdint>
#include <assert.h>
#include <optional>
#include <ctime>

#include <random> // for rand

class State {
public:
    // Dimensions are part of the type's definition, not the object's instance
    static constexpr int ROWS = BOARD_SIZE_ROWS;
    static constexpr int COLS = BOARD_SIZE_COLS;
    static constexpr int SIZE = ROWS * COLS;

    state_type data_;

    int8_t operator()(int r, int c) const;
    int8_t& operator()(int r, int c);
    friend bool operator==(const State& s1, const State& s2);

    State();
    State(const state_type& data);

    std::optional<State> player_move(action_type a) const;
    std::vector<Coord> all_nature_moves() const;
    std::optional<State> random_nature_move() const;

private:
    static constexpr size_t to_index(int r, int c);
    bool shift_up(int i, int j);
    bool shift_down(int i, int j);
    bool shift_left(int i, int j);
    bool shift_right(int i, int j);
};

// Outside of class
std::ostream& operator<<(std::ostream& os, const State& s);
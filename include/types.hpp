#pragma once
#include <vector>
#include <cstdint>
#include <iostream>

enum action_type { Up, Down, Left, Right, None};

inline std::ostream& operator<<(std::ostream& os, action_type action) {
    switch (action) {
        case Up: os << "Up"; break;
        case Down: os << "Down"; break;
        case Left: os << "Left"; break;
        case Right: os << "Right"; break;
        case None: os << "None"; break;
    }
    return os;
}

typedef std::vector<std::vector<int8_t>> state_type;
// Next implementation:
// typedef std::vector<int8_t> state_size_type;

typedef double reward_type;

struct Coord {
    int i;
    int j;
};
#pragma once
#include <vector>
#include <cstdint>
#include <iostream>

enum class Action : uint8_t {
    Up, Down, Left, Right, None
};

// Collection of all actions to iterate over
// Order is important so that None is last
// so that it is never chosen when other actions have the same best value
// Note : this is unlikely to happen but is good for safety and interpretability of the policy
namespace Actions {
    inline constexpr Action All[] = { Action::Up, Action::Down, Action::Left, Action::Right, Action::None };
}

typedef Action action_type;

inline std::ostream& operator<<(std::ostream& os, action_type action) {
    switch (action) {
        case Action::Up:    return os << "Up";
        case Action::Down:  return os << "Down";
        case Action::Left:  return os << "Left";
        case Action::Right: return os << "Right";
        case Action::None:  return os << "None";
        default:            return os << "Unknown Action";
    }
}

typedef std::vector<std::vector<int8_t>> state_type;
// Next implementation:
// typedef std::vector<int8_t> state_size_type;

typedef double reward_type;

struct Coord {
    int i;
    int j;
};
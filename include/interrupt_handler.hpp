#pragma once
#include <atomic>

// New namespace util
// TODO: will be used for utils.hpp/cpp as well
namespace util {
    // Globally accessible flag
    // Note: extern tells the compiler it's defined in the .cpp
    extern std::atomic<bool> global_stop_requested;

    // Call this once at the start of main()
    void setup_signal_handlers();
}
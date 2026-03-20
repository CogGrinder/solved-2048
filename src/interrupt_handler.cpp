#include "interrupt_handler.hpp"
#include <csignal>
#include <iostream>

namespace util {
    std::atomic<bool> global_stop_requested(false);

    void handle_interrupt_sigint([[maybe_unused]] int sig) {
        if (global_stop_requested.load()) {
            // If the user already pressed Ctrl+C, we force exit the program
            // to restore Ctrl+C functionality
            // Note: FUTURE: this skips cleanup 
            std::cerr << "\n[EMERGENCY EXIT] Force quitting..." << std::endl;
            std::exit(sig); 
        }
        global_stop_requested.store(true);
    }

    void setup_signal_handlers() {
        std::signal(SIGINT, handle_interrupt_sigint);
    }
}
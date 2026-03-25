# 2048 Backwards Induction MDP Solver v1.1

An optimized C++17 Backwards Induction solver for the game 2048 using Markov Decision Processes.
Performance
- 2x3 Board Calculation: 3.0s (previously 12.0s) — 4x Speedup.


## Build Instructions

Requires CMake 3.10+, and C++17 compatible compiler.

Example build script:
```bash
# Create build directory
mkdir build && cd build

# Configure
cmake .. -DCMAKE_BUILD_TYPE=Release -DBOARD_SIZE_ROWS=2 -DBOARD_SIZE_COLS=3
# Build
cmake --build .

# Run unit tests from build directory
./unit_tests
```

Gride size must be set at compile time. To change the grid size (as of V1.1, supported up to 3x3), create a new build folder and configure CMake:

``cmake .. -DCMAKE_BUILD_TYPE=[Release/Debug] -DBOARD_SIZE_ROWS=[rows] -DBOARD_SIZE_COLS=[columns]``

Example:
```bash
cmake .. -DCMAKE_BUILD_TYPE=Release -DBOARD_SIZE_ROWS=3 -DBOARD_SIZE_COLS=3
```

## Execution

Run the solver from build directory with optional parameters:

```./build/solver_2048 [winning_objective] [time_horizon]```

- ``winning_objective``: Target tile as power of 2 (e.g., 6 for 2^6=64). Default: 5.

- ``time_horizon``: Number of MDP steps. Default: Auto-calculated, leave blank for accurate values (to complete the backwards induction).

Example:
```bash
./build/solver_2048 6 10
```

## Features

- Signal Handling: interruption of policy computation via Ctrl+C (run gameloop simulation using optimal policy calcuted from current progress, second Ctrl+C force exit).

- Type safe enum class "action" types, written with aliased types for easily swappable memory implementation (single source of truth: ``types.hpp``).

- Validation: Integrated GoogleTest test suite (movement, hashes and creation of gamestates).
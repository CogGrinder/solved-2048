#pragma once
#include "utils.hpp"

// DEBUG macros

#ifdef DEBUG

#define PRINT_TILE(x) do {std::cout << #x << "= " << std::setw(2) << ( 2 << (x-1) )<< std::endl;} while(0)
#define PRINT(x) do {std::cout << #x << "= " << x << std::endl;} while(0)
#define OK() do {std::cout << " OK at " << __LINE__ << std::endl;} while(0)
#define PRINT_GAMESTATE(x) do {print_gamestate(x);} while(0)
#define PRINT_MOVE(a) do {print_move(a);} while(0)

#else
#define PRINT_TILE(x) do {} while(0)
#define PRINT(x) do {} while(0)
#define OK() do {} while(0)
#define PRINT_GAMESTATE(x) do {} while(0)
#define PRINT_MOVE(a) do {} while(0)

#endif
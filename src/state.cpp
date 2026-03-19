#include "state.hpp"
#include <iostream>
#include <iomanip>

/*
Class representing the game state on a board of dimensions ROWS x COLS,
where ROWS and COLS are compile-time constants defined in CMakeLists.txt.
*/

// Overloading () call for board_instance(i,j) access
// Getting with const
int8_t State::operator()(int r, int c) const {
    return this->data_[r][c];

    // Next implementation:
    // return data_[r * rows + c];
}
// Setting with non-const reference
int8_t& State::operator()(int r, int c) {
    return this->data_[r][c];

    // Next implementation:
    // return data_[r * rows + c];
}

// Overloading <<
std::ostream& operator<<(std::ostream& os, const State& s) {
    int rows = State::ROWS;
    int cols = State::COLS;
    // set this to true for easy test generation for copy and paste
    bool display_as_cpp_vectors = false;

    os << std::string(3*cols,'_') << std::endl;
    for (int i = 0; i < rows; i++)
    {
        if (i>0) {
            os << "\n";
        }
        if (display_as_cpp_vectors) {
            os << "{";
        }
        for (int j = 0; j < cols; j++)
        {
            if (display_as_cpp_vectors) {
                os << std::setw(2) << s(i, j)*1;
                if (j < cols-1) os << ",";
            } else {
                if (s(i, j) != 0) {
                    os << std::setw(2) << ( 2 << (s(i, j)-1) ) << " ";
                } else {
                    os << "   ";
                }
            }
        }
        if (display_as_cpp_vectors) {
            os << "}";
            if (i < rows-1) os << ",";
        }
    }
    os << std::endl;
    os << std::string(3*cols,'-') << std::endl;

    return os;
}

bool operator==(const State& s1, const State& s2) {
    return s1.data_ == s2.data_;
}

State::State() : data_(State::ROWS, std::vector<int8_t>(State::COLS, 0)){}
// Next implementation:
// data_ = std::vector<int8_t>(State::ROWS * State::COLS, 0);

State::State(const state_type& data) : data_(data) {}

constexpr size_t State::to_index(int r, int c) {
    // Professional tip: add an assertion here in Debug mode
    assert(r >= 0 && r < State::ROWS && c >= 0 && c < State::COLS);
    return static_cast<size_t>(r * State::COLS + c);
}

// shifting functions:

// move function:
// takes Move (Up, Down, Left, Right)
// eg Up: swipe up ie "compact" blocks upward until impact on top edge while prioritising top 2 blocks in case of 3 similar blocks

// Question: return legal moves ? Or just modified gamestate ?
// Since data_ is small, just copy the gamestate and modify it


/* shifting functions:
* for use in algorithms
* i and j are coordinates for tile to shift to (overwrites tile)
* in other words: removes element in row or column and shifts all others,
* adding 0 on the end
* 
* @returns:
*  bool: indicates wether non-zero values were shifted, used to test if move was legal
*/

inline bool State::shift_up(int i, int j){
    bool moved_non_zero_tile = false;
    int8_t new_value;
    for (int i_rewrite = i; i_rewrite < State::ROWS-1; i_rewrite++) {
        new_value = this->data_[i_rewrite+1][j];
        this->data_[i_rewrite][j] = new_value;
        if (new_value!=0) moved_non_zero_tile = true;
    }
    // insert a 0 at the end
    (*this)(State::ROWS-1,j) = 0;
    return moved_non_zero_tile;
}

inline bool State::shift_down(int i, int j){
    bool moved_non_zero_tile = false;
    int8_t new_value;
    for (int i_rewrite = i; i_rewrite > 0; i_rewrite--) {
        new_value = this->data_[i_rewrite-1][j];
        this->data_[i_rewrite][j] = new_value;
        if (new_value!=0) moved_non_zero_tile = true;
    }
    // insert a 0 at the end
    (*this)(0,j) = 0;
    return moved_non_zero_tile;
}

inline bool State::shift_left(int i, int j){
    bool moved_non_zero_tile = false;
    int8_t new_value;
    for (int j_rewrite = j; j_rewrite < State::COLS-1; j_rewrite++) {
        new_value = this->data_[i][j_rewrite+1];
        this->data_[i][j_rewrite] = new_value;
        if (new_value!=0) moved_non_zero_tile = true;
    }
    // insert a 0 at the end
    (*this)(i,State::COLS-1) = 0;
    return moved_non_zero_tile;
}

inline bool State::shift_right(int i, int j){
    bool moved_non_zero_tile = false;
    int8_t new_value;
    for (int j_rewrite = j; j_rewrite > 0; j_rewrite--) {
        new_value = this->data_[i][j_rewrite-1];
        this->data_[i][j_rewrite] = new_value;
        if (new_value!=0) moved_non_zero_tile = true;
    }
    // insert a 0 at the end
    (*this)(i,0) = 0;
    return moved_non_zero_tile;
}

// this part of a move is deterministic and is independent of Nature move
// revision: no longer changes gamestate
std::optional<State> State::player_move(action_type a) const {
    State state_new = *this; // copies current gamestate to explore the move

    bool is_valid_move = false;
    switch (a)
    {
    case Up:
        for (int j = 0; j < State::COLS; j++)
        {
            // on each column, first check for holes
            int i = 0;
            int number_of_zeros = 0;
            int8_t previous_non_zero_tile = 0;
            int previous_non_zero_tile_index = -1;

            // note: consecutive zeros may slow things down
            while (i < State::ROWS && number_of_zeros < State::ROWS)
            {
                // shifts due to adding tiles and "gravity"
                if (state_new(i,j)!=0) {
                    // check that previous tile was the same
                    if (previous_non_zero_tile==state_new(i,j)) {
                        // add tiles together
                        state_new(previous_non_zero_tile_index,j)++;
                        state_new.shift_up(i, j);
                        // reset previous_non_zero_tile
                        // (cannot add tiles to the same tile in the same turn)
                        previous_non_zero_tile = 0;
                        is_valid_move=true;
                    } else {
                        previous_non_zero_tile=state_new(i,j);
                        previous_non_zero_tile_index = i;
                        // move to next tile
                        i++;
                    }
                } else {
                    // "gravity" shift on top of blank space
                    if (state_new.shift_up(i, j)) is_valid_move=true;
                    number_of_zeros++;
                }
            }
        }
        break;
    case Down:
        for (int j = 0; j < State::COLS; j++)
        {
            // on each column, first check for holes
            int i = State::ROWS-1;
            int number_of_zeros = 0;
            int8_t previous_non_zero_tile = 0;
            int previous_non_zero_tile_index = -1;

            // note: consecutive zeros may slow things down
            while (i >= 0 && number_of_zeros < State::ROWS)
            {
                // shifts due to adding tiles and "gravity"
                if (state_new(i,j)!=0) {
                    // check that previous tile was the same
                    if (previous_non_zero_tile==state_new(i,j)) {
                        // add tiles together
                        state_new(previous_non_zero_tile_index,j)++;
                        state_new.shift_down(i, j);
                        previous_non_zero_tile = 0;
                        is_valid_move=true;
                    } else {
                        previous_non_zero_tile=state_new(i,j);
                        previous_non_zero_tile_index = i;
                        // move to next tile
                        i--;
                    }
                } else {
                    // "gravity" shift on top of blank space
                    if (state_new.shift_down(i, j)) is_valid_move=true;
                    number_of_zeros++;
                }
            }
        }
        break;
    case Left:
        for (int i = 0; i < State::ROWS; i++)
        {
            // on each column, first check for holes
            int j = 0;
            int number_of_zeros = 0;
            int8_t previous_non_zero_tile = 0;
            int previous_non_zero_tile_index = -1;

            // note: consecutive zeros may slow things down
            while (j < State::COLS && number_of_zeros < State::COLS)
            {
                // shifts due to adding tiles and "gravity"
                if (state_new(i,j)!=0) {
                    // check that previous tile was the same
                    if (previous_non_zero_tile==state_new(i,j)) {
                        // add tiles together
                        state_new(i,previous_non_zero_tile_index)++;
                        state_new.shift_left(i, j);
                        previous_non_zero_tile = 0;
                        is_valid_move=true;
                    } else {
                        previous_non_zero_tile=state_new(i,j);
                        previous_non_zero_tile_index = j;
                        // move to next tile
                        j++;
                    }
                } else {
                    // "gravity" shift on top of blank space
                    if (state_new.shift_left(i, j)) is_valid_move=true;
                    number_of_zeros++;
                }
            }
        }
        break;
    case Right:
        for (int i = 0; i < State::ROWS; i++)
        {
            // on each column, first check for holes
            int j = State::COLS-1;
            int number_of_zeros = 0;
            int8_t previous_non_zero_tile = 0;
            int previous_non_zero_tile_index = -1;

            // note: consecutive zeros may slow things down
            while (j >= 0 && number_of_zeros < State::COLS)
            {
                // shifts due to adding tiles and "gravity"
                if (state_new(i,j)!=0) {
                    // check that previous tile was the same
                    if (previous_non_zero_tile==state_new(i,j)) {
                        // add tiles together
                        state_new(i,previous_non_zero_tile_index)++;
                        state_new.shift_right(i, j);
                        previous_non_zero_tile = 0;
                        is_valid_move=true;
                    } else {
                        previous_non_zero_tile=state_new(i,j);
                        previous_non_zero_tile_index = j;
                        // move to next tile
                        j--;
                    }
                } else {
                    // "gravity" shift on top of blank space
                    if (state_new.shift_right(i, j)) is_valid_move=true;
                    number_of_zeros++;
                }
            }
        }
        break;
    
    default:
        break;
    }

    return is_valid_move? std::optional<State>(state_new) : std::nullopt;
    
}



std::vector<Coord> State::all_nature_moves() const {
    int rows = State::ROWS;
    int cols = State::COLS;
    std::vector<Coord> list_of_empty_tiles;
    for (int i = 0; i < rows; i++)
    {
        for (int j = 0; j < cols; j++)
        {
        if ((*this)(i,j)==0)
        {
            list_of_empty_tiles.push_back({i,j});
        }        
        }
    }

    return list_of_empty_tiles;
}

std::optional<State> State::random_nature_move() const{
    std::vector<Coord> list_of_empty_tiles = State::all_nature_moves();
    // Nature move is valid if there are still available tiles
    bool is_valid_move = list_of_empty_tiles.size()>0;
    
    
    if (is_valid_move)
    {
        State new_state = *this;
        srand((unsigned) time(NULL) + list_of_empty_tiles.size()); 
        Coord chosen_square = list_of_empty_tiles[rand()%list_of_empty_tiles.size()];
        int8_t new_value = rand()%2 + 1;

        new_state(chosen_square.i, chosen_square.j) = new_value;
        return std::optional<State>(new_state);
    }
    else return std::nullopt;
}
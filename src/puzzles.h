#pragma once

#include <cstdint>
#include <string_view>

#define PUZZLE(name)  \
  extern char _binary_puzzles_##name##_txt_start;  \
  extern char _binary_puzzles_##name##_txt_end;  \
  inline const std::string_view kPuzzle##name{  \
      &_binary_puzzles_##name##_txt_start,  \
      static_cast<std::size_t>(&_binary_puzzles_##name##_txt_end -  \
                               &_binary_puzzles_##name##_txt_start)}

PUZZLE(1);

#undef PUZZLE

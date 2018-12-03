#!/bin/bash

TEMP_DIR="$(mktemp -d)"
function cleanup {
  rm -r $TEMP_DIR
}
trap cleanup EXIT

cp src/*.* "$TEMP_DIR"

# Build puzzles.h from all available puzzles in puzzles/*.txt
cat >"$TEMP_DIR/puzzles.h" <<EOF
#include <cstdint>
#include <string_view>

#define PUZZLE(name)  \\
  extern char _binary_puzzles_##name##_txt_start;  \\
  extern char _binary_puzzles_##name##_txt_end;  \\
  inline const std::string_view kPuzzle##name{  \\
      &_binary_puzzles_##name##_txt_start,  \\
      static_cast<std::size_t>(&_binary_puzzles_##name##_txt_end -  \\
                               &_binary_puzzles_##name##_txt_start)}
EOF

for puzzle in puzzles/*.txt; do
  puzzle_id="$(basename --suffix=.txt "$puzzle")"
  objcopy -I binary -O elf64-x86-64 -B i386  \
          "$puzzle" "$TEMP_DIR/puzzle$puzzle_id.o"
  echo "PUZZLE($puzzle_id);" >>"$TEMP_DIR/puzzles.h"
done

# Build the main file from all available solutions in day*.h
SOLUTIONS="$(grep -oh 'Solve[0-9][AB]' "$TEMP_DIR"/day*.h | sort -ui)"

for day in "$TEMP_DIR"/day*.h; do
  echo "#include \"$(basename "$day")\"" >> "$TEMP_DIR/main.cc"
done

cat >>"$TEMP_DIR/main.cc" <<EOF

#include "timing.h"
#include "puzzles.h"

#include <iostream>

int main() {
  std::cout
EOF

for solution in $SOLUTIONS; do
  echo "    << \"$solution: \" << Time($solution) << \"\\n\""  \
      >>"$TEMP_DIR/main.cc"
done

cat >>"$TEMP_DIR/main.cc" <<EOF
  ;
}
EOF

# Compile each source file.
CXX="clang++ -stdlib=libc++"
CXXFLAGS=(
  -std=c++17
  "-I$TEMP_DIR"
  -Ofast
)

function compile {
  echo "Compiling $2"
  ${CXX} "${CXXFLAGS[@]}" -c "$1" -o "$2"
}
compile "$TEMP_DIR/main.cc" "$TEMP_DIR/main.o"

for day in "$TEMP_DIR"/day*.cc; do
  day_id="$(basename --suffix=.cc "$day")"
  compile "$day" "$TEMP_DIR/$day_id.o"
done

# Link the full program.
echo "Linking $1"
${CXX} "${CXXFLAGS[@]}" "$TEMP_DIR"/*.o -o "$1"

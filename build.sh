#!/bin/bash

INPUT_DIR="$PWD"
TEMP_DIR="$(mktemp -d)"
echo "Building in $TEMP_DIR."
cd "$TEMP_DIR"
function cleanup {
  cd "$INPUT_DIR"
  rm -r "$TEMP_DIR"
}
trap cleanup EXIT

mkdir obj
cp -r "$INPUT_DIR/"{puzzles,src} .

# Build puzzles.h from all available puzzles in puzzles/*.txt
cat >src/puzzles.h <<EOF
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
          "$puzzle" "obj/puzzle$puzzle_id.o"
  echo "PUZZLE($puzzle_id);" >>src/puzzles.h
done

# Build the main file from all available solutions in day*.h
SOLUTIONS="$(grep -oh 'Solve[0-9][AB]' src/day*.h | sort -ui)"

for day in src/day*.h; do
  echo "#include \"$(basename "$day")\"" >> src/main.cc
done

cat >> src/main.cc <<EOF

#include "timing.h"
#include "puzzles.h"

#include <iostream>

int main() {
  std::cout
EOF

for solution in $SOLUTIONS; do
  echo "    << \"$solution: \" << Time($solution) << \"\\n\""  \
      >> src/main.cc
done

cat >> src/main.cc <<EOF
  ;
}
EOF

# Compile each source file.
CXX="clang++ -stdlib=libc++"
CXXFLAGS=(
  -std=c++17
  "-I$TEMP_DIR"
  -Ofast
  -ffunction-sections
  -fdata-sections
  -flto
)
LDFLAGS=(
  -Wl,--gc-sections
  -s
)

function compile {
  echo "Compiling $2"
  ${CXX} "${CXXFLAGS[@]}" -c "$1" -o "$2"
}
compile "src/main.cc" "obj/main.o" &

for day in src/day*.cc; do
  day_id="$(basename --suffix=.cc "$day")"
  compile "$day" "obj/$day_id.o" &
done

wait

# Link the full program.
echo "Linking $1"
${CXX} "${CXXFLAGS[@]}" "${LDFLAGS[@]}" obj/*.o -o "$INPUT_DIR/$1"

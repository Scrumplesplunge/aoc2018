#!/bin/bash

if [[ -z $1 ]] || [[ -z $2 ]]; then
  >&2 echo "Usage: ./build.sh [debug|release] <output_name>"
  exit 1
fi

MODE="$1"
OUTPUT="$2"

CXX="clang++ -stdlib=libc++"
if [[ ! -d /usr/include/c++/v1 ]]; then
  echo 'No libc++ headers detected. Reverting to libstdc++.'
  CXX="clang++"
fi

CXXFLAGS=(
  -std=c++17
  "-I$TEMP_DIR"
)
LDFLAGS=()

if [[ "$MODE" == debug ]]; then
  CXXFLAGS+=(
    -g
  )
else
  CXXFLAGS+=(
    -Ofast
    -DNDEBUG
    -ffunction-sections
    -fdata-sections
    -flto
  )
  LDFLAGS+=(
    -Wl,--gc-sections
    -s
  )
fi

function compile {
  >&2 echo "${CXX} ${CXXFLAGS[@]} -c $1 -o $2"
  ${CXX} "${CXXFLAGS[@]}" -c "$1" -o "$2"
}

INPUT_DIR="$PWD"
TEMP_DIR="$(mktemp -d)"
>&2 echo "Building in $TEMP_DIR."
cd "$TEMP_DIR"
function cleanup {
  cd "$INPUT_DIR"
  rm -r "$TEMP_DIR"
}
trap cleanup EXIT

mkdir obj
cp -r "$INPUT_DIR/"{puzzles,src} .

# Build puzzles.h from all available puzzles in puzzles/*.txt
>&2 echo 'Generating puzzles.h..'
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

LDFLAGS+=(-Wl,--format=binary)
for puzzle in puzzles/*.txt; do
  puzzle_id="$(basename --suffix=.txt "$puzzle")"
  >&2 echo "Found puzzle $puzzle_id"
  echo "PUZZLE($puzzle_id);" >>src/puzzles.h
  LDFLAGS+=(-Wl,"$puzzle")
done
LDFLAGS+=(-Wl,--format=default)

# Try to compile each day, include any which compile.
DAYS=($(
  find src -name 'day*.cc' |
  sort -ui |
  while read filename; do
    day_id="$(basename --suffix=.cc "$filename")"
    if compile "$filename" "obj/$day_id.o"; then
      # Compile succeeded. Include the solvers.
      >&2 echo "Including $filename"
      echo "$filename"
    else
      >&2 echo "Failed to compile $filename. Will exclude it."
    fi
  done | sort -ui
))

cat >> src/main.cc <<EOF

#include "timing.h"
#include "puzzles.h"

#include <cassert>
#include <iostream>

EOF

grep -ohP '^.*\bSolve[0-9]+[AB]\(\)' "${DAYS[@]}" |
while read solution; do
  echo "$solution;" >> src/main.cc
done

cat src/allocation.cc >> src/main.cc

cat >> src/main.cc <<EOF
int main() {
  std::cout
EOF

grep -ohP '\bSolve[0-9]+[AB]\b' "${DAYS[@]}" | sort -ui |
while read solution; do
  echo "    << \"$solution: \" << Time($solution) << \"\\n\"" >> src/main.cc
done

cat >> src/main.cc <<EOF
  ;
  dump_allocation_stats();
}
EOF

if ! compile "src/main.cc" "obj/main.o"; then
  echo "Oh bollocks, the generated main.cc is invalid:"
  cat src/main.cc
  exit 1
fi

# Link the full program.
>&2 echo "Linking $OUTPUT"
>&2 echo "${CXX} ${CXXFLAGS[@]} ${LDFLAGS[@]} obj/*.o -o $INPUT_DIR/$OUTPUT"
${CXX} "${CXXFLAGS[@]}" "${LDFLAGS[@]}" obj/*.o -o "$INPUT_DIR/$OUTPUT"

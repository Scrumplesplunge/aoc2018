#!/bin/bash

if [[ -z $1 ]] || [[ -z $2 ]]; then
  >&2 echo "Usage: ./build.sh [debug|release] <output_name>"
  exit 1
fi

MODE="$1"
OUTPUT="$2"

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

# Discover the parameters for objcopy.
echo 'int main(){}' > info_check.c
gcc -c info_check.c -o info_check.o
info="$(objdump -f info_check.o)"
format="$(grep -oP '(?<=file format )[^\s]+' <<< "$info")"
arch="$(grep -oP '(?<=architecture: )[^,]+' <<< "$info")"
>&2 echo "Deduced format=$format, arch=$arch."

for puzzle in puzzles/*.txt; do
  puzzle_id="$(basename --suffix=.txt "$puzzle")"
  objcopy -I binary -O "$format" -B "$arch" "$puzzle" "obj/puzzle$puzzle_id.o"
  echo "PUZZLE($puzzle_id);" >>src/puzzles.h
done

# Compile each source file.
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
  echo "Compiling $2"
  ${CXX} "${CXXFLAGS[@]}" -c "$1" -o "$2"
}

for day in src/day*.cc; do
  day_id="$(basename --suffix=.cc "$day")"
  compile "$day" "obj/$day_id.o" &
done
wait

# Build the main file from all available solutions.
SOLUTIONS="$(
  for day in obj/day*.o; do
    day_id="$(basename --suffix=.o "$day")"
    cat "src/$day_id.cc"
  done |
  grep -ohP '^.*\bSolve[0-9]+[AB]\(\)'
)"

cat >> src/main.cc <<EOF

#include "timing.h"
#include "puzzles.h"

#include <iostream>

$(
  echo "$SOLUTIONS" |
  while read solution; do
    echo "$solution;"
  done
)

$(cat src/allocation.cc)

int main() {
  std::cout $(
    grep -oP '\bSolve[0-9]+[AB]\b' <<< "$SOLUTIONS" |
    sort -gk 1.6 |
    uniq |
    while read solution; do
      echo "    << \"$solution: \" << Time($solution) << \"\\n\""
    done
  );
  dump_allocation_stats();
}
EOF

compile src/main.cc obj/main.o

# Link the full program.
echo "Linking $OUTPUT"
${CXX} "${CXXFLAGS[@]}" "${LDFLAGS[@]}" obj/*.o -o "$INPUT_DIR/$OUTPUT"

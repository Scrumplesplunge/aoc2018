# Advent of Code 2018 Solutions

**SPOILERS, DUH**

The puzzle input for each day is in `puzzles/X.txt`, while the solution is in
`src/dayX.cc`. The script `build.sh` embeds the puzzles into the program and
provides a `std::string_view kPuzzleX` for each puzzle.

    $ make
    ./build.sh solve
    ./build.sh solve
    Compiling /tmp/tmp.H3n0nQrXax/main.o
    Compiling /tmp/tmp.H3n0nQrXax/day1.o
    Compiling /tmp/tmp.H3n0nQrXax/day2.o
    Compiling /tmp/tmp.H3n0nQrXax/day3.o
    Linking solve
    $ ./solve
    Solve1A: ??? in 683us
    Solve1B: ??? in 25ms
    Solve2A: ??? in 1325us
    ...

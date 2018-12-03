include config.mk
include rules.mk

.PHONY: all

INPUTS=$(wildcard puzzles/*.txt)
PUZZLE_O=${INPUTS:puzzles/%.txt=build/puzzle%.o}

# Embed all the input files as objects.
build/puzzle%.o: puzzles/%.txt
	@echo Embedding puzzle$*
	@mkdir -p build
	@objcopy -I binary -O elf64-x86-64 -B i386 $^ $@

BINARIES =  \
	main
all: $(patsubst %, bin/%, ${BINARIES})

MAIN_DEPS =  \
	day1  \
	day2  \
	day3  \
	puzzle1  \
	puzzle2  \
	puzzle3  \
	main
bin/main: $(patsubst %, build/%.o, ${MAIN_DEPS})

-include ${DEPENDS}

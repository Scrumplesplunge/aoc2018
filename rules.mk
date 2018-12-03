MAKEFILES = Makefile rules.mk config.mk

.PHONY: default
default: opt

# Build with optimized configuration.
.PHONY: opt
opt: all

# Build with debug configuration.
.PHONY: debug
debug: all

.PHONY: clean
clean:
	@echo Removing output directories
	@rm -rf {build,bin}

# Pattern rule for generating a binary.
bin/%: | ${MAKEFILES}
	@echo Linking $*
	@mkdir -p $(dir $@)
	@${CXX} ${LDFLAGS} $^ -o $@ ${LDLIBS}

# Pattern rule for compiling a cc file into an o file.
build/%.o: src/%.cc $(wildcard src/%.h) | ${MAKEFILES}
	@echo Compiling $*
	@mkdir -p build/$(dir $*)
	@${CXX} ${CXXFLAGS} ${CPPFLAGS} -MMD -MF build/$*.d $< -c -o build/$*.o

DEPENDS = $(shell [[ -d build/ ]] && find build -name '*.d')

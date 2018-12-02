CXX = clang++

CXXFLAGS += -std=c++17 -stdlib=libc++ -Wall -Wextra -Werror -pedantic
opt: CXXFLAGS += -ffunction-sections -fdata-sections -flto -Ofast -march=native
debug: CXXFLAGS += -O0 -g

LDFLAGS += -fuse-ld=gold -stdlib=libc++
opt: LDFLAGS += -s -Wl,--gc-sections -flto -Ofast

LDLIBS = -lpthread

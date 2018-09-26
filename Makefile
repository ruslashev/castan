warnings = -Wall -Wextra -Wno-unused-parameter -Wno-unused-variable \
           -Wdouble-promotion -Wformat=2 -Wdisabled-optimization \
           -Wsuggest-override -Wlogical-op -Wtrampolines

flags = -std=c++1y -lSDL2 -g -O3 -ffast-math \
        -funsafe-math-optimizations # FUN, safe math optimizations

default:
	g++ main.cc framebuffer.cc state.cc -o castan $(flags) $(warnings)
	./castan


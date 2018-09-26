warnings = -Wall -Wextra -Wno-unused-parameter -Wno-unused-variable \
           -Wdouble-promotion -Wformat=2 -Wdisabled-optimization \
           -Wsuggest-override -Wlogical-op -Wtrampolines

default:
	g++ main.cc framebuffer.cc state.cc -o castan -std=c++1y -lSDL2 -g $(warnings)
	./castan


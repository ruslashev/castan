default:
	g++ main.cc framebuffer.cc state.cc -o castan -std=c++1y -lSDL2 -g
	./castan


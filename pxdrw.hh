#pragma once

#include <SDL2/SDL.h>
#include <memory>

class pixeldrawer
{
  SDL_Window *window;
  SDL_Renderer *renderer;
  SDL_Texture *texture;
  std::unique_ptr<uint32_t> data;

  void resize();
public:
  pixeldrawer(int wwidth, int wheight);
  ~pixeldrawer();

  int wwidth, wheight;

  void draw();
  void write(int x, int y, uint32_t color);
  void clear();
  void mainloop(void (*update_cb)(double, uint32_t),
      void (*draw_cb)(pixeldrawer*));
};


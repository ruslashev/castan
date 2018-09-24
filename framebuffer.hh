#pragma once

#include "state.hh"
#include <SDL2/SDL.h>
#include <memory>

class framebuffer
{
  SDL_Window *_window;
  SDL_Renderer *_renderer;
  SDL_Texture *_texture;
  std::vector<uint32_t> _data;
  int _width, _height;

  double _get_time_in_seconds();
  void _resize();
  void _draw() const;
public:
  framebuffer(int width, int height);
  ~framebuffer();
  int get_width() const;
  int get_height() const;
  void write(int x, int y, uint32_t color);
  void draw_vert_line(int x, int h, uint32_t color);
  void draw_square(int x, int y, int size, uint32_t color);
  void clear();
  void mainloop(bool *running, const state_t &initial,
      void (*update_cb)(state_t*, double, uint32_t),
      void (*render_cb)(framebuffer*, const state_t&));
};


#pragma once

#include "state.hh"
#include <SDL2/SDL.h>
#include <memory>

class framebuffer
{
  SDL_Window *_window;
  SDL_Renderer *_renderer;
  SDL_Texture *_texture;
  std::unique_ptr<uint32_t[]> _data;
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
  void clear();
  void mainloop(bool *running, void (*update_cb)(double, uint32_t),
      void (*render_cb)(framebuffer*, const state_t&));
};


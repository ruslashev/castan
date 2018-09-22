#pragma once

#include <SDL2/SDL.h>
#include <memory>

class framebuffer
{
  SDL_Window *_window;
  SDL_Renderer *_renderer;
  SDL_Texture *_texture;
  std::unique_ptr<uint32_t[]> _data;
  int _width, _height;

  void _resize();
public:
  framebuffer(int width, int height);
  ~framebuffer();
  int get_width() const;
  int get_height() const;
  void draw();
  void write(int x, int y, uint32_t color);
  void clear();
  void mainloop(void (*update_cb)(double, uint32_t),
      void (*draw_cb)(framebuffer*));
};


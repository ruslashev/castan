#include "pxdrw.hh"
#include "utils.hh"
#include <memory>

void framebuffer::_resize()
{
  if (_window)
    SDL_DestroyWindow(_window);

  _window = SDL_CreateWindow("castan", SDL_WINDOWPOS_UNDEFINED,
      SDL_WINDOWPOS_UNDEFINED, _width, _height, SDL_WINDOW_SHOWN);

  if (_window == NULL)
    die("Failed to create _window: %s", SDL_GetError());

  if (_renderer)
    SDL_DestroyRenderer(_renderer);

  _renderer = SDL_CreateRenderer(_window, -1, SDL_RENDERER_ACCELERATED);

  if (_renderer == NULL)
    die("Failed to create _renderer: %s", SDL_GetError());

  if (_texture)
    SDL_DestroyTexture(_texture);

  _texture = SDL_CreateTexture(_renderer, SDL_PIXELFORMAT_RGBA8888,
      SDL_TEXTUREACCESS_STREAMING, _width, _height);

  _data = std::make_unique<uint32_t[]>(_width * _height);
}

framebuffer::framebuffer(int width, int height)
  : _window(nullptr)
  , _renderer(nullptr)
  , _texture(nullptr)
  , _width(width)
  , _height(height)
{
  if (SDL_Init(SDL_INIT_VIDEO) < 0)
    die("Failed to initialize SDL: %s", SDL_GetError());

  _resize();
}

int framebuffer::get_width() const
{
  return _width;
}

int framebuffer::get_height() const
{
  return _height;
}

void framebuffer::draw()
{
  SDL_UpdateTexture(_texture, NULL, _data.get(), _width * sizeof(uint32_t));
  SDL_RenderClear(_renderer);
  SDL_RenderCopy(_renderer, _texture, NULL, NULL);
  SDL_RenderPresent(_renderer);
}

void framebuffer::write(int x, int y, uint32_t color)
{
  if (x < 0 || x > _width || y < 0 || y > _height)
    return;
  _data.get()[y * _width + x] = (color << 8) + 0xFF; // XXX
}

void framebuffer::clear()
{
  for (int y = 0; y < _height; y++)
    for (int x = 0; x < _width; x++)
      write(x, y, 0);
}

void framebuffer::mainloop(void (*update_cb)(double, uint32_t),
    void (*draw_cb)(framebuffer*)) {
  extern bool running;
  uint32_t simtime = 0;

  while (running) {
    uint32_t realtime = SDL_GetTicks();

    while (simtime < realtime) {
      simtime += 16;

      update_cb(16. / 1000., simtime);
    }
    clear();

    draw_cb(this);

    draw();
  }
}

framebuffer::~framebuffer()
{
  SDL_DestroyTexture(_texture);
  SDL_DestroyRenderer(_renderer);
  SDL_DestroyWindow(_window);
  SDL_Quit();
}


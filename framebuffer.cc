#include "framebuffer.hh"
#include "state.hh"
#include "utils.hh"
#include <memory>
#include <chrono>

double framebuffer::_get_time_in_seconds() {
  return SDL_GetTicks() / 1000.;
}

void framebuffer::_resize()
{
  if (_window)
    SDL_DestroyWindow(_window);

  _window = SDL_CreateWindow("castan", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, _width,
      _height, SDL_WINDOW_SHOWN);

  if (_window == NULL)
    die("Failed to create _window: %s", SDL_GetError());

  if (_renderer)
    SDL_DestroyRenderer(_renderer);

  _renderer = SDL_CreateRenderer(_window, -1, SDL_RENDERER_ACCELERATED);

  if (_renderer == NULL)
    die("Failed to create _renderer: %s", SDL_GetError());

  if (_texture)
    SDL_DestroyTexture(_texture);

  _texture = SDL_CreateTexture(_renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING,
      _width, _height);

  _data.resize(_width * _height, 0);
}

void framebuffer::_draw() const
{
  SDL_UpdateTexture(_texture, NULL, _data.data(), _width * sizeof(uint32_t));
  SDL_RenderClear(_renderer);
  SDL_RenderCopy(_renderer, _texture, NULL, NULL);
  SDL_RenderPresent(_renderer);
}

framebuffer::framebuffer(int width, int height)
  : _window(nullptr),
    _renderer(nullptr),
    _texture(nullptr),
    _width(width),
    _height(height)
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

void framebuffer::write(int x, int y, uint32_t color)
{
  if (x < 0 || x > _width || y < 0 || y > _height)
    return;
  _data[y * _width + x] = (color << 8) + 0xFF;
}

void framebuffer::draw_vert_line(int x, int h, uint32_t color) {
  for (int y = 0; y < h; ++y)
    write(x, _height / 2 - h / 2 + y, color);
}

void framebuffer::draw_square(int x, int y, int size, uint32_t color) {
  for (int dy = 0; dy < size; dy++)
    for (int dx = 0; dx < size; dx++)
      write(x + dx, y + dy, color);
}

void framebuffer::clear()
{
  std::fill(_data.begin(), _data.end(), 0);
}

void framebuffer::mainloop(bool *running, const state_t &initial,
      void (*update_cb)(state_t*, double, uint32_t),
      void (*render_cb)(framebuffer*, const state_t&))
{
  const int ticks_per_second = 60, max_update_ticks = 15;
  double t = 0, dt = 1. / ticks_per_second, current_time = _get_time_in_seconds(), accumulator = 0;

  state_t previous = initial, current = initial, draw;

  uint64_t frame = 0;

  while (*running) {
    double real_time = _get_time_in_seconds(), elapsed = real_time - current_time;
    elapsed = std::min(elapsed, max_update_ticks * dt);
    current_time = real_time;
    accumulator += elapsed;

    while (accumulator >= dt) {
      previous = current;
      update_cb(&current, dt, t);
      current.integrate(t, dt);
      t += dt;
      accumulator -= dt;
    }

    clear();

    state_lerp(&draw, previous, current, accumulator / dt);

    auto draw_begin_w = std::chrono::high_resolution_clock::now();
    std::clock_t draw_begin_c = std::clock();
    double draw_begin_s = _get_time_in_seconds();

    render_cb(this, draw);

    auto draw_end_w = std::chrono::high_resolution_clock::now();
    std::clock_t draw_end_c = std::clock();
    double draw_end_s = _get_time_in_seconds();

    std::chrono::duration<double, std::milli> draw_duration_w = draw_end_w - draw_begin_w;
    double draw_duration_c = ((float)(draw_end_c - draw_begin_c) / CLOCKS_PER_SEC) * 1000.f;
    double draw_duration_s = (draw_end_s - draw_begin_s) * 1000.;

    _draw();

    ++frame;

    char title[256];
    snprintf(title, 256, "w %7.2f c %7.2f s %7.2f", draw_duration_w.count(), draw_duration_c,
        draw_duration_s);
    SDL_SetWindowTitle(_window, title);
  }
}

framebuffer::~framebuffer()
{
  SDL_DestroyTexture(_texture);
  SDL_DestroyRenderer(_renderer);
  SDL_DestroyWindow(_window);
  SDL_Quit();
}


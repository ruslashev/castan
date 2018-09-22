#include "pxdrw.hh"
#include "utils.hh"

pixeldrawer::pixeldrawer(int wwidth, int wheight) : wwidth(wwidth),
  wheight(wheight)
{
  window = NULL;
  renderer = NULL;
  texture = NULL;

  assert(SDL_Init(SDL_INIT_VIDEO) >= 0, "Failed to initialize SDL: %s",
      SDL_GetError());

  resize();
}

void pixeldrawer::resize()
{
  if (window)
    SDL_DestroyWindow(window);

  window = SDL_CreateWindow("vfk", SDL_WINDOWPOS_UNDEFINED,
      SDL_WINDOWPOS_UNDEFINED, wwidth, wheight, SDL_WINDOW_SHOWN);

  assert(window != NULL, "Failed to create Window: %s", SDL_GetError());

  if (renderer)
    SDL_DestroyRenderer(renderer);

  renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

  assert(renderer != NULL, "Failed to create Renderer: %s",
      SDL_GetError());

  if (texture)
    SDL_DestroyTexture(texture);

  texture = SDL_CreateTexture(renderer,
      SDL_PIXELFORMAT_RGBA8888,
      SDL_TEXTUREACCESS_STREAMING, wwidth, wheight);

  data = std::unique_ptr<uint32_t>(new uint32_t [wwidth * wheight]);
}

void pixeldrawer::draw()
{
  SDL_UpdateTexture(texture, NULL, data.get(), wwidth * sizeof(uint32_t));
  SDL_RenderClear(renderer);
  SDL_RenderCopy(renderer, texture, NULL, NULL);
  SDL_RenderPresent(renderer);
}

void pixeldrawer::write(int x, int y, uint32_t color)
{
  if (x < 0 || x > wwidth || y < 0 || y > wheight)
    return;
  data.get()[y * wwidth + x] = (color << 8) + 0xFF; // XXX
}

void pixeldrawer::clear()
{
  for (int y = 0; y < wheight; y++)
    for (int x = 0; x < wwidth; x++)
      write(x, y, 0);
}

void pixeldrawer::mainloop(void (*update_cb)(double, uint32_t),
    void (*draw_cb)(pixeldrawer*)) {
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

pixeldrawer::~pixeldrawer()
{
  SDL_DestroyTexture(texture);
  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  SDL_Quit();
}


#include "framebuffer.hh"
#include "utils.hh"
#include <fstream>

static int tilecolor(int t)
{
  switch (t) {
    case -1: return 0xFFFF00;
    case  1: return 0xFFFFFF;
    case  2: return 0xFF0000;
    case  3: return 0x00FF00;
    case  4: return 0x0000FF;
    case  5: return 0xFF00FF;
    case  6: return 0x444444;
    default: return 0xFF00FF;
  }
}

static const int mapsz = 10;
static const int map[mapsz][mapsz] = {
  {1,1,1,2,3,4,2,1,1,1},
  {1,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,0,0,0,5},
  {1,0,0,0,0,0,0,0,0,5},
  {1,0,0,0,0,0,0,0,0,6},
  {1,0,0,0,0,0,0,0,0,5},
  {1,0,0,0,0,0,0,0,0,5},
  {1,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,0,0,0,1},
  {1,1,5,2,3,4,2,1,1,1},
};
static double playerx = 30, playery = 40, playerang = 0;
static const double tilesize = 10;
static const double plyspeed = 30, plyturnspeed = 100;
static double fov = 60;
static bool running = true;

static int sign(double x)
{
  if (abs(x) < 1e-5)
    return 0;
  if (x < 0)
    return -1;
  else
    return 1;
}

static double to_rads(double degs)
{
  return degs * (M_PI / 180.0);
}

static void update(double dt, uint32_t t)
{
  SDL_Event event;
  while (SDL_PollEvent(&event) != 0) {
    if (event.type == SDL_QUIT)
      running = false;
    else if (event.type == SDL_KEYDOWN || event.type == SDL_KEYUP) {
      uint8_t *keystates = (uint8_t*)SDL_GetKeyboardState(nullptr);
      int fw = keystates[SDL_SCANCODE_W] - keystates[SDL_SCANCODE_S];
      int side = keystates[SDL_SCANCODE_D] - keystates[SDL_SCANCODE_A];
      playerx += fw * cos(to_rads(playerang)) * plyspeed * dt;
      playery += fw * sin(to_rads(playerang)) * plyspeed * dt;
      playerang += side * plyturnspeed * dt;
    }
  }
}

static int getmap(int x, int y)
{
  return (x < 0 || y < 0 || x > mapsz - 1 || y > mapsz - 1) ? -1 : map[y][x];
}

static void draw_minimap(framebuffer *pd, const state_t &draw)
{
  const int offset = 5, scale = 5;
  int plx = offset + (playerx / tilesize) * scale,
      ply = offset + (playery / tilesize) * scale;

  for (int y = 0; y < mapsz; ++y)
    for (int x = 0; x < mapsz; ++x)
      if (map[y][x])
        pd->draw_square(offset + x * scale, offset + y * scale, scale, tilecolor(map[y][x]));

  pd->draw_square(plx - 1, ply - 1, 3, 0xAAAAAA);

  for (int i = 0; i < 70; i++) {
    pd->write(round(plx + i * cos(to_rads(playerang - fov / 1.0))),
        round(ply + i * sin(to_rads(playerang - fov / 1.0))), 0x00AA00);
    pd->write(round(plx + i * cos(to_rads(playerang + fov / 1.0))),
        round(ply + i * sin(to_rads(playerang + fov / 1.0))), 0x00AA00);
  }

  for (int i = 0; i < 10; i++)
    pd->write(round(plx + i * cos(to_rads(playerang))),
        round(ply + i * sin(to_rads(playerang))),
        0xFF0000);
}

static void render(framebuffer *pd, const state_t &draw) {
  draw_minimap(pd, draw);

  for (int x = 0; x < pd->get_width(); x++) {
    const double screenxnorm = ((double)x / pd->get_width()) * 2.0 - 1.0;
    double thisrayang = playerang + screenxnorm * fov;
    double dirx = cos(to_rads(thisrayang)), diry = sin(to_rads(thisrayang));
    int mapx = playerx / tilesize, mapy = playery / tilesize;
    double ddx = abs(1 / dirx), ddy = abs(1 / diry);
    int stepx = sign(dirx), stepy = sign(diry);
    double raydifx = mapx - playerx / tilesize, raydify = mapy - playery / tilesize;
    double sidedx = (sign(dirx) * raydifx + sign(dirx) * 0.5 + 0.5) * ddx,
           sidedy = (sign(diry) * raydify + sign(diry) * 0.5 + 0.5) * ddy;
    bool maskx, masky;

    for (int i = 0; i < 1e3; i++) {
      if (getmap(mapx, mapy) != 0)
        break;
      if (sidedx < sidedy) {
        sidedx += ddx;
        mapx += stepx;
        maskx = true;
        masky = false;
      } else {
        sidedy += ddy;
        mapy += stepy;
        maskx = false;
        masky = true;
      }
    }
    double dist;
    if (maskx)
      dist = abs((mapx - playerx / tilesize + (1.0 - stepx) / 2.0) / dirx);
    else
      dist = abs((mapy - playery / tilesize + (1.0 - stepy) / 2.0) / diry);
    dist = 600 / dist;
    if (dist > 600)
      dist = 600;
    uint32_t color = tilecolor(getmap(mapx, mapy)) * (maskx ? 0.9 : 1.0);
    pd->draw_vert_line(x, dist, color);
  }
}

int main() {
  framebuffer screen(800, 600);

  screen.mainloop(&running, update, render);

  return 0;
}


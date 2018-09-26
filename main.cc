#include "framebuffer.hh"
#include "utils.hh"
#include <fstream>
#include <cmath>

static const int palette_size = 6;
static const uint32_t palette[palette_size + 1][2] = {
  { 0xFF00FF, 0xFF00FF },
  { 0xFFFFFF, 0xDDDDDD },
  { 0xFF0000, 0xDD0000 },
  { 0x00FF00, 0x00DD00 },
  { 0x0000FF, 0x0000DD },
  { 0xFF00FF, 0xDD00DD },
  { 0x444444, 0x333333 }
};
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
static const double tilesize = 10;
static const double player_acc = 50, player_vel_limit = 30, player_vel_damping = 0.85,
                    player_turn_acc = 600, player_turn_vel_limit = 120, player_turn_damping = 0.8;
static double fov = 60;
static bool running = true;

static int tilecolor(int t, bool shaded = false)
{
  return (t >= 1 && t <= palette_size) ? palette[t][(int)shaded] : palette[0][0];
}

static int sign(double x)
{
  if (std::abs(x) < 1e-5)
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

static void update(state_t *state, double dt, uint32_t t)
{
  SDL_Event event;

  if (state->player.vel.magnitude_sq() > player_vel_limit * player_vel_limit) {
    state->player.vel.normalize();
    state->player.vel *= player_vel_limit;
  }
  state->player.angvel = clamp(state->player.angvel, -player_turn_vel_limit, player_turn_vel_limit);

  while (SDL_PollEvent(&event) != 0) {
    if (event.type == SDL_QUIT)
      running = false;
    else if (event.type == SDL_KEYDOWN || event.type == SDL_KEYUP) {
      uint8_t *keystates = (uint8_t*)SDL_GetKeyboardState(nullptr);
      int fw = keystates[SDL_SCANCODE_W] - keystates[SDL_SCANCODE_S];
      int side = keystates[SDL_SCANCODE_D] - keystates[SDL_SCANCODE_A];
      state->player.angacc = side * player_turn_acc;
      state->player.angveldamping = side == 0 ? player_turn_damping : 1;

      state->player.acc.x = fw * cos(to_rads(state->player.ang)) * player_acc;
      state->player.acc.y = fw * sin(to_rads(state->player.ang)) * player_acc;
      state->player.veldamping = fw == 0 ? player_vel_damping : 1;
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
  int plx = offset + (draw.player.pos.x / tilesize) * scale,
      ply = offset + (draw.player.pos.y / tilesize) * scale;

  pd->draw_square(offset, offset, mapsz * scale, 0x000000);

  for (int y = 0; y < mapsz; ++y)
    for (int x = 0; x < mapsz; ++x)
      if (map[y][x])
        pd->draw_square(offset + x * scale, offset + y * scale, scale, tilecolor(map[y][x]));

  for (int i = 0; i < 70; i++) {
    pd->write(round(plx + i * cos(to_rads(draw.player.ang - fov / 1.0))),
        round(ply + i * sin(to_rads(draw.player.ang - fov / 1.0))), 0x00AA00);
    pd->write(round(plx + i * cos(to_rads(draw.player.ang + fov / 1.0))),
        round(ply + i * sin(to_rads(draw.player.ang + fov / 1.0))), 0x00AA00);
  }

  for (int i = 0; i < 40; i++)
    pd->write(round(plx + i * cos(to_rads(draw.player.ang))),
        round(ply + i * sin(to_rads(draw.player.ang))),
        0xFF0000);

  pd->draw_square(plx - 1, ply - 1, 3, 0xAAAAAA);
}

static void render(framebuffer *pd, const state_t &draw)
{
  for (int x = 0; x < pd->get_width(); x++) {
    const double screenx = (double)x / pd->get_width(),
          screenxnorm = screenx * 2.0 - 1.0,
          rayang = draw.player.ang + screenxnorm * fov,
          rayangrad = to_rads(rayang),
          dirx = std::cos(rayangrad),
          diry = std::sin(rayangrad),
          ddx = std::abs(1. / dirx),
          ddy = std::abs(1. / diry),
          player_tile_x = draw.player.pos.x / tilesize,
          player_tile_y = draw.player.pos.y / tilesize,
          raydifx = std::floor(player_tile_x) - player_tile_x,
          raydify = std::floor(player_tile_y) - player_tile_y,
          stepx = sign(dirx),
          stepy = sign(diry);
    int mapx = player_tile_x,
        mapy = player_tile_y;
    double sidedx = (stepx * raydifx + stepx * 0.5 + 0.5) * ddx,
           sidedy = (stepy * raydify + stepy * 0.5 + 0.5) * ddy,
           dist,
           height;
    bool maskx = false;

    for (int i = 0; i < 1e3; i++) {
      if (getmap(mapx, mapy) != 0)
        break;
      if (sidedx < sidedy) {
        sidedx += ddx;
        mapx += stepx;
        maskx = true;
      } else {
        sidedy += ddy;
        mapy += stepy;
        maskx = false;
      }
    }

    if (maskx)
      dist = std::abs((mapx - player_tile_x + (1.0 - stepx) / 2.0) / dirx);
    else
      dist = std::abs((mapy - player_tile_y + (1.0 - stepy) / 2.0) / diry);

    height = pd->get_height() / dist;
    if (height > pd->get_height())
      height = pd->get_height();

    uint32_t color = tilecolor(getmap(mapx, mapy), maskx);
    pd->draw_vert_line(x, height, color);
  }

  draw_minimap(pd, draw);
}

int main() {
  framebuffer screen(800, 600);
  state_t initial;
  initial.player.pos = vec2(30, 40);

  screen.mainloop(&running, initial, update, render);

  return 0;
}


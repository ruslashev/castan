#include "state.hh"

vec2::vec2() : x(0), y(0)
{
}

vec2::vec2(double _x, double _y) : x(_x), y(_y)
{
}

vec2::vec2(const vec2 &v) : x(v.x), y(v.y)
{
}

vec2& vec2::operator=(const vec2& v)
{
  x = v.x;
  y = v.y;
  return *this;
}

vec2 vec2::operator+(const vec2 &v)
{
  return vec2(x + v.x, y + v.y);
}

vec2 vec2::operator-(const vec2 &v)
{
  return vec2(x - v.x, y - v.y);
}

vec2 vec2::operator*(double r)
{
  return vec2(x * r, y * r);
}

vec2& vec2::operator+=(const vec2& v) {
  x += v.x;
  y += v.y;
  return *this;
}

vec2& vec2::operator-=(const vec2& v) {
  x -= v.x;
  y -= v.y;
  return *this;
}

state_t::state_t() : _entities { &player }
{
}

void state_t::integrate(double t, double dt)
{
  for (entity_t *ent : _entities) {
    ent->vel += ent->acc * dt;
    ent->pos += ent->vel * dt;
  }
}

static double lerp(double a, double b, double t)
{
  return a * (1. - t) + b * t;
}

static vec2 lerp(const vec2 &a, const vec2 &b, double t)
{
  return vec2(lerp(a.x, b.x, t), lerp(a.y, b.y, t));
}

state_t state_lerp(const state_t &a, const state_t &b, double t)
{
  // TODO: not all entities are interpolated
  // TODO: not all entity's attributes are interpolated
  state_t new_state;
  new_state.player.pos = lerp(a.player.pos, b.player.pos, t);
  return new_state;
}


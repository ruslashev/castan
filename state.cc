#include "state.hh"
#include <cmath>

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

vec2& vec2::operator*=(double r) {
  x *= r;
  y *= r;
  return *this;
}

double vec2::magnitude_sq()
{
  return x * x + y * y;
}

void vec2::normalize()
{
  double mag_sq = magnitude_sq();
  if (mag_sq != 0) {
    double len = sqrt(mag_sq);
    x /= len;
    y /= len;
  }
}

double clamp(double x, double low, double high)
{
  return std::min(std::max(x, low), high);
}

entity_t::entity_t() : ang(0), angvel(0), angacc(0)
{
}

void entity_t::integrate(double t, double dt)
{
  vel += acc * dt;
  vel *= veldamping;
  pos += vel * dt;

  angvel += angacc * dt;
  angvel *= angveldamping;
  ang += angvel * dt;
}

state_t::state_t() : _entities { &player }
{
}

void state_t::integrate(double t, double dt)
{
  player.integrate(t, dt);
  // for (entity_t *ent : _entities) {
  //   ent->integrate(t, dt);
  // }
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
  new_state.player.ang = lerp(a.player.ang, b.player.ang, t);
  return new_state;
}


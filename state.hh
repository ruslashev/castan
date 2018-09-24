#pragma once

#include <vector>

struct vec2
{
  double x, y;
  vec2();
  vec2(double _x, double _y);
  vec2(const vec2 &v);
  vec2& operator=(const vec2 &v);
  vec2 operator+(const vec2 &v);
  vec2 operator-(const vec2 &v);
  vec2 operator*(double r);
  vec2& operator+=(const vec2 &v);
  vec2& operator-=(const vec2 &v);
  vec2& operator*=(double);
  double magnitude_sq();
  void normalize();
};

double clamp(double x, double low, double high);

struct entity_t
{
  vec2 pos;
  vec2 vel;
  vec2 acc;
  double ang;
  double angvel;
  double angacc;

  double veldamping;
  double angveldamping;
  entity_t();
  void integrate(double t, double dt);
};

class state_t
{
  std::vector<entity_t*> _entities;
public:
  entity_t player;
  state_t();
  void integrate(double t, double dt);
};

state_t state_lerp(const state_t &a, const state_t &b, double t);


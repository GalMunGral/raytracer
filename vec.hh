#pragma once

#include <iostream>

struct color
{
  float r, g, b, a;
  color() : color(0, 0, 0, 0){};
  color(float r, float g, float b) : color(r, g, b, 1){};
  color(float r, float g, float b, float a) : r(r), g(g), b(b), a(a){};
};

struct vec
{
  float x, y, z;
  vec();
  vec(float x, float y, float z);
  vec operator+(vec o);
  vec operator-(vec o);
  vec operator*(float c);
  vec operator/(float c);
  float dot(vec o);
  float squared_norm();
  float norm();
  vec normalize();
};

vec operator*(float c, vec v);

std::ostream &operator<<(std::ostream &os, vec v);
std::ostream &operator<<(std::ostream &os, color c);
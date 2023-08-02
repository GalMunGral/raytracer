#pragma once

#include <iostream>

struct color
{
  float r, g, b;
  color() : color(0, 0, 0){};
  color(float r, float g, float b) : r(r), g(g), b(b){};
  color operator+(color o);
  color operator*(color o);
  color operator*(float c);
  color &operator+=(color o);
};

color operator*(float c, color l);

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
  vec cross(vec o);
  float squared_norm();
  float norm();
  vec normalize();
};

vec operator-(vec v);
vec operator*(float c, vec v);

std::ostream &operator<<(std::ostream &os, vec v);
std::ostream &operator<<(std::ostream &os, color c);
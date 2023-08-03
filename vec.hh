#pragma once

#include <iostream>

struct vec
{
  float x, y, z;
  vec();
  vec(float x, float y, float z);
  vec operator+(vec o);
  vec &operator+=(vec o);
  vec operator-(vec o);
  vec operator*(float c);
  vec operator*(vec c);
  vec operator/(float c);
  float dot(vec o);
  vec cross(vec o);
  float squared_norm();
  float norm();
  vec normalize();
  vec clamp();
};

vec operator-(vec v);
vec operator*(float c, vec v);

std::ostream &operator<<(std::ostream &os, vec v);
std::ostream &operator<<(std::ostream &os, vec c);
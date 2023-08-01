#pragma once

#include <vector>
#include <string>
#include "vec.hh"

struct sphere
{
  vec c;
  float r;
  color color;
};

struct light
{
  vec pos;
  color color;
};

class scene
{
public:
  int width, height;
  std::string filename;
  std::vector<sphere> spheres;
  std::vector<light> lights;
};

scene parse(char *filename);

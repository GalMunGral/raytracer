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

class light
{

public:
  virtual ~light() = 0;
  virtual vec dir(vec) = 0;
  virtual color intensity(vec) = 0;
};

class directional_light : public light
{
  vec _dir;
  color _color;

public:
  ~directional_light();
  directional_light(vec dir, color color) : _dir(dir), _color(color){};
  vec dir(vec o);
  color intensity(vec o);
};

class point_light : public light
{
  vec _pos;
  color _color;

public:
  ~point_light();
  point_light(vec pos, color color) : _pos(pos), _color(color){};
  vec dir(vec o);
  color intensity(vec o);
};

class scene
{
public:
  int width, height;
  std::string filename;
  std::vector<sphere> spheres;
  std::vector<light *> lights;
};

scene parse(char *filename);

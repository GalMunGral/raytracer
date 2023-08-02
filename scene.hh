#pragma once

#include <vector>
#include <string>
#include "vec.hh"

class object
{
public:
  virtual ~object() = 0;
  virtual float intersect(vec o, vec dir) = 0;
  virtual vec norm_at(vec p) = 0;
  virtual color color_at(vec p) = 0;
};

class sphere : public object
{
public:
  sphere(vec c, float r, color color) : c(c), r(r), _color(color){};
  ~sphere();
  vec c;
  float r;
  color _color;
  float intersect(vec o, vec dir);
  vec norm_at(vec p);
  color color_at(vec p);
};

class light
{

public:
  virtual ~light() = 0;
  virtual vec dir(vec) = 0;
  virtual color intensity(vec) = 0;
  virtual float dist(vec) = 0;
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
  float dist(vec o);
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
  float dist(vec o);
};

class scene
{
public:
  int width, height;
  std::string filename;
  std::vector<object *> objects;
  std::vector<light *> lights;
};

scene parse(char *filename);

#pragma once

#include <vector>
#include <string>
#include "vec.hh"
#include "lodepng.h"
#include "texture.hh"

class object
{
public:
  vec shininess, transparency;
  float ior, roughness;
  object(vec shininess, vec transparency, float ior, float roughness)
      : shininess(shininess), transparency(transparency), ior(ior), roughness(roughness){};
  virtual ~object() = 0;
  virtual float intersect(vec o, vec dir) = 0;
  virtual vec norm_at(vec p) = 0;
  virtual vec color_at(vec p) = 0;
};

class sphere : public object
{
public:
  vec c;
  float r;
  texture *_texture;
  vec _color;
  sphere(float x, float y, float z, float r, texture *texture, vec color,
         vec shininess, vec transparency, float ior, float roughness)
      : object(shininess, transparency, ior, roughness),
        c(x, y, z), r(r), _texture(texture), _color(color){};
  ~sphere();
  float intersect(vec o, vec dir);
  vec norm_at(vec p);
  vec color_at(vec p);
};

class plane : public object
{
public:
  float a, b, c, d;
  vec _color;
  plane(float a, float b, float c, float d, vec color,
        vec shininess, vec transparency, float ior, float roughness)
      : object(shininess, transparency, ior, roughness),
        a(a), b(b), c(c), d(d), _color(color){};
  ~plane();
  float intersect(vec o, vec dir);
  vec norm_at(vec p);
  vec color_at(vec p);
};

class triangle : public object
{
public:
  vec p0, p1, p2, n0, n1, n2, st0, st1, st2, e1, e2;
  texture *_texture;
  vec _color;
  triangle(vec p0, vec p1, vec p2, vec n0, vec n1, vec n2,
           vec st0, vec st1, vec st2, texture *texture, vec color,
           vec shininess, vec transparency, float ior, float roughness)
      : object(shininess, transparency, ior, roughness),
        p0(p0), p1(p1), p2(p2), n0(n0), n1(n1), n2(n2),
        st0(st0), st1(st1), st2(st2), _texture(texture), _color(color)
  {
    auto n = (p1 - p0).cross(p2 - p0);
    if (!n0.norm() || !n1.norm() || !n2.norm())
    {
      this->n0 = this->n1 = this->n2 = n.normalize();
    }
    e1 = (p2 - p0).cross(n);
    e1 = e1 / (e1.dot(p1 - p0));
    e2 = (p1 - p0).cross(n);
    e2 = e2 / (e2.dot(p2 - p0));
  }
  ~triangle();
  float intersect(vec o, vec dir);
  vec norm_at(vec p);
  vec color_at(vec p);
};

class light
{

public:
  virtual ~light() = 0;
  virtual vec dir(vec) = 0;
  virtual vec intensity(vec) = 0;
  virtual float dist(vec) = 0;
};

class directional_light : public light
{
  vec _dir;
  vec _color;

public:
  ~directional_light();
  directional_light(vec dir, vec color) : _dir(dir), _color(color){};
  vec dir(vec o);
  vec intensity(vec o);
  float dist(vec o);
};

class point_light : public light
{
  vec _pos;
  vec _color;

public:
  ~point_light();
  point_light(vec pos, vec color) : _pos(pos), _color(color){};
  vec dir(vec o);
  vec intensity(vec o);
  float dist(vec o);
};

class scene
{
public:
  int width, height, aa = 1, d = 0, bounces = 4;
  std::string filename;
  std::vector<object *> objects;
  std::vector<light *> lights;
};

scene parse(char *filename);

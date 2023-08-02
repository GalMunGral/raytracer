#include <fstream>
#include <iostream>
#include <string>
#include "scene.hh"

scene parse(char *filename)
{
  scene sc;
  std::ifstream fs(filename);
  std::string cmd;
  color cur_color(1, 1, 1);
  while (fs >> cmd)
  {
    if (cmd == "png")
    {
      fs >> sc.width >> sc.height >> sc.filename;
    }
    else if (cmd == "color")
    {
      fs >> cur_color.r >> cur_color.g >> cur_color.b;
    }
    else if (cmd == "sphere")
    {
      float x, y, z, r;
      fs >> x >> y >> z >> r;
      sc.objects.push_back(new sphere(x, y, z, r, cur_color));
    }
    else if (cmd == "plane")
    {
      float a, b, c, d;
      fs >> a >> b >> c >> d;
      sc.objects.push_back(new plane(a, b, c, d, cur_color));
    }
    else if (cmd == "sun")
    {
      vec dir;
      fs >> dir.x >> dir.y >> dir.z;
      sc.lights.push_back(new directional_light(dir, cur_color));
    }
    else if (cmd == "bulb")
    {

      vec pos;
      fs >> pos.x >> pos.y >> pos.z;
      sc.lights.push_back(new point_light(pos, cur_color));
    }
  }

  return sc;
}

object::~object(){};

sphere::~sphere(){};

float sphere::intersect(vec o, vec dir)
{
  auto inside = (c - o).norm() < r;
  auto t_center = (c - o).dot(dir);
  if (!inside && t_center < 0)
    return 0;
  auto d = (o + (t_center * dir) - c).norm();
  if (!inside && r < d)
    return 0;
  auto t_offset = sqrt(r * r - d * d);
  return inside ? t_center + t_offset : t_center - t_offset;
}

vec sphere::norm_at(vec p)
{
  return (p - c).normalize();
}

color sphere::color_at(vec)
{
  return _color;
}

plane::~plane(){};

float plane::intersect(vec o, vec dir)
{
  auto p = a   ? vec(-d / a, 0, 0)
           : b ? vec(0, -d / b, 0)
               : vec(0, 0, -d / c);
  auto n = vec(a, b, c);
  ;
  auto t = (p - o).dot(n) / (dir.dot(n));
  return std::max(t, 0.0f);
}

vec plane::norm_at(vec)
{
  return vec(a, b, c).normalize();
}

color plane::color_at(vec)
{
  return _color;
}

light::~light(){};

directional_light::~directional_light(){};

vec directional_light::dir(vec)
{
  return _dir.normalize();
}

color directional_light::intensity(vec)
{
  return _color;
}

float directional_light::dist(vec)
{
  return std::numeric_limits<float>::max();
}

point_light::~point_light(){};

vec point_light::dir(vec o)
{
  return (_pos - o).normalize();
}

color point_light::intensity(vec o)
{
  auto d = dist(o);
  return _color * std::pow(1 / d, 2);
}

float point_light::dist(vec o)
{
  return (_pos - o).norm();
}
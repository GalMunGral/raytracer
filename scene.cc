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
      sphere s;
      fs >> s.c.x >> s.c.y >> s.c.z >> s.r;
      s.color = cur_color;
      sc.spheres.push_back(s);
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

point_light::~point_light(){};

vec point_light::dir(vec o)
{
  return (_pos - o).normalize();
}

color point_light::intensity(vec o)
{
  auto d = (_pos - o).norm();
  return _color * std::pow(1 / d, 2);
}
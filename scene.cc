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
      light l;
      fs >> l.dir.x >> l.dir.y >> l.dir.z;
      l.color = cur_color;
      sc.lights.push_back(l);
    }
  }

  return sc;
}
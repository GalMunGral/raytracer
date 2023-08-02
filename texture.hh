#pragma once

#include <vector>
#include <string>
#include "vec.hh"

class texture
{
  std::vector<unsigned char> buffer;
  unsigned int w, h;

public:
  texture(std::string &filename);

  color pixel(int i, int j);

  color color_at(vec st);
};
#include <iostream>
#include "lodepng.h"
#include "scene.hh"
#include "render.hh"

using namespace std;

int main(int argc, char *argv[])
{
  if (argc < 2)
    return 1;
  scene sc = parse(argv[1]);
  std::vector<unsigned char> buffer(4 * sc.width * sc.height);
  render(sc, buffer);
  lodepng::encode(sc.filename, buffer, sc.width, sc.height);
  return 0;
}

#include "lodepng.h"
#include "texture.hh"

float linear(float g)
{
  return g <= .04045 ? g / 12.92 : pow(((g + 0.055) / 1.055), 2.4);
}

texture::texture(std::string &filename)
{
  unsigned error = 0;
  std::vector<unsigned char> tmp;
  if ((error = lodepng::load_file(tmp, filename)))
  {
    std::cout << lodepng_error_text(error) << std::endl;
  }
  lodepng::State s;
  if ((error = lodepng::decode(buffer, w, h, s, tmp)))
  {
    {
      std::cout << lodepng_error_text(error) << std::endl;
    }
  }
}

vec texture::pixel(int i, int j)
{
  return vec(
      linear(buffer[(i * w + j) * 4] / 255.0),
      linear(buffer[(i * w + j) * 4 + 1] / 255.0),
      linear(buffer[(i * w + j) * 4 + 2] / 255.0));
}

vec texture::color_at(vec st)
{
  float x = st.x * (w - 1), y = st.y * (h - 1);
  int i = y, j = x;
  return pixel(i, j);
  return pixel(i, j) * (i + 1 - y) * (j + 1 - x) +
         pixel(i, j + 1) * (i + 1 - y) * (x - j) +
         pixel(i + 1, j) * (y - i) * (j + 1 - x) +
         pixel(i + 1, j + 1) * (y - i) * (x - j);
}
#include <iostream>
#include <algorithm>
#include "render.hh"

float gamma(float l)
{
  l = std::clamp(l, 0.0f, 1.0f);
  return l <= 0.0031308 ? 12.92 * l : 1.055 * std::pow(l, 1 / 2.4) - 0.055;
}

float intersect(sphere &s, vec o, vec dir)
{
  auto inside = (s.c - o).norm() < s.r;
  auto t_center = (s.c - o).dot(dir);
  if (!inside && t_center < 0)
    return 0;
  auto d = (o + (t_center * dir) - s.c).norm();
  if (!inside && s.r < d)
    return 0;
  auto t_offset = sqrt(s.r * s.r - d * d);
  return inside ? t_center + t_offset : t_center - t_offset;
}

color trace(scene &sc, vec o, vec dir)
{
  sphere *sphere_hit = nullptr;
  auto t_hit = std::numeric_limits<float>::max();
  for (auto &s : sc.spheres)
  {
    auto t = intersect(s, o, dir);
    if (t && t < t_hit)
    {
      t_hit = t;
      sphere_hit = &s;
    }
  }
  if (!sphere_hit)
    return color(-1, -1, -1);

  auto p = o + t_hit * dir;
  auto n = (p - sphere_hit->c).normalize();

  // use the other side
  if (n.dot(dir) > 0)
    n = -n;

  color color;
  for (auto &l : sc.lights)
  {
    auto lambert = std::max(.0f, l.dir.normalize().dot(n));
    color = color + lambert * l.color * sphere_hit->color;
  }
  return color;
}

void render(scene &sc, std::vector<unsigned char> &image)
{
  vec eye, forward(0, 0, -1), right(1, 0, 0), up(0, 1, 0);

  for (int i = 0; i < sc.height; ++i)
  {
    for (int j = 0; j < sc.width; ++j)
    {
      auto x = float(2 * j - sc.width) / std::max(sc.width, sc.height);
      auto y = float(sc.height - 2 * i) / std::max(sc.width, sc.height);
      auto dir = (forward + x * right + y * up).normalize();
      auto color = trace(sc, eye, dir);
      if (color.r < 0)
        continue;
      image[4 * (i * sc.width + j)] = gamma(color.r) * 255;
      image[4 * (i * sc.width + j) + 1] = gamma(color.g) * 255;
      image[4 * (i * sc.width + j) + 2] = gamma(color.b) * 255;
      image[4 * (i * sc.width + j) + 3] = 255;
    }
  }
}

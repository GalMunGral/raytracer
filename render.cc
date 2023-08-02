#include <iostream>
#include <algorithm>
#include "render.hh"

float gamma(float l)
{
  l = std::clamp(l, 0.0f, 1.0f);
  return l <= 0.0031308 ? 12.92 * l : 1.055 * std::pow(l, 1 / 2.4) - 0.055;
}

color *trace(scene &sc, vec o, vec dir)
{
  int hit = -1;
  auto t_hit = std::numeric_limits<float>::max();
  int n_objects = sc.objects.size();
  for (int i = 0; i < n_objects; ++i)
  {
    auto t = sc.objects[i]->intersect(o, dir);
    if (t && t < t_hit)
    {
      t_hit = t;
      hit = i;
    }
  }
  if (hit == -1)
    return nullptr;

  auto p = o + t_hit * dir;
  auto n = sc.objects[hit]->norm_at(p);

  // use the other side
  if (n.dot(dir) > 0)
    n = -n;

  auto c = new color;
  for (auto &l : sc.lights)
  {
    auto l_dir = l->dir(p);
    auto l_dist = l->dist(p);
    bool in_shadow = false;
    for (int i = 0; i < n_objects; ++i)
    {
      float o_dist = 0;
      if (i != hit && (o_dist = sc.objects[i]->intersect(p, l_dir)) && o_dist < l_dist)
      {
        in_shadow = true;
        break;
      }
    }
    if (!in_shadow)
    {
      auto lambert = std::max(.0f, l_dir.dot(n));
      *c += lambert * l->intensity(p) * sc.objects[hit]->color_at(p);
    }
  }
  return c;
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
      auto c = trace(sc, eye, dir);
      if (c)
      {

        image[4 * (i * sc.width + j)] = gamma(c->r) * 255;
        image[4 * (i * sc.width + j) + 1] = gamma(c->g) * 255;
        image[4 * (i * sc.width + j) + 2] = gamma(c->b) * 255;
        image[4 * (i * sc.width + j) + 3] = 255;
      }
    }
  }
}

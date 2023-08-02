#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <cmath>
#include "scene.hh"

scene parse(char *filename)
{
  scene sc;
  std::ifstream fs(filename);
  std::string cmd;
  std::vector<vec> points;
  std::vector<vec> normals;
  std::vector<vec> texcoords;
  texture *cur_texture = nullptr;
  color cur_color(1, 1, 1);
  vec cur_normal;
  vec cur_texcoord;

  while (fs >> cmd)
  {
    if (cmd == "png")
    {
      fs >> sc.width >> sc.height >> sc.filename;
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
    else if (cmd == "color")
    {
      float r, g, b;
      fs >> r >> g >> b;
      cur_color = color(r, g, b);
      // cur_color = color(std::clamp(r, -1.0f, 1.0f),
      //                   std::clamp(g, -1.0f, 1.0f),
      //                   std::clamp(b, -1.0f, 1.0f));
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
    else if (cmd == "xyz")
    {
      float x, y, z;
      fs >> x >> y >> z;
      points.push_back(vec(x, y, z));
      normals.push_back(cur_normal);
      texcoords.push_back(cur_texcoord);
    }
    else if (cmd == "texcoord")
    {
      fs >> cur_texcoord.x >> cur_texcoord.y;
    }
    else if (cmd == "normal")
    {
      fs >> cur_normal.x >> cur_normal.y >> cur_normal.z;
    }
    else if (cmd == "trif" || cmd == "trit")
    {
      int i, j, k;
      fs >> i >> j >> k;
      int n = points.size();
      i = i > 0 ? i - 1 : i + n;
      j = j > 0 ? j - 1 : j + n;
      k = k > 0 ? k - 1 : k + n;
      sc.objects.push_back(
          new triangle(points[i], points[j], points[k],
                       normals[i], normals[j], normals[k],
                       texcoords[i], texcoords[j], texcoords[k],
                       cur_texture, cur_color));
    }
    else if (cmd == "texture")
    {
      std::string filename;
      fs >> filename;
      cur_texture = new texture(filename);
    }
    else if (cmd == "aa")
    {
      fs >> sc.aa;
    }
    else if (cmd == "gi")
    {
      fs >> sc.d;
    }
    else if (cmd == "shininess")
    {
      int s;
      fs >> s;
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

triangle::triangle(vec p0, vec p1, vec p2, vec n0, vec n1, vec n2,
                   vec st0, vec st1, vec st2, texture *texture, color color)
    : p0(p0), p1(p1), p2(p2), n0(n0), n1(n1), n2(n2),
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

triangle::~triangle(){};

float triangle::intersect(vec o, vec dir)
{
  auto n = (p1 - p0).cross(p2 - p0);
  auto t = (p0 - o).dot(n) / (dir.dot(n));
  if (t < 0)
    return 0;
  auto p = o + t * dir;
  auto b1 = (p - p0).dot(e1), b2 = (p - p0).dot(e2), b0 = 1 - b1 - b2;
  if (b0 < 0 || b1 < 0 || b2 < 0)
    return 0;
  return t;
}

vec triangle::norm_at(vec p)
{
  auto b1 = (p - p0).dot(e1), b2 = (p - p0).dot(e2), b0 = 1 - b1 - b2;
  return (b0 * n0 + b1 * n1 + b2 * n2).normalize();
}

color triangle::color_at(vec p)
{
  if (_texture)
  {
    auto b1 = (p - p0).dot(e1), b2 = (p - p0).dot(e2), b0 = 1 - b1 - b2;
    return _texture->color_at(b0 * st0 + b1 * st1 + b2 * st2);
  }
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
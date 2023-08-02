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
  float cur_ior = 1.458;
  float cur_roughness = 0;
  vec cur_color(1, 1, 1);
  vec cur_normal;
  vec cur_texcoord;
  vec cur_shininess;
  vec cur_transparency;

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
      cur_color = vec(r, g, b);
    }
    else if (cmd == "sphere")
    {
      float x, y, z, r;
      fs >> x >> y >> z >> r;
      sc.objects.push_back(
          new sphere(x, y, z, r, cur_texture, cur_color,
                     cur_shininess, cur_transparency, cur_ior, cur_roughness));
    }
    else if (cmd == "plane")
    {
      float a, b, c, d;
      fs >> a >> b >> c >> d;
      sc.objects.push_back(
          new plane(a, b, c, d, cur_color,
                    cur_shininess, cur_transparency, cur_ior, cur_roughness));
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
                       cur_texture, cur_color,
                       cur_shininess, cur_transparency,
                       cur_ior, cur_roughness));
    }
    else if (cmd == "texture")
    {
      std::string filename;
      fs >> filename;
      cur_texture = filename == "none" ? nullptr : new texture(filename);
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
      float s;
      fs >> s;
      cur_shininess = vec(s, s, s);
      if (fs >> s)
      {
        cur_shininess.y = s;
        fs >> cur_shininess.z;
      }
      else
      {
        fs.clear();
      }
    }
    else if (cmd == "transparency")
    {
      float t;
      fs >> t;
      cur_transparency = vec(t, t, t);
      if (fs >> t)
      {
        cur_transparency.y = t;
        fs >> cur_transparency.z;
      }
      else
      {
        fs.clear();
      }
    }
    else if (cmd == "ior")
    {
      fs >> cur_ior;
    }
    else if (cmd == "bounces")
    {
      fs >> sc.bounces;
    }
    else if (cmd == "roughness")
    {
      fs >> cur_roughness;
    }
    else if (cmd == "eye")
    {
      fs >> sc.eye.x >> sc.eye.y >> sc.eye.z;
    }
    else if (cmd == "forward")
    {
      fs >> sc.forward.x >> sc.forward.y >> sc.forward.z;
      sc.right = sc.forward.cross(sc.up).normalize();
      sc.up = sc.right.cross(sc.forward).normalize();
    }
    else if (cmd == "up")
    {
      fs >> sc.up.x >> sc.up.y >> sc.up.z;
      sc.right = sc.forward.cross(sc.up).normalize();
      sc.up = sc.right.cross(sc.forward).normalize();
    }
    else if (cmd == "fisheye")
    {
      sc.fisheye = true;
    }
    else if (cmd == "dof")
    {
      fs >> sc.focus >> sc.lens;
      sc.dof = true;
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

vec sphere::color_at(vec p)
{
  if (_texture)
  {
    float s = (std::atan2f(c.z - p.z, p.x - c.x) + M_PI) / (2 * M_PI);
    float t = std::abs(std::atan2f(p.z - c.z, p.y - c.y)) / M_PI;
    return _texture->color_at(vec(s, t, 0));
  }
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

vec plane::color_at(vec)
{
  return _color;
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

vec triangle::color_at(vec p)
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

vec directional_light::intensity(vec)
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

vec point_light::intensity(vec o)
{
  auto d = dist(o);
  return _color * std::pow(1 / d, 2);
}

float point_light::dist(vec o)
{
  return (_pos - o).norm();
}
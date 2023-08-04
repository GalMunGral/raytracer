#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <cmath>
#include "scene.hh"

bool has_multiple_args(std::istream &s)
{
  char c;
  s >> c;
  s.unget();
  return !std::isalpha(c);
}

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
      sc.lights.push_back(
          std::unique_ptr<light>(new directional_light(dir, cur_color)));
    }
    else if (cmd == "bulb")
    {

      vec pos;
      fs >> pos.x >> pos.y >> pos.z;
      sc.lights.push_back(
          std::unique_ptr<light>(new point_light(pos, cur_color)));
    }
    else if (cmd == "color")
    {
      float r, g, b;
      fs >> r >> g >> b;
      cur_color = vec(r, g, b);
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
      sc.objects.add(std::shared_ptr<object>(
          new triangle(points[i], points[j], points[k],
                       normals[i], normals[j], normals[k],
                       texcoords[i], texcoords[j], texcoords[k],
                       cur_texture, cur_color,
                       cur_shininess, cur_transparency,
                       cur_ior, cur_roughness)));
    }
    else if (cmd == "sphere")
    {
      float x, y, z, r;
      fs >> x >> y >> z >> r;
      sc.objects.add(std::shared_ptr<object>(
          new sphere(x, y, z, r, cur_texture, cur_color,
                     cur_shininess, cur_transparency, cur_ior, cur_roughness)));
    }
    else if (cmd == "plane")
    {
      float a, b, c, d;
      fs >> a >> b >> c >> d;
      sc.objects.add(std::shared_ptr<object>(
          new plane(a, b, c, d, cur_color,
                    cur_shininess, cur_transparency, cur_ior, cur_roughness)));
    }
    else if (cmd == "texture")
    {
      std::string filename;
      fs >> filename;
      cur_texture = filename == "none" ? nullptr : new texture(filename);
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
    else if (cmd == "expose")
    {
      fs >> sc.expose;
    }
    else if (cmd == "aa")
    {
      fs >> sc.aa;
    }
    else if (cmd == "gi") // global illumination
    {
      fs >> sc.d;
    }
    else if (cmd == "shininess")
    {
      float s;
      fs >> s;
      cur_shininess = vec(s, s, s);
      if (has_multiple_args(fs))
      {
        fs >> cur_shininess.y >> cur_shininess.z;
      }
    }
    else if (cmd == "transparency")
    {
      float t;
      fs >> t;
      cur_transparency = vec(t, t, t);
      if (has_multiple_args(fs))
      {
        fs >> cur_transparency.y >> cur_transparency.z;
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
  }

  return sc;
}

float aabb::size()
{
  return std::min({x2 - x1, y2 - y1, z2 - z1});
}

bool aabb::intersect(vec o, vec dir)
{
  auto tx1 = (x1 - o.x) / dir.x;
  auto tx2 = (x2 - o.x) / dir.x;
  auto ty1 = (y1 - o.y) / dir.y;
  auto ty2 = (y2 - o.y) / dir.y;
  auto tz1 = (z1 - o.z) / dir.z;
  auto tz2 = (z2 - o.z) / dir.z;
  auto t1 = std::max({0.0f,
                      std::min(tx1, tx2), std::min(ty1, ty2), std::min(tz1, tz2)});
  auto t2 = std::min({std::numeric_limits<float>::max(),
                      std::max(tx1, tx2), std::max(ty1, ty2), std::max(tz1, tz2)});
  return t1 < t2;
}

object::~object(){};

sphere::~sphere(){};

bool sphere::might_intersect(aabb &box)
{
  return c.x > box.x1 - r && c.x < box.x2 + r &&
         c.y > box.y1 - r && c.y < box.y2 + r &&
         c.z > box.z1 - r && c.z < box.z2 + r;
}

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
    if (s > 1)
      s -= 1;
    return _texture->color_at(vec(s, t, 0));
  }
  return _color;
}

plane::~plane(){};

bool plane::might_intersect(aabb &box)
{
  bool prev = (a * box.x1 + b * box.y1 + c * box.z1 + d) > 0;
  for (auto x : {box.x1, box.x2})
  {
    for (auto y : {box.y1, box.y2})
    {
      for (auto z : {box.z1, box.z2})
      {
        if ((a * x + b * y + c * z + d) > 0 != prev)
        {
          return true;
        }
      }
    }
  }
  return false;
}

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

bool triangle::might_intersect(aabb &box)
{
  float x1 = p0.x, x2 = p0.x, y1 = p0.y, y2 = p0.y, z1 = p0.z, z2 = p0.z;
  for (auto &p : {p1, p2})
  {
    x1 = std::min(x1, p.x);
    x2 = std::max(x2, p.x);
    y1 = std::min(y1, p.y);
    y2 = std::max(y2, p.y);
    z1 = std::min(z1, p.z);
    z2 = std::max(z2, p.z);
  }
  return std::max(x1, box.x1) <= std::min(x2, box.x2) &&
         std::max(y1, box.y1) <= std::min(y2, box.y2) &&
         std::max(z1, box.z1) <= std::min(z2, box.z2);
}

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
  return _color * std::min(100.0f, 1 / (d * d));
}

float point_light::dist(vec o)
{
  return (_pos - o).norm();
}

void bvh_node::split()
{
  auto [x1, x2, y1, y2, z1, z2] = box;
  float mx = (x1 + x2) / 2, my = (y1 + y2) / 2, mz = (z1 + z2) / 2;

  children.emplace_back(new bvh_node(x1, mx, y1, my, z1, mz));
  children.emplace_back(new bvh_node(x1, mx, y1, my, mz, z2));
  children.emplace_back(new bvh_node(x1, mx, my, y2, z1, mz));
  children.emplace_back(new bvh_node(x1, mx, my, y2, mz, z2));
  children.emplace_back(new bvh_node(mx, x2, y1, my, z1, mz));
  children.emplace_back(new bvh_node(mx, x2, y1, my, mz, z2));
  children.emplace_back(new bvh_node(mx, x2, my, y2, z1, mz));
  children.emplace_back(new bvh_node(mx, x2, my, y2, mz, z2));

  for (auto &obj : objects)
  {
    for (auto &child : children)
    {
      if (obj->might_intersect(child->box))
      {
        child->add(obj);
      }
    }
  }
  objects.clear();
  is_leaf = false;
}

void bvh_node::add(std::shared_ptr<object> obj)
{
  if (objects.size() == 5 && box.size() > 0.1)
  {
    split();
  }
  if (is_leaf)
  {
    objects.push_back(obj);
    return;
  }
  for (auto &child : children)
  {
    if (obj->might_intersect(child->box))
    {
      child->add(obj);
    }
  }
}

std::pair<object *const, float> bvh_node::intersect(vec o, vec dir)
{
  object *obj_hit = nullptr;
  float t_hit = std::numeric_limits<float>::max();

  if (is_leaf)
  {
    for (auto &obj : objects)
    {
      /**
       * Advance the ray by a small offset to compensate for numerical errors.
       * There are two cases where this is necessary:
       *  - when doing shadow testing, we need to start from outside the object.
       *  - when computing refraction, we need to start from inside the object.
       */
      auto t = obj->intersect(o + 1e-3 * dir, dir);
      if (t > 0 && t < t_hit)
      {
        obj_hit = obj.get();
        t_hit = t;
      }
    }
  }
  else
  {
    for (auto &child : children)
    {
      if (child->box.intersect(o, dir))
      {

        auto [obj, t] = child->intersect(o, dir);
        if (obj && t < t_hit)
        {
          obj_hit = obj;
          t_hit = t;
        }
      }
    }
  }

  return {obj_hit, t_hit};
}

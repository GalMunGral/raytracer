#include <iostream>
#include <random>
#include <algorithm>
#include <utility>
#include "render.hh"

std::default_random_engine gen;
std::uniform_real_distribution<float> uniform(0.0f, 1.0f);

float gamma(float l, float exposure)
{
  l = std::clamp(l, 0.0f, 1.0f);
  if (exposure)
  {
    l = 1 - std::exp(-l * exposure);
  }
  return l <= 0.0031308 ? 12.92 * l : 1.055 * std::pow(l, 1 / 2.4) - 0.055;
}

vec sample_unit_disk()
{
  vec q;
  do
  {
    q = vec(2 * uniform(gen) - 1, 2 * uniform(gen) - 1, 0);
  } while (q.norm() >= 1);
  return q;
}

vec sample_unit_sphere()
{
  vec q;
  do
  {
    q = vec(2 * uniform(gen) - 1, 2 * uniform(gen) - 1, 2 * uniform(gen) - 1);
  } while (q.norm() >= 1);
  return q;
}

struct ray_trace_result
{
  object *obj_hit = nullptr;
  vec p;
  vec intensity;
  ray_trace_result(){};
  ray_trace_result(object *obj, vec p, vec c)
      : obj_hit(obj), p(p), intensity(c){};
};

vec illuminate(scene &sc, light *light, object *obj, vec p, vec n)
{
  vec color;
  auto l_dir = light->dir(p);
  auto l_dist = light->dist(p);

  bool in_shadow = false;
  for (auto *o : sc.objects)
  {
    float o_dist = 0;
    if (o != obj && (o_dist = o->intersect(p, l_dir)) && o_dist < l_dist)
    {
      in_shadow = true;
      break;
    }
  }

  if (!in_shadow)
  {
    auto lambert = std::max(.0f, l_dir.dot(n));
    color = color + lambert * light->intensity(p) * obj->color_at(p);
  }

  return color.clamp();
}

ray_trace_result ray_trace(scene &sc, vec o, vec dir, int d, int bounces)
{
  o += 1e-3 * dir; // prevent self-intersection
  object *obj_hit = nullptr;
  auto t_hit = std::numeric_limits<float>::max();
  for (auto *obj : sc.objects)
  {
    float t;
    if ((t = obj->intersect(o, dir)) && t < t_hit)
    {
      t_hit = t;
      obj_hit = obj;
    }
  }

  if (!obj_hit)
    return ray_trace_result();

  auto p = o + t_hit * dir;
  auto n = obj_hit->norm_at(p);

  // use the other side
  if (n.dot(dir) > 0)
    n = -n;

  if (obj_hit->roughness)
  {
    std::normal_distribution<float> gaussian(0.0, obj_hit->roughness);
    n.x += gaussian(gen);
    n.y += gaussian(gen);
    n.z += gaussian(gen);
  }

  vec diffuse, refraction, reflection;

  for (auto *l : sc.lights)
  {
    diffuse += illuminate(sc, l, obj_hit, p, n);
  }

  if (d)
  {
    // shoot secondary rays
    auto random_dir = (n + sample_unit_sphere()).normalize();
    auto res = ray_trace(sc, p, random_dir, d - 1, bounces);
    if (res.obj_hit)
    {
      point_light l(res.p, res.intensity);
      diffuse += illuminate(sc, &l, obj_hit, p, n);
    }
  }

  if (bounces)
  {
    // reflection
    auto r = (dir - 2 * dir.dot(n) * n).normalize();
    reflection = ray_trace(sc, p, r, d, bounces - 1).intensity;

    // refraction
    bool entering = dir.dot(obj_hit->norm_at(p)) < 0;
    auto eta = entering ? 1 / obj_hit->ior : obj_hit->ior;
    float k = 1.0 - std::pow(eta, 2) * (1 - n.dot(dir) * n.dot(dir));
    if (k < 0)
    {
      refraction = reflection;
    }
    else
    {
      auto r = (eta * dir - (eta * n.dot(dir) + std::sqrt(k)) * n).normalize();
      refraction = ray_trace(sc, p, r, d, bounces - 1).intensity;
    }
  }

  auto s = obj_hit->shininess, t = obj_hit->transparency;
  auto color = s * reflection +
               (vec(1, 1, 1) - s) * t * refraction +
               (vec(1, 1, 1) - s) * (vec(1, 1, 1) - t) * diffuse;

  return {obj_hit, p, color};
}

void render(scene &sc, std::vector<unsigned char> &image)
{
  float w = sc.width, h = sc.height;

  for (int i = 0; i < sc.height; ++i)
  {
    for (int j = 0; j < sc.width; ++j)
    {
      bool hit_any = false;
      vec c;

      // randomly sample rays in a pixel
      for (int k = 0; k < sc.aa; ++k)
      {
        auto origin = sc.eye;
        auto forward = sc.forward;

        float x = j + uniform(gen), y = i + uniform(gen);
        float sx = (2 * x - w) / std::max(w, h);
        float sy = float(h - 2 * y) / std::max(w, h);

        if (sc.fisheye)
        {
          sx /= sc.forward.norm();
          sy /= sc.forward.norm();
          float r2 = (sx * sx + sy * sy);
          if (r2 > 1)
            continue;
          forward = std::sqrt(1 - r2) * (forward.normalize());
        }

        auto dir = (forward + sx * sc.right + sy * sc.up).normalize();

        if (sc.dof)
        {
          auto focal_point = sc.eye + sc.focus * dir;
          auto offset = sc.lens * sample_unit_disk();
          origin += offset.x * sc.right + offset.y * sc.up;
          dir = (focal_point - origin).normalize();
        }

        auto res = ray_trace(sc, origin, dir, sc.d, sc.bounces);

        if (res.obj_hit)
        {
          hit_any = true;
          c = c + (1.0f / sc.aa) * res.intensity;
        }
      }

      if (hit_any)
      {
        image[4 * (i * sc.width + j)] = gamma(c.x, sc.expose) * 255;
        image[4 * (i * sc.width + j) + 1] = gamma(c.y, sc.expose) * 255;
        image[4 * (i * sc.width + j) + 2] = gamma(c.z, sc.expose) * 255;
        image[4 * (i * sc.width + j) + 3] = 255;
      }

      std::cout << "progress: " << float(i * sc.width + j) / (sc.width * sc.height) << '\r' << std::flush;
    }
  }
}

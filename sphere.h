#ifndef SPHERE_H
#define SPHERE_H

#include "hittable.h"

class sphere : public hittable {
public:
  sphere(const point3 &center, double radius)
      : center(center), radius(std::fmax(0, radius)) {}

  bool hit(const ray &r, interval ray_t, hit_record &rec) const override {
    vec3 CQ = center - r.origin();
    auto a = r.direction().length_squared();
    auto h = dot(r.direction(), CQ);
    auto c = CQ.length_squared() - radius * radius;
    auto discriminant = h * h - a * c;

    // Cannot simplify further since dot product isn't associative

    if (discriminant < 0) {
      return false;
    }

    // Calculate hit_record
    auto sqrtd = std::sqrt(discriminant);

    // Find nearest root in acceptable range of t
    auto root = (h - sqrtd) / a;
    if (!ray_t.surrounds(root)) {
      root = (h + sqrtd) / a;
      if (!ray_t.surrounds(root)) {
        return false;
      }
    }

    rec.t = root;
    rec.p = r.at(rec.t);
    vec3 outward_normal = (rec.p - center) / radius;
    rec.set_face_normal(r, outward_normal);

    return true;
  }

private:
  point3 center;
  double radius;
};

#endif // !SPHERE_H

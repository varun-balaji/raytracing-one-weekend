#ifndef HITTABLE_H
#define HITTABLE_H

class hit_record {
public:
  point3 p;    // Point where ray hits hittable object.
  vec3 normal; // Surface normal from p.
  double t; // Value of t in equation of incident ray: P(t) = A + t*B where A is
            // ray origin and B is ray direction.
  bool front_face; // Used to determine whether the incident ray is hitting the
                   // front or back face of the hittable object. This will
                   // inform us of the color to use.

  void set_face_normal(const ray &r, const vec3 &outward_normal) {
    // Sets the hit record normal vector.
    // NOTE: The Parameter "outward_normal" is assumed to have unit length.

    front_face = dot(r.direction(), outward_normal) < 0;
    normal = front_face ? outward_normal : -outward_normal;
  }
};

class hittable {
public:
  virtual ~hittable() = default;

  virtual bool hit(const ray &r, interval ray_t, hit_record &rec) const = 0;
};

#endif // !HITTABLE_H

#include "rtweekend.h"

#include "camera.h"
#include "hittable.h"
#include "hittable_list.h"
#include "sphere.h"

int main() {

  // World
  hittable_list world;
  world.add(
      make_shared<sphere>(point3(0, 0, -1), 0.5)); // Sphere in front of camera.
  world.add(make_shared<sphere>(point3(0, -100.5, -1), 100)); // The earth.

  // Camera
  camera cam;
  cam.aspect_ratio = 16.0 / 9.0;
  cam.image_width = 400;
  cam.samples_per_pixel = 100;
  cam.max_depth = 50;

  cam.render(world);
}

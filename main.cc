#include "color.h"
#include "ray.h"
#include <iostream>

bool hit_sphere(const point3 &sphere_center, double sphere_radius,
                const ray &incident_ray) {
  vec3 CQ = sphere_center - incident_ray.origin();
  auto a = dot(incident_ray.direction(), incident_ray.direction());
  auto b = -2.0 * dot(incident_ray.direction(), CQ);
  auto c = dot(CQ, CQ) - sphere_radius * sphere_radius;
  auto discriminant = b * b - 4 * a * c;

  // Cannot simplify further since dot product isn't associative

  return (discriminant >= 0);
}

color ray_color(const ray &r) {
  if (hit_sphere(point3(0, 0, -1), 0.5, r)) {
    return color(1, 0, 0);
  }

  vec3 unit_direction = unit_vector(r.direction());
  auto a = 0.5 * (unit_direction.y() + 1.0);
  return (1.0 - a) * color(1.0, 1.0, 1.0) + a * color(0.5, 0.7, 1.0);
}

int main() {

  // Image
  auto aspect_ratio = 16.0 / 9.0;
  int image_width = 400;
  int image_height = int(image_width / aspect_ratio);
  if (image_height < 1) { // Ensure image_height is at least 1 pixel
    image_height = 1;
  }

  // Camera
  auto focal_length = 1.0; // Distance from viewport to camera
  auto viewport_height = 2.0;
  auto viewport_width = (viewport_height * image_width) / image_height;
  auto camera_center = point3(0, 0, 0); // Camera at origin

  // Viewport dimensions are mapped to the image dimensions. This is where the
  // scalar mapping of pixels to 3D coordinate unit dimensions happens.

  // Viewport edge vectors
  auto viewport_x = vec3(viewport_width, 0, 0);
  // - sign since Y-axis pixel coords are reverse in 3D coords
  auto viewport_y = vec3(0, -viewport_height, 0);

  // Pixel delta along viewport axes. We'll use these to calculate the 3D point
  // where the ray originating from the camera passes through.
  auto pixel_delta_x = viewport_x / image_width;
  auto pixel_delta_y = viewport_y / image_height;

  // Calculate 3D coords of upper left pixel.
  auto viewport_upper_left = camera_center - vec3(0, 0, focal_length) -
                             viewport_x / 2 - viewport_y / 2;
  // Offset the pixel00 center by 0.5 of pixel_delta from viewport_upper_left so
  // that each pixel is at the center of each grid square (which has dimensions
  // pixel_delta_x by pixel_delta_y)
  auto pixel00_loc =
      viewport_upper_left + 0.5 * (pixel_delta_x + pixel_delta_y);

  // Render
  std::cout << "P3\n" << image_width << ' ' << image_height << "\n255\n";

  for (int j = 0; j < image_height; j++) {
    std::clog << "\rScanlines remaining: " << (image_height - j) << ' '
              << std::flush;
    for (int i = 0; i < image_width; i++) {

      auto pixel_center =
          pixel00_loc + (i * pixel_delta_x) + (j * pixel_delta_y);
      auto ray_direction = pixel_center - camera_center;

      // Currently traced ray
      ray r(camera_center, ray_direction);

      // Get color for the ray
      color pixel_color = ray_color(r);
      write_color(std::cout, pixel_color);
    }
  }

  std::clog << "\rDone.                         \n";
}

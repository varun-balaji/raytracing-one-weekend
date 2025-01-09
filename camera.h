#ifndef CAMERA_H
#define CAMERA_H

#include "hittable.h"

class camera {
public:
  // Simple default values.
  double aspect_ratio = 1.0;
  int image_width = 100;
  int samples_per_pixel = 10;
  int max_depth = 10;

  void render(const hittable &world) {
    initialize();

    std::cout << "P3\n" << image_width << ' ' << image_height << "\n255\n";

    for (int j = 0; j < image_height; j++) {
      std::clog << "\rScanlines remaining: " << (image_height - j) << ' '
                << std::flush;
      for (int i = 0; i < image_width; i++) {
        color pixel_color(0, 0, 0);
        // anti-aliasing sampling here.
        for (int sample = 0; sample < samples_per_pixel; sample++) {
          ray r = get_ray(i, j);
          pixel_color += ray_color(r, max_depth, world);
        }
        write_color(std::cout, pixel_color * pixel_samples_scale);
      }
    }

    std::clog << "\rDone.                         \n";
  }

private:
  int image_height;
  double pixel_samples_scale;
  point3 camera_center;
  point3 pixel00_loc;
  vec3 pixel_delta_x;
  vec3 pixel_delta_y;

  void initialize() {
    // Image
    image_height = int(image_width / aspect_ratio);
    // Ensure image_height is at least 1 pixel
    image_height = (image_height < 1) ? 1 : image_height;

    pixel_samples_scale = 1.0 / samples_per_pixel;

    // Camera
    camera_center = point3(0, 0, 0); // Camera at origin
    auto focal_length = 1.0;         // Distance from viewport to camera
    auto viewport_height = 2.0;
    auto viewport_width =
        viewport_height * (double(image_width) / image_height);

    // Viewport dimensions are mapped to the image dimensions. This is where the
    // scalar mapping of pixels to 3D coordinate unit dimensions happens.

    // Viewport edge vectors
    auto viewport_x = vec3(viewport_width, 0, 0);
    // - sign since Y-axis pixel coords are reverse in 3D coords
    auto viewport_y = vec3(0, -viewport_height, 0);

    // Pixel delta along viewport axes. We'll use these to calculate the 3D
    // point where the ray originating from the camera passes through.
    pixel_delta_x = viewport_x / image_width;
    pixel_delta_y = viewport_y / image_height;

    // Calculate 3D coords of upper left pixel.
    auto viewport_upper_left = camera_center - vec3(0, 0, focal_length) -
                               viewport_x / 2 - viewport_y / 2;
    // Offset the pixel00 center by 0.5 of pixel_delta from viewport_upper_left
    // so that each pixel is at the center of each grid square (which has
    // dimensions pixel_delta_x by pixel_delta_y)
    pixel00_loc = viewport_upper_left + 0.5 * (pixel_delta_x + pixel_delta_y);
  }

  color ray_color(const ray &r, int depth, const hittable &world) const {
    if (depth <= 0)
      return color(0, 0, 0);

    hit_record rec;
    // Ignore rays which hit very close to the point due to floating point
    // approximations.
    if (world.hit(r, interval(0.001, infinity), rec)) {
      // Calculate diffused ray reflection.
      vec3 direction = random_on_hemisphere(rec.normal);
      return 0.5 * ray_color(ray(rec.p, direction), depth - 1, world);
    }

    // Skybox background
    vec3 unit_direction = unit_vector(r.direction());
    auto a = 0.5 * (unit_direction.y() + 1.0);
    return (1.0 - a) * color(1.0, 1.0, 1.0) + a * color(0.5, 0.7, 1.0);
  }

  ray get_ray(int i, int j) const {
    // Create a ray from camera origin to pixel (i,j) with random samples around
    // the pixel location.

    auto offset = sample_square();
    auto pixel_sample = pixel00_loc + ((i + offset.x()) * pixel_delta_x) +
                        ((j + offset.y()) * pixel_delta_y);
    auto ray_origin = camera_center;
    auto ray_direction = pixel_sample - ray_origin;
    return ray(ray_origin, ray_direction);
  }

  vec3 sample_square() const {
    // Return a random vector in the [-0.5, -0.5] - [0.5, 0.5] unit square for
    // anti-aliasing.
    return vec3(random_double() - 0.5, random_double() - 0.5, 0);
  }
};

#endif // !CAMERA_H

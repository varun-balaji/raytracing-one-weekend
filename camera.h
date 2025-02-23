#ifndef CAMERA_H
#define CAMERA_H

#include "hittable.h"
#include "material.h"
#include "rtweekend.h"
#include "vec3.h"

class camera {
public:
  // Simple default values.
  double aspect_ratio = 1.0;
  int image_width = 100;
  int samples_per_pixel = 10;
  int max_depth = 10;

  // Adjustable fov
  double vfov = 90;

  // Adjustable camera position and orientation
  point3 lookfrom = point3(0, 0, 0);
  point3 lookat = point3(0, 0, -1);
  vec3 vup = vec3(0, 1, 0);

  // Defocus blur (I really don't care for this much)
  double defocus_angle = 0; // "Variation angle of rays through each pixel".
                            // What does this even mean?
  double focus_dist =
      10; // Distance from camera lookfrom point to plane of perfect focus.

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
  int image_height; // Rendered image height in pixels
  double pixel_samples_scale;
  point3 camera_center;
  point3 pixel00_loc;
  vec3 pixel_delta_x;
  vec3 pixel_delta_y;
  vec3 u, v, w;        // Camera frame basis vectors
  vec3 defocus_disk_u; // Defocus disk horizontal radius
  vec3 defocus_disk_v; // Defocus disk vertical radius

  void initialize() {
    // Image
    image_height = int(image_width / aspect_ratio);
    // Ensure image_height is at least 1 pixel
    image_height = (image_height < 1) ? 1 : image_height;

    pixel_samples_scale = 1.0 / samples_per_pixel;

    // Camera
    camera_center = lookfrom;
    auto theta = degrees_to_radians(vfov);
    auto h = std::tan(theta / 2);
    auto viewport_height = 2 * h * focus_dist;
    auto viewport_width =
        viewport_height * (double(image_width) / image_height);

    // Viewport dimensions are mapped to the image dimensions. This is where the
    // scalar mapping of pixels to 3D coordinate unit dimensions happens.

    // Camera basis vectors
    w = unit_vector(lookfrom - lookat);
    u = unit_vector(cross(vup, w));
    v = cross(w, u);
    // Viewport edge vectors
    auto viewport_x = viewport_width * u;
    // - sign since Y-axis pixel coords are reverse in 3D coords
    auto viewport_y = viewport_height * -v;

    // Pixel delta along viewport axes. We'll use these to calculate the 3D
    // point where the ray originating from the camera passes through.
    pixel_delta_x = viewport_x / image_width;
    pixel_delta_y = viewport_y / image_height;

    // Calculate 3D coords of upper left pixel.
    auto viewport_upper_left =
        camera_center - (w * focus_dist) - viewport_x / 2 - viewport_y / 2;
    // Offset the pixel00 center by 0.5 of pixel_delta from viewport_upper_left
    // so that each pixel is at the center of each grid square (which has
    // dimensions pixel_delta_x by pixel_delta_y)
    pixel00_loc = viewport_upper_left + 0.5 * (pixel_delta_x + pixel_delta_y);

    // Calculate camera defocus disk basis vectors
    auto defocus_radius =
        focus_dist * std::tan(degrees_to_radians(defocus_angle / 2));
    defocus_disk_u = u * defocus_radius;
    defocus_disk_v = v * defocus_radius;
  }

  color ray_color(const ray &r, int depth, const hittable &world) const {
    if (depth <= 0)
      return color(0, 0, 0);

    hit_record rec;
    // Ignore rays which hit very close to the point due to floating point
    // approximations.
    if (world.hit(r, interval(0.001, infinity), rec)) {
      ray scattered;
      color attenuation;

      if (rec.mat->scatter(r, rec, attenuation, scattered)) {
        return attenuation * ray_color(scattered, depth - 1, world);
      }
      return color(0, 0, 0);
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
    auto ray_origin =
        (defocus_angle <= 0) ? camera_center : defocus_disk_sample();
    auto ray_direction = pixel_sample - ray_origin;
    return ray(ray_origin, ray_direction);
  }

  vec3 sample_square() const {
    // Return a random vector in the [-0.5, -0.5] - [0.5, 0.5] unit square for
    // anti-aliasing.
    return vec3(random_double() - 0.5, random_double() - 0.5, 0);
  }

  point3 defocus_disk_sample() const {
    // Return a random point on the camera defocus disk
    auto p = random_in_unit_disk();
    return camera_center + (p[0] * defocus_disk_u) + (p[1] * defocus_disk_v);
  }
};

#endif // !CAMERA_H

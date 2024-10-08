#ifndef CAMERA_H
#define CAMERA_H

#include "rt.h"

#include "hittable.h"
#include "material.h"

class camera {
public:
    double aspect_ratio = 1.0;  // Ratio of image width over height
    int    image_width = 100;  // Rendered image width in pixel count
    int    samples_per_pixel = 10;   // Count of random samples for each pixel
    int    max_depth = 10;   // Maximum number of ray bounces into scene

    double vfov = 90;  // Vertical view angle (field of view)
    point3 lookfrom = point3(0, 0, 0);   // Point camera is looking from
    point3 lookat = point3(0, 0, -1);  // Point camera is looking at
    vec3   vup = vec3(0, 1, 0);     // Camera-relative "up" direction

    double defocus_angle = 0;  // Variation angle of rays through each pixel
    double focus_dist = 10;    // Distance from camera lookfrom point to plane of perfect focus

        int total_frames = 1;
    double frame_duration = 1.0 / 24.0; // Default to 24 fps
    double shutter_duration = 1.0 / 48.0; // Default to half the frame duration

    void render(const hittable& world, uint8_t* pixels) {
        initialize();

        for (int j = 0; j < image_height; j++) {
            for (int i = 0; i < image_width; i++) {
                color pixel_color(0, 0, 0);
                for (int sample = 0; sample < samples_per_pixel; sample++) {
                    ray r = get_ray(i, j);
                    pixel_color += ray_color(r, max_depth, world);
                }
                int index = (j * image_width + i) * 3;

                // Gamma correction
                pixel_color = color(linear_to_gamma(pixel_samples_scale * pixel_color.x()), linear_to_gamma(pixel_samples_scale * pixel_color.y()), linear_to_gamma(pixel_samples_scale * pixel_color.z()));

                pixels[index] = std::clamp((int)(255.999 * pixel_color.x()), 0, 255);
                pixels[index + 1] = std::clamp((int)(255.999 * pixel_color.y()), 0, 255);
                pixels[index + 2] = std::clamp((int)(255.999 * pixel_color.z()), 0, 255);
            }
        }

    }


    void render_sequence(const hittable& world, SDL_Renderer* renderer, SDL_Texture* texture) {
        initialize();

        std::vector<uint8_t> pixels(image_width * image_height * 3);

        for (int frame = 0; frame < total_frames; ++frame) {
            double frame_start_time = frame * frame_duration;
            double frame_end_time = frame_start_time + shutter_duration;

            // Update all objects in the world for this frame
            update_world(world, frame_start_time);

            for (int j = 0; j < image_height; ++j) {
                for (int i = 0; i < image_width; ++i) {
                    color pixel_color(0, 0, 0);
                    for (int sample = 0; sample < samples_per_pixel; ++sample) {
                        auto time = RandomGenerator::instance().random_double(frame_start_time, frame_end_time);
                        ray r = get_ray(i, j, time);
                        pixel_color += ray_color(r, max_depth, world);
                    }

                    int index = (j * image_width + i) * 3;

                    // Gamma correction
                    pixel_color = color(linear_to_gamma(pixel_samples_scale * pixel_color.x()), linear_to_gamma(pixel_samples_scale * pixel_color.y()), linear_to_gamma(pixel_samples_scale * pixel_color.z()));

                    pixels[index] = std::clamp((int)(255.999 * pixel_color.x()), 0, 255);
                    pixels[index + 1] = std::clamp((int)(255.999 * pixel_color.y()), 0, 255);
                    pixels[index + 2] = std::clamp((int)(255.999 * pixel_color.z()), 0, 255);
                }
            }

            // Update SDL texture and render
            SDL_UpdateTexture(texture, nullptr, pixels.data(), image_width * 3);
            SDL_RenderClear(renderer);
            SDL_RenderCopy(renderer, texture, nullptr, nullptr);
            SDL_RenderPresent(renderer);

            SDL_Event event;
            while (SDL_PollEvent(&event)) {
                if (event.type == SDL_QUIT) {
                    return;
                }
            }

            // Add a small delay to control frame rate
            SDL_Delay(static_cast<Uint32>(1000 * frame_duration));
        }
    }

private:
    int    image_height;    // Rendered image height
    double pixel_samples_scale;  // Color scale factor for a sum of pixel samples
    point3 center;         // Camera center
    point3 pixel00_loc;    // Location of pixel 0, 0
    vec3   pixel_delta_u;  // Offset to pixel to the right
    vec3   pixel_delta_v;  // Offset to pixel below
    vec3   u, v, w;        // Camera frame basis vectors
    vec3   defocus_disk_u;       // Defocus disk horizontal radius
    vec3   defocus_disk_v;       // Defocus disk vertical radius

    void initialize() {
        image_height = int(image_width / aspect_ratio);
        image_height = (image_height < 1) ? 1 : image_height;

        pixel_samples_scale = 1.0 / samples_per_pixel;

        center = lookfrom;

        // Determine viewport dimensions.
        auto theta = degrees_to_radians(vfov);
        auto h = std::tan(theta / 2);
        auto viewport_height = 2 * h * focus_dist;
        auto viewport_width = viewport_height * (double(image_width) / image_height);

        // Calculate the u,v,w unit basis vectors for the camera coordinate frame.
        w = unit_vector(lookfrom - lookat);
        u = unit_vector(cross(vup, w));
        v = cross(w, u);

        // Calculate the vectors across the horizontal and down the vertical viewport edges.
        vec3 viewport_u = viewport_width * u;    // Vector across viewport horizontal edge
        vec3 viewport_v = viewport_height * -v;  // Vector down viewport vertical edge

        // Calculate the horizontal and vertical delta vectors from pixel to pixel.
        pixel_delta_u = viewport_u / image_width;
        pixel_delta_v = viewport_v / image_height;

        // Calculate the location of the upper left pixel.
        auto viewport_upper_left = center - (focus_dist * w) - viewport_u/2 - viewport_v/2;
        pixel00_loc = viewport_upper_left + 0.5 * (pixel_delta_u + pixel_delta_v);

        // Calculate the camera defocus disk basis vectors.
        auto defocus_radius = focus_dist * std::tan(degrees_to_radians(defocus_angle / 2));
        defocus_disk_u = u * defocus_radius;
        defocus_disk_v = v * defocus_radius;
    }

    color ray_color(const ray& r, int depth, const hittable& world) const {
        if (depth <= 0)
            return color(0, 0, 0);

        hit_record rec;

        if (world.hit(r, interval(0.001, infinity), rec)) {
            ray scattered;
            color attenuation;
            if (rec.mat->scatter(r, rec, attenuation, scattered))
                return attenuation * ray_color(scattered, depth - 1, world);
            return color(0, 0, 0);
        }

        vec3 unit_direction = unit_vector(r.direction());
        auto a = 0.5 * (unit_direction.y() + 1.0);
        return (1.0 - a) * color(1.0, 1.0, 1.0) + a * color(0.5, 0.7, 1.0);
    }


    ray get_ray(int i, int j) const {
        // Construct a camera ray originating from the origin and directed at randomly sampled
        // point around the pixel location i, j.

        auto offset = sample_square();
        auto pixel_sample = pixel00_loc
            + ((i + offset.x()) * pixel_delta_u)
            + ((j + offset.y()) * pixel_delta_v);

        auto ray_origin = (defocus_angle <= 0) ? center : defocus_disk_sample();
        auto ray_direction = pixel_sample - ray_origin;
        auto ray_time = RandomGenerator::instance().random_double();

        return ray(ray_origin, ray_direction, ray_time);
    }

    ray get_ray(int i, int j, double ray_time) const {
        // Construct a camera ray originating from the origin and time directed from function arguments.

        auto offset = sample_square();
        auto pixel_sample = pixel00_loc
            + ((i + offset.x()) * pixel_delta_u)
            + ((j + offset.y()) * pixel_delta_v);

        auto ray_origin = (defocus_angle <= 0) ? center : defocus_disk_sample();
        auto ray_direction = pixel_sample - ray_origin;

        return ray(ray_origin, ray_direction, ray_time);
    }

    void update_world(const hittable& world, double time) const {
        const hittable_list& world_list = static_cast<const hittable_list&>(world);
        for (const auto& object : world_list.objects) {
            const_cast<hittable*>(object.get())->update(time);
        }
    }

    vec3 sample_square() const {
        // Returns the vector to a random point in the [-.5,-.5]-[+.5,+.5] unit square.
        return vec3(RandomGenerator::instance().random_double() - 0.5, RandomGenerator::instance().random_double() - 0.5, 0);
    }
    
    point3 defocus_disk_sample() const {
        // Returns a random point in the camera defocus disk.
        auto p = random_in_unit_disk();
        return center + (p[0] * defocus_disk_u) + (p[1] * defocus_disk_v);
    }

    inline double linear_to_gamma(double linear_component)
    {
        if (linear_component > 0)
            return std::sqrt(linear_component);

        return 0;
    }

};

#endif
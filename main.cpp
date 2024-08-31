#define SDL_MAIN_HANDLED
#include "SDL.h"
#include "rt.h"

#include "bvh.h"
#include "camera.h"
#include "hittable.h"
#include "hittable_list.h"
#include "material.h"
#include "sphere.h"
#include "obj_loader.h"

void bouncing_spheres(SDL_Window* window, SDL_Renderer* renderer, SDL_Texture* texture, int image_width){
    
    int image_height = int(image_width / (16.0 / 9.0));

    hittable_list world;

    auto checker = make_shared<checker_texture>(0.32, color(.2, .3, .1), color(.9, .9, .9));
    world.add(make_shared<sphere>(point3(0,-1000,0), 1000, make_shared<lambertian>(checker)));

    for (int a = -11; a < 11; a++) {
        for (int b = -11; b < 11; b++) {
            auto choose_mat = RandomGenerator::instance().random_double();
            point3 center(a + 0.9 * RandomGenerator::instance().random_double(), 0.2, b + 0.9 * RandomGenerator::instance().random_double());

            if ((center - point3(4, 0.2, 0)).length() > 0.9) {
                shared_ptr<material> sphere_material;

                if (choose_mat < 0.8) {
                    // diffuse
                    auto albedo = color::random() * color::random();
                    sphere_material = make_shared<lambertian>(albedo);
                    auto center2 = center + vec3(0, RandomGenerator::instance().random_double(0, .5), 0);
                    world.add(make_shared<sphere>(center, center2, 0.2, sphere_material));
                }
                else if (choose_mat < 0.95) {
                    auto albedo = color::random(0.5, 1);
                    auto fuzz = RandomGenerator::instance().random_double(0, 0.5);
                    sphere_material = make_shared<metal>(albedo, fuzz);
                    world.add(make_shared<sphere>(center, 0.2, sphere_material));
                }
                else {
                    sphere_material = make_shared<dielectric>(1.5);
                    world.add(make_shared<sphere>(center, 0.2, sphere_material));
                }
            }
        }
    }

    auto material1 = make_shared<dielectric>(1.5);
    world.add(make_shared<sphere>(point3(0, 1, 0), 1.0, material1));

    auto material2 = make_shared<lambertian>(color(0.4, 0.2, 0.1));
    world.add(make_shared<sphere>(point3(-4, 1, 0), 1.0, material2));

    auto material3 = make_shared<metal>(color(0.7, 0.6, 0.5), 0.0);
    world.add(make_shared<sphere>(point3(4, 1, 0), 1.0, material3));

    world = hittable_list(make_shared<bvh_node>(world));

    camera cam;

    cam.aspect_ratio = 16.0 / 9.0;
    cam.image_width = image_width;
    cam.samples_per_pixel = 10;
    cam.max_depth = 50;

    cam.vfov = 105;
    cam.lookfrom = point3(13, 2, 3);
    cam.lookat = point3(0, 0, 0);
    cam.vup = vec3(0, 1, 0);

    cam.defocus_angle = 0.6;
    cam.focus_dist = 10.0;

    std::vector<uint8_t> pixels(image_width * image_height * 3);

    cam.render(world, pixels.data());

    SDL_UpdateTexture(texture, nullptr, pixels.data(), image_width * 3);
    SDL_RenderClear(renderer);
    SDL_RenderCopy(renderer, texture, nullptr, nullptr);
    SDL_RenderPresent(renderer);
}

void checkered_spheres(SDL_Window* window, SDL_Renderer* renderer, SDL_Texture* texture, int image_width) {
    int image_height = int(image_width / (16.0 / 9.0));

    hittable_list world;

    auto checker = make_shared<checker_texture>(0.32, color(.2, .3, .1), color(.9, .9, .9));

    world.add(make_shared<sphere>(point3(0, -10, 0), 10, make_shared<lambertian>(checker)));
    world.add(make_shared<sphere>(point3(0, 10, 0), 10, make_shared<lambertian>(checker)));

    camera cam;

    cam.aspect_ratio = 16.0 / 9.0;
    cam.image_width = image_width;
    cam.samples_per_pixel = 5;
    cam.max_depth = 50;

    cam.vfov = 20;
    cam.lookfrom = point3(13, 2, 3);
    cam.lookat = point3(0, 0, 0);
    cam.vup = vec3(0, 1, 0);

    cam.defocus_angle = 0;

    std::vector<uint8_t> pixels(image_width * image_height * 3);

    cam.render(world, pixels.data());

    SDL_UpdateTexture(texture, nullptr, pixels.data(), image_width * 3);
    SDL_RenderClear(renderer);
    SDL_RenderCopy(renderer, texture, nullptr, nullptr);
    SDL_RenderPresent(renderer);
}

void earth(SDL_Window* window, SDL_Renderer* renderer, SDL_Texture* texture, int image_width) {

    int image_height = int(image_width / (16.0 / 9.0));

    auto earth_texture = make_shared<image_texture>("earthmap.jpg");
    auto earth_surface = make_shared<lambertian>(earth_texture);
    auto globe = make_shared<sphere>(point3(0, 0, 0), 2, earth_surface);
    auto rotating_globe = make_shared<rotating_sphere>(globe, 240.25);  // rotation degree per frame

    camera cam;

    cam.aspect_ratio = 16.0 / 9.0;
    cam.image_width = image_width;
    cam.samples_per_pixel = 2;
    cam.max_depth = 50;

    cam.vfov = 20;
    cam.lookfrom = point3(0, 0, 12);
    cam.lookat = point3(0, 0, 0);
    cam.vup = vec3(0, 1, 0);

    cam.defocus_angle = 0;

    std::vector<uint8_t> pixels(image_width * image_height * 3);

    cam.total_frames = 240;  // 10 seconds at 24 fps
    cam.frame_duration = 1.0 / 24.0;  // 24 fps
    cam.shutter_duration = 1.0 / 48.0;  // Half the frame duration

    cam.render_sequence(hittable_list(rotating_globe), renderer, texture);
}

int main(int argc, char* argv[]) {

    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        std::cerr << "SDL_Init Error: " << SDL_GetError() << std::endl;
        return 1;
    }

    int image_width = 400;
    int image_height = int(image_width / (16.0 / 9.0));

    SDL_Window* window = SDL_CreateWindow("Ray Tracer", 100, 100, image_width, image_height, SDL_WINDOW_SHOWN);
    if (window == nullptr) {
        std::cerr << "SDL_CreateWindow Error: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return 1;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (renderer == nullptr) {
        SDL_DestroyWindow(window);
        std::cerr << "SDL_CreateRenderer Error: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return 1;
    }

    SDL_Texture* texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGB24, SDL_TEXTUREACCESS_STREAMING, image_width, image_height);
    if (texture == nullptr) {
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        std::cerr << "SDL_CreateTexture Error: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return 1;
    }

    switch (3) {
        case 1: bouncing_spheres(window,renderer,texture,image_width);  break;
        case 2: checkered_spheres(window, renderer, texture, image_width); break;
        case 3: earth(window, renderer, texture, image_width); break;
    }

    SDL_Event e;
    bool quit = false;
    while (!quit) {
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) {
                quit = true;
            }
        }
    }

    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}

#include <iostream>
#include "scene.h"
#include "opengl_handlers.h"
#include "opengl_utils.h"

int main(int argc, char* argv[]) {
    if (argc < 3 || argc > 5) {
        std::cerr << "Usage: " << argv[0] << " [scene_description_file.txt] [xres] [yres] [mode]" << std::endl;
        std::cerr << "or: " << argv[0] << " [color_texture.png] [normal_map.png] [xres] [yres]" << std::endl;
        return 1;
    }

    std::string arg4 = argv[1];
    bool ends_with_png = arg4.length() >= 4 && arg4.substr(arg4.length() - 4) == ".png";

    std::string scene_filename = ends_with_png ? "../data/part2_hardcoded.txt" : argv[1];
    
    // Create and load the scene
    scene::SceneFile scene(scene_filename);
    opengl_handlers::scene = &scene;
    int res_x = 1280;
    int res_y = 720;

    // std::cout << "ends_with_png: " << ends_with_png << std::endl;
    if (ends_with_png && (argc == 5 || argc == 3)) {
        opengl_handlers::shader_mode = 2;
        if (argc == 5) {
            res_x = std::stoi(argv[3]);
            res_y = std::stoi(argv[4]);
        }
        opengl_handlers::texture_png_path = std::string(argv[1]);
        opengl_handlers::normal_png_path = std::string(argv[2]);
    } else {
        opengl_handlers::shader_mode = 0;

        if (argc == 5) {
            int mode = std::stoi(argv[4]);
            if (mode != 0 && mode != 1) {
                std::cerr << "Invalid mode: " << mode << " (0 for Gouraud, 1 for Phong)" << std::endl;
                return 1;
            }
            opengl_handlers::shader_mode = mode;
        }
        
        res_x = std::stoi(argv[2]);
        res_y = std::stoi(argv[3]);
    }


    opengl_utils::init_window(argc, argv, res_x, res_y);
    opengl_utils::start_scene_rendering(scene);
    
    return 0;
}


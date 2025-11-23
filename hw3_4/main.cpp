#include <iostream>
#include "scene.h"
#include "opengl_handlers.h"
#include "opengl_utils.h"

int main(int argc, char* argv[]) {
    if (argc != 4 && argc != 5) {
        std::cerr << "Usage: " << argv[0] << " [scene_description_file.txt] [xres] [yres] [mode]" << std::endl;
        return 1;
    }


    std::string scene_filename = argv[1];
    
    // Create and load the scene
    scene::SceneFile scene(scene_filename);
    opengl_handlers::scene = &scene;
    opengl_handlers::shader_mode = 0;

    if (argc == 5) {
        int mode = std::stoi(argv[4]);
        if (mode != 0 && mode != 1) {
            std::cerr << "Invalid mode: " << mode << " (0 for Gouraud, 1 for Phong)" << std::endl;
            return 1;
        }
        opengl_handlers::shader_mode = mode;
    }

    opengl_utils::init_window(argc, argv, std::stoi(argv[2]), std::stoi(argv[3]));
    opengl_utils::start_scene_rendering(scene);
    
    return 0;
}


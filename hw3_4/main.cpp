#include <iostream>
#include "scene.h"
#include "opengl_handlers.h"
#include "opengl_utils.h"

int main(int argc, char* argv[]) {
    if (argc != 4) {
        std::cerr << "Usage: " << argv[0] << " [scene_description_file.txt] [xres] [yres]" << std::endl;
        return 1;
    }

    std::string scene_filename = argv[1];
    
    // Create and load the scene
    scene::SceneFile scene(scene_filename);
    opengl_handlers::scene = &scene;

    opengl_utils::init_window(argc, argv, std::stoi(argv[2]), std::stoi(argv[3]));
    opengl_utils::start_scene_rendering(scene);
    
    return 0;
}


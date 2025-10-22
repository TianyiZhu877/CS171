#include <iostream>
#include "scene.h"

int main(int argc, char* argv[]) {
    if (argc != 4 && argc != 5) {
        std::cerr << "Usage: " << argv[0] << " [scene_description_file.txt] [xres] [yres] [optional mode]" << std::endl;
        return 1;
    }

    std::string scene_filename = argv[1];
    
    // Create and load the scene
    scene::SceneFile scene(scene_filename);

    // Parse optional mode argument
    scene::SceneFile::RenderMode mode = scene::SceneFile::RenderMode::GOURAUD;
    if (argc == 5) {
        int mode_int = std::stoi(argv[4]);
        if (mode_int < 0 || mode_int > 2) {
            std::cerr << "Error: Mode must be 0 (GOURAUD), 1 (PHONG), or 2 (EDGES)" << std::endl;
            return 1;
        }
        mode = static_cast<scene::SceneFile::RenderMode>(mode_int);
    }
    
    // Print scene information
    // std::cout << "Scene loaded successfully!" << std::endl;
    // std::cout << "\nCamera settings:" << std::endl;
    // scene.camera.serialize();
    
    // std::cout << "\nLoaded " << scene.objects.size() << " object(s):" << std::endl;
    // for (const auto& obj : scene.objects) {
    //     std::cout << "  - " << obj.name << " (" << obj.points.cols() << " vertices)" << std::endl;
    // }

    ::ppm_image::PPMImage<float> image = scene.render(std::stoi(argv[2]), std::stoi(argv[3]), mode);
    image.serialize();
    
    return 0;
}


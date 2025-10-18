#include <iostream>
#include "scene.h"

int main(int argc, char* argv[]) {
    if (argc != 4) {
        std::cerr << "Usage: " << argv[0] << "[scene_description_file.txt] [xres] [yres]" << std::endl;
        return 1;
    }

    std::string scene_filename = argv[1];
    
    // Create and load the scene
    scene::SceneFile scene(scene_filename);
    
    // Print scene information
    // std::cout << "Scene loaded successfully!" << std::endl;
    // std::cout << "\nCamera settings:" << std::endl;
    // scene.camera.serialize();
    
    // std::cout << "\nLoaded " << scene.objects.size() << " object(s):" << std::endl;
    // for (const auto& obj : scene.objects) {
    //     std::cout << "  - " << obj.name << " (" << obj.points.cols() << " vertices)" << std::endl;
    // }

    ::ppm_image::PPMImage<uint8_t> image = scene.render(std::stoi(argv[2]), std::stoi(argv[3]));
    image.serialize();
    
    return 0;
}


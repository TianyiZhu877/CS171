#ifndef OPENGL_UTILS_H
#define OPENGL_UTILS_H
#include <string>
#include "scene.h"
#include "opengl_handlers.h"

/* Some utility functions to interact with OpenGL and setup the scene*/
namespace opengl_utils {
    // The functions to be called in order to start the scene rendering
    void init_window(int argc, char* argv[], int xres = 1280, int yres = 720, std::string window_name = "OpenGL Scene");
    void start_scene_rendering(scene::SceneFile& scene = (*opengl_handlers::scene));

    // Tow helper functions to called in start_scene_rendering
    void init_lights(scene::SceneFile& scene);
    void init_camera(scene::SceneFile& scene);

    // Print the OpenGL modelview matrix at the instance it is called for debugging
    void print_model_matrices();
}

#endif // OPENGL_UTILS_H

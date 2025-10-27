#ifndef OPENGL_UTILS_H
#define OPENGL_UTILS_H
#include <string>
#include "scene.h"
#include "opengl_handlers.h"

namespace opengl_utils {
    void init_window(int argc, char* argv[], int xres = 1280, int yres = 720, std::string window_name = "OpenGL Scene");
    void init_lights(scene::SceneFile& scene);
    void init_camera(scene::SceneFile& scene);
    void start_scene_rendering(scene::SceneFile& scene = (*opengl_handlers::scene));
}

#endif // OPENGL_UTILS_H

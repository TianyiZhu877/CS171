#ifndef OPENGL_HANDLERS_H
#define OPENGL_HANDLERS_H

// Forward declarations
namespace scene {
    class Scene;
}

namespace opengl_handlers {
    // Global scene pointer, needs to be manually point to the scene object to be rendered
    extern scene::SceneFile* scene;
    
    // Helper functions
    namespace helpers {
        void camera_transform();
        Eigen::Matrix4d compute_rotation_quaternion(int x, int y, int p_start_x, int p_start_y);
        void set_lights();
        void draw_objects();
    }
    
    // Main GLUT callback functions:
    void display(void);
    void window_resize(int width, int height);
    void mouse_pressed(int button, int state, int x, int y);
    void mouse_motion(int x, int y);
}

#endif // OPENGL_HANDLERS_H

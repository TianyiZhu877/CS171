#ifndef OPENGL_HANDLERS_H
#define OPENGL_HANDLERS_H

// Forward declarations
namespace scene {
    class Scene;
}

namespace opengl_handlers {
    // Global scene pointer
    extern scene::SceneFile* scene;
    
    // Helper functions
    namespace helpers {
        void set_lights();
        void draw_objects();
        void print_model_matrices();
    }
    
    // Main GLUT callback functions
    void display(void);
    void window_resize(int width, int height);
}

#endif // OPENGL_HANDLERS_H

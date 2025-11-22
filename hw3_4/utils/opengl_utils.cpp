#include "include/opengl_utils.h"

#include <GL/glew.h>
#include <GL/glut.h>
#include <iostream>
#include <string>
#include <cmath>

namespace opengl_utils {

void init_window(int argc, char* argv[], int xres, int yres, std::string window_name) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(xres, yres);
    glutInitWindowPosition(0, 0);
    glutCreateWindow(window_name.c_str());
    
    glShadeModel(GL_SMOOTH);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_NORMALIZE);
    
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_NORMAL_ARRAY);
}


void init_lights(scene::SceneFile& scene) {
    glEnable(GL_LIGHTING);
    int light_id = GL_LIGHT0;
    for (const auto& light : scene.lights) {
        glEnable(light_id);
        glLightfv(light_id, GL_AMBIENT, light.color.data());
        glLightfv(light_id, GL_DIFFUSE, light.color.data());
        glLightfv(light_id, GL_SPECULAR, light.color.data());
        glLightf(light_id, GL_QUADRATIC_ATTENUATION, light.k);
        light_id++;
    }
    // std::cout << "number of lights: " << light_id - GL_LIGHT0 << std::endl;
}

void init_camera(scene::SceneFile& scene) {
    // Set the camear projection 
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    const auto& camera = scene.camera;
    glFrustum(camera.l, camera.r, camera.b, camera.t, camera.n, camera.f);

    // Set the camera transformation
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glRotatef(-camera.orientation(3) * 180.0 / M_PI, camera.orientation(0), camera.orientation(1), camera.orientation(2));
    glTranslatef(-camera.position.x(), -camera.position.y(), -camera.position.z());
}

void start_scene_rendering(scene::SceneFile& scene) {
    init_camera(scene);
    init_lights(scene);

    // Set the GLUT callback functions
    glutDisplayFunc(opengl_handlers::display);
    glutReshapeFunc(opengl_handlers::window_resize);
    glutMouseFunc(opengl_handlers::mouse_pressed);
    glutMotionFunc(opengl_handlers::mouse_motion);
    glutMainLoop();
}


void print_model_matrices() {
    GLfloat modelview[16];
    GLfloat projection[16];
    
    glGetFloatv(GL_MODELVIEW_MATRIX, modelview);
    glGetFloatv(GL_PROJECTION_MATRIX, projection);
    
    std::cout << "ModelView Matrix:" << std::endl;
    for (int i = 0; i < 4; i++) {
        std::cout << "  ";
        for (int j = 0; j < 4; j++) {
            std::cout << std::fixed << std::setw(10) << std::setprecision(4) << modelview[i + j * 4];
        }
        std::cout << std::endl;
    }

    std::cout << "================================\n\n" << std::endl;
}

} // namespace opengl_utils

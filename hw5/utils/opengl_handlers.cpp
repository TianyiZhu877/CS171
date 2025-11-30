#include <GL/glew.h>
#include <GL/glut.h>
#include "Eigen/Dense"

#include "include/scene.h"
#include "include/models.h"
#include "iostream"
#include <iomanip>
#include "include/transformation.h"

// forward declaration
namespace opengl_utils {
    void print_model_matrices();
} 


namespace opengl_handlers {
    scene::SceneFile* scene;
    static Eigen::Vector4d last_rotation_quat = Eigen::Vector4d(0, 0, 0, 1);  // Identity quaternion (x,y,z,w)
    static Eigen::Vector4d current_rotation_quat = Eigen::Vector4d(0, 0, 0, 1);  // Identity quaternion (x,y,z,w)
    static int p_start_x, p_start_y;
    static bool is_pressed = false;


namespace helpers {



void camera_transform() {
    // First, apply the the initial camera transformation
    const auto& camera = scene->camera;
    glLoadIdentity();
    glRotatef(-camera.orientation(3) * 180.0 / M_PI, camera.orientation(0), camera.orientation(1), camera.orientation(2));
    glTranslatef(-camera.position.x(), -camera.position.y(), -camera.position.z());

    // Then, apply the current rotation quaternion
    Eigen::Vector4d combined_quat = ::transformation::quaternion_multiply(current_rotation_quat, last_rotation_quat);
    Eigen::Matrix4d rotation_matrix = ::transformation::get_martix_4x4_from_quaternion(combined_quat);
    glMultMatrixd(rotation_matrix.data());
}

Eigen::Vector4d compute_rotation_quaternion(int x, int y, int p_start_x, int p_start_y) {
    // Get window dimensions
    int width = glutGet(GLUT_WINDOW_WIDTH);
    int height = glutGet(GLUT_WINDOW_HEIGHT);
    
    Eigen::Vector3d current_ndc = transformation::screen_to_ndc_unit_sphere(x, y, width, height);
    Eigen::Vector3d start_ndc = transformation::screen_to_ndc_unit_sphere(p_start_x, p_start_y, width, height);
    
    // Compute rotation quaternion from two unit vectors
    Eigen::Vector3d rotation_axis = start_ndc.cross(current_ndc);
    double rotation_angle = std::acos(std::min(1.0, start_ndc.dot(current_ndc)));
    
    rotation_axis.normalize();
    
    double half_angle = rotation_angle * 0.5;
    double s = std::sin(half_angle);
    double c = std::cos(half_angle);
    
    Eigen::Vector4d quaternion;
    quaternion(0) = rotation_axis.x() * s;  // x
    quaternion(1) = rotation_axis.y() * s;  // y  
    quaternion(2) = rotation_axis.z() * s;  // z
    quaternion(3) = c;                     // w (scalar)
    
    return quaternion;
}

void set_lights() {
    int light_id = GL_LIGHT0;
    for (const auto& light : scene->lights) {
        glLightfv(light_id, GL_POSITION, light.position.data());
        light_id++;
    }
}

void draw_objects() {
    for (const auto& model : scene->objects) {
        glPushMatrix(); {
            glMultMatrixd(model.transform.data());

            
            // Set material properties
            glMaterialfv(GL_FRONT, GL_AMBIENT, model.ambient.data());
            glMaterialfv(GL_FRONT, GL_DIFFUSE, model.diffuse.data());
            glMaterialfv(GL_FRONT, GL_SPECULAR, model.specular.data());
            glMaterialf(GL_FRONT, GL_SHININESS, model.shininess);
            
            // Set vertex and normal pointers
            // glNormalPointer(GL_FLOAT, sizeof(Eigen::Vector3f), model.obj_file->normals.data());
            // glVertexPointer(3, GL_FLOAT, sizeof(Eigen::Vector3f), model.obj_file->vertexes.data());
            // glDrawElements(GL_TRIANGLES, 3*model.obj_file->faces_opengl.size(), GL_UNSIGNED_INT, model.obj_file->faces_opengl.data());
            
            if (!model.halfedges_vertices.empty() && model.vertices_cached && model.normals_cached && model.raw_faces) {

                // std::cout << "drawing" << std::endl;
                
                glVertexPointer(3, GL_FLOAT, sizeof(halfedge::Vertex), model.vertices_cached+1);
                glNormalPointer(GL_FLOAT, sizeof(halfedge::Vec3f), model.normals_cached+1);
                glDrawElements(GL_TRIANGLES, 3*model.obj_file->faces_opengl.size(), GL_UNSIGNED_INT, model.obj_file->faces_opengl.data());
                // glDrawElements(GL_TRIANGLES, 3*model.num_faces, GL_UNSIGNED_INT, reinterpret_cast<const GLuint*>(model.raw_faces));
                
            }

        } glPopMatrix();
    }
}
} // namespace helpers

void display(void) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    helpers::set_lights();
    glMatrixMode(GL_MODELVIEW);
    helpers::camera_transform();
    helpers::draw_objects();
    
    glutSwapBuffers();
}

void window_resize(int width, int height) {
    height = (height == 0) ? 1 : height;
    width = (width == 0) ? 1 : width;
    
    glViewport(0, 0, width, height);
    
    glutPostRedisplay();
}


void mouse_pressed(int button, int state, int x, int y) {
    if(button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
        p_start_x = x;
        p_start_y = y;
        is_pressed = true;
        // std::cout << "Mouse pressed at: " << x << ", " << y << std::endl;
    }
    else if(button == GLUT_LEFT_BUTTON && state == GLUT_UP) {
        is_pressed = false;
        last_rotation_quat = ::transformation::quaternion_multiply(current_rotation_quat, last_rotation_quat);
        current_rotation_quat = Eigen::Vector4d(0, 0, 0, 1);  // Reset to identity quaternion
    }
}


void mouse_motion(int x, int y) {
    if (is_pressed) {
        // std::cout << "Mouse dragging at: " << x << ", " << y << std::endl;
        current_rotation_quat = helpers::compute_rotation_quaternion(x, y, p_start_x, p_start_y);
        glutPostRedisplay();  
    }
}



} // namespace opengl_handlers



#include <GL/glew.h>
#include <GL/glut.h>
#include "Eigen/Dense"

#include "include/scene.h"
#include "include/models.h"
#include "iostream"
#include <iomanip>

// forward declaration
namespace opengl_utils {
    void print_model_matrices();
} // namespace opengl_utils


namespace opengl_handlers {
    scene::SceneFile* scene;

namespace helpers {

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
            // Don't know why but openGL supposingly use column-major order for matrices,
            // but doing this transpose generates the incorrect result?!
            // Eigen::Matrix4d transposed_transform = model.transform.transpose();
            glMultMatrixd(model.transform.data());

            // glLoadIdentity();
            // Eigen::Matrix4d hardcoded_matrix;
            // // hardcoded_matrix << 1.0, 0.0, 0.0, 0.4,
            // //                     0.0, 1.0, 0.0, -0.9,
            // //                     0.0, 0.0, 1.0, -4.0,
            // //                     0.0, 0.0, 0.0, 1.0;
            // hardcoded_matrix <<  1.0222 , -0.3592 , 0.7184 , 0.9330, 
            //                   0.3592 , 1.2444 , 0.1111 , -0.8319,
            //                   -0.7184 , 0.1111 , 1.0778 , -5.5361,
            //                   0.0000 , 0.0000 , 0.0000 , 1.0000;
            // // hardcoded_matrix <<  0.853588 ,  0.146412 , 0.499951 , 0,
            // //                    0.146412 , 0.853588 , -0.499951 , 0,
            // //                    -0.499951 , 0.499951 , 0.707176 , -5,
            // //                    0 , 0 , 0 , 1;
            // // hardcoded_matrix = hardcoded_matrix.transpose();
            // glMultMatrixd(hardcoded_matrix.data());
            
            // Set material properties
            glMaterialfv(GL_FRONT, GL_AMBIENT, model.ambient.data());
            glMaterialfv(GL_FRONT, GL_DIFFUSE, model.diffuse.data());
            glMaterialfv(GL_FRONT, GL_SPECULAR, model.specular.data());
            glMaterialf(GL_FRONT, GL_SHININESS, model.shininess);
            
            // std::cout << "Model: " << model.name << std::endl;
            // std::cout << "Model Transform Matrix (camera frame):" << std::endl;
            // std::cout << scene->camera.get_transformation().inverse()*(model.transform) << std::endl;
            // opengl_utils::print_model_matrices();
            
            // Set vertex and normal pointers
            glVertexPointer(3, GL_FLOAT, sizeof(Eigen::Vector3f), model.obj_file->vertexes.data());
            glNormalPointer(GL_FLOAT, sizeof(Eigen::Vector3f), model.obj_file->normals.data());
            
            // glDrawArrays(GL_TRIANGLES, 0, model.obj_file->vertexes.size());
            // glDrawArrays(GL_LINE_STRIP, 0, model.obj_file->vertexes.size());
            // glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, model.obj_file->faces_opengl.data());
            
            if (model.obj_file->drawElement_compatible) {
                // std::cout << "Drawing model: " << model.name << " with DrawElements" << std::endl;
                glDrawElements(GL_TRIANGLES, 3*model.obj_file->faces_opengl.size(), GL_UNSIGNED_INT, model.obj_file->faces_opengl.data());
            } else {
                // std::cout << "Drawing model: " << model.name << " with drawArrays" << std::endl;
                glDrawArrays(GL_TRIANGLES, 0, model.obj_file->vertexes.size());
            }

            // std::cout << "Drawing model: " << model.name << std::endl;
        } glPopMatrix();
    }
}
} // namespace helpers

void display(void) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    helpers::set_lights();
    glMatrixMode(GL_MODELVIEW);
    helpers::draw_objects();
    
    glutSwapBuffers();
}

void window_resize(int width, int height) {
    height = (height == 0) ? 1 : height;
    width = (width == 0) ? 1 : width;
    
    glViewport(0, 0, width, height);
    
    glutPostRedisplay();
}

}
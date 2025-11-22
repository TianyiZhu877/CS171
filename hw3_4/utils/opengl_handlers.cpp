#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glew.h>
#include <GL/glut.h>
#include "Eigen/Dense"
#include <iostream>
#include <iomanip>
#include <fstream>
#include <string>
#include <vector>
#include <sstream>

#include "include/scene.h"
#include "include/models.h"
#include "include/transformation.h"

// forward declaration
namespace opengl_utils {
    void print_model_matrices();
} 


namespace opengl_handlers {
    scene::SceneFile* scene;
    int shader_mode = 0;
    static Eigen::Vector4d last_rotation_quat = Eigen::Vector4d(0, 0, 0, 1);  // Identity quaternion (x,y,z,w)
    static Eigen::Vector4d current_rotation_quat = Eigen::Vector4d(0, 0, 0, 1);  // Identity quaternion (x,y,z,w)
    static int p_start_x, p_start_y;
    static bool is_pressed = false;


namespace helpers {

static const char* vertProgFileName = "shaders/vertex.glsl";
static const char* fragProgFileName = "shaders/fragment.glsl";
static GLuint shaderProgram = 0;
static bool shadersLoaded = false;

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

void set_light_uniforms() {
    // Pass lighting information to shader as uniforms
    // This assumes your shader has uniforms like:
    // - numLights (int)
    // - lightPositions[] (vec3 array)
    // - lightColors[] (vec3 array)
    // - lightAttenuations[] (float array)
    
    if (shaderProgram == 0) return;
    
    int numLights = scene->lights.size();
    
    // Set number of lights
    GLint numLightsLoc = glGetUniformLocation(shaderProgram, "numLights");
    if (numLightsLoc != -1) {
        glUniform1i(numLightsLoc, numLights);
    }
    
    // Set light positions, colors, and attenuations
    // Assuming max 8 lights (common limit), you can adjust based on your shader
    for (size_t i = 0; i < numLights && i < 8; i++) {
        const auto& light = scene->lights[i];
        
        // Light position
        std::string posName = "lightPositions[" + std::to_string(i) + "]";
        GLint posLoc = glGetUniformLocation(shaderProgram, posName.c_str());
        if (posLoc != -1) {
            glUniform3fv(posLoc, 1, light.position.data());
        }
        
        // Light color
        std::string colorName = "lightColors[" + std::to_string(i) + "]";
        GLint colorLoc = glGetUniformLocation(shaderProgram, colorName.c_str());
        if (colorLoc != -1) {
            glUniform3fv(colorLoc, 1, light.color.data());
        }
        
        // Light attenuation (k)
        std::string attenName = "lightAttenuations[" + std::to_string(i) + "]";
        GLint attenLoc = glGetUniformLocation(shaderProgram, attenName.c_str());
        if (attenLoc != -1) {
            glUniform1f(attenLoc, light.k);
        }
    }
}

void draw_objects() {
    for (const auto& model : scene->objects) {
        glPushMatrix(); {
            // Don't know why but openGL supposingly use post-multiplication for matrices,
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
            
            // Draw the model using the appropriate method
            if (model.obj_file->drawElement_compatible) {
            // If the vertexes and normal indices are the same for each face, DrawElements could be used
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


static void readShaders() {
    if (shadersLoaded) {
        return; // Already loaded
    }
    
    std::string vertProgramSource, fragProgramSource;
    
    // Read vertex shader
    std::ifstream vertProgFile(vertProgFileName);
    if (!vertProgFile.is_open()) {
        std::cerr << "Error opening vertex shader program: " << vertProgFileName << std::endl;
        return;
    }
    
    std::stringstream vertBuffer;
    vertBuffer << vertProgFile.rdbuf();
    vertProgramSource = vertBuffer.str();
    vertProgFile.close();
    
    // Read fragment shader
    std::ifstream fragProgFile(fragProgFileName);
    if (!fragProgFile.is_open()) {
        std::cerr << "Error opening fragment shader program: " << fragProgFileName << std::endl;
        return;
    }
    
    std::stringstream fragBuffer;
    fragBuffer << fragProgFile.rdbuf();
    fragProgramSource = fragBuffer.str();
    fragProgFile.close();
    
    const char* vertShaderSource = vertProgramSource.c_str();
    const char* fragShaderSource = fragProgramSource.c_str();
 
    // Initialize shaders
    GLuint vertShader, fragShader;
 
    shaderProgram = glCreateProgram();
    if (shaderProgram == 0) {
        std::cerr << "Error creating shader program" << std::endl;
        return;
    }
 
    // Compile vertex shader
    vertShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertShader, 1, &vertShaderSource, NULL);
    glCompileShader(vertShader);
     
    GLint isCompiled = 0;
    glGetShaderiv(vertShader, GL_COMPILE_STATUS, &isCompiled);
    if(isCompiled == GL_FALSE) {
       GLint maxLength = 0;
       glGetShaderiv(vertShader, GL_INFO_LOG_LENGTH, &maxLength);
     
       std::vector<GLchar> errorLog(maxLength);
       glGetShaderInfoLog(vertShader, maxLength, &maxLength, &errorLog[0]);
     
       std::cerr << "Vertex shader compilation error:" << std::endl;
       for (int i = 0; i < errorLog.size(); i++)
          std::cerr << errorLog[i];
       std::cerr << std::endl;
       glDeleteShader(vertShader);
       glDeleteProgram(shaderProgram);
       shaderProgram = 0;
       return;
    }
 
    // Compile fragment shader
    fragShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragShader, 1, &fragShaderSource, NULL);
    glCompileShader(fragShader);
 
    isCompiled = 0;
    glGetShaderiv(fragShader, GL_COMPILE_STATUS, &isCompiled);
    if(isCompiled == GL_FALSE) {
       GLint maxLength = 0;
       glGetShaderiv(fragShader, GL_INFO_LOG_LENGTH, &maxLength);
     
       std::vector<GLchar> errorLog(maxLength);
       glGetShaderInfoLog(fragShader, maxLength, &maxLength, &errorLog[0]);
     
       std::cerr << "Fragment shader compilation error:" << std::endl;
       for (int i = 0; i < errorLog.size(); i++)
          std::cerr << errorLog[i];
       std::cerr << std::endl;
       glDeleteShader(vertShader);
       glDeleteShader(fragShader);
       glDeleteProgram(shaderProgram);
       shaderProgram = 0;
       return;
    }
 
    // Link program
    glAttachShader(shaderProgram, vertShader);
    glAttachShader(shaderProgram, fragShader);
    glLinkProgram(shaderProgram);
    
    GLint isLinked = 0;
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &isLinked);
    if (isLinked == GL_FALSE) {
        GLint maxLength = 0;
        glGetProgramiv(shaderProgram, GL_INFO_LOG_LENGTH, &maxLength);
        std::vector<GLchar> errorLog(maxLength);
        glGetProgramInfoLog(shaderProgram, maxLength, &maxLength, &errorLog[0]);
        std::cerr << "Shader program linking error:" << std::endl;
        for (int i = 0; i < errorLog.size(); i++)
            std::cerr << errorLog[i];
        std::cerr << std::endl;
        glDeleteShader(vertShader);
        glDeleteShader(fragShader);
        glDeleteProgram(shaderProgram);
        shaderProgram = 0;
        return;
    }
    
    // Clean up shader objects (they're now attached to the program)
    glDeleteShader(vertShader);
    glDeleteShader(fragShader);
    
    shadersLoaded = true;
    std::cerr << "Shaders loaded successfully" << std::endl;
 }


} // namespace helpers

void display(void) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    // Load shaders if mode is 1 and not already loaded
    if (shader_mode == 1 && !shadersLoaded) {
        helpers::readShaders();
    }
    
    glMatrixMode(GL_MODELVIEW);
    helpers::camera_transform();
    
    // Use shader program if mode is 1, otherwise use default pipeline
    if (shader_mode == 1 && shaderProgram != 0) {
        glUseProgram(shaderProgram);
    } else {
        glUseProgram(0); // Use default pipeline
    }
    
    helpers::draw_objects();
    
    // Use shader program if mode is 1, otherwise use default pipeline
    if (shader_mode == 1 && shaderProgram != 0) {
        helpers::set_light_uniforms(); // Pass lights to shader as uniforms
    } else {
        helpers::set_lights(); // Use fixed-function lighting
    }

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



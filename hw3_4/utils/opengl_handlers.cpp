#include <GL/glew.h>     // OpenGL extension functions (MUST come before gl.h)
#include <GL/gl.h>       // Core OpenGL functions (glLoadIdentity, glRotatef, glMaterialfv, etc.)
#include <GL/glut.h>     // GLUT functions (glutGet, glutSwapBuffers, glutPostRedisplay, etc.)
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

static const char* vertProgFileName = "../shaders/vertex.glsl";
static const char* fragProgFileName = "../shaders/fragment.glsl";
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

void set_material_uniforms(const models::Model& model) {
    // Pass material properties to shader
    if (shaderProgram == 0) return;
    
    GLint ambientLoc = glGetUniformLocation(shaderProgram, "materialAmbient");
    if (ambientLoc != -1) {
        glUniform3fv(ambientLoc, 1, model.ambient.data());
    }
    
    GLint diffuseLoc = glGetUniformLocation(shaderProgram, "materialDiffuse");
    if (diffuseLoc != -1) {
        glUniform3fv(diffuseLoc, 1, model.diffuse.data());
    }
    
    GLint specularLoc = glGetUniformLocation(shaderProgram, "materialSpecular");
    if (specularLoc != -1) {
        glUniform3fv(specularLoc, 1, model.specular.data());
    }
    
    GLint shininessLoc = glGetUniformLocation(shaderProgram, "materialShininess");
    if (shininessLoc != -1) {
        glUniform1f(shininessLoc, model.shininess);
    }
}

void set_light_uniforms() {
    // Pass lighting information to shader as uniforms
    if (shaderProgram == 0) return;
    
    // Pass camera position to shader (in camera space, eye is at origin)
    GLint eyePosLoc = glGetUniformLocation(shaderProgram, "eyePosition");
    if (eyePosLoc != -1) {
        glUniform3fv(eyePosLoc, 1, scene->camera.position.data());
    }

    int numLights = scene->lights.size();

    // Set number of lights
    GLint numLightsLoc = glGetUniformLocation(shaderProgram, "numLights");
    if (numLightsLoc != -1) {
        glUniform1i(numLightsLoc, numLights);
    }
    
    // Get current modelview matrix from OpenGL state
    GLfloat modelview[16];
    glGetFloatv(GL_MODELVIEW_MATRIX, modelview);
    // Convert to Eigen matrix (OpenGL uses column-major order)
    Eigen::Matrix4d cameraTransform;
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            cameraTransform(i, j) = modelview[j * 4 + i];  // Transpose from column-major to row-major
        }
    }
    
    // Set light positions, colors, and attenuations
    for (size_t i = 0; i < numLights && i < 8; i++) {
        const auto& light = scene->lights[i];
        
        // Transform light position from world space to camera space
        Eigen::Vector4d lightPosWorld(light.position.x(), light.position.y(), light.position.z(), 1.0);
        Eigen::Vector4d lightPosCamera = cameraTransform * lightPosWorld;
        Eigen::Vector3f lightPosCam(lightPosCamera.x(), lightPosCamera.y(), lightPosCamera.z());
        
        // Light position in camera space
        std::string posName = "lightPositions[" + std::to_string(i) + "]";
        GLint posLoc = glGetUniformLocation(shaderProgram, posName.c_str());
        if (posLoc != -1) {
            glUniform3fv(posLoc, 1, lightPosCam.data());
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
            glMultMatrixd(model.transform.data());
            
            // Set material properties
            if (shader_mode == 1 && shaderProgram != 0) {
                helpers::set_material_uniforms(model);  // Set material properties
            } else {
                // For default pipeline: use fixed-function
                glMaterialfv(GL_FRONT, GL_AMBIENT, model.ambient.data());
                glMaterialfv(GL_FRONT, GL_DIFFUSE, model.diffuse.data());
                glMaterialfv(GL_FRONT, GL_SPECULAR, model.specular.data());
                glMaterialf(GL_FRONT, GL_SHININESS, model.shininess);
            }
            
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


void readShaders() {
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
    if (shader_mode == 1 && !helpers::shadersLoaded) {
        helpers::readShaders();
    }
    
    glMatrixMode(GL_MODELVIEW);
    helpers::camera_transform();
    
    // Use shader program if mode is 1, otherwise use default pipeline
    if (shader_mode == 1 && helpers::shaderProgram != 0) {
        glUseProgram(helpers::shaderProgram);
        helpers::set_light_uniforms(); // Pass lights to shader as uniforms
    } else {
        glUseProgram(0); // Use default pipeline
        helpers::set_lights(); // Use fixed-function lighting
    }
    
    helpers::draw_objects();
    
    // // Use shader program if mode is 1, otherwise use default pipeline
    // if (shader_mode == 1 && helpers::shaderProgram != 0) {
    //     helpers::set_light_uniforms(); // Pass lights to shader as uniforms
    // } else {
    //     helpers::set_lights(); // Use fixed-function lighting
    // }

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



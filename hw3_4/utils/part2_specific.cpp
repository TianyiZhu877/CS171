
#include <GL/glew.h>
#include <GL/gl.h>
#include <iostream>
#include "include/part2_specific.h"
extern GLenum readpng(const char *filename);
namespace opengl_handlers {
    namespace helpers {
        GLuint get_shader_program();
    }
}

namespace part2 {
    
    static GLenum texture_png, normal_png;
    static GLint texture_uniform_pos, normal_uniform_pos;

    static inline void set_normal_up(GLint vertexTangentAttr) {
        glNormal3f(0.0, 0.0, 1.0);
        if (vertexTangentAttr != -1) {
            glVertexAttrib3f(vertexTangentAttr, 1.0, 0.0, 0.0);
        }
    }

    // loaded in the beginning when opengl initializes
    bool load_textures(const char* texture_png_path, const char* normal_png_path) {
        if(!(texture_png = readpng(texture_png_path))) {
            std::cerr << "Error loading texture png: " << texture_png_path << std::endl;
            return false;
        } 

        if(!(normal_png = readpng(normal_png_path))) {
            std::cerr << "Error loading normal png: " << normal_png_path << std::endl;
            return false;
        }

        GLuint shaderProgram = opengl_handlers::helpers::get_shader_program();
        texture_uniform_pos = glGetUniformLocation(shaderProgram, "textureMap");
        normal_uniform_pos = glGetUniformLocation(shaderProgram, "normalMap");
     
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture_png);
        glUniform1i(texture_uniform_pos, 0);
     
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, normal_png);
        glUniform1i(normal_uniform_pos, 1);

        // std::cout << "texture_uniform_pos: " << texture_uniform_pos << std::endl;
        // std::cout << "normal_uniform_pos: " << normal_uniform_pos << std::endl;
        return true;
    }


    // hardcoded surface, called for every draw object cycle
    void draw_surface() {
        GLuint shaderProgram = opengl_handlers::helpers::get_shader_program();
        GLint vertexTangentAttr = glGetAttribLocation(shaderProgram, "vertexTangent");
        if (vertexTangentAttr != -1) {

            glBegin(GL_TRIANGLES);
                set_normal_up(vertexTangentAttr);
                glTexCoord2f(0.0, 1.0);            
                glVertex3f(-1.0, 1.0, 0.0);       

                set_normal_up(vertexTangentAttr);
                glTexCoord2f(0.0, 0.0);            
                glVertex3f(-1.0, -1.0, 0.0);      

                set_normal_up(vertexTangentAttr);     
                glTexCoord2f(1.0, 0.0);            
                glVertex3f(1.0, -1.0, 0.0);      
            glEnd();

            
            glBegin(GL_TRIANGLES);
                set_normal_up(vertexTangentAttr);
                glTexCoord2f(1.0, 1.0);            
                glVertex3f(1.0, 1.0, 0.0);       

                set_normal_up(vertexTangentAttr);
                glTexCoord2f(0.0, 1.0);            
                glVertex3f(-1.0, 1.0, 0.0);       

                set_normal_up(vertexTangentAttr);     
                glTexCoord2f(1.0, 0.0);            
                glVertex3f(1.0, -1.0, 0.0);    
            glEnd();
        }
    }

}

#include <GL/glew.h>
#include <GL/glut.h>
#include <string>
#include <iostream>
#include "part1_imple.h"
#include "I-bar_code.h"

static float light_color[3] = {1, 1, 1};
static float light_position[3] = {0, 0, -2};
const float ambient_reflect[3] = {0.3, 0.2, 0.4};
const float diffuse_reflect[3] = {0.7, 0.2, 0.8};
const float specular_reflect[3] = {1, 1, 1};
const float shininess = 0.1;

static void init_window(int argc, char* argv[], int xres, int yres, const std::string& window_name) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(xres, yres);
    glutInitWindowPosition(0, 0);
    glutCreateWindow(window_name.c_str());
    
    // glShadeModel(GL_SMOOTH);
    // glEnable(GL_CULL_FACE);
    // glCullFace(GL_BACK);
    // glEnable(GL_DEPTH_TEST);
    // glEnable(GL_NORMALIZE);
    
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_NORMAL_ARRAY);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    glFrustum(-1.0f, 1.0f,
              -1.0f, 1.0f,
              1.0f, 60.0f);

              

    glMatrixMode(GL_MODELVIEW);
    
    glutDisplayFunc(part1_imple::display);
    glutReshapeFunc(part1_imple::window_resize);
    glutKeyboardFunc(part1_imple::key_pressed);

    glEnable(GL_COLOR_MATERIAL);
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);
    glEnable(GL_LIGHT0);
    glLightfv(GL_LIGHT0, GL_AMBIENT, light_color);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, light_color);
    glLightfv(GL_LIGHT0, GL_SPECULAR, light_color);
    glLightfv(GL_LIGHT0, GL_POSITION, light_position);
    
    glMaterialfv(GL_FRONT, GL_AMBIENT, ambient_reflect);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, diffuse_reflect);
    glMaterialfv(GL_FRONT, GL_SPECULAR, specular_reflect);
    glMaterialf(GL_FRONT, GL_SHININESS, shininess);

    glutMainLoop();
}

int main(int argc, char* argv[]) {
    using namespace part1_imple;
    if (argc != 4 && argc != 5) {
        std::cerr << "Usage: " << argv[0] << " [.script file] [xres] [yres]" << std::endl;
        return 1;
    }
    std::string script_file_path = argv[1];

    auto [frame_indices, pos_scale_rots, num_frames] = parse_pos_scale_rot_file(script_file_path);
    // std::cout << "main num_frames: " << num_frames << std::endl;
    part1_imple::interpolated_pos_scale_rots = interpolate_pos_scale_rot(frame_indices, pos_scale_rots, num_frames);
    part1_imple::wrap_around = num_frames;
    quadratic = gluNewQuadric();

    init_window(argc, argv, atoi(argv[2]), atoi(argv[3]), script_file_path);
    return 0;
}

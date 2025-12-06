#include <fstream>
#include <sstream>
#include <stdexcept>
#include <cassert>
#include "part1_imple.h"
#include "I-bar_code.h"

namespace part1_imple {
    static int wrapped_distance(int a, int b, int n) {
        return b > a ? b - a : n - (a - b);
    }

    static Eigen::MatrixXf build_key_frame_window_matrix(const std::vector<PosScaleRot>& pos_scale_rots, int i0, int i1, int i2, int i3) {
        Eigen::MatrixXf window_matrix(4, 10);
        window_matrix.row(0) = pos_scale_rots[i0].to_eigen_matrix_row();
        window_matrix.row(1) = pos_scale_rots[i1].to_eigen_matrix_row();
        window_matrix.row(2) = pos_scale_rots[i2].to_eigen_matrix_row();
        window_matrix.row(3) = pos_scale_rots[i3].to_eigen_matrix_row();
        return window_matrix;
    }

    int current_frame = 0;
    int wrap_around;
    std::vector<PosScaleRot> interpolated_pos_scale_rots;

    std::tuple<std::vector<int>, std::vector<PosScaleRot>, int> parse_pos_scale_rot_file(const std::string& file_path) {
        std::ifstream file(file_path);
        if (!file.is_open()) {
            throw std::runtime_error("Failed to open file: " + file_path);
        }

        // returns
        std::vector<int> frame_indices;
        std::vector<PosScaleRot> pos_scale_rots;
        int num_frames = 0;

        int state = 0;
        std::string line;
        PosScaleRot current_pos_scale_rot;
        while (std::getline(file, line)) {
            std::istringstream line_stream(line);
            std::string token0;
            line_stream >> token0;
            if (state == 0) {
                if (token0 == "Frame") {
                    std::string frame_idx_token;
                    line_stream >> frame_idx_token;
                    frame_indices.push_back(std::stoi(frame_idx_token));
                    state = 1;
                } else {
                    try {
                        // std::cout << "num_frames: " << token0 << std::endl;
                        num_frames = std::stoi(token0);
                    } catch (const std::invalid_argument& e) {}
                }
            } else if (state == 1) {
                if (token0 == "Frame") {
                    std::string frame_idx_token;
                    line_stream >> frame_idx_token;
                    pos_scale_rots.push_back(std::move(current_pos_scale_rot));
                    current_pos_scale_rot = PosScaleRot();
                    frame_indices.push_back(std::stoi(frame_idx_token));
                } else {
                    std::istringstream new_line_stream(line);
                    current_pos_scale_rot.try_parse_line(new_line_stream);
                }
            }
        }
        pos_scale_rots.push_back(std::move(current_pos_scale_rot));
        return std::make_tuple(frame_indices, pos_scale_rots, num_frames);
    }

    std::vector<PosScaleRot> interpolate_pos_scale_rot(const std::vector<int>& frame_indices, const std::vector<PosScaleRot>& pos_scale_rots, int num_frames) {
        std::vector<PosScaleRot> interpolated_pos_scale_rots(num_frames);
        int num_key_frames = pos_scale_rots.size();
        int i = 0; 
        do {
            int i0 = (i+num_key_frames-1) % num_key_frames;
            int i1 = i;
            int i2 = (i+1) % num_key_frames;
            int i3 = (i+2) % num_key_frames;
            int k0_idx = frame_indices[i0];
            int k1_idx = frame_indices[i1];
            int k2_idx = frame_indices[i2];
            int k3_idx = frame_indices[i3];

            interpolated_pos_scale_rots[k1_idx] = pos_scale_rots[i];
            Eigen::MatrixXf key_frame_window_matrix = build_key_frame_window_matrix(pos_scale_rots, i0, i1, i2, i3);

            for (int f = k1_idx+1; f != k2_idx; f = (f+1)%num_frames) {
                float u = (float)wrapped_distance(k1_idx, f, num_frames) / (float)wrapped_distance(k1_idx, k2_idx, num_frames);
                assert(u >= 0.0f && u <= 1.0f);
                
                Eigen::RowVector4f u_B = CR_spine::get_u_vector(u).transpose() * CR_spine::B;
                PosScaleRot::eigenMatrixRow result_row = u_B * key_frame_window_matrix;
                PosScaleRot new_pos_scale_rot;
                new_pos_scale_rot.from_eigen_matrix_row(result_row);
                interpolated_pos_scale_rots[f] = new_pos_scale_rot;
            }     
        
            i = (i + 1) % num_key_frames;
        } while (i != 0);

        return interpolated_pos_scale_rots;
    }

    
    void display(void) {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        glLoadIdentity();
        glTranslatef(0.0f, 0.0f, -40.0f);

        glEnable(GL_LIGHTING);
        glPushMatrix();
        PosScaleRot& current_pos_scale_rot = interpolated_pos_scale_rots[current_frame];
        // current_pos_scale_rot.print();
        glTranslatef(current_pos_scale_rot.pos_x, current_pos_scale_rot.pos_y, current_pos_scale_rot.pos_z);
        glScalef(current_pos_scale_rot.scale_x, current_pos_scale_rot.scale_y, current_pos_scale_rot.scale_z);
        glRotatef(current_pos_scale_rot.rot_angle, current_pos_scale_rot.rot_x, current_pos_scale_rot.rot_y, current_pos_scale_rot.rot_z);
        // glRotatef(0.0f, 1.0f, 0.0f, 0.0f);
        drawIBar();
        glPopMatrix();
        glDisable(GL_LIGHTING);
        
        glutSwapBuffers();
    }

    void window_resize(int width, int height) {
        height = (height == 0) ? 1 : height;
        width = (width == 0) ? 1 : width;
        
        glViewport(0, 0, width, height);
        
        glutPostRedisplay();
    }

    void key_pressed(unsigned char key, int x, int y) {
        if(key == 'q') {
            exit(0);
        } else if(key == 'f') {
            current_frame = (current_frame + 1) % wrap_around;
            std::cout << "frame " << current_frame << std::endl;
            glutPostRedisplay(); 
        } else if (key == 'd') {
            current_frame = (current_frame - 1 + wrap_around) % wrap_around;
            std::cout << "frame " << current_frame << std::endl;
            glutPostRedisplay(); 
        }
    }
}
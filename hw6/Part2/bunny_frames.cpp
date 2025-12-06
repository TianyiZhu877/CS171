#include <iostream>
#include <cstdio>
#include <vector>
#include <algorithm>
#include <Eigen/Dense>
#include "obj_file.h"
#include "CR_spine.h"

constexpr int num_keyframes = 5;
constexpr int keyframe_interval = 5;
static const char* keyframes_path = "../Bunny_Frames/keyframes/bunny%02d.obj";
static const char* export_path = "../Bunny_Frames/interpolate_results/bunny%02d.obj";
static const char* expected_interpolated_path = "../Bunny_Frames/interpolated_frames/bunny%02d.obj";
static constexpr float tolerance = 1e-4f;

// buffer for path formatting
static char path_buffer[256];

int main(int argc, char* argv[]) {

    bool export_mode = false;
    if (argc > 1 && std::string(argv[1]) == "--export") {
        export_mode = true;
    }

    int num_input_rows = num_keyframes + 2;
    int num_interpolated_frames = num_keyframes * keyframe_interval;

    int target_row = 1;
    std::vector<obj_file::Face> faces;
    std::vector<obj_file::Face> dummy_faces;
    std::vector<float> first_row_vertices;

    snprintf(path_buffer, sizeof(path_buffer), keyframes_path, 0);
    obj_file::load_obj_file_vertex_vectors(path_buffer, first_row_vertices, faces);
    // target_row++;
    
    Eigen::MatrixXf key_frame_vertices(num_input_rows, first_row_vertices.size());

    for (int f = 0; f < num_keyframes*keyframe_interval; f+=keyframe_interval) {
        std::vector<float> vertices;
        snprintf(path_buffer, sizeof(path_buffer), keyframes_path, f);
        obj_file::load_obj_file(path_buffer, target_row, key_frame_vertices, dummy_faces, false);
        target_row++;
    }

    for (int i = 0; i < first_row_vertices.size(); i++) {
        key_frame_vertices(1, i) = first_row_vertices[i];
    }
    key_frame_vertices.row(0) = key_frame_vertices.row(1);
    key_frame_vertices.row(num_input_rows - 1) = key_frame_vertices.row(num_input_rows - 2);

    // test print the first 9 columns of each row
    for (int i = 0; i < num_input_rows; i++) {
        for (int j = 0; j < 9; j++) {
            std::cout << key_frame_vertices(i, j) << " ";
        }
        std::cout << std::endl;
    }


    Eigen::MatrixXf output_vertices(num_interpolated_frames, first_row_vertices.size());
    Eigen::MatrixXf expected_interpolated_vertices(num_interpolated_frames, first_row_vertices.size());
    int output_row = 0;
    for (int f = 0; f < (num_keyframes-1)*keyframe_interval; f++) 
        if (f % keyframe_interval != 0) {
            using namespace CR_spine;
            int row_range_begin = f / keyframe_interval;
            float u = (f % keyframe_interval) / (float)keyframe_interval;
            
            Eigen::RowVector4f u_B = get_u_vector(u).transpose() * B;
            // Slice entire 4 rows and multiply all columns at once (no copy, just a view)
            // u_B (1x4) * block (4xN) = (1xN) result assigned to output row
            output_vertices.row(output_row).noalias() = u_B * key_frame_vertices.block(row_range_begin, 0, 4, first_row_vertices.size());

            snprintf(path_buffer, sizeof(path_buffer), expected_interpolated_path, f);
            obj_file::load_obj_file(path_buffer, output_row, expected_interpolated_vertices, dummy_faces, false);

            // Compare the newly obtained row of the two matrices
            Eigen::VectorXf computed_row = output_vertices.row(output_row);
            Eigen::VectorXf expected_row = expected_interpolated_vertices.row(output_row);
            Eigen::VectorXf diff = computed_row - expected_row;
            float max_diff = diff.cwiseAbs().maxCoeff();
            if (!computed_row.isApprox(expected_row, tolerance)) {
                std::cout << "Frame " << f << " (row " << output_row << "): Max difference = " << max_diff << std::endl;
                // print sample elements (first 9 elements, which are 3 vertices)
                int num_samples = std::min(9, (int)computed_row.size());
                std::cout << "  Sample elements (first " << num_samples << "):" << std::endl;
                for (int i = 0; i < num_samples; i++) {
                    std::cout << "    [" << i << "] computed: " << computed_row(i) 
                              << ", expected: " << expected_row(i) 
                              << ", diff: " << diff(i) << std::endl;
                }
            } else {
                std::cout << "Frame " << f << " (row " << output_row << "): Passed" << std::endl;
            }

            if (export_mode) {
                snprintf(path_buffer, sizeof(path_buffer), export_path, f);
                obj_file::export_obj_file(path_buffer, output_row, output_vertices, faces);
            }
            
            output_row++;
        }

    return 0;
}


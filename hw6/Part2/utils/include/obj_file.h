#ifndef OBJ_FILE_H
#define OBJ_FILE_H

#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <array>
#include <filesystem>
#include <iostream>
#include <Eigen/Dense>

namespace obj_file {
    using Face = std::array<int, 3>;

    void inline load_obj_file_vertex_vectors(const char* path, std::vector<float>& vertices, std::vector<Face>& faces, bool read_faces = true) {
        std::ifstream file(path);
        std::string line;
        while (std::getline(file, line)) {
            std::istringstream iss(line);
            std::string type;
            iss >> type;
            if (type == "v") {
                float x, y, z;
                iss >> x >> y >> z;
                vertices.push_back(x);
                vertices.push_back(y);
                vertices.push_back(z);
            }
            else if (read_faces && type == "f") {
                int p1, p2, p3;
                iss >> p1 >> p2 >> p3;
                faces.push_back({p1, p2, p3});
            }
        }
        file.close();
    }

    // assume the vertices matrix is already initialized with the correct number of columns (3 * num_vertices)
    void inline load_obj_file(const char* path, int target_row, Eigen::MatrixXf& vertices, std::vector<Face>& faces, bool read_faces = true) {
        std::ifstream file(path);
        std::string line;
        int num_vertices = 0;
        while (std::getline(file, line)) {
            std::istringstream iss(line);
            std::string type;
            iss >> type;
            if (type == "v") {
                float x, y, z;
                iss >> x >> y >> z;
                vertices(target_row, num_vertices*3) = x;
                vertices(target_row, num_vertices*3 + 1) = y;
                vertices(target_row, num_vertices*3 + 2) = z;
                num_vertices++;
            }
            else if (read_faces && type == "f") {
                int p1, p2, p3;
                iss >> p1 >> p2 >> p3;
                faces.push_back({p1, p2, p3});
            }
        }
        file.close();
    }

    void inline export_obj_file(const char* path, int target_row, const Eigen::MatrixXf& vertices, const std::vector<Face>& faces) {
        // Create parent directories if they don't exist
        std::filesystem::path file_path(path);
        std::filesystem::path parent_dir = file_path.parent_path();
        if (!parent_dir.empty() && !std::filesystem::exists(parent_dir)) 
            std::filesystem::create_directories(parent_dir);
        
        std::ofstream file(path);
        for (int i = 0; i < vertices.cols() / 3; i++) {
            file << "v " << vertices(target_row, i*3) << " " << vertices(target_row, i*3 + 1) << " " << vertices(target_row, i*3 + 2) << std::endl;
        }
        for (const auto& face : faces) {
            file << "f " << face[0] << " " << face[1] << " " << face[2] << std::endl;
        }
        file.close();
    }
}

#endif
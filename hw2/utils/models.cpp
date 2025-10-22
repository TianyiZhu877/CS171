#include "models.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <Eigen/Dense>


namespace models {

bool ObjModel::load_from_obj_file(const std::string& filename) {
    this->filename = filename;
    vertexes.clear();
    faces.clear();
    
    std::ifstream file(filename);
    if (!file.is_open()) {
        return false;
    }
    
    std::string line;
    while (std::getline(file, line)) {
        // Skip empty lines
        if (line.empty()) continue;
        
        std::istringstream iss(line);
        std::string prefix;
        iss >> prefix;
        
        if (prefix == "v") {
            double x, y, z;

            // Will keep corrupted vetex lines to ensure index correctness
            iss >> x >> y >> z;
            vertexes.emplace_back(x, y, z);
            // Any additional numbers after x,y,z are ignored
        }
        else if (prefix == "vn") {
            double x, y, z;
            iss >> x >> y >> z;
            normals.emplace_back(x, y, z);
        }
        else if (prefix == "f") {
            // Parse face data - supports both "f v1 v2 v3" and "f v1//n1 v2//n2 v3//n3" formats
            std::string token1, token2, token3;
            
            // If less than 3 tokens, skip this face
            if (iss >> token1 >> token2 >> token3) {
                Face new_face;
                bool valid_face = true;

                // Parse each token
                std::string tokens[3] = {token1, token2, token3};
                for (int i = 0; i < 3; i++) {
                    size_t slash_pos = tokens[i].find("//");
                    if (slash_pos != std::string::npos) {
                        // Format: vertex//normal
                        // has_normals = true;
                        int v_idx = std::stoi(tokens[i].substr(0, slash_pos)) - 1;
                        int vn_idx = std::stoi(tokens[i].substr(slash_pos + 2)) - 1;
                        if (v_idx < 0 || vn_idx < 0) {
                            valid_face = false;
                            break;
                        }
                        new_face[i] = static_cast<size_t>(v_idx);
                        new_face[i+3] = static_cast<size_t>(vn_idx);
                    } else {
                        // Simple format: just vertex index
                        int v_idx = std::stoi(tokens[i]) - 1;
                        if (v_idx < 0) {
                            valid_face = false;
                            break;
                        }
                        new_face[i] = static_cast<size_t>(v_idx);
                        new_face[i+3] = static_cast<size_t>(INVALID_SURFACE_NORMAL);
                    }
                }
                if (valid_face) {
                    faces.emplace_back(std::move(new_face));
                }
            }
        }
        // Lines not starting with 'v' or 'f' followed by space are ignored
    }
    
    return true;
}

} // namespace models
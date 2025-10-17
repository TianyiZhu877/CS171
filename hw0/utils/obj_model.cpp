#include "obj_model.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <Eigen/Dense>


namespace obj_model {

bool ObjModel::load_from_obj_file(const std::string& filename) {
    this->filename = filename;
    vertexes.clear();
    faces.clear();
    
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error: Could not load file " << filename << std::endl;
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
        else if (prefix == "f") {
            // Parse only the first 3 face indices, ignore any additional ones
            int idx1, idx2, idx3;
            
            // If less than 3 indices, skip this face
            if (iss >> idx1 >> idx2 >> idx3) {
                // Looks like the idxes of obj files are 1-based?
                // idx1--; idx2--; idx3--;
                
                if (idx1 >= 0 && idx2 >= 0 && idx3 >= 0)
                    faces.emplace_back(Face{static_cast<std::size_t>(idx1), static_cast<std::size_t>(idx2), static_cast<std::size_t>(idx3)});
            }
        }
        // Lines not starting with 'v' or 'f' followed by space are ignored
    }
    
    return true;
}

} // namespace obj_model
#include <vector>
#include <iostream>
#include "obj_model.h"

static std::vector<obj_model::ObjModel> results;

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " obj_file1.obj obj_file2.obj ... obj_fileN.obj" << std::endl;
        return 1;
    }
        
    // Iterate through the each .obj files
    for (int i = 1; i < argc; ++i) {
        obj_model::ObjModel obj_data;
        obj_data.load_from_obj_file(argv[i]);
        // Store the .obj into the std::vector
        results.emplace_back(std::move(obj_data));
        
        // Print the vertices and faces of the new OBJ file
        std::cout << argv[i] << ":" << std::endl << std::endl;
        obj_model::ObjModel::vertexList& current_vertices = results.back().vertexes;
        obj_model::ObjModel::FaceList& current_faces = results.back().faces;
        for (const auto& vertex : current_vertices) {
            std::cout << "v " << vertex.x() << " " << vertex.y() << " " << vertex.z() << std::endl;
        }
        for (const auto& face : current_faces) {
            std::cout << "f " << face[0] << " " << face[1] << " " << face[2] << std::endl;
        }
        std::cout << std::endl;
    }
    
    return 0;
    // std::cout << "\nSuccessfully processed " << results.size() << " OBJ files." << std::endl;
    
}
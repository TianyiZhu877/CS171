
#ifndef OBJ_MODEL_H
#define OBJ_MODEL_H

#include <vector>
#include <utility>
#include <string>
#include <array>
#include <Eigen/Dense>

namespace obj_model{

// Object file class that stores the vertexes and faces, and support laoding from .obj file by calling load_from_obj_file()
struct ObjModel {

    using Face = std::array<std::size_t, 3>;
    using vertexList = std::vector<Eigen::Vector3d>;
    using FaceList = std::vector<Face>;

    void clear() {
        vertexes.clear();
        faces.clear();
    }

    bool load_from_obj_file(const std::string& filename);

    Eigen::Matrix4Xd export_vertexes_matrix_homo(){
        const int num_cols = static_cast<int>(vertexes.size());
        Eigen::Matrix4Xd M(4, num_cols);
        for (int i = 0; i < num_cols; ++i) {
            M(0, i) = vertexes[i].x();
            M(1, i) = vertexes[i].y();
            M(2, i) = vertexes[i].z();
            M(3, i) = 1.0;
        }
        return M;
    }

    Eigen::Matrix3Xd export_vertexes_matrix_3d(){
        const int num_cols = static_cast<int>(vertexes.size());
        Eigen::Matrix3Xd M(3, num_cols);
        for (int i = 0; i < num_cols; ++i) {
            M(0, i) = vertexes[i].x();
            M(1, i) = vertexes[i].y();
            M(2, i) = vertexes[i].z();
        }
        return M;
    }

    
    void load_vertexes_from_homo_matrix(Eigen::Matrix4Xd& matrix) {
        vertexes.clear();
        const int cols = static_cast<int>(matrix.cols());
        vertexes.reserve(static_cast<std::size_t>(cols));
        for (int i = 0; i < cols; ++i) {
            const double w = matrix(3, i);
            const double inv_w = (w != 0.0) ? (1.0 / w) : 0.0;
            const double x = matrix(0, i) * inv_w;
            const double y = matrix(1, i) * inv_w;
            const double z = matrix(2, i) * inv_w;
            vertexes.emplace_back(x, y, z);
        }
    }
    
    vertexList vertexes;
    FaceList faces;
    std::string filename;
};


}

#endif // OBJ_MODEL_H

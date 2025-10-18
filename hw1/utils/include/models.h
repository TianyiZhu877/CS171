
#ifndef MODELS_H
#define MODELS_H

#include <vector>
#include <utility>
#include <string>
#include <array>
#include <variant>
#include <Eigen/Dense>

namespace models{

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


struct Model {
    std::variant<ObjModel, std::reference_wrapper<ObjModel>> obj_file;
    std::string name;
    Eigen::Matrix4Xd points;

    Model() {}

    Model(std::string model_name): name(model_name) {}

    void initialize_obj_by_ref(ObjModel& obj_model) {
        points = obj_model.export_vertexes_matrix_homo();
        obj_file = std::ref(obj_model);
    }

    void initialize_obj_by_value(const ObjModel& obj_model) {
        points = obj_model.export_vertexes_matrix_homo();
        obj_file = obj_model;
    }

    void initialize_obj_by_value(ObjModel&& obj_model) {
        points = obj_model.export_vertexes_matrix_homo();
        obj_file = std::move(obj_model);
    }

    // get faces from the obj file
    FaceList& faces() {
        if (std::holds_alternative<ObjModel>(obj_file)) {
            return std::get<ObjModel>(obj_file).faces;
        } else {
            return std::get<std::reference_wrapper<ObjModel>>(obj_file).get().faces;
        }
    }

    const FaceList& faces() const {
        if (std::holds_alternative<ObjModel>(obj_file)) {
            return std::get<ObjModel>(obj_file).faces;
        } else {
            return std::get<std::reference_wrapper<ObjModel>>(obj_file).get().faces;
        }
    }
};


} // namespace models

#endif // MODELS_H

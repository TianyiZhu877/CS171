
#ifndef MODELS_H
#define MODELS_H

#include <vector>
#include <utility>
#include <string>
#include <array>
#include <variant>
#include <memory>
#include <optional>
#include <Eigen/Dense>
#include "ppm_image.h"

// These model classes are only containers providing data storage, io and type conversions, transformation logic should be implemented elsewhere
namespace models{

// Object file class that stores the vertexes and faces, and support laoding from .obj file by calling load_from_obj_file()
struct ObjModel {

    constexpr static size_t INVALID_SURFACE_NORMAL = -3;

    using Face = std::array<std::size_t, 6>;
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
    vertexList normals;
    FaceList faces;
    std::string filename;
};


struct Model {
    std::shared_ptr<ObjModel> obj_file;
    std::string name;
    Eigen::Matrix4d transform;
    ppm_image::Pixel<float> ambient;
    ppm_image::Pixel<float> diffuse;
    ppm_image::Pixel<float> specular;
    float shininess;

    Model(const std::shared_ptr<ObjModel>& init_obj, Eigen::Matrix4d init_transform = Eigen::Matrix4d::Identity(), std::string model_name = ""): 
    obj_file(init_obj), name(model_name), transform(init_transform) { }

    // get faces from the obj file
    ObjModel::FaceList& faces() {
        return obj_file->faces;
    }

    const ObjModel::FaceList& faces() const {
        return obj_file->faces;
    }

    ::Eigen::Matrix4Xd points_homo_transformed() const {
        return transform * (obj_file->export_vertexes_matrix_homo());
    }

    bool try_parse_material_line(std::istringstream& line) {
        std::string field_name;
        line >> field_name;
        if (field_name == "ambient") {
            line >> ambient.r >> ambient.g >> ambient.b;
            return true;
        }
        if (field_name == "diffuse") {
            line >> diffuse.r >> diffuse.g >> diffuse.b;
            return true;
        }
        if (field_name == "specular") {
            line >> specular.r >> specular.g >> specular.b;
            return true;
        }
        if (field_name == "shininess") {
            line >> shininess;
            return true;
        }
        return false;
    }
};


} // namespace models

#endif // MODELS_H

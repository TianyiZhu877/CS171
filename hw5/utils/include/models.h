
#ifndef MODELS_H
#define MODELS_H

#include <vector>
#include <utility>
#include <string>
#include <array>
#include <variant>
#include <memory>
#include <optional>
#include <GL/glew.h>
#include <Eigen/Dense>

#include "halfedge_structs.h"
#include "halfedge.h"
#include "halfedge_utils.h"

// These model classes are only containers providing data storage, io and type conversions, transformation logic should be implemented elsewhere
namespace models{

// Object file class that stores the vertexes and faces, and support laoding from .obj file by calling load_from_obj_file()
struct ObjModel {

    constexpr static size_t INVALID_SURFACE_NORMAL = -3;

    using Face = std::array<std::size_t, 6>;
    using FaceOpenGL = std::array<GLuint, 3>;
    using vertexList = std::vector<Eigen::Vector3f>;
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

    Eigen::Matrix3Xd export_normals_matrix(){
        const int num_cols = static_cast<int>(normals.size());
        Eigen::Matrix3Xd M(3, num_cols);
        for (int i = 0; i < num_cols; ++i) {
            M(0, i) = normals[i].x();
            M(1, i) = normals[i].y();
            M(2, i) = normals[i].z();
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
    std::vector<FaceOpenGL> faces_opengl;
    std::string filename;
    bool drawElement_compatible;
};


struct Model {
    std::shared_ptr<ObjModel> obj_file;
    std::string name;
    Eigen::Matrix4d transform;
    Eigen::Vector3f ambient;
    Eigen::Vector3f diffuse;
    Eigen::Vector3f specular;
    float shininess;

    std::vector<halfedge::HEV*> halfedges_vertices;
    std::vector<halfedge::HEF*> halfedges_faces;

    halfedge::Face* raw_faces;
    halfedge::Vertex* vertices_cached;
    halfedge::Vec3f* normals_cached;
    int num_vertices;
    int num_faces;
    

    Model(const std::shared_ptr<ObjModel>& init_obj, Eigen::Matrix4d init_transform = Eigen::Matrix4d::Identity(), std::string model_name = "", bool setup_halfedges = true): 
    obj_file(init_obj), name(model_name), transform(init_transform), raw_faces(nullptr), vertices_cached(nullptr), normals_cached(nullptr), num_vertices(0), num_faces(0) { 
        // if (obj_file == nullptr) {
        //     std::cerr << "Error: obj_file is nullptr" << std::endl;
        //     // return;
        // }
    }

    void setup_halfedges() {
        if (obj_file == nullptr) {
            std::cerr << "Error: obj_file is nullptr" << std::endl;
            return;
        }

        num_vertices = obj_file->vertexes.size()+1;
        num_faces = obj_file->faces.size();
        raw_faces = new halfedge::Face[num_faces];
        vertices_cached = new halfedge::Vertex[num_vertices];
        normals_cached = new halfedge::Vec3f[num_vertices];

        halfedge::Mesh_Data mesh_data;
        mesh_data.vertices.reserve(num_vertices);
        mesh_data.faces.reserve(num_faces);
        
        mesh_data.vertices.push_back(NULL);
        for (int i = 1; i < num_vertices; i++) {
            vertices_cached[i] = halfedge::Vertex{obj_file->vertexes[i-1].x(), obj_file->vertexes[i-1].y(), obj_file->vertexes[i-1].z()};
            mesh_data.vertices.push_back(vertices_cached+i);
        }
        for (int i = 0; i < num_faces; i++) {
            raw_faces[i] = halfedge::Face{obj_file->faces[i][0]+1, obj_file->faces[i][1]+1, obj_file->faces[i][2]+1};
            mesh_data.faces.push_back(raw_faces+i);
        }
        halfedge::build_HE(&mesh_data, &halfedges_vertices, &halfedges_faces);
        // delete[] vertices_cached;
        compute_halfedge_vertex_normals();
        // print_vertexes_normals_to_draw();
    }

    void compute_halfedge_vertex_normals() {
        using namespace halfedge;
        for (HEF *face : halfedges_faces) {
            compute_face_normal_area(face);
        }

        for (size_t i = 1; i < halfedges_vertices.size(); i++) {
            HEV *vertex = halfedges_vertices[i];
            if (vertex) {
                compute_vertex_normal(vertex);
                normals_cached[i] = vertex->normal;
                vertices_cached[i] = Vertex{static_cast<float>(vertex->x), static_cast<float>(vertex->y), static_cast<float>(vertex->z)};
            }
        }
    }

    ~Model() {
        std::cout << "deleting" << name << " " << halfedges_vertices.size() << " " << halfedges_faces.size() << std::endl;
        halfedge::delete_HE(&halfedges_vertices, &halfedges_faces);
        delete[] normals_cached;
        delete[] raw_faces;
        delete[] vertices_cached;
    }

    // get faces from the obj file
    ObjModel::FaceList& faces() {
        return obj_file->faces;
    }

    const ObjModel::FaceList& faces() const {
        return obj_file->faces;
    }

    Eigen::Matrix4Xd points_homo_transformed() const {
        return transform * (obj_file->export_vertexes_matrix_homo());
    }

    Eigen::Matrix3Xd normals_transformed() const {
        Eigen::Matrix3Xd transformed_normals = transform.block<3,3>(0,0) * (obj_file->export_normals_matrix());
        
        // Normalize each normal vector
        for (int i = 0; i < transformed_normals.cols(); ++i) {
            transformed_normals.col(i).normalize();
        }
        
        return transformed_normals;
    }

    bool try_parse_material_line(std::istringstream& line) {
        std::string field_name;
        line >> field_name;
        if (field_name == "ambient") {
            line >> ambient.x() >> ambient.y() >> ambient.z();
            return true;
        }
        if (field_name == "diffuse") {
            line >> diffuse.x() >> diffuse.y() >> diffuse.z();
            return true;
        }
        if (field_name == "specular") {
            line >> specular.x() >> specular.y() >> specular.z();
            return true;
        }
        if (field_name == "shininess") {
            line >> shininess;
            return true;
        }
        return false;
    }

    void print_vertexes_normals_to_draw() {
        for (int i = 0; i < num_faces; i++) {
            std::cout << "face " << i << ": " << raw_faces[i].idx1 << " " << raw_faces[i].idx2 << " " << raw_faces[i].idx3 << std::endl;
            std::cout << "vertex " << raw_faces[i].idx1 << ": " << vertices_cached[raw_faces[i].idx1].x << " " << vertices_cached[raw_faces[i].idx1].y << " " << vertices_cached[raw_faces[i].idx1].z << std::endl;
            std::cout << "vertex " << raw_faces[i].idx2 << ": " << vertices_cached[raw_faces[i].idx2].x << " " << vertices_cached[raw_faces[i].idx2].y << " " << vertices_cached[raw_faces[i].idx2].z << std::endl;
            std::cout << "vertex " << raw_faces[i].idx3 << ": " << vertices_cached[raw_faces[i].idx3].x << " " << vertices_cached[raw_faces[i].idx3].y << " " << vertices_cached[raw_faces[i].idx3].z << std::endl;
            std::cout << "normal " << raw_faces[i].idx1 << ": " << normals_cached[raw_faces[i].idx1].x << " " << normals_cached[raw_faces[i].idx1].y << " " << normals_cached[raw_faces[i].idx1].z << std::endl;
            std::cout << "normal " << raw_faces[i].idx2 << ": " << normals_cached[raw_faces[i].idx2].x << " " << normals_cached[raw_faces[i].idx2].y << " " << normals_cached[raw_faces[i].idx2].z << std::endl;
            std::cout << "normal " << raw_faces[i].idx3 << ": " << normals_cached[raw_faces[i].idx3].x << " " << normals_cached[raw_faces[i].idx3].y << " " << normals_cached[raw_faces[i].idx3].z << std::endl;
            std::cout << "vertex from obj file " << static_cast<int>(obj_file->faces_opengl[i][0]) << std::endl;
            //  << ": " << obj_file->vertexes[obj_file->faces_opengl[i][0]].x << " " << obj_file->vertexes[obj_file->faces_opengl[i][0]].y << " " << obj_file->vertexes[obj_file->faces_opengl[i][0]].z << std::endl;
            // std::cout << "vertex from obj file " << obj_file->faces_opengl[i][1] << ": " << obj_file->vertexes[obj_file->faces_opengl[i][1]].x << " " << obj_file->vertexes[obj_file->faces_opengl[i][1]].y << " " << obj_file->vertexes[obj_file->faces_opengl[i][1]].z << std::endl;
            // std::cout << "vertex from obj file " << obj_file->faces_opengl[i][2] << ": " << obj_file->vertexes[obj_file->faces_opengl[i][2]].x << " " << obj_file->vertexes[obj_file->faces_opengl[i][2]].y << " " << obj_file->vertexes[obj_file->faces_opengl[i][2]].z << std::endl;
            std::cout << "--------------------------------" << std::endl;
        }
    }
};


} // namespace models

#endif // MODELS_H

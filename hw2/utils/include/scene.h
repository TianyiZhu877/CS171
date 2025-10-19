#ifndef SCENE_H
#define SCENE_H

#include <fstream>
#include <sstream>
#include <memory>
#include <iostream>
#include <unordered_map>
#include <filesystem>
#include <Eigen/Dense>

#include "transformation.h"
#include "models.h"
#include "rendering.h"

namespace scene {

// Parse a line of "t x y z", "s x y z", "r ux uy uz angle", return the transformation matrix
inline Eigen::Matrix4d parse_transformation_line(std::istringstream& line, std::string& prefix, double& x, double& y, double& z, double& angle) {
    line >> prefix >> x >> y >> z;

    if (prefix == "t") 
        return ::transformation::matrix_from_translation_vector(x, y, z);

    if (prefix == "s") 
        return ::transformation::matrix_from_scaling_vector(x, y, z);

    if (prefix == "r") {
        line >> angle;
        return ::transformation::matrix_from_rotation_vector(x, y, z, angle);
    }

    return Eigen::Matrix4d();
}

// Reload the function when we don't need to get the raw values
inline Eigen::Matrix4d parse_transformation_line(std::istringstream& line) {
    std::string prefix;
    double x, y, z, angle;
    return parse_transformation_line(line, prefix, x, y, z, angle);
}


// Class for a camera, contain all key params of a camera for rendering
struct Camera {
    Eigen::Vector3d position;
    Eigen::Vector4d orientation;
    double n;
    double f;
    double l;
    double r;
    double t;
    double b;
    
    // Print camera info to screen for debug
    void serialize(std::ostream& os = std::cout) const {
        os << "position " << position.x() << " " << position.y() << " " << position.z() << "\n";
        os << "orientation " << orientation(0) << " " << orientation(1) << " " 
           << orientation(2) << " " << orientation(3) << "\n";
        os << "near " << n << "\n";
        os << "far " << f << "\n";
        os << "left " << l << "\n";
        os << "right " << r << "\n";
        os << "top " << t << "\n";
        os << "bottom " << b << "\n";
    }

    // Read a line to fill in camera
    void fill_field(std::istringstream& line) {
        std::string field_name;
        line >> field_name;
        
        if (field_name == "position") {
            line >> position.x() >> position.y() >> position.z();
        } else if (field_name == "orientation") {
            line >> orientation(0) >> orientation(1) >> orientation(2) >> orientation(3);
        } else if (field_name == "near") {
            line >> n;
        } else if (field_name == "far") {
            line >> f;
        } else if (field_name == "left") {
            line >> l;
        } else if (field_name == "right") {
            line >> r;
        } else if (field_name == "top") {
            line >> t;
        } else if (field_name == "bottom") {
            line >> b;
        }
    }

    // Get the transformation matrix of the camera
    Eigen::Matrix4d get_transformation() const {
        return ::transformation::matrix_from_translation_vector(position.x(), position.y(), position.z()) *
               ::transformation::matrix_from_rotation_vector(orientation(0), orientation(1), orientation(2), orientation(3));
    }

    // Get the perspective projection matrix of the camera
    Eigen::Matrix4d get_perspective_projection_matrix() const {
        Eigen::Matrix4d proj = Eigen::Matrix4d::Zero();
        
        proj(0, 0) = (2.0 * n) / (r - l);
        proj(0, 2) = (r + l) / (r - l);
        proj(1, 1) = (2.0 * n) / (t - b);
        proj(1, 2) = (t + b) / (t - b);
        proj(2, 2) = -(f + n) / (f - n);
        proj(2, 3) = -(2.0 * f * n) / (f - n);
        proj(3, 2) = -1.0;
        
        return proj;
    }
};

// Stroing all objects in the scene, and provide interface to organize and render the scene
class SceneFile {
public:
    enum States {
        CAMERA,
        FILES_FIRST_LINE,
        FILES,
        WAIT_SECTION,
        ONE_SECTION_END,
        NEW_SECTION,
        GETTING_TRANSFORM
    };

    // Read the scene file and parse the camera and object information, parse the transformation matrix for each object
    SceneFile(const std::string& path) {
        state = States::CAMERA;
        current_label = "NONE";
        current_transform = Eigen::Matrix4d::Identity();
        scene_path = path;

        std::ifstream file(scene_path);
        if (!file.is_open()) {
            std::cerr << "Error: Could not open file " << scene_path << std::endl;
            return;   
        }

        std::string line;
        while (std::getline(file, line)) {
            state_transition(line);
            
            std::istringstream iss(line);
            // Handle each line according to different states
            if (state == States::CAMERA) {
                camera.fill_field(iss);
            } else if (state == States::FILES) {
                std::string label, obj_filename;
                iss >> label >> obj_filename;
                // std::cout << "Getting association: " << label << " -> " << obj_filename << std::endl;
                auto obj_data = std::make_shared<models::ObjModel>();
                
                // Try loading from raw filename first
                bool loaded = obj_data->load_from_obj_file(obj_filename);
                
                // If failed, try loading from scene directory + obj_filename
                if (!loaded) {
                    std::filesystem::path scene_dir = std::filesystem::path(scene_path).parent_path();
                    std::string full_path = (scene_dir / obj_filename).string();
                    loaded = obj_data->load_from_obj_file(full_path);
                }
                
                // If both failed, report error and skip
                if (!loaded) {
                    std::cerr << "Error: Could not load file " << obj_filename << std::endl;
                    continue;
                }
                
                object_files.emplace(label, std::make_pair(obj_data, 1));
            } else if (state == States::WAIT_SECTION) {
            } else if (state == States::NEW_SECTION) {
                iss >> current_label;
                // std::cout << "New section: " << current_label << std::endl;
                current_transform = Eigen::Matrix4d::Identity();
            } else if (state == States::GETTING_TRANSFORM) {
                Eigen::Matrix4d new_T = parse_transformation_line(iss);
                current_transform = new_T * current_transform; 
            } else if (state == States::ONE_SECTION_END) {
                do_transform();
            }
        }
    
        if (state == States::GETTING_TRANSFORM)
            do_transform();

        // if (apply_camera_transform)
        //     transform_to_camera_frame();
    }

    // void transform_to_camera_frame() {
    //     for (auto& object : objects) {
    //         object.points = camera.get_transformation().inverse() * object.points;
    //     }
    // }

    // Rendering pipeline
    ::ppm_image::PPMImage<uint8_t> render(int width, int height) const {
        ::ppm_image::PPMImage<uint8_t> result(height, width, ::ppm_image::BLACK);
        Eigen::Matrix4d T_ndc_pt = camera.get_perspective_projection_matrix() 
                            * camera.get_transformation().inverse();
                            
        for (const auto& object : objects) {
            Eigen::Matrix3Xd ndc_points_3d = ::transformation::points_homo_to_points_3d(T_ndc_pt * object.points);
            ::rendering::draw_object_edges(result, ndc_points_3d, object.faces());
        }
        
        return result;
    }

    Camera camera;
    std::vector<models::Model> objects;
    std::string scene_path;

private:
    States state;
    std::unordered_map<std::string, std::pair<std::shared_ptr<models::ObjModel>, int>> object_files;
    std::string current_label;
    Eigen::Matrix4d current_transform;

    void state_transition(std::string& line) {
        if (state == States::CAMERA) {
            if (line == "objects:") 
                state = States::FILES_FIRST_LINE;
        } else if (state == States::FILES_FIRST_LINE) {
            state = States::FILES;
        } else if (state == States::FILES) {
            if (line.empty()) 
                state = States::WAIT_SECTION;
        } else if (state == States::WAIT_SECTION) {
            if (!line.empty()) state = States::NEW_SECTION;
        } else if (state == States::NEW_SECTION) {
            state = States::GETTING_TRANSFORM;
        } else if (state == States::GETTING_TRANSFORM) {
            if (line.empty()) state = States::ONE_SECTION_END; 
        } else if (state == States::ONE_SECTION_END) {
            if (!line.empty()) state = States::NEW_SECTION;
                        else state = States::WAIT_SECTION;
        }
    }

    void do_transform() {
        auto it = object_files.find(current_label);
        if (it == object_files.end()) {
            std::cerr << "Error: Label \"" << current_label << "\" not found" << std::endl;
            return;
        }
    
        int& count = it->second.second;
        std::string new_label = current_label+"_copy"+std::to_string(count);
        models::Model new_object(it->second.first, new_label);
        new_object.points = current_transform * new_object.points;
        objects.emplace_back(std::move(new_object));
        
        // std::cout << "new copy: " << new_label << " added" << std::endl;
        count++;
        current_transform = Eigen::Matrix4d::Identity();
        current_label = "NONE";
    }
};


} // namespace scene


#endif // SCENE_H

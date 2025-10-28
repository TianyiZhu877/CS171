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

struct PointLight {
    Eigen::Vector3f position;
    Eigen::Vector3f color;
    float k;

    static std::optional<PointLight> parse_point_light_line(std::istringstream& line) {
        std::string field_name;
        line >> field_name;
        std::string seperation;
        if (field_name == "light") {
            PointLight light;
            line >> light.position.x() >> light.position.y() >> light.position.z() >> seperation;
            line >> light.color.x() >> light.color.y() >> light.color.z() >> seperation;
            line >> light.k;
            // light.serialize();
            return light;
        }
        return std::nullopt;
    }

    void serialize(std::ostream& os = std::cout) const {
        os << "light " << position.x() << " " << position.y() << " " << position.z() << " , ";
        os << color.x() << " " << color.y() << " " << color.z() << " , " << k << std::endl;
    }
    
};


// Class for a camera, contain all key params of a camera for rendering
struct Camera {
    Eigen::Vector3f position;
    Eigen::Vector4f orientation;
    float n;
    float f;
    float l;
    float r;
    float t;
    float b;
    
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

    enum RenderMode {
        GOURAUD,
        PHONG,
        EDGES
    };

    // Read the scene file and parse the camera and object information, parse the transformation matrix for each object
    SceneFile(const std::string& path): current_model(nullptr) {
        state = States::CAMERA;
        // current_label = "NONE";
        // current_transform = Eigen::Matrix4d::Identity();
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
                std::optional<PointLight> light = PointLight::parse_point_light_line(iss);
                if (light.has_value()) {
                    // light.value().serialize();
                    lights.push_back(std::move(light.value()));
                } else {
                    std::istringstream iss_camera(line);
                    camera.fill_field(iss_camera);
                }
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
                std::string new_label;
                iss >> new_label;
                create_new_object_copy(new_label);
                // std::cout << "New section: " << current_label << std::endl;
                // current_transform = Eigen::Matrix4d::Identity();
            } else if (state == States::GETTING_TRANSFORM) {
                if (!current_model.try_parse_material_line(iss)) {
                    std::istringstream iss_transform(line);
                    Eigen::Matrix4d new_T = parse_transformation_line(iss_transform);
                    // std::cout << line << std::endl;
                    // std::cout << "new_T: " << new_T << std::endl;
                    // std::cout << "current_transform: " << current_transform << std::endl;
                    current_model.transform = new_T * current_model.transform; 
                }
            } else if (state == States::ONE_SECTION_END) {
                objects.emplace_back(std::move(current_model));
            }
        }
    
        if (state == States::GETTING_TRANSFORM)
            objects.emplace_back(std::move(current_model));

        // if (apply_camera_transform)
        //     transform_to_camera_frame();
    }

    // void transform_to_camera_frame() {
    //     for (auto& object : objects) {
    //         object.points = camera.get_transformation().inverse() * object.points;
    //     }
    // }

    Camera camera;
    std::vector<models::Model> objects;
    std::string scene_path;
    std::vector<PointLight> lights;

private:
    States state;
    std::unordered_map<std::string, std::pair<std::shared_ptr<models::ObjModel>, int>> object_files;
    // std::string current_label;
    // Eigen::Matrix4d current_transform;
    models::Model current_model;

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

    void create_new_object_copy(const std::string& label) {
        current_model = models::Model(nullptr);
        auto it = object_files.find(label);
        if (it == object_files.end()) {
            std::cerr << "Error: Label \"" << label << "\" not found" << std::endl;
            return;
        }
    
        int& count = it->second.second;
        std::string new_label = label+"_copy"+std::to_string(count);
        current_model.obj_file = it->second.first;
        current_model.name = new_label;
        // std::cout << "new_object.transform: " << new_object.transform << std::endl;
        // std::cout << "current transform: " << current_transform << std::endl;
        // new_object.points = current_transform * new_object.points;;
        
        // std::cout << "new copy: " << new_label << " added" << std::endl;
        count++;
    }
};


} // namespace scene


#endif // SCENE_H

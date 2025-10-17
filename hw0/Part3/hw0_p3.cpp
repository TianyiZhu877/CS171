#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <unordered_map>
#include <Eigen/Dense>
#include "obj_model.h"
#include "hw_specific.h"
#include "transformation.h"

enum States {
    FILES,
    WAIT_SECTION,
    ONE_SECTION_END,
    NEW_SECTION,
    GETTING_TRANSFORM
};

/*
A state machine is used to parse each part of the input file, each state corresponds to a part of the input file, such as:
- Files: "object1 object1_filename.obj", needs to read the files
- Wait Section: To handle if there are multiple blank lines between sections
- New Section: "object1", get the label of the vertexes it use
- Getting Transform: "r ux uy uz angle", read the actual transform
- One Section End: The empty line after each section, perform the transform
*/
static States state = States::FILES;
// Where we store the .obj read, mapped from their labels
static std::unordered_map<std::string, std::pair<obj_model::ObjModel, int>> object_files;
// The current transform matrix and the label of the vertexes it use
static Eigen::Matrix4d current_transform = Eigen::Matrix4d::Identity();
static std::string current_label = "NONE";
// Where we store the output, with each element being the new name and the matrix of the vertexes
static std::vector<std::pair<std::string, Eigen::Matrix3Xd>> output;

static void state_transition(std::string& line) {
    if (state == States::FILES) {
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

static void do_transform() {
    auto it = object_files.find(current_label);
    if (it == object_files.end()) {
        std::cerr << "Error: Label \"" << current_label << "\" not found" << std::endl;
        return;
    }

    int& count = it->second.second;
    Eigen::Matrix4Xd points = it->second.first.export_vertexes_matrix_homo();
    points = current_transform * points;
    Eigen::Matrix3Xd points_3d = ::transformation::points_homo_to_points_3d(points);
    std::string new_label = current_label+"_copy"+std::to_string(count);
    output.emplace_back(std::make_pair(new_label, std::move(points_3d)));
    // std::cout << "new copy: " << new_label << " added" << std::endl;
    count++;
    current_transform = Eigen::Matrix4d::Identity();
    current_label = "NONE";
}

int main(int argc, char* argv[]) {

    std::ifstream file(argv[1]);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open file " << argv[1] << std::endl;

        return 1;   
    }
    
    std::string line;
    while (std::getline(file, line)) {
        state_transition(line);
        
        std::istringstream iss(line);
        // Handle each line according to different states
        if (state == States::FILES) {
            std::string label, filename;
            iss >> label >> filename;
            // std::cout << "Getting association: " << label << " -> " << filename << std::endl;
            obj_model::ObjModel obj_data;
            obj_data.load_from_obj_file(filename);
            object_files.emplace(label, std::make_pair(std::move(obj_data), 1));
        } else if (state == States::WAIT_SECTION) {
        } else if (state == States::NEW_SECTION) {
            iss >> current_label;
            // std::cout << "New section: " << current_label << std::endl;
            current_transform = Eigen::Matrix4d::Identity();
        } else if (state == States::GETTING_TRANSFORM) {
            Eigen::Matrix4d new_T = hw_specific::parse_transformation_line(iss);
            current_transform = new_T * current_transform; 
        } else if (state == States::ONE_SECTION_END) {
            do_transform();
        }
    }

    if (state == States::GETTING_TRANSFORM)
        do_transform();

    // Print the output
    for (const auto& [label, points] : output) {
        std::cout << label << std::endl;
        for (int i = 0; i < points.cols(); i++) {
            std::cout << points(0, i) << " " << points(1, i) << " " << points(2, i) << std::endl;
        }
        std::cout << std::endl;
    }

    return 0;
}

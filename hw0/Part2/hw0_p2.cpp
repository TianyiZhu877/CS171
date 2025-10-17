#include "transformation.h"
#include "hw_specific.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <Eigen/Dense>

int main(int argc, char* argv[]) {
    
    std::ifstream file(argv[1]);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open file " << argv[1] << std::endl;
        return 1;
    }

    Eigen::Matrix4d final_T = Eigen::Matrix4d::Identity();
    std::string line;
    // Iterate through the each line in the file
    while (std::getline(file, line)) {
        // Skip empty lines
        if (line.empty()) continue;
        
        std::istringstream iss(line);
        std::string prefix;
        double x, y, z, angle;
        Eigen::Matrix4d new_T = hw_specific::parse_transformation_line(iss, prefix, x, y, z, angle);

        final_T = new_T * final_T;
        if (prefix == "r")
            std::cout << "Got line: " << prefix << " " << x << " " << y << " " << z << " " << angle << std::endl;
        else
            std::cout << "Got line: " << prefix << " " << x << " " << y << " " << z << " " << std::endl;
    
        std::cout << "Its Transformation Matrx is: " << std::endl << new_T << std::endl << std::endl;
    }
    std::cout << "Combined Transformation Matrx is: " << std::endl << final_T << std::endl;
}

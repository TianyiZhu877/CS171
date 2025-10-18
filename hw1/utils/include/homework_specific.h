#include "transformation.h"
#include <fstream>
#include <sstream>
#include <Eigen/Dense>

namespace homework_specific {

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

class SceneFileReader {
public:



}



}

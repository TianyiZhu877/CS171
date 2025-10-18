
#ifndef TRANSFORMATION_H
#define TRANSFORMATION_H

#include <Eigen/Dense>
// Some utility functions to help transform the points and built the transformation matrix. 
// The function names are self-explanatory.
namespace transformation{

inline Eigen::Matrix4d matrix_from_translation_vector(double tx, double ty, double tz) {
    Eigen::Matrix4d T = Eigen::Matrix4d::Identity();
    T(0, 3) = tx;
    T(1, 3) = ty;
    T(2, 3) = tz;
    return T;
}
    
inline Eigen::Matrix4d matrix_from_scaling_vector(double sx, double sy, double sz) {
    Eigen::Matrix4d T = Eigen::Matrix4d::Identity();
    T(0, 0) = sx;
    T(1, 1) = sy;
    T(2, 2) = sz;
    return T;
}


inline Eigen::Matrix4d matrix_from_rotation_vector(double ux, double uy, double uz, double angle) {
    using namespace Eigen;

    // Normalize the rotation axis
    Vector3d axis(ux, uy, uz);
    axis.normalize();

    Matrix3d R;
    double c = std::cos(angle);
    double s = std::sin(angle);
    double one_c = 1.0 - c;

    double x = axis.x(), y = axis.y(), z = axis.z();

    R << 
        c + x*x*one_c,     x*y*one_c - z*s, x*z*one_c + y*s,
        y*x*one_c + z*s,   c + y*y*one_c,   y*z*one_c - x*s,
        z*x*one_c - y*s,   z*y*one_c + x*s, c + z*z*one_c;

    Matrix4d T = Matrix4d::Identity();
    T.block<3,3>(0,0) = R;

    return T;
}


inline Eigen::Matrix4d matrix_from_eular_angles(double rx, double ry, double rz) {
    using namespace Eigen;
    Matrix3d Rx, Ry, Rz;

    Rx = AngleAxisd(rx, Vector3d::UnitX()).toRotationMatrix();
    Ry = AngleAxisd(ry, Vector3d::UnitY()).toRotationMatrix();
    Rz = AngleAxisd(rz, Vector3d::UnitZ()).toRotationMatrix();

    Matrix3d R = Rz * Ry * Rx;

    Matrix4d T = Matrix4d::Identity();
    T.block<3,3>(0,0) = R;

    return T;
}

inline Eigen::Matrix3Xd points_homo_to_points_3d(Eigen::Matrix4Xd& matrix) {
    Eigen::Matrix3Xd result(3, matrix.cols());

    result = matrix.topRows<3>().array().rowwise() / matrix.row(3).array();

    return result;
}

inline Eigen::Matrix4Xd points_3d_to_points_homo(Eigen::Matrix3Xd& matrix) {
    const int cols = static_cast<int>(matrix.cols());
    Eigen::Matrix4Xd result(4, cols);
    
    result.block(0, 0, 3, cols) = matrix;
    result.row(3).setOnes();
    
    return result;
}

} // namespace transformation

#endif // TRANSFORMATION_H

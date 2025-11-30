
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

inline Eigen::Matrix3Xd points_homo_to_points_3d(const Eigen::Matrix4Xd& matrix) {
    Eigen::Matrix3Xd result(3, matrix.cols());

    result = matrix.topRows<3>().array().rowwise() / matrix.row(3).array();

    return result;
}

inline Eigen::Matrix4Xd points_3d_to_points_homo(const Eigen::Matrix3Xd& matrix) {
    const int cols = static_cast<int>(matrix.cols());
    Eigen::Matrix4Xd result(4, cols);
    
    result.block(0, 0, 3, cols) = matrix;
    result.row(3).setOnes();
    
    return result;
}

inline Eigen::Matrix3d get_matrix_3x3_from_quaternion(const Eigen::Vector4d& quaternion) {
    using namespace Eigen;
    
    double x = quaternion(0);
    double y = quaternion(1);
    double z = quaternion(2);
    double s = quaternion(3);

    double norm = std::sqrt(x*x + y*y + z*z + s*s);
    if (norm > 0) {
        x /= norm;
        y /= norm;
        z /= norm;
        s /= norm;
    }
    
    Matrix3d R;
    R  <<   1 - 2*(y*y + z*z),  2*(x*y - s*z),      2*(x*z + s*y),
            2*(x*y + s*z),      1 - 2*(x*x + z*z),  2*(y*z - s*x),
            2*(x*z - s*y),      2*(y*z + s*x),      1 - 2*(x*x + y*y);

    return R;

}

inline Eigen::Matrix4d get_martix_4x4_from_quaternion(const Eigen::Vector4d& quaternion) {
    Eigen::Matrix4d T = Eigen::Matrix4d::Identity();
    T.block<3,3>(0,0) = get_matrix_3x3_from_quaternion(quaternion);
    return T;
}

inline Eigen::Vector3d screen_to_ndc_unit_sphere(int x, int y, int width, int height) {
    double ndc_x = (2.0 * x) / width - 1.0;
    double ndc_y = 1.0 - (2.0 * y) / height;  // Flip Y axis
    
    double radius_squared = ndc_x * ndc_x + ndc_y * ndc_y;
    double ndc_z;
    
    if (radius_squared <= 1.0) {
        // Inside unit circle - project onto upper hemisphere
        ndc_z = std::sqrt(1.0 - radius_squared);
    } else {
        // Outside unit circle - normalize to unit circle edge
        double norm = std::sqrt(radius_squared);
        ndc_x /= norm;
        ndc_y /= norm;
        ndc_z = 0.0;
    }
    
    return Eigen::Vector3d(ndc_x, ndc_y, ndc_z);
}

inline Eigen::Vector4d quaternion_multiply(const Eigen::Vector4d& q1, const Eigen::Vector4d& q2) {
    double x1 = q1(0), y1 = q1(1), z1 = q1(2), w1 = q1(3);
    double x2 = q2(0), y2 = q2(1), z2 = q2(2), w2 = q2(3);
    
    Eigen::Vector4d result;
    result(0) = w1*x2 + x1*w2 + y1*z2 - z1*y2;  // x
    result(1) = w1*y2 - x1*z2 + y1*w2 + z1*x2;  // y
    result(2) = w1*z2 + x1*y2 - y1*x2 + z1*w2;  // z
    result(3) = w1*w2 - x1*x2 - y1*y2 - z1*z2;  // w
    
    return result;
}

} // namespace transformation

#endif // TRANSFORMATION_H

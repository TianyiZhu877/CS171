#ifndef CR_SPINE_H
#define CR_SPINE_H

#include <Eigen/Dense>
#include <vector>
#include <array>

namespace CR_spine {
    // Catmull-Rom spline basis matrix (4x4)
    // This matrix is used for cubic interpolation between control points
    // The matrix is typically multiplied by 0.5, so the values are:
    inline const Eigen::Matrix4f B = (Eigen::Matrix4f() <<
        0.0f,  1.0f,  0.0f,  0.0f,
       -0.5f,  0.0f,  0.5f,  0.0f,
        1.0f, -2.5f,  2.0f, -0.5f,
       -0.5f,  1.5f, -1.5f,  0.5f
    ).finished();

    Eigen::Vector4f inline get_u_vector(float u) {
        return (Eigen::Vector4f() << 1.0f, u, u*u, u*u*u).finished();
    }
}

#endif
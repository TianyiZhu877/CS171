#ifndef QUATERNION_MATH_H
#define QUATERNION_MATH_H

#include <Eigen/Dense>
#include <cmath>


namespace quaternion_math {
    
    inline constexpr float RAD_TO_DEG = 180.0f / M_PI;
    inline constexpr float DEG_TO_RAD = M_PI / 180.0f;

    struct Quaternion {
        float w;
        float x;
        float y;
        float z;
    };

    Quaternion inline normalized(const Quaternion& q) {
        Quaternion q_normalized;
        float norm = sqrt(q.w * q.w + q.x * q.x + q.y * q.y + q.z * q.z);
        if (norm == 0) {
            return q;
        }
        q_normalized.w = q.w / norm;
        q_normalized.x = q.x / norm;
        q_normalized.y = q.y / norm;
        q_normalized.z = q.z / norm;
        return q_normalized;
    }

    inline Quaternion rot_to_quat(float x, float y, float z, float angle) {
        float half_angle = angle * DEG_TO_RAD * 0.5f;
        
        Quaternion q;
        q.w = std::cos(half_angle);
        q.x = x * std::sin(half_angle);
        q.y = y * std::sin(half_angle);
        q.z = z * std::sin(half_angle);
    
        return normalized(q);
    }

    inline void quat_to_rot(const Quaternion& q, float& x, float& y, float& z, float& angle) {
        Quaternion q_normalized = normalized(q);
        float sin_half = sqrt(1 - q_normalized.w * q_normalized.w);


        if (sin_half < 1e-6f) {
            x = 1;
            y = 0;
            z = 0;
            angle = 0;
        } else {
            x = q_normalized.x / sin_half;
            y = q_normalized.y / sin_half;
            z = q_normalized.z / sin_half;
            angle = 2 * std::acos(q_normalized.w) * RAD_TO_DEG;
        }
    }
    

}

#endif
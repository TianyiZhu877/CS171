#ifndef POS_SCALE_ROT_H
#define POS_SCALE_ROT_H
#include <sstream>
#include <string>
#include <iostream>
#include <Eigen/Dense>
#include "quaternion_math.h"

struct PosScaleRot {
    using eigenMatrixRow = Eigen::Matrix<float, 1, 10>;

    float pos_x;
    float pos_y;
    float pos_z;
    float scale_x;
    float scale_y;
    float scale_z;
    float rot_x;
    float rot_y;
    float rot_z;
    float rot_angle;

    bool try_parse_line(std::istringstream& line) {
        bool succeed = true;
        std::string token;
        line >> token;
        if (token == "translation") {
            line >> pos_x >> pos_y >> pos_z;
        } else if (token == "scale") {
            line >> scale_x >> scale_y >> scale_z;
        } else if (token == "rotation") {
            line >> rot_x >> rot_y >> rot_z >> rot_angle;
        } else {
            succeed = false;
        }
        return succeed;
    }


    eigenMatrixRow to_eigen_matrix_row() const {
        eigenMatrixRow row;
        quaternion_math::Quaternion quat = quaternion_math::rot_to_quat(rot_x, rot_y, rot_z, rot_angle);
        row << pos_x, pos_y, pos_z, scale_x, scale_y, scale_z, quat.w, quat.x, quat.y, quat.z;
        return row;
    }

    void from_eigen_matrix_row(const eigenMatrixRow& row) {
        pos_x = row(0);
        pos_y = row(1);
        pos_z = row(2);
        scale_x = row(3);
        scale_y = row(4);
        scale_z = row(5);
        quaternion_math::quat_to_rot(quaternion_math::Quaternion{row(6), row(7), row(8), row(9)}, rot_x, rot_y, rot_z, rot_angle);
    }

    void print() const {
        std::cout << pos_x << " " << pos_y << " " << pos_z << " " << scale_x << " " << scale_y << " " << scale_z << " " << rot_x << " " << rot_y << " " << rot_z << " " << rot_angle << std::endl;
    }

};

#endif
#ifndef RENDERING_H
#define RENDERING_H

#include <Eigen/Dense>
#include <functional>
#include "ppm_image.h"
#include "models.h"

namespace rendering {

    void draw_object_edges(::ppm_image::PPMImage<uint8_t>& image, const Eigen::Matrix3Xd& ndc_points, 
        const models::ObjModel::FaceList& faces, ::ppm_image::Pixel<uint8_t> color = ::ppm_image::WHITE);

    void bresenham_draw_line(int x0, int y0, int x1, int y1, std::function<void(int, int, float)> fill);
        
    void draw_object_dots(::ppm_image::PPMImage<uint8_t>& image, const Eigen::Matrix3Xd& ndc_points);

} // namespace rendering

#endif // RENDERING_H


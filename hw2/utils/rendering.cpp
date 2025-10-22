#include "rendering.h"
#include "ppm_image.h"
#include "models.h"
#include <Eigen/Dense>
#include <cmath>
#include <algorithm>

namespace rendering {

void draw_object_edges(::ppm_image::PPMImage<float>& image, const Eigen::Matrix3Xd& ndc_points, Eigen::VectorXi points_within_ndc_cube,
                       const models::ObjModel::FaceList& faces, ::ppm_image::Pixel<float> color) {
    // Draw vertices for now
    for (const auto& face: faces) {

        for (int i=0; i<3; i++) {
            int start_point_idx = face[i];
            int end_point_idx = face[(i+1)%3];

            if (points_within_ndc_cube(start_point_idx) > 0 || points_within_ndc_cube(end_point_idx) > 0) {
            
                auto [x0, y0] = ndc_to_screen(ndc_points.col(start_point_idx), image.w(), image.h());
                auto [x1, y1] = ndc_to_screen(ndc_points.col(end_point_idx), image.w(), image.h());            
                // int x0 = static_cast<int>((ndc_points(0, start_point_idx) + 1.0) / 2.0 * image.w());
                // int y0 = static_cast<int>((1.0 - ndc_points(1, start_point_idx)) / 2.0 * image.h());
                // int x1 = static_cast<int>((ndc_points(0, end_point_idx) + 1.0) / 2.0 * image.w());
                // int y1 = static_cast<int>((1.0 - ndc_points(1, end_point_idx)) / 2.0 * image.h());

                // std::cout << ndc_points(0, start_point_idx) << "; " << ndc_points(0, end_point_idx) << std::endl;

                rendering::bresenham_draw_line(x0, y0, x1, y1, [&](int x, int y, float alpha){
                    if (x >= 0 && x < static_cast<int>(image.w()) && y >= 0 && y < static_cast<int>(image.h())) {
                        image[y][x] = image[y][x] * (1 - alpha) + color * alpha;
                    }
                    // std::cout << "filling " << y << " " << x << std::endl;
                });

            }
        }
    }
}


void draw_object_dots(::ppm_image::PPMImage<float>& image, const Eigen::Matrix3Xd& ndc_points) {

    for (int i = 0; i < ndc_points.cols(); ++i) {
        int x0 = static_cast<int>((ndc_points(0, i) + 1.0) / 2.0 * image.w());
        int y0 = static_cast<int>((1.0 - ndc_points(1, i)) / 2.0 * image.h());

        // Check bounds
        if (x0 >= 0 && x0 < static_cast<int>(image.w()) && 
            y0 >= 0 && y0 < static_cast<int>(image.h())) {
            image[y0][x0] = ppm_image::colors_f::WHITE;
        }
    }
}


} // namespace rendering
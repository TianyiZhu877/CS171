#include "rendering.h"
#include "ppm_image.h"
#include "models.h"
#include <Eigen/Dense>
#include <cmath>
#include <algorithm>

namespace rendering {

void draw_object_edges(::ppm_image::PPMImage<uint8_t>& image, const Eigen::Matrix3Xd& ndc_points, 
                       const models::ObjModel::FaceList& faces, ::ppm_image::Pixel<uint8_t> color) {
    // Draw vertices for now
    for (const auto& face: faces) {

        for (int i=0; i<3; i++) {
            int start_point_idx = face[i];
            int end_point_idx = face[(i+1)%3];
            
            int x0 = static_cast<int>((ndc_points(0, start_point_idx) + 1.0) / 2.0 * image.w());
            int y0 = static_cast<int>((1.0 - ndc_points(1, start_point_idx)) / 2.0 * image.h());
            int x1 = static_cast<int>((ndc_points(0, end_point_idx) + 1.0) / 2.0 * image.w());
            int y1 = static_cast<int>((1.0 - ndc_points(1, end_point_idx)) / 2.0 * image.h());

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

void bresenham_draw_line(int x0, int y0, int x1, int y1, std::function<void(int, int, float)> fill) {
    bool steep = std::abs(y1 - y0) > std::abs(x1 - x0);
    
    if (steep) {
        std::swap(x0, y0);
        std::swap(x1, y1);
    }

    if (x0 > x1) {
        std::swap(x0, x1);
        std::swap(y0, y1);
    }
    int dx = x1 - x0;

    int dy = std::abs(y1 - y0);
    int y_step = (y0 < y1) ? 1 : -1;

    int y = y0;

    // std::cout << "drawing line" << x0 << "," << y0 << "; " << x1 << "," << y1 << std::endl;

    int e = 0;
    for (int x = x0; x < x1; x++) {
        float alpha = 1.0;
        if (steep) {
            fill(y, x, alpha);
        }
        else {
            fill(x, y, alpha);
        }
        e += dy;
        if (2 * (e + dy) >= dx) {
            y += y_step;
            e -= dx;
        }
    }
}

void draw_object_dots(::ppm_image::PPMImage<uint8_t>& image, const Eigen::Matrix3Xd& ndc_points) {

    for (int i = 0; i < ndc_points.cols(); ++i) {
        int x0 = static_cast<int>((ndc_points(0, i) + 1.0) / 2.0 * image.w());
        int y0 = static_cast<int>((1.0 - ndc_points(1, i)) / 2.0 * image.h());

        // Check bounds
        if (x0 >= 0 && x0 < static_cast<int>(image.w()) && 
            y0 >= 0 && y0 < static_cast<int>(image.h())) {
            image[y0][x0] = ::ppm_image::WHITE;
        }
    }
}


} // namespace rendering
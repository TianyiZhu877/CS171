#ifndef RENDERING_H
#define RENDERING_H

#include <utility>
#include <vector>
#include <Eigen/Dense>
#include "ppm_image.h"
#include "models.h"

namespace rendering {

    constexpr bool ENABLE_ANTIALIASING = false;

    // Convert NDC coordinates to screen coordinates
    // NDC range: [-1, 1], Screen range: [0, width/height]
    inline std::pair<int, int> ndc_to_screen(const Eigen::Vector3d& ndc_point, std::size_t width, std::size_t height) {
        int x = static_cast<int>((ndc_point(0) + 1.0) / 2.0 * width);
        int y = static_cast<int>((ndc_point(1) + 1.0) / 2.0 * height);
        return {x, y};
    }

    // Check if a single point is within the NDC cube [-1, 1]^3
    inline bool within_ndc_cube(const Eigen::Vector3d& ndc_point) {
        return ndc_point(0) >= -1.0 && ndc_point(0) <= 1.0 &&
               ndc_point(1) >= -1.0 && ndc_point(1) <= 1.0 &&
               ndc_point(2) >= -1.0 && ndc_point(2) <= 1.0;
    }

    // Check which points are within the NDC cube, returns a mask (1 = inside, 0 = outside)
    inline Eigen::VectorXi within_ndc_cube_mask(const Eigen::Matrix3Xd& ndc_points) {
        Eigen::VectorXi mask(ndc_points.cols());
        
        for (int i = 0; i < ndc_points.cols(); ++i) {
            mask(i) = within_ndc_cube(ndc_points.col(i)) ? 1 : 0;
        }
        
        return mask;
    }


    /* Draw the edges of the object on the image.
        @param image: the image to draw on, pass as a reference to incrementally draws on it
        @param ndc_points: the ndc points of the object
        @param faces: the surfaces of the object
        @param color: the color of the edges
    */
    void draw_object_edges(::ppm_image::PPMImage<float>& image, const Eigen::Matrix3Xd& ndc_points, Eigen::VectorXi points_within_ndc_cube,
        const models::ObjModel::FaceList& faces, ::ppm_image::Pixel<float> color = ppm_image::colors_f::WHITE);

    // FillFunc should have the signature void(int x, int y, float alpha)
    template<typename FillFunc>
    void bresenham_draw_line(int x0, int y0, int x1, int y1, const FillFunc& fill) {
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
            if constexpr (ENABLE_ANTIALIASING) {
                float alpha = static_cast<float>(e) / static_cast<float>(dx);
                if (steep) {
                    fill(y, x, alpha);
                    fill(y+y_step, x, 1-alpha);
                }
                else {
                    fill(x, y, alpha);
                    fill(x, y+y_step, 1-alpha);
                }
            } else {
                float alpha = 1.0;
                if (steep) {
                    fill(y, x, alpha);
                }
                else {
                    fill(x, y, alpha);
                }
            }
        
            e += dy;
            if (2 * e >= dx) {
                y += y_step;
                e -= dx;
            }
        }
    }
        
    // A test function to draw the only the dots of vertexes of the object
    void draw_object_dots(::ppm_image::PPMImage<uint8_t>& image, const Eigen::Matrix3Xd& ndc_points);

} // namespace rendering

#endif // RENDERING_H


#ifndef RENDERING_H
#define RENDERING_H

#include <Eigen/Dense>
#include "ppm_image.h"
#include "models.h"

namespace rendering {

    constexpr bool ENABLE_ANTIALIASING = false;

    /* Draw the edges of the object on the image.
        @param image: the image to draw on, pass as a reference to incrementally draws on it
        @param ndc_points: the ndc points of the object
        @param faces: the surfaces of the object
        @param color: the color of the edges
    */
    void draw_object_edges(::ppm_image::PPMImage<uint8_t>& image, const Eigen::Matrix3Xd& ndc_points, 
        const models::ObjModel::FaceList& faces, ::ppm_image::Pixel<uint8_t> color = ::ppm_image::WHITE);

    // FillFunc should have the signature void(int x, int y, float alpha)
    template<typename FillFunc>
    void bresenham_draw_line(int x0, int y0, int x1, int y1, FillFunc fill) {
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


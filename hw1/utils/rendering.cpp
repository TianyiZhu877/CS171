#include "rendering.h"
#include "ppm_image.h"
#include "models.h"
#include <Eigen/Dense>

namespace rendering {

void draw_object_edge(::ppm_image::PPMImage<uint8_t>& image, const Eigen::Matrix3Xd& ndc_points, 
                      const models::ObjModel::FaceList& faces) {
    // Draw vertices for now
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
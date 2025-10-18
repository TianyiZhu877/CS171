#ifndef RENDERING_H
#define RENDERING_H

#include <Eigen/Dense>
#include "ppm_image.h"
#include "models.h"

namespace rendering {

void draw_object_edge(::ppm_image::PPMImage<uint8_t>& image, const Eigen::Matrix3Xd& ndc_points, 
                      const models::ObjModel::FaceList& faces);

} // namespace rendering

#endif // RENDERING_H


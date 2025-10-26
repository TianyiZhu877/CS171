#include <Eigen/Dense>
#include <cmath>
#include <algorithm>
#include "rendering.h"
#include "ppm_image.h"
#include "models.h"
#include "scene.h"
#include "shader.h"

namespace cpu {
namespace rendering {

ppm_image::Pixel<float> lighting(const Eigen::Vector3d& P, const Eigen::Vector3d& normal, const models::Model& model, const std::vector<scene::PointLight>& lights, const Eigen::Vector3d& eye_pos) {
    
    
    
    ppm_image::Pixel<float> diffuse_sum(0.0f, 0.0f, 0.0f);
    ppm_image::Pixel<float> specular_sum(0.0f, 0.0f, 0.0f);
    Eigen::Vector3d e_dir = (eye_pos - P).normalized();

    
    for (const auto& light : lights) {
        Eigen::Vector3d L_vec = light.position - P;
        double distance = L_vec.norm();
        Eigen::Vector3d L_dir = L_vec.normalized();
        
        // Distance attenuation
        // double attenuation = 1.0;
        double attenuation = 1.0 / (1.0 + light.k * distance * distance);
        
        // Diffuse component
        ppm_image::Pixel<float> L_diffuse = light.color * std::max(0.0, normal.dot(L_dir)) * attenuation;
        diffuse_sum += L_diffuse;

        // Specular component
        Eigen::Vector3d half_vector = (e_dir + L_dir).normalized();
        ppm_image::Pixel<float> L_specular = light.color * std::pow(std::max(0.0, normal.dot(half_vector)), model.shininess) * attenuation;
        specular_sum += L_specular;
    }
    
    ppm_image::Pixel<float> result;
    const ppm_image::Pixel<float>& Ca = model.ambient;
    const ppm_image::Pixel<float>& Cs = model.specular;
    const ppm_image::Pixel<float>& Cd = model.diffuse;
    result.r = Ca.r + diffuse_sum.r*Cd.r + specular_sum.r*Cs.r;
    result.g = Ca.g + diffuse_sum.g*Cd.g + specular_sum.g*Cs.g;
    result.b = Ca.b + diffuse_sum.b*Cd.b + specular_sum.b*Cs.b;
    result.clamp(1.0);
    
    return result;
}



void render_object(ppm_image::PPMImage<float>& image, const models::Model& model, const scene::Camera& camera, cpu::shader::Shader& shader, Eigen::MatrixXd& z_buffer) {
    // Draw vertices for now
    Eigen::Matrix4d T_ndc_pt = camera.get_perspective_projection_matrix() * camera.get_transformation().inverse();
    Eigen::Matrix4Xd vertexes_homo = model.points_homo_transformed();
    Eigen::Matrix3Xd vertexes = transformation::points_homo_to_points_3d(vertexes_homo);
    Eigen::Matrix3Xd normals = model.normals_transformed();
    Eigen::Matrix3Xd ndc_points = transformation::points_homo_to_points_3d(T_ndc_pt *  vertexes_homo);
    Eigen::VectorXi points_within_ndc_cube = rendering::compute_within_ndc_cube_mask(ndc_points);
    
    for (const auto& face: model.faces()) {
        // if (points_within_ndc_cube(face[0]) > 0 && points_within_ndc_cube(face[1]) > 0 && points_within_ndc_cube(face[2]) > 0) {
            Eigen::Vector3d ndc_a = ndc_points.col(face[0]);
            Eigen::Vector3d ndc_b = ndc_points.col(face[1]);
            Eigen::Vector3d ndc_c = ndc_points.col(face[2]);
            Eigen::Vector3d cross = (ndc_b - ndc_a).cross(ndc_c - ndc_a);
            if (cross.z() > 0) {
                shader.new_triangle(vertexes.col(face[0]), vertexes.col(face[1]), vertexes.col(face[2]),
                                    normals.col(face[3]), normals.col(face[4]), normals.col(face[5]));

                auto [xa, ya] = ndc_to_screen(ndc_a, image.w(), image.h());
                auto [xb, yb] = ndc_to_screen(ndc_b, image.w(), image.h());
                auto [xc, yc] = ndc_to_screen(ndc_c, image.w(), image.h());

                int xmin = std::max(0, std::min(xa, std::min(xb, xc)));
                int xmax = std::min(static_cast<int>(image.w()), std::max(xa, std::max(xb, xc)));
                int ymin = std::max(0, std::min(ya, std::min(yb, yc)));
                int ymax = std::min(static_cast<int>(image.h()), std::max(ya, std::max(yb, yc)));

                for (int x = xmin; x <= xmax; x++) {
                    for (int y = ymin; y <= ymax; y++) {
                        auto [alpha, beta, gamma] = compute_alpha_beta_gamma(xa, ya, xb, yb, xc, yc, x, y);
                        if (alpha >= 0 && beta >= 0 && gamma >= 0 && alpha <= 1 && beta <= 1 && gamma <= 1) {
                            
                            Eigen::Vector3d ndc = alpha * ndc_a + beta * ndc_b + gamma * ndc_c;
                            if (within_ndc_cube(ndc) && ndc.z() < z_buffer(y, x)) {
                                z_buffer(y, x) = ndc.z();
                                ppm_image::Pixel<float> color = shader.compute_color(alpha, beta, gamma);
                                color.clamp(1.0);
                                image[y][x] = color;
                            }
                        }
                    }
                }
            }
              
        // }
    }
}

void draw_object_edges(ppm_image::PPMImage<float>& image, const models::Model& model, 
    const scene::Camera& camera, ppm_image::Pixel<float> color) {
    // Draw vertices for now
    
    Eigen::Matrix4d T_ndc_pt = camera.get_perspective_projection_matrix() * camera.get_transformation().inverse();
    Eigen::Matrix3Xd ndc_points = transformation::points_homo_to_points_3d(T_ndc_pt *  model.points_homo_transformed());
    Eigen::VectorXi points_within_ndc_cube = rendering::compute_within_ndc_cube_mask(ndc_points);

    for (const auto& face: model.faces()) {

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
} // namespace cpu
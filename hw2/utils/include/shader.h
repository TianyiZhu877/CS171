#ifndef SHADER_H
#define SHADER_H

#include <Eigen/Dense>
#include "ppm_image.h"
#include "models.h"

// Forward declarations
namespace scene {
    struct PointLight;
}
namespace rendering {
    ppm_image::Pixel<float> lighting(const Eigen::Vector3d& P, const Eigen::Vector3d& normal, 
                                   const models::Model& model, const std::vector<scene::PointLight>& lights, 
                                   const Eigen::Vector3d& eye_pos);
}

namespace shader {
class Shader {
public:
    Shader(models::Model& model, std::vector<scene::PointLight>& lights, Eigen::Vector3d eye_pos): model(model), lights(lights), eye_pos(eye_pos) {}

    virtual ppm_image::Pixel<float> compute_color(float alpha, float beta, float gamma) = 0;

    virtual void new_triangle(const Eigen::Vector3d& va, const Eigen::Vector3d& vb, const Eigen::Vector3d& vc, 
        const Eigen::Vector3d& na, const Eigen::Vector3d& nb, const Eigen::Vector3d& nc) = 0;

protected:
    models::Model& model;
    std::vector<scene::PointLight>& lights;
    Eigen::Vector3d eye_pos;
};


class Gouraud : public Shader {
public:
    Gouraud(models::Model& model, std::vector<scene::PointLight>& lights, Eigen::Vector3d eye_pos) 
        : Shader(model, lights, eye_pos) {}
    
    ppm_image::Pixel<float> compute_color(float alpha, float beta, float gamma) override {
        return color_a * alpha + color_b * beta + color_c * gamma;
    }

    void new_triangle(const Eigen::Vector3d& va, const Eigen::Vector3d& vb, const Eigen::Vector3d& vc, 
        const Eigen::Vector3d& na, const Eigen::Vector3d& nb, const Eigen::Vector3d& nc) override {
            color_a = rendering::lighting(va, na, model, lights, eye_pos);
            color_b = rendering::lighting(vb, nb, model, lights, eye_pos);
            color_c = rendering::lighting(vc, nc, model, lights, eye_pos);
    }

protected:
    ppm_image::Pixel<float> color_a;
    ppm_image::Pixel<float> color_b;
    ppm_image::Pixel<float> color_c;
};


class Phong : public Shader {
public:
    Phong(models::Model& model, std::vector<scene::PointLight>& lights, Eigen::Vector3d eye_pos) 
        : Shader(model, lights, eye_pos) {}

    
    void new_triangle(const Eigen::Vector3d& va, const Eigen::Vector3d& vb, const Eigen::Vector3d& vc, 
        const Eigen::Vector3d& na, const Eigen::Vector3d& nb, const Eigen::Vector3d& nc) override {
        this->va = va;
        this->vb = vb;
        this->vc = vc;
        this->na = na;
        this->nb = nb;
        this->nc = nc;
    }
    
    ppm_image::Pixel<float> compute_color(float alpha, float beta, float gamma) override {
        Eigen::Vector3d normal = (alpha * na + beta * nb + gamma * nc).normalized();
        Eigen::Vector3d vertex = alpha * va + beta * vb + gamma * vc;
        return rendering::lighting(vertex, normal, model, lights, eye_pos);
    }

protected:
    Eigen::Vector3d va;
    Eigen::Vector3d vb;
    Eigen::Vector3d vc;
    Eigen::Vector3d na;
    Eigen::Vector3d nb;
    Eigen::Vector3d nc;
};

} // namespace shaders

#include "rendering.h"

#endif // SHADER_H
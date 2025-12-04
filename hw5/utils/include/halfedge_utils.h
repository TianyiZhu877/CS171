#ifndef HALFEDGE_UTILS_H
#define HALFEDGE_UTILS_H

#include "halfedge.h"
#include <cmath>

namespace halfedge {

inline void compute_face_normal_area(HEF *face) {
    halfedge::HEV *v1 = face->edge->vertex;
    halfedge::HEV *v2 = face->edge->next->vertex;
    halfedge::HEV *v3 = face->edge->next->next->vertex;
    Eigen::Vector3f edge1(v2->x - v1->x, v2->y - v1->y, v2->z - v1->z);
    Eigen::Vector3f edge2(v3->x - v1->x, v3->y - v1->y, v3->z - v1->z);
    Eigen::Vector3f normal = edge1.cross(edge2);
    face->area = 0.5 * normal.norm();
    normal.normalize();
    face->normal = halfedge::Vec3f{normal.x(), normal.y(), normal.z()};
}

// assuming the face normals are already computed
inline void compute_vertex_normal(HEV *vertex) {
    // Vec3f normal;
    double normal_x = 0;
    double normal_y = 0;
    double normal_z = 0;

    HE* he = vertex->out; // get outgoing halfedge from given vertex
    do {
        // accummulate onto our normal vector
        normal_x += he->face->normal.x * he->face->area;
        normal_y += he->face->normal.y * he->face->area;
        normal_z += he->face->normal.z * he->face->area;

        // gives us the halfedge to the next adjacent vertex
        he = he->flip->next;
    } while(he != vertex->out);

    double normal_length = std::sqrt(normal_x * normal_x + normal_y * normal_y + normal_z * normal_z);
    normal_x = normal_x / normal_length;
    normal_y = normal_y / normal_length;
    normal_z = normal_z / normal_length;

    vertex->normal = halfedge::Vec3f{static_cast<float>(normal_x), static_cast<float>(normal_y), static_cast<float>(normal_z)};
}

// Computes the cot(alpha) or cot(beta) for the given halfedge
inline double cot(HE *he) {
    halfedge::HEV *v1 = he->vertex;
    halfedge::HEV *v2 = he->next->vertex;
    halfedge::HEV *v3 = he->next->next->vertex;
    
    double dot_product = (v1->x - v3->x) * (v2->x - v3->x) 
                    + (v1->y - v3->y) * (v2->y - v3->y) 
                    + (v1->z - v3->z) * (v2->z - v3->z);

    return dot_product / static_cast<double>(he->face->area);
}

}
#endif

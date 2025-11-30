#ifndef HALFEDGE_STRUCTS_H
#define HALFEDGE_STRUCTS_H

#include <vector>

namespace halfedge {

struct Vec3f
{
	float x, y, z;
};

struct Vertex
{
    float x, y, z;
};

struct Face
{
    size_t idx1, idx2, idx3;
};

struct Mesh_Data
{
    std::vector<Vertex*> vertices;
    std::vector<Face*> faces;
};

}

#endif

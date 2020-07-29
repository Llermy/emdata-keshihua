#ifndef MARCHING_CUBES_HPP
#define MARCHING_CUBES_HPP

#define MC_SIZE 100.0f

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "ccp4reader.h"
#include "ComputeShaderManager.hpp"

struct PosDuplet
{
    int p1[3];
    int p2[3];
};

class MarchingCuber
{
    ComputeShaderManager csManager;
    VolumeData *data;
    //float position[3] = {0, 0, 0};
    float edgeLength;

public:
    float dataThreshold;
    
    MarchingCuber();
    MarchingCuber(VolumeData *data, float dataThreshold);
    void setup(VolumeData *data, float dataThreshold);

    int polygonize(float **vertices);
    int polygonizeGPU(float **vertices);
    int polygonizeGPU2(float **vertices);

    int voxelToTableIndex(int x, int y, int z);
    int tableIndexToVertices(int index, int x, int y, int z);
    int tableIndexVertNum(int index);
    glm::vec3 interpolateEdge(int edgeIndex, int x, int y, int z);

    
    int tableIndexToVertices(int index, int x, int y, int z, float *vertices);
    void edgesToTriangle(int *voxelInd, int *edges);
    void cubeToTriangles(float *cubeData);
    void voxelToTriangles(unsigned int index);
};

#endif
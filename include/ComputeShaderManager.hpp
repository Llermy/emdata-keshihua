#ifndef COMPUTE_SHADER_MANAGER_HPP_
#define COMPUTE_SHADER_MANAGER_HPP_

#include <glad/glad.h> // 包含glad来获取所有的必须OpenGL头文件

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "ccp4reader.h"

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

#define TEST_SIZE 5

class ComputeShaderManager
{
    // test计算着色器
    const char *testFilename = "shaders/compute/test.comp";
    GLuint testPH;
    int testBuffer[TEST_SIZE] = {0, 0, 1, 2, 0};
    GLuint testSsbo;

    // Marching cubes计算着色器
    const char *mcFilename = "shaders/compute/mc.comp";
    GLuint mcPH;
    // triangle indices SSBO
    GLuint triIndSsbo;
    // volume data SSBO
    GLuint volDataSsbo;
    GLuint mcParamSsbo;
    GLuint verticesSsbo;
    GLuint vertsNumSumSsbo;
    GLuint triTableSsbo;

    /** 
     * 设置进去计算着色器
     * return：GLuint 计算着色器的programme handle
     */
    GLuint setupComputeShader(std::string filename);

public:
    /**
     * 设置进去所有的计算着色器
     */
    void setup();
    void bufferMCData(VolumeData *data);

    void initSSBO(GLuint progHandle, GLuint *ssbo, GLuint index);
    void bufferData(GLuint ssbo, void *data, GLsizeiptr size);

    /****** 计算着色器的自制调用 ******/
    void dispatchTest(int x, int y, int z, int **dataptr);
    //void dispatchMC(float **dataptr, VolumeData *data, float threshold);
    void dispatchMC(float **vertices, VolumeData *data, float threshold, float edgeLength, int *triTableIndices, int *vertsSumIndices);
};

#endif
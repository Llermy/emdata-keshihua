#include <glad/glad.h> // 包含glad来获取所有的必须OpenGL头文件

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

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

    /****** 计算着色器的自制调用 ******/
    void dispatchTest(int x, int y, int z, int **dataptr);
};
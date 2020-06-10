#include "ComputeShaderManager.hpp"

GLuint ComputeShaderManager::setupComputeShader(std::string filename)
{
    std::string csCode;
    std::ifstream csFile;
    try 
    {
        // 打开文件
        csFile.open(filename.c_str());
        std::stringstream csStream;
        // 读取文件的缓冲内容到数据流中
        csStream << csFile.rdbuf();     
        // 关闭文件处理器
        csFile.close();
        // 转换数据流到string
        csCode   = csStream.str();  
    }
    catch(std::ifstream::failure e)
    {
        std::cout << "ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ" << std::endl;
    }
    const char* csSource = csCode.c_str();
    GLuint progHandle = glCreateProgram();
    GLuint cs = glCreateShader(GL_COMPUTE_SHADER);
    glShaderSource(cs, 1, &csSource, NULL);
    glCompileShader(cs);

    int rvalue;
    glGetShaderiv(cs, GL_COMPILE_STATUS, &rvalue);
    if (!rvalue) {
        fprintf(stderr, "Error in compiling the compute shader\n");
        GLchar log[10240];
        GLsizei length;
        glGetShaderInfoLog(cs, 10239, &length, log);
        fprintf(stderr, "Compiler log:\n%s\n", log);
        exit(40);
    }
    glAttachShader(progHandle, cs);

    glLinkProgram(progHandle);
    glGetProgramiv(progHandle, GL_LINK_STATUS, &rvalue);
    if (!rvalue) {
        fprintf(stderr, "Error in linking compute shader program\n");
        GLchar log[10240];
        GLsizei length;
        glGetProgramInfoLog(progHandle, 10239, &length, log);
        fprintf(stderr, "Linker log:\n%s\n", log);
        exit(41);
    }   
    return progHandle;
}

void ComputeShaderManager::setup()
{
    // Test计算着色器
    testPH = this->setupComputeShader(testFilename);
    glUseProgram(testPH);
    glGenBuffers(1, &testSsbo);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, testSsbo);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(testBuffer), testBuffer, GL_DYNAMIC_COPY);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, testSsbo);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}

void ComputeShaderManager::dispatchTest(int x, int y, int z, int **dataptr)
{
    glDispatchCompute(5, 1, 1);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, testSsbo);
    *dataptr = (int *) glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_ONLY);
    glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
}
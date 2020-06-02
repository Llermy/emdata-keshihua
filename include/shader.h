#ifndef SHADER_H
#define SHADER_H

#include <glad/glad.h> // 包含glad来获取所有的必须OpenGL头文件

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

class Shader
{
    public:
    // 程序ID
    unsigned int ID;

    // 构造器读取并构建着色器
    Shader(const GLchar* vertexPath, const GLchar* fragmentPath);
    // 使用/激活程序
    void use();
    // uniform工具函数
    void setBool(const std::string &name, bool value) const;
    void setInt(const std::string &name, int value) const;
    void setFloat(const std::string &name, float value) const;
    void setTexture(const std::string &name, int unit) const;
    void setMat4(const std::string &name, glm::mat4 transform) const;
    void setVec3(const std::string &name, float vx, float vy, float vz) const;
    void setVec3(const std::string &name, glm::vec3 vec3) const;
};

#endif
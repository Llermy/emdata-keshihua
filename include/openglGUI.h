#ifndef OPENGL_GUI_H
#define OPENGL_GUI_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <stdio.h>

#include "shader.h"

#define LEFT_POS 0.8f
#define RIGHT_POS 0.9f
#define VERTEX_NUM 18
#define MOVE_SPEED 0.002f

/**
 * Slider openGL简单的滑动按钮
 */
class Slider
{
    static Shader shader;
    static glm::mat4 orthProjection;

    unsigned int VAO;
    unsigned int VBO;
    unsigned int EBO;
    
    float max;
    float min;
    float curVal;

    float height;
    float vertices[VERTEX_NUM];
    
    unsigned int indices[12] = {
        0, 1, 2, // 滑动按钮的背景的顶点索引
        1, 2, 3,
        0, 1, 4, // 滑动按钮的顶点索引
        1, 4, 5
    };
    glm::vec3 colorSlideBack = glm::vec3(0.3f, 0.3f, 0.5f);
    glm::vec3 colorSlide = glm::vec3(0.9f, 0.9f, 0.9f);

    void updateVertices(float newVal)
    {
        float slidePos = this->valToY(this->curVal);
        vertices[13] = slidePos;
        vertices[16] = slidePos;

        glBindBuffer(GL_ARRAY_BUFFER, this->VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    }

    float valToY(float val)
    {
        if (val > this->max) val = this->max;
        else if (val < this->min) val = this->min;

        return ( (val - min) * height / (max - min) ) - height/2;
    }
    
public:
    Slider(float min = 0, float max = 1, float height = 0.4)
    {
        this->max = max;
        this->min = min;
        this->height = height;
        curVal = (max-min)*0.5 + min; // 默认设置滑动按钮在中间值

        init();
    }

    static void setup()
    {
        Slider::shader.setup("shaders/vertex/shaderGui.vs", "shaders/fragment/shaderGui.fs");
        Slider::orthProjection = glm::ortho(-1.0f, 1.0f, -1.0f, 1.0f, -1.0f, 1.0f);
    }

    void init()
    {
        float initialVertices[] = {
            LEFT_POS, -height/2, 0.0f,
            RIGHT_POS, -height/2, 0.0f,
            LEFT_POS, height/2, 0.0f,
            RIGHT_POS, height/2, 0.0f,
            
            LEFT_POS, height/2, 1.0f,
            RIGHT_POS, height/2, 1.0f
        };
        memcpy(vertices, initialVertices, VERTEX_NUM * sizeof(vertices[0]));
        this->updateVertices(this->curVal);

        glGenVertexArrays(1, &this->VAO);
        glBindVertexArray(this->VAO);
        glGenBuffers(1, &this->VBO);
        glGenBuffers(1, &this->EBO);

        glBindBuffer(GL_ARRAY_BUFFER, this->VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
    }

    // 更新滑动按钮对应的最高最低值
    void updateValues(float min, float max)
    {
        this->min = min;
        this->max = max;
        this->curVal = (max-min)*0.5 + min; // 默认设置滑动按钮在中间值
        this->updateVertices(this->curVal);
    }

    // 处理click事件，并更新按钮。click按钮里面的话会返回true，要不false
    bool click(float x, float y, unsigned int sWidth, unsigned int sHeight)
    {
        // 把屏幕空间的x，y转换-1，1滑动按钮的空间坐标
        float normX = 2*x / (float)sWidth - 1;
        float normY = 1- 2*y / (float)sHeight;
        if( normX >= LEFT_POS && normX <= RIGHT_POS
            && normY <= this->height/2 && normY >= -this->height/2) {
            this->curVal = (normY + height/2) * (max-min) / height + min;
            this->updateVertices(this->curVal);
            return true;
        }
        return false;
    }

    void render()
    {
        glBindVertexArray(this->VAO);
        //glBindBuffer(GL_ARRAY_BUFFER, this->VBO);

        Slider::shader.use();
        Slider::shader.setMat4("projection", orthProjection);

        // 滑动按钮背景
        Slider::shader.setVec3("color", this->colorSlideBack);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        // 滑动按钮
        Slider::shader.setVec3("color", this->colorSlide);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (void*)(6 * sizeof(GLuint)) );
    }

    float read()
    {
        return this->curVal;
    }

    float move(bool moveUp)
    {
        float sign = moveUp ? 1.0f : -1.0f;
        this->curVal += sign*MOVE_SPEED*(this->max-this->min);
        this->updateVertices(this->curVal);
        return this->curVal;
    }
};

Shader Slider::shader;
glm::mat4 Slider::orthProjection;
/*
unsigned int Slider::VAO = 0;
unsigned int Slider::VBO = 0;
unsigned int Slider::EBO = 0;*/

#endif
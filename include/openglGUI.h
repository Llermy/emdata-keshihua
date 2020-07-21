#ifndef OPENGL_GUI_H
#define OPENGL_GUI_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <stdio.h>
#include <map>
#include <iomanip>
#include <sstream>

#include "shader.h"

#define LEFT_POS 0.8f
#define RIGHT_POS 0.9f
#define VERTEX_NUM 18
#define MOVE_SPEED 0.2f

/// Holds all state information relevant to a character as loaded using FreeType
struct Character {
    unsigned int TextureID; // ID handle of the glyph texture
    glm::ivec2   Size;      // Size of glyph
    glm::ivec2   Bearing;   // Offset from baseline to left/top of glyph
    unsigned int Advance;   // Horizontal offset to advance to next glyph
};

class TextRenderer
{
    static std::map<GLchar, Character> characters;
    static unsigned int textVAO;
    static unsigned int textVBO;
    static Shader* textShader;

public:
    static void init(int scrWidth, int scrHeight);
    static void updateScreen(int scrWidth, int scrHeight);
    void renderText(std::string text, float x, float y, float scale, glm::vec3 color);
};

/**
 * Slider openGL简单的滑动按钮
 */
class Slider
{
    static Shader shader;
    static glm::mat4 orthProjection;
    TextRenderer textRenderer;

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

    static void setup();

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

    void render(int screenx, int screeny)
    {
        glBindVertexArray(this->VAO);

        Slider::shader.use();
        Slider::shader.setMat4("projection", Slider::orthProjection);

        // 滑动按钮背景
        Slider::shader.setVec3("color", this->colorSlideBack);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        // 滑动按钮
        Slider::shader.setVec3("color", this->colorSlide);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (void*)(6 * sizeof(GLuint)) );

        std::stringstream stream;
        stream << std::fixed << std::setprecision(3) << this->curVal;
        std::string valStr = stream.str();
        textRenderer.renderText(valStr, screenx - 150.0f, 25.0f, 1.0f, glm::vec3(1.0f, 1.0f, 1.0f));
    }

    float read()
    {
        return this->curVal;
    }

    float move(bool moveUp, float deltaTime)
    {
        float sign = moveUp ? 1.0f : -1.0f;
        this->curVal += sign*MOVE_SPEED*deltaTime*(this->max-this->min);
        if(this->curVal > this->max)
            this->curVal = this->max;
        if(this->curVal < this->min)
            this->curVal = this->min;
        this->updateVertices(this->curVal);
        return this->curVal;
    }
};

#endif
#ifndef CCP4_READER_HPP
#define CCP4_READER_HPP

#include <fstream>
#include <vector>
#include <iostream>
#include <stdio.h>
#include <cstring>

typedef unsigned char BYTE;

// 3D的数据的类
class VolumeData
{
public:
    float maxValue;
    float minValue;
    uint size[3];
    float *data;

    VolumeData() {
        size[0] = 0;
        size[1] = 0;
        size[2] = 0;
    }

    ~VolumeData() {
        if(size[0] != 0) {
            free(data);
        }
    }

    void allocate(uint sizex, uint sizey, uint sizez)
    {
        this->data = new float[sizex*sizey*sizez];
        size[0] = sizex;
        size[1] = sizey;
        size[2] = sizez;
    }

    void newData(uint sizex, uint sizey, uint sizez, float *data) {
        this->data = data;
        size[0] = sizex;
        size[1] = sizey;
        size[2] = sizez;
    }

    void clear() {
        size[0] = 0;
        size[1] = 0;
        size[2] = 0;
        free(this->data);
    }

    float get(uint x, uint y, uint z) {
        return this->data[x + y*size[0] + z*size[0]*size[1]];
    }

    void set(uint x, uint y, uint z, float data) {
        this->data[x + y*size[0] + z*size[0]*size[1]] = data;
    }

    void printSize() {
        std::cout << "数据大小为（" << size[0] << "，" << size[1] << "，" << size[2] << "）\n";
    }

    void fillDummyData() {
        if(size[0] != 0) {
            this->clear();
        }
        size[0] = 2;
        size[1] = 2;
        size[2] = 2;
        uint length = size[0]*size[1]*size[2];
        this->data = new float[length];
        float dt[length] = {1,  1.5, 0.5, 1, 2, 1, 0, 0.5};
        memcpy(this->data, dt, length*sizeof(float));

        this->minValue = 0;
        this->maxValue = 2;
    }
};

class CCP4Reader
{
public:
    // 解析ccp4数据的大小
    // 大小的每3个成分为uint array
    void ccp4size(std::vector<BYTE> &ccp4Data, uint *size)
    {
        size[0] = ccp4Data[2]*65536 + ccp4Data[1]*256 + ccp4Data[0];
        size[1] = ccp4Data[6]*65536 + ccp4Data[5]*256 + ccp4Data[4];
        size[2] = ccp4Data[10]*65536 + ccp4Data[9]*256 + ccp4Data[8];
    }

    VolumeData* read(std::string filename)
    {
        FILE *pFile;
        long fileSize;
        VolumeData *voldata = new VolumeData();

        // 打开文件
        pFile = fopen(filename.c_str(), "rb");
        if (pFile == NULL) {fputs ("File error", stderr); exit (1);}

        // 取文件大小
        fseek(pFile , 0 , SEEK_END);
        fileSize = ftell(pFile);
        rewind (pFile);
        std::cout << "向量大小：" << fileSize << "\n";

        // 解析ccp4数据的大小
        uint *size = (uint*) malloc(sizeof(uint)*3);
        fread((void*)(size), sizeof(size[0]), 3, pFile);

        // 解析数据的最高最低值
        fseek(pFile, 19*4, SEEK_SET);
        fread((void*)(&voldata->minValue), sizeof(voldata->minValue), 1, pFile);
        fread((void*)(&voldata->maxValue), sizeof(voldata->maxValue), 1, pFile);

        // 读ccp4数据
        uint numfloats = (fileSize - 1024) / 4; // 头部为1024byte，每浮点数等于4byte。TODO：有可能头部与数据中间有symmetry records（现在假设没有）
        fseek(pFile, 1024, SEEK_SET);
        float *buffer = (float*) malloc(sizeof(float)*numfloats);
        fread((void*)(buffer), sizeof(buffer[0]), numfloats, pFile); // TODO：看看ccp4文件的二进制数据是低字节序还是高字节序（现在默认处理为低字节序）

        voldata->newData(size[0], size[1], size[2], buffer);

        fclose(pFile);
        free(size);

        return voldata;
    }
};

#endif
//
// Created by Zara Hussain on 2023-04-14.
//

#include "PixelObject.h"

#include <utility>
#include <fstream>


PixelObject::PixelObject(PixDevice* device, std::vector<Vertex> vertices, std::vector<uint32_t> indices): m_device(device), m_vertices(std::move(vertices)), m_indices(std::move(indices)) {
    //create the vertex buffer form the vertices
    printf("PixelObject user constructed\n");
    //createVertexBuffer(vertices);
}

void PixelObject::cleanup() {

    for(auto texture : m_textures)
    {
        texture.cleanUp();
    }

    vkFreeMemory(m_device->logicalDevice, vertexBufferMemory, nullptr);
    vkDestroyBuffer(m_device->logicalDevice, vertexBuffer, nullptr);
    vkFreeMemory(m_device->logicalDevice, indexBufferMemory, nullptr);
    vkDestroyBuffer(m_device->logicalDevice, indexBuffer, nullptr);
}

std::vector<PixelObject::Vertex>* PixelObject::getVertices() {
    return &m_vertices;
}

int PixelObject::getVertexCount() {
    return m_vertices.size();
}

VkDeviceSize PixelObject::getVertexBufferSize() {
    return (sizeof(Vertex) * m_vertices.size());
}

VkBuffer* PixelObject::getVertexBuffer() {
    return &vertexBuffer;
}

VkDeviceMemory *PixelObject::getVertexBufferMemory() {
    return &vertexBufferMemory;
}

std::vector<uint32_t> *PixelObject::getIndices() {
    return &m_indices;
}

int PixelObject::getIndexCount() {
    return m_indices.size();
}

VkDeviceSize PixelObject::getIndexBufferSize() {
    return (sizeof(uint32_t) * m_indices.size());
}

VkBuffer *PixelObject::getIndexBuffer() {
    return &indexBuffer;
}

VkDeviceMemory *PixelObject::getIndexBufferMemory() {
    return &indexBufferMemory;
}

void PixelObject::setDynamicUBObj(DynamicUBObj pushObjData) {
    dynamicUBO = pushObjData;
}

PixelObject::PObj* PixelObject::getPushObj() {
    return &pushObj;
}

PixelObject::PixelObject(PixDevice *device, std::string filename) : m_device(device){
    importFile(filename);
}

void PixelObject::importFile(const std::string& filename) {

    enum Point {POSITION, TEXTURE, NORMAL};

    //default color
    glm::vec4 color = {0.9f,0.9f,0.9f,1.0f};

    std::ifstream myfile("../../../objects/"+ filename + ".obj");
    if(myfile.fail())
    {
        throw std::runtime_error("failed to open the following file: " + filename + ".obj");
    }
    std::string data;

    std::vector<glm::vec4> normals;
    std::vector<glm::vec4> positions;
    std::vector<glm::vec2> texture;
    std::vector<glm::mat3> faces; // face 1 = {0/1/2, 3/4/5, 6/7/8}

    std::vector<int> normInd;

    getline(myfile, data);

    int numslash = 0, numspace = 0, counter = 0;

    while(!data.empty()){
        if(data[0] == 'v' && data[1] == ' '){
            std::string posVal;
            std::vector<float> posList;
            for(int i=2; i<data.size(); i++){
                posVal.push_back(data[i]);
                if(data[i] == ' ' || i==data.size()-1){
                    posVal.pop_back();
                    posList.push_back(std::stof(posVal));
                    posVal.clear();
                }
            }
            positions.push_back(glm::vec4(posList[0],posList[1],posList[2], 1.0f));
        }

        if(data[0] == 'v' && data[1] == 'n'){
            std::string normVal;
            std::vector<float> normList;
            for(int i=3; i<data.size(); i++){
                normVal.push_back(data[i]);
                if(data[i] == ' ' || i==data.size()-1){
                    normVal.pop_back();
                    normList.push_back(std::stof(normVal));
                    normVal.clear();
                }
            }
            normals.push_back(glm::vec4(normList[0],normList[1],normList[2], 1.0f));
        }

        if(data[0] == 'f'){
            int posInd1,posInd2,posInd3;
            int normInd1,normInd2,normInd3;
            std::string num;
            numslash = 0;
            numspace = 0;
            for(int i = 2; i<data.size(); i++){

                num.push_back(data[i]);

                if(data[i] == '/'){
                    numslash++;
                    num.pop_back();
                    if(numslash == 1){ //if this is the first element of the first set ie           (xx/../..   ../../..    ../../..)
                        posInd1 = std::stoi(num);
                    } else if( numslash == 3){ //if this is the first element of the second set ie  (../../..   xx/../..    ../../..)
                        posInd2 = std::stoi(num);
                    } else if( numslash == 5){ //if this is the first element of the third set ie   (../../..   ../../..    xx/../..)
                        posInd3 = std::stoi(num);
                    }

                    num.clear();
                }

                if(data[i] == ' '){
                    numspace++;
                    num.pop_back();
                    if(numspace == 1){ //if this is the last element of the first set ie           (../../xx   ../../..    ../../..)
                        normInd1 = std::stoi(num);
                    } else if( numspace == 2){ //if this is the last element of the second set ie  (../../..   ../../xx    ../../..)
                        normInd2 = std::stoi(num);
                    }
                    num.clear();
                }

                if (i == (data.size()-1)){
                    normInd3 = std::stoi(num); //if this is the last element of the last set   (../../..   ../../..    ../../xx)
                    num.clear();
                }

            }
            num.clear();

            m_indices.push_back(counter);
            m_indices.push_back(counter+1);
            m_indices.push_back(counter+2);

            counter+=3;

            //obj files are 1 indexed. need to bring them to 0 indexed
            posInd1--;posInd2--;posInd3--;
            normInd1--;normInd2--;normInd3--;

            faces.push_back({{posInd1,0,normInd1},
                             {posInd2,0,normInd2},
                             {posInd3,0,normInd3}} );

        }
        getline(myfile, data);
    }

    for(int i = 0; i < faces.size(); i++)
    {
        //face[faceIndx][pointIndx][type ie 0=position 1=texture 2=normal
        int posIndex1 = faces[i][0][POSITION];
        int posIndex2 = faces[i][1][POSITION];
        int posIndex3 = faces[i][2][POSITION];

        int texIndex1 = faces[i][0][TEXTURE];
        int texIndex2 = faces[i][1][TEXTURE];
        int texIndex3 = faces[i][2][TEXTURE];

        int normIndex1 = faces[i][0][NORMAL];
        int normIndex2 = faces[i][1][NORMAL];
        int normIndex3 = faces[i][2][NORMAL];

        m_vertices.push_back({positions[posIndex1], normals[normIndex1], color});
        m_vertices.push_back({positions[posIndex2], normals[normIndex2], color});
        m_vertices.push_back({positions[posIndex3], normals[normIndex3], color});
    }

    myfile.close();
}

void PixelObject::addTransform(glm::mat4 matTransform) {
    pushObj.M = matTransform * pushObj.M;
    pushObj.MinvT = glm::transpose(glm::inverse(pushObj.M));
}

void PixelObject::setTransform(glm::mat4 matTransform) {
    pushObj.M = matTransform;
    pushObj.MinvT = glm::transpose(glm::inverse(pushObj.M));
}

void PixelObject::setPushObj(PixelObject::PObj pushObjData) {
    pushObj = PObj(pushObjData);
}

PixelObject::DynamicUBObj* PixelObject::getDynamicUBObj() {
    return &dynamicUBO;
}

void PixelObject::addTexture(std::string textureFile) {
    PixelImage textureImage = PixelImage(m_device, 0, 0, false);
    textureImage.loadTexture(textureFile);

    m_textures.push_back(textureImage);

}


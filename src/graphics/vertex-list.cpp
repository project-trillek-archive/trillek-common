
#include "graphics/vertex-list.hpp"

namespace trillek {
namespace graphics {

VertexList::VertexList()
    : vao(0) {
    format = VEC3D_NCTW;
    buf[0] = 0;
    buf[1] = 0;
    vertexsize = 0;
}
VertexList::~VertexList() {
}

bool VertexList::SystemStart(const std::list<Property> &) {
    return true;
}
bool VertexList::SystemReset(const std::list<Property> &) {
    return true;
}
bool VertexList::Parse(const std::string &object_name, const rapidjson::Value& node) {
    return true;
}
bool VertexList::Serialize(rapidjson::Document& document) {
    return true;
}
void VertexList::Generate() {
    if(!this->vao) {
        glGenVertexArrays(1, &this->vao);
        glGenBuffers(2, this->buf);
        CheckGLError();
    }
}
void VertexList::Release() {
    if(this->vao) {
        glDeleteVertexArrays(1, &this->vao);
        glDeleteBuffers(2, this->buf);
        this->vao = 0;
    }
}
void VertexList::Bind() {
    glBindVertexArray(this->vao);
    glBindBuffer(GL_ARRAY_BUFFER, this->buf[0]); // Bind the vertex buffer.
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->buf[1]); // Bind the index buffer.
    CheckGLError();
}
void VertexList::Configure() {
    CheckGLError();

    void *offset = nullptr;
    uint32_t attrnum = 0;

    switch(this->format) {
    case VEC3D_NCT:
        vertexsize = sizeof(float) * (3+3+2) + sizeof(uint32_t) * (1);
        // 3 position, 3 normal, 4 color(byte), 2 texcoord
        glVertexAttribPointer(attrnum, 3, GL_FLOAT, GL_FALSE, vertexsize, (GLvoid*)offset);
        glEnableVertexAttribArray(attrnum++);
        offset = ((float*)offset) + 3; // move to the next attribute offset
        glVertexAttribPointer(attrnum, 3, GL_FLOAT, GL_FALSE, vertexsize, (GLvoid*)offset);
        glEnableVertexAttribArray(attrnum++);
        offset = ((float*)offset) + 3;
        glVertexAttribPointer(attrnum, 4, GL_UNSIGNED_BYTE, GL_TRUE, vertexsize, (GLvoid*)offset);
        glEnableVertexAttribArray(attrnum++);
        offset = ((uint32_t*)offset) + 1;
        glVertexAttribPointer(attrnum, 2, GL_FLOAT, GL_FALSE, vertexsize, (GLvoid*)offset);
        glEnableVertexAttribArray(attrnum++);
        offset = ((float*)offset) + 2;
        break;
    case VEC3D_NCTW:
        vertexsize = sizeof(float) * (3+3+2+4) + sizeof(uint32_t) * (1+4);
        // 3 position, 3 normal, 4 color(byte), 2 texcoord, 4 Weight index, 4 Weights
        glVertexAttribPointer(attrnum, 3, GL_FLOAT, GL_FALSE, vertexsize, (GLvoid*)offset);
        glEnableVertexAttribArray(attrnum++);
        offset = ((float*)offset) + 3; // move to the next attribute offset
        glVertexAttribPointer(attrnum, 3, GL_FLOAT, GL_FALSE, vertexsize, (GLvoid*)offset);
        glEnableVertexAttribArray(attrnum++);
        offset = ((float*)offset) + 3;
        glVertexAttribPointer(attrnum, 4, GL_UNSIGNED_BYTE, GL_TRUE, vertexsize, (GLvoid*)offset);
        glEnableVertexAttribArray(attrnum++);
        offset = ((uint32_t*)offset) + 1;
        glVertexAttribPointer(attrnum, 2, GL_FLOAT, GL_FALSE, vertexsize, (GLvoid*)offset);
        glEnableVertexAttribArray(attrnum++);
        offset = ((float*)offset) + 2;
        glVertexAttribPointer(attrnum, 4, GL_UNSIGNED_INT, GL_TRUE, vertexsize, (GLvoid*)offset);
        glEnableVertexAttribArray(attrnum++);
        offset = ((uint32_t*)offset) + 4;
        glVertexAttribPointer(attrnum, 4, GL_FLOAT, GL_FALSE, vertexsize, (GLvoid*)offset);
        glEnableVertexAttribArray(attrnum++);
        offset = ((float*)offset) + 4;
        break;
    case VEC3D_NfCT:
        vertexsize = sizeof(float) * (3+3+4+2);
        // 3 position, 3 normal, 4 color(float), 2 texcoord
        glVertexAttribPointer(attrnum, 3, GL_FLOAT, GL_FALSE, vertexsize, (GLvoid*)offset);
        glEnableVertexAttribArray(attrnum++);
        offset = ((float*)offset) + 3; // move to the next attribute offset
        glVertexAttribPointer(attrnum, 3, GL_FLOAT, GL_FALSE, vertexsize, (GLvoid*)offset);
        glEnableVertexAttribArray(attrnum++);
        offset = ((float*)offset) + 3;
        glVertexAttribPointer(attrnum, 4, GL_FLOAT, GL_FALSE, vertexsize, (GLvoid*)offset);
        glEnableVertexAttribArray(attrnum++);
        offset = ((float*)offset) + 4;
        glVertexAttribPointer(attrnum, 2, GL_FLOAT, GL_FALSE, vertexsize, (GLvoid*)offset);
        glEnableVertexAttribArray(attrnum++);
        offset = ((float*)offset) + 2;
        break;
    case VEC3D_NfCTW:
        vertexsize = sizeof(float) * (3+3+4+2+4) + sizeof(uint32_t) * (4);
        // 3 position, 3 normal, 4 color(float), 2 texcoord, 4 Weight index, 4 Weights
        glVertexAttribPointer(attrnum, 3, GL_FLOAT, GL_FALSE, vertexsize, (GLvoid*)offset);
        glEnableVertexAttribArray(attrnum++);
        offset = ((float*)offset) + 3; // move to the next attribute offset
        glVertexAttribPointer(attrnum, 3, GL_FLOAT, GL_FALSE, vertexsize, (GLvoid*)offset);
        glEnableVertexAttribArray(attrnum++);
        offset = ((float*)offset) + 3;
        glVertexAttribPointer(attrnum, 4, GL_FLOAT, GL_FALSE, vertexsize, (GLvoid*)offset);
        glEnableVertexAttribArray(attrnum++);
        offset = ((float*)offset) + 4;
        glVertexAttribPointer(attrnum, 2, GL_FLOAT, GL_FALSE, vertexsize, (GLvoid*)offset);
        glEnableVertexAttribArray(attrnum++);
        offset = ((float*)offset) + 2;
        glVertexAttribPointer(attrnum, 4, GL_UNSIGNED_INT, GL_TRUE, vertexsize, (GLvoid*)offset);
        glEnableVertexAttribArray(attrnum++);
        offset = ((uint32_t*)offset) + 4;
        glVertexAttribPointer(attrnum, 4, GL_FLOAT, GL_FALSE, vertexsize, (GLvoid*)offset);
        glEnableVertexAttribArray(attrnum++);
        offset = ((float*)offset) + 4;
        break;
    case VEC2D_CT:
        vertexsize = sizeof(float) * (2+2) + sizeof(uint32_t);
        // 2 position, 4 color(byte), 2 texcoord
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, vertexsize, (GLvoid*)offset);
        glEnableVertexAttribArray(0);
        offset = ((float*)offset) + 2; // move to the next attribute offset
        glVertexAttribPointer(1, 4, GL_UNSIGNED_BYTE, GL_TRUE, vertexsize, (GLvoid*)offset);
        glEnableVertexAttribArray(1);
        offset = ((uint32_t*)offset) + 1;
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, vertexsize, (GLvoid*)offset);
        glEnableVertexAttribArray(2);
        break;
    default:
        break;
    }
}
void VertexList::LoadVertexData(void * datablock, uint32_t size, uint32_t count) {
    glBindBuffer(GL_ARRAY_BUFFER, this->buf[0]); // Bind the vertex buffer.
    CheckGLError();
    glBufferData(GL_ARRAY_BUFFER, size * count, datablock, GL_STATIC_DRAW); CheckGLError();
}
void VertexList::LoadIndexData(uint32_t * datablock, uint32_t count) {
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->buf[1]); // Bind the index buffer.
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint32_t) * count, datablock, GL_STATIC_DRAW);
}

} // namespace graphics
} // namespace trillek

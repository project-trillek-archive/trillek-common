#ifndef VERTEX_LIST_HPP_INCLUDED
#define VERTEX_LIST_HPP_INCLUDED

#include "opengl.hpp"
#include "type-id.hpp"
#include "graphics/graphics-base.hpp"
#include <memory>
#include <vector>

namespace trillek {
namespace graphics {

class Shader;
class Texture;
class RenderSystem;

/**
 * VertexList - an abstraction of vertex arrays containing (possibly) multiple meshes.
 */
class VertexList : public GraphicsBase {
    friend class RenderSystem;
public:
    VertexList();
    virtual ~VertexList();

    virtual bool SystemStart(const std::list<Property> &);
    virtual bool SystemReset(const std::list<Property> &);
    virtual bool Parse(const std::string &object_name, const rapidjson::Value& node);
    virtual bool Serialize(rapidjson::Document& document);

    enum VECTOR_FORMAT : uint32_t {
        VEC3D_NCTW,
        VEC3D_NCT,
        VEC3D_NfCTW,
        VEC3D_NfCT,
        VEC2D_CT,
        VEC4D_C,
        VEC4D,
    };
    void SetFormat(VECTOR_FORMAT f) {
        this->format = f;
    }
private:
    void Generate();
    void Release();
    void Bind() const;
    void Configure();
    void LoadVertexData(void *, uint32_t size, uint32_t count);
    void LoadIndexData(uint32_t *, uint32_t count);
    GLuint vao;
    GLuint buf[2];
    VECTOR_FORMAT format;
    uint32_t vertexsize;
    std::shared_ptr<Shader> shader;
};

} // namespace graphics

namespace reflection {
TRILLEK_MAKE_IDTYPE_NS(graphics,VertexList,406)
} // End of reflection

} // namespace trillek

#endif

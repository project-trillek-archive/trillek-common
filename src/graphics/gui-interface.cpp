
#include "systems/graphics.hpp"
#include "trillek-game.hpp"
#include "trillek.hpp"
#include "graphics/texture.hpp"
#include "graphics/vertex-list.hpp"
#include "logging.hpp"

namespace trillek {
namespace graphics {

RenderSystem::GuiRenderInterface::GuiRenderInterface(RenderSystem * parent) {
    this->system = parent;
}
RenderSystem::GuiRenderInterface::~GuiRenderInterface() {

}

void RenderSystem::GuiRenderInterface::RenderGeometry(Rocket::Core::Vertex* vertices,
        int num_vertices, int* indices, int num_indices, Rocket::Core::TextureHandle texture,
        const Rocket::Core::Vector2f& translation) {

}
Rocket::Core::CompiledGeometryHandle RenderSystem::GuiRenderInterface::CompileGeometry(
        Rocket::Core::Vertex* vertices, int num_vertices, int* indices, int num_indices,
        Rocket::Core::TextureHandle texture) {
    LOGMSGFOR(INFO, RenderSystem) << "Compile geometry " << num_vertices << ", " << num_indices << ", " << static_cast<uint32_t>(texture);
    VertexList * vxl = new VertexList();
    vxl->SetFormat(VertexList::VEC2D_CT);
    uint32_t refid, renid;
    vxl->Generate();
    vxl->LoadVertexData(vertices, sizeof(Rocket::Core::Vertex), (uint32_t)num_vertices);
    vxl->Configure();
    vxl->LoadIndexData((uint32_t*)indices, (uint32_t)num_indices);
    glBindVertexArray(0);
    refid = this->system->Add(std::shared_ptr<VertexList>(vxl));
    renid = renderset.size() + 1;
    renderset.push_back(20);
    renderset.push_back(static_cast<uint32_t>(texture));
    renderset.push_back(21);
    renderset.push_back(refid);
    renderset.push_back(10);
    renderset.push_back(num_indices);
    renderset.push_back(5);
    return static_cast<Rocket::Core::CompiledGeometryHandle>(renid);
}
void RenderSystem::GuiRenderInterface::RenderCompiledGeometry(
        Rocket::Core::CompiledGeometryHandle geometry, const Rocket::Core::Vector2f& translation) {
    uint32_t refid, renid, ccode;
    renid = static_cast<uint32_t>(geometry) - 1;
    ccode = renderset[renid];
    while(renid < renderset.size() && ccode != 0 && ccode != 5) {
        switch(ccode) {
        case 20:
            refid = 0;
            if(++renid < renderset.size()) {
                refid = renderset[renid];
                if(refid) {
                    auto tex = system->Get<Texture>(refid);
                    if(tex) {
                        refid = tex->GetID();
                    }
                    else {
                        refid = 0;
                    }
                }
            }
            glBindTexture(GL_TEXTURE_2D, refid);
            break;
        case 21:
            if(++renid < renderset.size()) {
                refid = renderset[renid];
                if(refid) {
                    auto vex = system->Get<VertexList>(refid);
                    if(vex) {
                        vex->Bind();
                    }
                }
            }
            break;
        case 10:
            if(++renid < renderset.size()) {
                refid = renderset[renid];
                glDrawElements(GL_TRIANGLES, refid, GL_UNSIGNED_INT, 0);
            }
            break;
        case 5:
            break;
        }
        ++renid;
        if(renid < renderset.size()) {
            ccode = renderset[renid];
        }
    }
}
void RenderSystem::GuiRenderInterface::ReleaseCompiledGeometry(
        Rocket::Core::CompiledGeometryHandle geometry) {
    LOGMSGFOR(DEBUG, RenderSystem) << "Release-CGeometry";
}
void RenderSystem::GuiRenderInterface::EnableScissorRegion(bool enable) {
    //LOGMSGFOR(DEBUG, RenderSystem) << "E Scissor Region " << (enable ? "true" : "false");
}
void RenderSystem::GuiRenderInterface::SetScissorRegion(int x, int y, int width, int height) {
    LOGMSGFOR(DEBUG, RenderSystem) << "SetScissorRegion " << x << "," << y << "," << width << "," << height;
}
bool RenderSystem::GuiRenderInterface::LoadTexture(Rocket::Core::TextureHandle& texture_handle,
        Rocket::Core::Vector2i& texture_dimensions,const Rocket::Core::String& source) {
    LOGMSGFOR(DEBUG, RenderSystem) << "Load texture " << texture_dimensions.x << ", " << texture_dimensions.y;
    return false;
}
bool RenderSystem::GuiRenderInterface::GenerateTexture(Rocket::Core::TextureHandle& texture_handle,
        const Rocket::Core::byte* source, const Rocket::Core::Vector2i& source_dimensions) {
    LOGMSGFOR(DEBUG, RenderSystem) << "Generate texture " << source_dimensions.x << ", " << source_dimensions.y;
    Texture * gentex = new Texture();
    gentex->Generate(source_dimensions.x, source_dimensions.y, true);
    gentex->Load(source, source_dimensions.x, source_dimensions.y);
    uint32_t refid;
    refid = this->system->Add(std::shared_ptr<Texture>(gentex));
    texture_handle = static_cast<Rocket::Core::TextureHandle>(refid);
    return true;
}
void RenderSystem::GuiRenderInterface::ReleaseTexture(Rocket::Core::TextureHandle texture) {
    uint32_t refid = static_cast<uint32_t>(texture);
    LOGMSGFOR(WARNING, RenderSystem) << "(nonimpl) Release texture " << refid;
}

} // namespace graphics
} // namespace trillek


#include "systems/graphics.hpp"
#include "trillek-game.hpp"
#include "trillek.hpp"
#include "graphics/texture.hpp"
#include "graphics/vertex-list.hpp"
#include "graphics/shader.hpp"
#include "logging.hpp"

namespace trillek {
namespace graphics {

struct GUIVertex {
    float x, y;
    float ts, tt;
    uint8_t c[4];
};
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
    vxl->SetFormat(VertexList::VEC4D_C);
    uint32_t refid, renid;
    std::vector<GUIVertex> guiverts;
    std::vector<uint32_t> guiinds;
    uint32_t baseindex = 0;
    int i, sz;

    baseindex = guiverts.size();
    for(i = 0; i < num_vertices; i++) {
        GUIVertex v;
        v.x = vertices[i].position.x;
        v.y = vertices[i].position.y;
        v.c[0] = vertices[i].colour.red;
        v.c[1] = vertices[i].colour.green;
        v.c[2] = vertices[i].colour.blue;
        v.c[3] = vertices[i].colour.alpha;
        v.ts = vertices[i].tex_coord.x;
        v.tt = vertices[i].tex_coord.y;
        guiverts.push_back(v);
    }
    for(i = 0; i < num_indices; i++) {
        guiinds.push_back(indices[i] + baseindex);
    }
    vxl->Generate();
    vxl->LoadVertexData(guiverts.data(), sizeof(GUIVertex), guiverts.size());
    vxl->Configure();
    vxl->LoadIndexData(guiinds.data(), guiinds.size());
    glBindVertexArray(0);
    refid = this->system->Add(std::shared_ptr<VertexList>(vxl));
    sz = renderset.size();
    renid = 0;
    for(i = 0; i < sz; i++) {
        if(renderset[i] == 0xffffffff) {
            renid = i;
            break;
        }
    }
    if(renid) {
        LOGMSGFOR(INFO, RenderSystem) << "Reusing renderset";
        renderset[renid  ] = 21;
        renderset[renid+1] = refid;
        renderset[renid+2] = 20;
        renderset[renid+3] = static_cast<uint32_t>(texture);
        renderset[renid+4] = 10;
        renderset[renid+5] = guiinds.size();
        renderset[renid+6] = 5;
        renid++;
    }
    else {
        renid = renderset.size() + 1;
        renderset.push_back(21);
        renderset.push_back(refid);
        renderset.push_back(20);
        renderset.push_back(static_cast<uint32_t>(texture));
        renderset.push_back(10);
        renderset.push_back(guiinds.size());
        renderset.push_back(5);
    }
    return static_cast<Rocket::Core::CompiledGeometryHandle>(renid);
}
void RenderSystem::GuiRenderInterface::RenderCompiledGeometry(
        Rocket::Core::CompiledGeometryHandle geometry, const Rocket::Core::Vector2f& translation) {
    uint32_t refid, renid, ccode;
    renid = static_cast<uint32_t>(geometry) - 1;
    glUniform2f(system->guisysshader->Uniform("pxofs"), translation.x, translation.y);
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
            if(!refid) {
                glUniform1i(system->guisysshader->Uniform("on_tex1"), 0);
            }
            else {
                glUniform1i(system->guisysshader->Uniform("on_tex1"), 1);
            }
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, refid);
            glBindSampler(0,0);
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
    uint32_t refid, renid, ccode;
    renid = static_cast<uint32_t>(geometry) - 1;
    ccode = renderset[renid + 1];
    this->system->Remove(ccode);
    renderset[renid  ] = 0xffffffff;
    renderset[renid+1] = 0;
    renderset[renid+2] = 0;
    renderset[renid+3] = 0;
    renderset[renid+4] = 0;
    renderset[renid+5] = 0;
    renderset[renid+6] = 0;
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

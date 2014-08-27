
#include "systems/graphics.hpp"
#include "trillek-game.hpp"
#include "trillek.hpp"
#include "graphics/texture.hpp"
#include "graphics/vertex-list.hpp"
#include "graphics/shader.hpp"
#include "logging.hpp"

namespace trillek {
namespace graphics {

RenderSystem::GuiRenderInterface::GuiRenderInterface(RenderSystem * parent) {
    this->system = parent;
    this->vertlistid = 0;
    this->reload_index = false;
    this->reload_vert = false;
}
RenderSystem::GuiRenderInterface::~GuiRenderInterface() {

}

void RenderSystem::GuiRenderInterface::RenderGeometry(Rocket::Core::Vertex* vertices,
        int num_vertices, int* indices, int num_indices, Rocket::Core::TextureHandle texture,
        const Rocket::Core::Vector2f& translation) {

}
void RenderSystem::GuiRenderInterface::CheckReload() {
    std::shared_ptr<VertexList> vxl;
    if(!vertlistid) {
        vxl.reset(new VertexList());
        vxl->SetFormat(VertexList::VEC4D_C);
        vxl->Generate();
        vxl->LoadVertexData(nullptr, 0, 0);
        vxl->LoadIndexData(nullptr, 0);
        vxl->Configure();
        vertlistid = this->system->Add(vxl);
    }
    else {
        vxl = this->system->Get<VertexList>(vertlistid);
        if(!vxl) {
            return;
        }
        if(reload_vert || reload_index) {
            vxl->Bind();
        }
    }
    if(reload_vert) {
        LOGMSGFOR(INFO, RenderSystem) << "Reloading vert data: " << renderverts.size();
        vxl->LoadVertexData(renderverts.data(), sizeof(GUIVertex), renderverts.size());
    }
    if(reload_index) {
        LOGMSGFOR(INFO, RenderSystem) << "Reloading index data: " << renderindices.size();
        vxl->LoadIndexData(renderindices.data(), renderindices.size());
        LOGMSGFOR(INFO, RenderSystem) << "Reloading done";
    }
    glBindVertexArray(0);
    reload_vert = false;
    reload_index = false;
}
Rocket::Core::CompiledGeometryHandle RenderSystem::GuiRenderInterface::CompileGeometry(
        Rocket::Core::Vertex* vertices, int num_vertices, int* indices, int num_indices,
        Rocket::Core::TextureHandle texture) {
    LOGMSGFOR(INFO, RenderSystem) << "Compile geometry " << num_vertices << ", " << num_indices << ", " << static_cast<uint32_t>(texture);

    uint32_t refid, renid;
    uint32_t basevertex = 0;
    uint32_t baseindex = 0;
    bool addition;
    int i, sz;
    VertexListEntry vle;
    vle.indexcount = 0;
    vle.offset = 0;
    vle.textureref = 0;
    vle.vertexcount = 0;

    if(renderverts.size() < num_vertices) {
        basevertex = renderverts.size();
        baseindex = renderindices.size();
        addition = true;
    }
    else {
        basevertex = 0;
        baseindex = 0;
        addition = true;
        sz = vertlist.size();
        renid = 0;
        for(i = 0; i < sz; i++) {
            auto &vlref = vertlist[i];
            if((vlref.offset == ~0) && (vlref.vertexcount == num_vertices) && (vlref.indexcount == num_indices)) {
                renid = i;
                vle = vlref;
                addition = false;
                break;
            }
            basevertex += vlref.vertexcount;
            baseindex += vlref.indexcount;
        }
        if(addition) {
            basevertex = renderverts.size();
            baseindex = renderindices.size();
        }
    }
    if(addition) {
        for(i = 0; i < num_vertices; i++) {
            GUIVertex v;
            v.x = vertices[i].position.x;
            v.y = vertices[i].position.y;
            v.ts = vertices[i].tex_coord.x;
            v.tt = vertices[i].tex_coord.y;
            v.c[0] = vertices[i].colour.red;
            v.c[1] = vertices[i].colour.green;
            v.c[2] = vertices[i].colour.blue;
            v.c[3] = vertices[i].colour.alpha;
            renderverts.push_back(v);
            vle.vertexcount++;
        }
        for(i = 0; i < num_indices; i++) {
            renderindices.push_back(indices[i] + basevertex);
            vle.indexcount++;
        }
        vle.offset = baseindex;
        vle.textureref = static_cast<uint32_t>(texture);
        renid = vertlist.size() + 1;
        vertlist.push_back(vle);
    }
    else {
        for(i = 0; i < num_vertices; i++) {
            GUIVertex& v = renderverts.at(basevertex + i);
            v.x = vertices[i].position.x;
            v.y = vertices[i].position.y;
            v.ts = vertices[i].tex_coord.x;
            v.tt = vertices[i].tex_coord.y;
            v.c[0] = vertices[i].colour.red;
            v.c[1] = vertices[i].colour.green;
            v.c[2] = vertices[i].colour.blue;
            v.c[3] = vertices[i].colour.alpha;
        }
        for(i = 0; i < num_indices; i++) {
            renderindices[i + baseindex] = indices[i] + basevertex;
        }
        vle.offset = baseindex;
        vle.textureref = static_cast<uint32_t>(texture);
        vertlist[renid] = vle;
        renid++;
    }

    reload_vert = true;
    reload_index = true;

    return static_cast<Rocket::Core::CompiledGeometryHandle>(renid);
}
void RenderSystem::GuiRenderInterface::RenderCompiledGeometry(
        Rocket::Core::CompiledGeometryHandle geometry, const Rocket::Core::Vector2f& translation) {
    uint32_t refid;

    refid = offsets.size();
    offsets.push_back(glm::vec2(translation.x, translation.y));
    RenderEntry re;
    re.mode = 1;
    re.extension = refid;
    re.entryref = static_cast<uint32_t>(geometry);
    gui_renderset.push_back(re);
}
void RenderSystem::GuiRenderInterface::ReleaseCompiledGeometry(
        Rocket::Core::CompiledGeometryHandle geometry) {
    uint32_t refid, renid, ccode;
    renid = static_cast<uint32_t>(geometry) - 1;
    LOGMSGFOR(DEBUG, RenderSystem) << "Release-CGeometry " << vertlist[renid].vertexcount << ", " << vertlist[renid].indexcount;
    vertlist[renid].offset = ~0;
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

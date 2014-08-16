
#include "systems/graphics.hpp"
#include "trillek-game.hpp"
#include "trillek.hpp"
#include "graphics/texture.hpp"
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
    return static_cast<Rocket::Core::CompiledGeometryHandle>(77);
}
void RenderSystem::GuiRenderInterface::RenderCompiledGeometry(
        Rocket::Core::CompiledGeometryHandle geometry, const Rocket::Core::Vector2f& translation) {
    LOGMSGFOR(DEBUG, RenderSystem) << "RenderGUI-CGeometry";
}
void RenderSystem::GuiRenderInterface::ReleaseCompiledGeometry(
        Rocket::Core::CompiledGeometryHandle geometry) {
    LOGMSGFOR(DEBUG, RenderSystem) << "Release-CGeometry";
}
void RenderSystem::GuiRenderInterface::EnableScissorRegion(bool enable) {
    LOGMSGFOR(DEBUG, RenderSystem) << "E Scissor Region " << (enable ? "true" : "false");
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
    uint32_t refid = this->system->current_ref++;
    this->system->Add("GUI_" + std::to_string(refid), std::shared_ptr<Texture>(gentex));
    texture_handle = static_cast<Rocket::Core::TextureHandle>(refid);
    return true;
}
void RenderSystem::GuiRenderInterface::ReleaseTexture(Rocket::Core::TextureHandle texture) {
    uint32_t refid = static_cast<uint32_t>(texture);
    LOGMSGFOR(WARNING, RenderSystem) << "(nonimpl) Release texture " << refid;
}

} // namespace graphics
} // namespace trillek

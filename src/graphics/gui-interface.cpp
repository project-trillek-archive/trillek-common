
#include "systems/graphics.hpp"
#include "trillek-game.hpp"
#include "trillek.hpp"
#include "graphics/texture.hpp"

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
    return Rocket::Core::RenderInterface::CompileGeometry(vertices, num_vertices, indices, num_indices, texture);
}
void RenderSystem::GuiRenderInterface::RenderCompiledGeometry(
        Rocket::Core::CompiledGeometryHandle geometry, const Rocket::Core::Vector2f& translation) {
    Rocket::Core::RenderInterface::RenderCompiledGeometry(geometry, translation);
}
void RenderSystem::GuiRenderInterface::ReleaseCompiledGeometry(
        Rocket::Core::CompiledGeometryHandle geometry) {
    Rocket::Core::RenderInterface::ReleaseCompiledGeometry(geometry);
}
void RenderSystem::GuiRenderInterface::EnableScissorRegion(bool enable) {

}
void RenderSystem::GuiRenderInterface::SetScissorRegion(int x, int y, int width, int height) {

}
bool RenderSystem::GuiRenderInterface::LoadTexture(Rocket::Core::TextureHandle& texture_handle,
        Rocket::Core::Vector2i& texture_dimensions,const Rocket::Core::String& source) {
    return false;
}
bool RenderSystem::GuiRenderInterface::GenerateTexture(Rocket::Core::TextureHandle& texture_handle,
        const Rocket::Core::byte* source, const Rocket::Core::Vector2i& source_dimensions) {
    return false;
}
void RenderSystem::GuiRenderInterface::ReleaseTexture(Rocket::Core::TextureHandle texture) {

}

} // namespace graphics
} // namespace trillek

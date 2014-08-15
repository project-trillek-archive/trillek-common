#include "trillek-game.hpp"
#include "systems/gui.hpp"
#include "systems/graphics.hpp"

namespace trillek {

TrillekScheduler TrillekGame::scheduler;
FakeSystem TrillekGame::fake_system;
OS TrillekGame::glfw_os;
std::once_flag TrillekGame::once_graphics;
std::shared_ptr<graphics::RenderSystem> TrillekGame::gl_sys_ptr;
physics::PhysicsSystem TrillekGame::phys_sys;
MetaEngineSystem TrillekGame::engine_sys;
std::unique_ptr<gui::GuiSystem> TrillekGame::gui_system;
bool TrillekGame::close_window = false;

graphics::RenderSystem& TrillekGame::GetGraphicSystem() {
    return *GetGraphicsInstance().get();
}

gui::GuiSystem& TrillekGame::GetGUISystem() {
    if(!gui_system) {
        gui_system.reset(new gui::GuiSystem(glfw_os, *gl_sys_ptr.get()));
    }
    return *gui_system.get();
}

std::shared_ptr<graphics::RenderSystem> TrillekGame::GetGraphicsInstance() {
    std::call_once(TrillekGame::once_graphics,
        [ ] () {
            TrillekGame::gl_sys_ptr.reset(new graphics::RenderSystem());
            TrillekGame::gl_sys_ptr->RegisterTypes();
    });
    return std::shared_ptr<graphics::RenderSystem>(gl_sys_ptr);
}

} // End of namespace trillek

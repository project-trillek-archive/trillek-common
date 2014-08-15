
#include <Rocket/Core.h>

#include "os.hpp"
#include "trillek-game.hpp"
#include "systems/dispatcher.hpp"
#include "systems/graphics.hpp"
#include "systems/gui.hpp"
#include "logging.hpp"

namespace trillek {
namespace gui {

float GuiSystem::GetElapsedTime() {
    return this->opsystem.GetTime().count() * 1.0E-9f;
}
bool GuiSystem::LogMessage(Rocket::Core::Log::Type type, const Rocket::Core::String& message) {
    using Rocket::Core::Log;
    switch(type) {
    case Log::LT_ALWAYS:
        LOGMSGC(FATAL) << message.CString();
        break;
    case Log::LT_ERROR:
        LOGMSGC(ERROR) << message.CString();
        break;
    case Log::LT_ASSERT:
    case Log::LT_WARNING:
        LOGMSGC(WARNING) << message.CString();
        break;
    case Log::LT_INFO:
        LOGMSGC(INFO) << message.CString();
        break;
    case Log::LT_DEBUG:
        LOGMSGC(DEBUG) << message.CString();
        break;
    case Log::LT_MAX:
        LOGMSGC(DEBUG_FINE) << message.CString();
        break;
    }
    return true;
}

void GuiSystem::Start() {
    Rocket::Core::SetSystemInterface(this);
    Rocket::Core::SetRenderInterface(this->grsystem.GetGUIInterface());
    Rocket::Core::Initialise();

    this->main_context.reset(Rocket::Core::CreateContext("default", Rocket::Core::Vector2i(1024, 768)));
}
} // namespace gui
} // namespace trillek

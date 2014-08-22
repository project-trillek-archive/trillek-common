
#include <Rocket/Core.h>
#include <Rocket/Controls.h>

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
void GuiSystem::Notify(const KeyboardEvent* key_event) {
}
void GuiSystem::Notify(const MouseBtnEvent* mouse_event) {
    if(mouse_event->action == MouseBtnEvent::DOWN) {
        this->main_context->ProcessMouseButtonDown(mouse_event->button, 0);
    }
    else if(mouse_event->action == MouseBtnEvent::UP) {
        this->main_context->ProcessMouseButtonUp(mouse_event->button, 0);
    }
}
void GuiSystem::Notify(const MouseMoveEvent* mouse_event) {
    this->main_context->ProcessMouseMove(mouse_event->new_x, mouse_event->new_y, 0);
}
void GuiSystem::ProcessEvent(Rocket::Core::Event& event) {
    Rocket::Core::Element* te = event.GetTargetElement();
    Rocket::Core::String idstring;
    if(te) {
        idstring = te->GetId();
    }
    LOGMSGC(NOTICE) << idstring.CString() << " Got event " << event.GetType().CString();
}
Rocket::Core::EventListener* GuiSystem::GuiInstancer::InstanceEventListener(
        const Rocket::Core::String& value, Rocket::Core::Element* element) {
    LOGMSGC(NOTICE) << "Register event " << element->GetTagName().CString() << ":" << value.CString();
    GuiEventInterface *ev = new GuiEventInterface(this->gs, ++this->gs.instance_id);
    this->gs.event_listeners.push_back(std::unique_ptr<GuiEventInterface>(ev));
    return ev;
}
void GuiSystem::GuiInstancer::Release() {
    LOGMSGC(NOTICE) << "Unregister event";
}
GuiSystem::GuiEventInterface::GuiEventInterface(GuiSystem &u, uint32_t id) :
    gs(u), instance_id(id) {
}
GuiSystem::GuiEventInterface::~GuiEventInterface() {

}
void GuiSystem::GuiEventInterface::ProcessEvent(Rocket::Core::Event& event) {
    Rocket::Core::Element* te = event.GetTargetElement();
    Rocket::Core::String idstring;
    if(te) {
        idstring = te->GetId();
    }
    LOGMSGC(NOTICE) << idstring.CString() << " Got custom [" << instance_id << "] event " << event.GetType().CString();
}

void GuiSystem::Update() {
    this->main_context->Update();
}
void GuiSystem::InvokeRender() {
    this->main_context->Render();
}
void GuiSystem::LoadDocument(const std::string &docname) {
    Rocket::Core::ElementDocument *elemdoc = this->main_context->LoadDocument(docname.c_str());
    if(elemdoc != nullptr) {
        elemdoc->Show();
        elemdoc->AddEventListener(Rocket::Core::String("click"), this, true);

        this->documents.push_back(unique_doc_ptr(elemdoc));
    } else {
        return; // failed
    }
}
void GuiSystem::LoadFont(const std::string &fname) {
    Rocket::Core::FontDatabase::LoadFontFace(fname.c_str());
}
void GuiSystem::Start() {
    Rocket::Core::SetSystemInterface(this);
    Rocket::Core::SetRenderInterface(this->grsystem.GetGUIInterface());
    Rocket::Core::Initialise();
    Rocket::Controls::Initialise();
    instancer.reset(new GuiInstancer(*this));
    Rocket::Core::Factory::RegisterEventListenerInstancer(instancer.get());

    this->main_context.reset(Rocket::Core::CreateContext(
        "default", Rocket::Core::Vector2i(this->opsystem.GetWindowWidth(), this->opsystem.GetWindowHeight())
    ));
    event::Dispatcher<KeyboardEvent>::GetInstance()->Subscribe(this);
    event::Dispatcher<MouseBtnEvent>::GetInstance()->Subscribe(this);
    event::Dispatcher<MouseMoveEvent>::GetInstance()->Subscribe(this);
}

} // namespace gui
} // namespace trillek

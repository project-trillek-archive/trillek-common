
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

GuiSystem::GuiSystem(OS &sys, graphics::RenderSystem &gsys)
        : opsystem(sys), grsystem(gsys) {
    nextdoc_id = 0;
    csystem_id = 0;
    instance_id = 0;
}
GuiSystem::~GuiSystem() {
    main_context.reset(nullptr);
    Rocket::Core::Shutdown();
}

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

uint32_t GuiSystem::AsyncLoadDocument(const std::string &file) {
    async_action_lock.lock();
    UIControlEvent event;
    uint32_t docid = ++this->nextdoc_id;
    if(!docid) docid = ++this->nextdoc_id;
    event.type = UIControlEvent::DOC_LOAD;
    event.number = docid;
    event.parameter = file;
    async_actions.push_back(event);
    async_action_lock.unlock();
    return docid;
}

void GuiSystem::AsyncCloseDocument(uint32_t id) {
    async_action_lock.lock();
    UIControlEvent event;
    event.type = UIControlEvent::DOC_UNLOAD;
    event.number = id;
    async_actions.push_back(event);
    async_action_lock.unlock();
}

void GuiSystem::HideDocument(uint32_t id) {
    async_action_lock.lock();
    UIControlEvent event;
    event.type = UIControlEvent::DOC_HIDE;
    event.number = id;
    async_actions.push_back(event);
    async_action_lock.unlock();
}

void GuiSystem::ShowDocument(uint32_t id) {
    async_action_lock.lock();
    UIControlEvent event;
    event.type = UIControlEvent::DOC_SHOW;
    event.number = id;
    async_actions.push_back(event);
    async_action_lock.unlock();
}

void GuiSystem::Notify(const KeyboardEvent* key_event) {
    if(this->opsystem.IsMouseLocked()) {
        return;
    }
}

void GuiSystem::Notify(const MouseBtnEvent* mouse_event) {
    if(this->opsystem.IsMouseLocked()) {
        return;
    }
    if(mouse_event->action == MouseBtnEvent::DOWN) {
        this->main_context->ProcessMouseButtonDown(mouse_event->button, 0);
    }
    else if(mouse_event->action == MouseBtnEvent::UP) {
        this->main_context->ProcessMouseButtonUp(mouse_event->button, 0);
    }
}

void GuiSystem::Notify(const MouseMoveEvent* mouse_event) {
    if(this->opsystem.IsMouseLocked()) {
        return;
    }
    this->main_context->ProcessMouseMove(mouse_event->new_x, mouse_event->new_y, 0);
}

void GuiSystem::ProcessEvent(Rocket::Core::Event& event) {
    Rocket::Core::Element* te = event.GetTargetElement();
    Rocket::Core::String idstring;
    if(te) {
        idstring = te->GetId();
    }
    LOGMSGC(DEBUG) << idstring.CString() << " Got event " << event.GetType().CString();
}

void GuiSystem::RegisterHandler(const std::string& event_type, UIEventHandler* handler) {
    uint32_t csid = ++this->csystem_id;
    this->handler_ids[event_type] = csid;
    this->handlers[csid] = handler;
}

Rocket::Core::EventListener* GuiSystem::GuiInstancer::InstanceEventListener(
        const Rocket::Core::String& value, Rocket::Core::Element* element) {
    std::string action(value.CString(), value.Length());
    std::string event_class, event_value;
    size_t cmark = action.find(':');
    if(cmark != std::string::npos) {
        event_class = action.substr(0, cmark);
        event_value = action.substr(cmark + 1, std::string::npos);
        auto handle_itr = this->gs.handler_ids.find(event_class);
        if(handle_itr != this->gs.handler_ids.end()) {
            auto system_itr = this->gs.handlers.find(handle_itr->second);
            uint32_t ev_id = ++this->gs.instance_id;
            GuiEventListener *ev = new GuiEventListener(this->gs, handle_itr->second, ev_id);
            system_itr->second->AddUIEventType(ev_id, event_class, event_value);
            this->gs.event_listeners.push_back(std::unique_ptr<GuiEventListener>(ev));
            LOGMSGC(DEBUG) << "Register: " << event_class << " event: \"" << event_value << "\" on " << element->GetTagName().CString();
            return ev;
        }
        LOGMSGC(NOTICE) << "Register failed: " << event_class << " event: \"" << event_value << "\" on " << element->GetTagName().CString();
    }
    else {
        LOGMSGC(NOTICE) << "Register event failed: \"" << action << "\" on " << element->GetTagName().CString();
    }
    return nullptr;
}

void GuiSystem::GuiInstancer::Release() {
    LOGMSGC(DEBUG_FINE) << "Unregister event instancer";
}

GuiSystem::GuiEventListener::GuiEventListener(GuiSystem &u, uint32_t sid, uint32_t id)
        : gs(u) {
    attachcount = 0;
    system_id = sid;
    instance_id = id;
}

GuiSystem::GuiEventListener::~GuiEventListener() {
    LOGMSGC(DEBUG_FINE) << "~GuiEventListener()";
    auto sys_itr = gs.handlers.find(system_id);
    if(sys_itr != gs.handlers.end()) {
        sys_itr->second->RemoveUIEvent(instance_id);
    }
}

void GuiSystem::GuiEventListener::ProcessEvent(Rocket::Core::Event& event) {
    Rocket::Core::Element* te = event.GetTargetElement();
    Rocket::Core::String idstring;
    if(te) {
        idstring = te->GetId();
    }
    auto sys_itr = gs.handlers.find(system_id);
    if(sys_itr != gs.handlers.end()) {
        sys_itr->second->UINotify(instance_id, std::string(idstring.CString(), idstring.Length()));
    }
}

void GuiSystem::GuiEventListener::OnAttach(Rocket::Core::Element* elem) {
    LOGMSGC(DEBUG_FINE) << "GuiEventListener-Attach " << ((uint32_t)elem);
    attachcount++;
}

void GuiSystem::GuiEventListener::OnDetach(Rocket::Core::Element* elem) {
    LOGMSGC(DEBUG_FINE) << "GuiEventListener-Detach " << ((uint32_t)elem);
    attachcount--;
}

void GuiSystem::Update() {
    async_action_lock.lock();
    auto action_itr = async_actions.begin();
    if(action_itr != async_actions.end()) {
        while(action_itr != async_actions.end()) {
            switch(action_itr->type) {
            case UIControlEvent::DOC_LOAD:
                LoadDocument(action_itr->parameter, action_itr->number);
                break;
            case UIControlEvent::DOC_UNLOAD:
                CloseDocument(action_itr->number);
                break;
            case UIControlEvent::FONT_LOAD:
                LoadFont(action_itr->parameter);
                break;
            case UIControlEvent::DOC_HIDE:
                {
                    auto doc_itr = documents.find(action_itr->number);
                    if(doc_itr != documents.end()) {
                        doc_itr->second->Hide();
                    }
                }
                break;
            case UIControlEvent::DOC_SHOW:
                {
                    auto doc_itr = documents.find(action_itr->number);
                    if(doc_itr != documents.end()) {
                        doc_itr->second->Show();
                    }
                }
                break;
            default:
                break;
            }
            action_itr++;
        }
        async_actions.clear();
    }
    async_action_lock.unlock();
    this->main_context->Update();
}

void GuiSystem::InvokeRender() {
    this->main_context->Render();
}

void GuiSystem::CleanUpObjects() {
    auto evlist_itr = event_listeners.begin();
    while(evlist_itr != event_listeners.end()) {
        if((*evlist_itr)->GetAttachCount() < 1) {
            event_listeners.remove(*evlist_itr);
            evlist_itr = event_listeners.begin();
        }
        else {
            evlist_itr++;
        }
    }
    this->grsystem.GetGUIInterface()->RequestClear();
}

uint32_t GuiSystem::LoadDocument(const std::string &docname) {
    uint32_t docid = ++this->nextdoc_id;
    if(!docid) docid = ++this->nextdoc_id;
    Rocket::Core::ElementDocument *elemdoc = this->main_context->LoadDocument(docname.c_str());
    if(elemdoc != nullptr) {
        elemdoc->AddEventListener(Rocket::Core::String("unload"), this, true);
        LOGMSGC(DEBUG) << "Loaded Document " << docid;
        this->documents[docid] = unique_doc_ptr(elemdoc);
        return docid;
    } else {
        return 0; // failed
    }
}

void GuiSystem::LoadDocument(const std::string &docname, uint32_t doc_id) {
    Rocket::Core::ElementDocument *elemdoc = this->main_context->LoadDocument(docname.c_str());
    if(elemdoc != nullptr) {
        elemdoc->AddEventListener(Rocket::Core::String("unload"), this, true);
        LOGMSGC(DEBUG) << "Loaded Document " << doc_id;
        this->documents[doc_id] = unique_doc_ptr(elemdoc);
    } else {
        return; // failed
    }
}

void GuiSystem::CloseDocument(uint32_t id) {
    auto doc_itr = documents.find(id);
    if(doc_itr != documents.end()) {
        LOGMSGC(DEBUG) << "Unload document " << id;
        doc_itr->second->GetContext()->UnloadDocument(doc_itr->second.get());
        documents.erase(doc_itr);
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
    docinstancer.reset(new GuiDocumentInstancer(*this));
    instancer.reset(new GuiInstancer(*this));
    Rocket::Core::Factory::RegisterElementInstancer("body", docinstancer.get());
    Rocket::Core::Factory::RegisterEventListenerInstancer(instancer.get());
    docinstancer->RemoveReference();
    instancer->RemoveReference();

    this->main_context.reset(Rocket::Core::CreateContext(
        "default", Rocket::Core::Vector2i(this->opsystem.GetWindowWidth(), this->opsystem.GetWindowHeight())
    ));

    RegisterTypes();
    event::Dispatcher<KeyboardEvent>::GetInstance()->Subscribe(this);
    event::Dispatcher<MouseBtnEvent>::GetInstance()->Subscribe(this);
    event::Dispatcher<MouseMoveEvent>::GetInstance()->Subscribe(this);
}

GuiSystem::GuiDocumentInstancer::GuiDocumentInstancer(GuiSystem &u) : gs(u) {}
GuiSystem::GuiDocumentInstancer::~GuiDocumentInstancer() {}
Rocket::Core::Element* GuiSystem::GuiDocumentInstancer::InstanceElement(
    Rocket::Core::Element* parent,
    const Rocket::Core::String& tag,
    const Rocket::Core::XMLAttributes& attributes) {
    LOGMSGC(DEBUG_FINE) << "DocInstancer - New Element " << tag.CString();
    Rocket::Core::ElementDocument *eldoc;
    eldoc = new Rocket::Core::ElementDocument(tag);
    eldoc->AddEventListener(Rocket::Core::String("load"), &this->gs, true);
    return eldoc;
}

void GuiSystem::GuiDocumentInstancer::ReleaseElement(Rocket::Core::Element* element) {
    delete element;
    LOGMSGC(DEBUG_FINE) << "DocInstancer - Release Element " << ((uint32_t)element);
    gs.CleanUpObjects();
}

void GuiSystem::GuiDocumentInstancer::Release() {
    LOGMSGC(DEBUG_FINE) << "DocInstancer - Release System";
}

} // namespace gui
} // namespace trillek

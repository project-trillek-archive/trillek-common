#ifndef GUI_HPP_INCLUDED
#define GUI_HPP_INCLUDED

#include <memory>
#include <string>
#include <map>
#include <vector>
#include <mutex>
#include <Rocket/Core/SystemInterface.h>
#include <Rocket/Core/FileInterface.h>
#include <Rocket/Core/EventListener.h>
#include <Rocket/Core/EventListenerInstancer.h>
#include <Rocket/Core/Context.h>
#include <Rocket/Core/ElementDocument.h>
#include <Rocket/Core/ElementInstancer.h>
#include <Rocket/Core/Dictionary.h>

#include "os-event.hpp"
#include "systems/ui-event.hpp"

namespace trillek {

class OS;
namespace graphics {
class RenderSystem;
}
namespace gui {

/**
 * Deleter for reference counted Rocket object pointers
 */
class ReferenceDeleter {
public:
    template<class T>
    void operator()(T* dd) {
        dd->RemoveReference();
    }
};
/**
 * Pointer type for Rocket Documents
 */
typedef std::unique_ptr<Rocket::Core::ElementDocument, ReferenceDeleter> unique_doc_ptr;

/**
 * GuiSystem is an extention of Graphics System, not a true trillek system
 */
class GuiSystem : public Rocket::Core::SystemInterface,
    public Rocket::Core::EventListener,
    public event::Subscriber<KeyboardEvent>,
    public event::Subscriber<MouseBtnEvent>,
    public event::Subscriber<MouseMoveEvent> {
    friend class graphics::RenderSystem;
public:
    GuiSystem(OS &sys, graphics::RenderSystem &gsys);
    virtual ~GuiSystem();

    virtual float GetElapsedTime();
    virtual bool LogMessage(Rocket::Core::Log::Type type, const Rocket::Core::String& message);

    virtual void ProcessEvent(Rocket::Core::Event& event);

    void Start();
    uint32_t LoadDocument(const std::string &);
    void LoadDocument(const std::string &, uint32_t doc_id);
    uint32_t AsyncLoadDocument(const std::string &);
    void LoadFont(const std::string &);
    void CloseDocument(uint32_t id);
    void AsyncCloseDocument(uint32_t id);
    void HideDocument(uint32_t id);
    void ShowDocument(uint32_t id);

    void RegisterHandler(const std::string& event_type, UIEventHandler* handler);

    void Notify(const KeyboardEvent*);
    void Notify(const MouseBtnEvent*);
    void Notify(const MouseMoveEvent*);

    class GuiInstancer : public Rocket::Core::EventListenerInstancer {
    public:
        GuiInstancer(GuiSystem &u) : gs(u) {}
        virtual ~GuiInstancer() {}
        virtual Rocket::Core::EventListener* InstanceEventListener(const Rocket::Core::String& value, Rocket::Core::Element* element);
        virtual void Release();
    private:
        GuiSystem &gs;
    };
    class GuiEventListener : public Rocket::Core::EventListener {
    public:
        GuiEventListener(GuiSystem &u, uint32_t sid, uint32_t id);
        virtual ~GuiEventListener();
        virtual void ProcessEvent(Rocket::Core::Event& event);
        virtual void OnAttach(Rocket::Core::Element*);
        virtual void OnDetach(Rocket::Core::Element*);
        int32_t GetAttachCount() { return attachcount; }
    private:
        int32_t attachcount;
        uint32_t instance_id;
        uint32_t system_id;
        GuiSystem &gs;
    };

private:
    class GuiDocumentInstancer : public Rocket::Core::ElementInstancer {
    public:
        GuiDocumentInstancer(GuiSystem &u);
        virtual ~GuiDocumentInstancer();
        virtual Rocket::Core::Element* InstanceElement(
            Rocket::Core::Element* parent,
            const Rocket::Core::String& tag,
            const Rocket::Core::XMLAttributes& attributes);
        virtual void ReleaseElement(Rocket::Core::Element* element);
        virtual void Release();
    private:
        GuiSystem &gs;
    };

private:
    void Update();
    void InvokeRender();
    void RegisterTypes();
    void CleanUpObjects();

    std::unique_ptr<GuiInstancer> instancer;
    std::unique_ptr<GuiDocumentInstancer> docinstancer;
    std::mutex async_action_lock;
    std::list<UIControlEvent> async_actions;
    uint32_t nextdoc_id;
    uint32_t instance_id;
    uint32_t csystem_id;
    std::list<std::unique_ptr<GuiEventListener>> event_listeners;
    OS &opsystem;
    graphics::RenderSystem &grsystem;
    std::map<std::string, uint32_t> handler_ids;
    std::map<uint32_t, UIEventHandler*> handlers;
    std::unique_ptr<Rocket::Core::Context, ReferenceDeleter> main_context;
    std::map<uint32_t, unique_doc_ptr> documents;
};

} // namespace gui
} // namespace trillek

#endif

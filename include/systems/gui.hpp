#ifndef GUI_HPP_INCLUDED
#define GUI_HPP_INCLUDED

#include <memory>
#include <vector>
#include <Rocket/Core/SystemInterface.h>
#include <Rocket/Core/FileInterface.h>
#include <Rocket/Core/Context.h>
#include <Rocket/Core/ElementDocument.h>

#include "os-event.hpp"

namespace trillek {

class OS;
namespace graphics {
class RenderSystem;
}
namespace gui {

class DocumentUnloader {
public:
    template<class T>
    void operator()(T* dd) {
        dd->GetContext()->UnloadDocument(dd);
    }
};
class ReferenceDeleter {
public:
    template<class T>
    void operator()(T* dd) {
        dd->RemoveReference();
    }
};
typedef std::unique_ptr<Rocket::Core::ElementDocument, DocumentUnloader> unique_doc_ptr;
class GuiSystem : public Rocket::Core::SystemInterface,
    public event::Subscriber<KeyboardEvent>,
    public event::Subscriber<MouseBtnEvent>,
    public event::Subscriber<MouseMoveEvent> {
    friend class graphics::RenderSystem;
public:
    GuiSystem(OS &sys, graphics::RenderSystem &gsys) :
        opsystem(sys), grsystem(gsys) {}
    ~GuiSystem() {}

    virtual float GetElapsedTime();
    virtual bool LogMessage(Rocket::Core::Log::Type type, const Rocket::Core::String& message);

    void Start();
    void LoadDocument(const std::string &);
    void LoadFont(const std::string &);

    void Notify(const KeyboardEvent*);
    void Notify(const MouseBtnEvent*);
    void Notify(const MouseMoveEvent*);
private:
    void Update();
    void InvokeRender();

    OS &opsystem;
    graphics::RenderSystem &grsystem;
    std::unique_ptr<Rocket::Core::Context, ReferenceDeleter> main_context;
    std::vector<unique_doc_ptr> documents;
};

} // namespace gui
} // namespace trillek

#endif

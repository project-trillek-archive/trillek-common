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
class GuiSystem : public Rocket::Core::SystemInterface {
public:
    GuiSystem(OS &sys, graphics::RenderSystem &gsys) :
        opsystem(sys), grsystem(gsys) {}
    ~GuiSystem() {}

    virtual float GetElapsedTime();
    virtual bool LogMessage(Rocket::Core::Log::Type type, const Rocket::Core::String& message);

    void Start();

private:
    OS &opsystem;
    graphics::RenderSystem &grsystem;
    std::unique_ptr<Rocket::Core::Context, ReferenceDeleter> main_context;
    std::vector<std::unique_ptr<Rocket::Core::ElementDocument, DocumentUnloader>> documents;
};

} // namespace gui
} // namespace trillek

#endif

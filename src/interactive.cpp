
#include "interactive.hpp"
#include "event-queue.hpp"

namespace trillek {

bool Interaction::Initialize(const std::vector<Property> &properties) {
    return true;
}

void Interaction::TriggerAction(size_t num) const {
    if(num < triggers.size()) {
        event::QueueEvent(InteractEvent(triggers.at(num)));
    }
}

std::string Interaction::GetActionText(size_t num) const {
    if(num < triggers.size()) {
        return "[TEST]";
    }
    return "[INVALID]";
}

size_t Interaction::AddAction(Action act) {
    size_t r = triggers.size();
    triggers.push_back(act);
    return r;
}

} // namespace trillek

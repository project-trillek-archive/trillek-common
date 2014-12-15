
#include "interaction.hpp"
#include "property.hpp"
#include "event-queue.hpp"

namespace trillek {

static const std::string empty_str("[ERR TEXT]");
int action_text_locale = 0;

std::map<Action, std::map<int, std::string>> ActionText::txtmap;
std::map<std::string, Action> ActionText::actmap;

void ActionText::RegisterStatic() {
    Register(Action::IA_FUNCTION, 0, "script");
    Register(Action::IA_USE, 0, "use");
    Register(Action::IA_MOVE, 0, "move");
    Register(Action::IA_POWER, 0, "power");
}

void ActionText::Register(Action act, int locale, const std::string& txt) {
    if(locale == 0) {
        actmap[txt] = act;
    }
    txtmap[act][locale] = txt;
}

const std::string& ActionText::Get(Action act, int locale) {
    auto actitr = txtmap.find(act);
    if(actitr == txtmap.end()) {
        return empty_str;
    }
    else {
        auto lcitr = actitr->second.find(locale);
        if(lcitr == actitr->second.end() && locale != 0) {
            lcitr = actitr->second.find(0);
        }
        if(lcitr == actitr->second.end()) {
            return empty_str;
        }
        else {
            return lcitr->second;
        }
    }
}

const std::string& ActionText::Get(Action act) {
    return Get(act, action_text_locale);
}

Action ActionText::GetAction(const std::string& txt) {
    auto actitr = actmap.find(txt);
    if(actitr == actmap.end()) {
        return Action::IA_INVALID;
    }
    else {
        return actitr->second;
    }
}

bool ActionText::Exists(Action act, int locale) {
    auto actitr = txtmap.find(act);
    if(actitr == txtmap.end()) {
        return false;
    }
    else {
        auto lcitr = actitr->second.find(locale);
        return !(lcitr == actitr->second.end());
    }
}

bool ActionText::Exists(const std::string& txt) {
    auto actitr = actmap.find(txt);
    return !(actitr == actmap.end());
}

bool Interaction::Initialize(const std::vector<Property> &properties) {
    for(auto& p : properties) {
        if(p.GetName() == "entity_id") {
            entity_id = p.Get<uint32_t>();
        }
        else if(p.GetName() == "actions") {
            auto actarray = p.Get<std::vector<std::string>>();
            for(auto& a : actarray) {
                Action actnum = ActionText::GetAction(a);
                if(actnum != Action::IA_INVALID) {
                    AddAction(actnum);
                }
            }
        }
    }
    return true;
}

void Interaction::TriggerAction(size_t num) const {
    if(num < triggers.size() && triggers.at(num).act != Action::IA_INVALID) {
        event::QueueEvent(triggers.at(num));
    }
}

std::string Interaction::GetActionText(size_t num) const {
    if(num < triggers.size()) {
        return ActionText::Get(triggers.at(num).act, 0);
    }
    return "[INVALID]";
}

InteractEvent Interaction::GetAction(size_t num) const {
    if(num < triggers.size()) {
        return triggers.at(num);
    }
    return InteractEvent(Action::IA_INVALID, 0, 0);
}

uint32_t Interaction::AddAction(Action act) {
    uint32_t r = triggers.size();
    triggers.push_back(InteractEvent(act, entity_id, r));
    return r;
}

void Interaction::AddAction(Action act, uint32_t sn) {
    triggers.push_back(InteractEvent(act, entity_id, sn));
}
void Interaction::AddAction(Action act, uint32_t sn, id_t entity) {
    triggers.push_back(InteractEvent(act, entity, sn));
}

} // namespace trillek

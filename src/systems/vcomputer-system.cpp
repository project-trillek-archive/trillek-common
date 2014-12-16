#include "systems/vcomputer-system.hpp"

#include "trillek-game.hpp"
#include "components/component.hpp"
#include "components/system-component.hpp"
#include "hardware/cpu.hpp"
#include "trillek.hpp"
#include "logging.hpp"

#include "vcomputer.hpp"
#include "tr3200/tr3200.hpp"
#include "auxiliar.hpp"
#include "devices/gkeyb.hpp"
#include "devices/tda.hpp"

namespace trillek {

using namespace component;

VComputerSystem::VComputerSystem() : system(game.GetSystemComponent()) {
    event::Dispatcher<KeyboardEvent>::GetInstance()->Subscribe(this);
    event::EventQueue<HardwareAction>::Subscribe(this);
    event::EventQueue<InteractEvent>::Subscribe(this);
};

VComputerSystem::~VComputerSystem() { }

void VComputerSystem::HandleEvents(frame_tp timepoint) {
    static frame_tp last_tp;
    this->delta = frame_unit(timepoint - last_tp);
    last_tp = timepoint;
    auto count = this->delta.count() * 0.000000001;
    event::EventQueue<HardwareAction>::ProcessEvents(this);
    event::EventQueue<InteractEvent>::ProcessEvents(this);
    OnTrue(Bitmap<Component::VComputer>(),
        [&](id_t entity_id) {
            auto& vcom = Get<Component::VComputer>(entity_id);
            vcom.vc->Update(count);
        });
    OnTrue(Bitmap<Component::VDisplay>(),
        [&](id_t entity_id) {
            auto& disp = system.Get<Component::VDisplay>(entity_id);
            disp.ScreenUpdate();
        });
}

void VComputerSystem::OnEvent(const HardwareAction& event) {
    if(!Has<Component::VDisplay>(event.entity_id)) {
        return;
    }
    auto& disp = system.Get<Component::VDisplay>(event.entity_id);
    disp.LinkDevice();
}

void VComputerSystem::OnEvent(const InteractEvent& event) {
    LOGMSG(INFO) << "VCom: got interact event " << event.entity << ", " << ActionText::Get(event.act) << ", " << event.num;
    switch(event.act) {
    case Action::IA_POWER:
        if(event.num == (uint32_t)Component::VDisplay && Has<Component::VDisplay>(event.entity)) {
            game.GetSystemComponent().Get<Component::VDisplay>(event.entity).PowerToggle();
            return;
        }
        else if(event.num == (uint32_t)Component::VComputer && Has<Component::VComputer>(event.entity)) {
            game.GetSystemComponent().Get<Component::VComputer>(event.entity).PowerToggle();
            return;
        }
        break;
    }
}

void VComputerSystem::Notify(const KeyboardEvent* key_event) {
    switch (key_event->action) {
    case KeyboardEvent::KEY_DOWN:
        //this->gkeyb->SendKeyEvent(key_event->scancode, key_event->key, computer::gkeyboard::KEY_MODS::KEY_MOD_NONE);
        break;
    default:
        break;
    }
}

} // namespace trillek

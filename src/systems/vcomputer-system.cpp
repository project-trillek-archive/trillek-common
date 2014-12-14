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
};

VComputerSystem::~VComputerSystem() { }

void VComputerSystem::HandleEvents(frame_tp timepoint) {
    static frame_tp last_tp;
    this->delta = frame_unit(timepoint - last_tp);
    last_tp = timepoint;
    auto count = this->delta.count() * 0.000000001;
    event::ProcessEvents<HardwareAction>();
    OnTrue(Bitmap<Component::VComputer>(),
        [&](id_t entity_id) {
            auto& vcom = Get<Component::VComputer>(entity_id);
            vcom.vc->Update(count);
        });
    OnTrue(Bitmap<Component::VDisplay>(),
        [&](id_t entity_id) {
            auto& disp = Get<Component::VDisplay>(entity_id);
            computer::tda::TDAScreen screen;
            std::static_pointer_cast<computer::tda::TDADev>(disp.device)->DumpScreen(screen);
            computer::tda::TDAtoRGBATexture(screen, (DWord*)disp.surface->LockWrite());
            disp.surface->UnlockWrite();
            disp.surface->Invalidate();
        });
}

void VComputerSystem::OnEvent(const HardwareAction& event) {
    if(!Has<Component::VDisplay>(event.entity_id)) {
        return;
    }
    auto& disp = system.Get<Component::VDisplay>(event.entity_id);
    disp.LinkDevice();
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

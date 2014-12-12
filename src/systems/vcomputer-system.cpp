#include "systems/vcomputer-system.hpp"

#include "vcomputer.hpp"
#include "tr3200/tr3200.hpp"
#include "auxiliar.hpp"
#include "trillek.hpp"
#include "logging.hpp"

namespace trillek {

VComputerSystem::VComputerSystem() {
    this->gkeyb = std::make_shared<computer::gkeyboard::GKeyboardDev>();

    AddComputer(9000, TR3200);
    if (LoadROMFile(9000, "meisys.ffi")) {
        auto pixa = resource::ResourceMap::Get<resource::PixelBuffer>("1005_common/assets/vidstand/Screen.png");
        pixa->Create(320, 240, 8, resource::ImageColorMode::COLOR_RGBA);
        pixa->meta.push_back(Property("wrap", std::string("clamp")));
        pixa->meta.push_back(Property("mip", true));
        pixa->meta.push_back(Property("mag-filter", std::string("linear")));
        pixa->meta.push_back(Property("min-filter", std::string("linear-mip-linear")));
        this->pixelBuffers[9000].first = pixa;
        this->pixelBuffers[9000].second = std::make_shared<computer::tda::TDADev>();
        SetDevice(9000, 5, this->pixelBuffers[9000].second);
        SetDevice(9000, 1, gkeyb);
        ComputerPowerOn(9000);
    }
    else {
        RemoveComputer(9000);
    }

    event::Dispatcher<KeyboardEvent>::GetInstance()->Subscribe(this);
};

VComputerSystem::~VComputerSystem() { }

void VComputerSystem::AddComputer(const id_t entity_id, CPU_TYPE type) {
    std::unique_ptr<computer::VComputer> vc(new computer::VComputer());
    std::unique_ptr<computer::ICPU> cpu;
    switch(type) {
    case TR3200:
        cpu.reset(new computer::TR3200());
        break;
    default:
        LOGMSGC(ERROR) << "Unsupported CPU Type";
    }
    if(cpu) {
        vc->SetCPU(std::move(cpu));
        this->computers[entity_id].vc = std::move(vc);
    }
}

void VComputerSystem::SetDevice(const id_t entity_id, const unsigned int slot, std::shared_ptr<computer::IDevice> device) {
    if (this->computers.find(entity_id) != this->computers.end()) {
        this->computers[entity_id].vc->AddDevice(slot, device);
        this->computers[entity_id].devices.push_back(device);
    }
}

void VComputerSystem::RemoveComputer(const id_t entity_id) {
    if (this->computers.find(entity_id) != this->computers.end()) {
        this->computers[entity_id].vc->Off();
        this->computers.erase(entity_id);
    }
}

void VComputerSystem::RemoveDevice(const id_t entity_id, const unsigned int slot) {
    if (this->computers.find(entity_id) != this->computers.end()) {
        auto dev = this->computers[entity_id].vc->GetDevice(slot);
        for (auto itr = this->computers[entity_id].devices.begin(); itr != this->computers[entity_id].devices.end(); ++itr) {
            if ((*itr) == dev) {
                this->computers[entity_id].devices.erase(itr);
                if (this->pixelBuffers.find(entity_id) != this->pixelBuffers.end()) {
                    if ((*itr) == this->pixelBuffers[entity_id].second) {
                        this->pixelBuffers.erase(entity_id);
                    }
                }
                this->computers[entity_id].devices.erase(itr);
                break;
            }
        }
        this->computers[entity_id].vc->RmDevice(slot);
    }
}

void VComputerSystem::HandleEvents(frame_tp timepoint) {
    static frame_tp last_tp;
    this->delta = frame_unit(timepoint - last_tp);
    last_tp = timepoint;
    auto count = this->delta.count() * 0.000000001;
    for (auto& comp : this->computers) {
        comp.second.vc->Update(count);
    }
    for (auto& pbuffer : this->pixelBuffers) {
        computer::tda::TDAScreen screen;
        pbuffer.second.second->DumpScreen(screen);
        computer::tda::TDAtoRGBATexture(screen, (DWord*)pbuffer.second.first->LockWrite());
        pbuffer.second.first->UnlockWrite();
        pbuffer.second.first->Invalidate();
    }
}


bool VComputerSystem::LoadROMFile(const id_t entity_id, std::string fname) {
    if (this->computers.find(entity_id) != this->computers.end()) {
        auto& vc = this->computers[entity_id].vc;
        int size = computer::LoadROM(fname, this->computers[entity_id].rom.get());
        if (size < 0) {
            LOGMSGC(ERROR) << "failed reading the ROM file " << fname;
            return false;
        }
        this->computers[entity_id].rom_size = size;
        vc->SetROM(this->computers[entity_id].rom.get(), this->computers[entity_id].rom_size);
        return true;
    }
    return false;
}

void VComputerSystem::ComputerPowerOn(const id_t entity_id) {
    auto vci = this->computers.find(entity_id);
    if(vci != this->computers.end()) {
        vci->second.vc->On();
    }
}

void VComputerSystem::ComputerPowerOff(const id_t entity_id) {
    auto vci = this->computers.find(entity_id);
    if(vci != this->computers.end()) {
        vci->second.vc->Off();
    }
}

void VComputerSystem::ComputerReset(const id_t entity_id) {
    auto vci = this->computers.find(entity_id);
    if(vci != this->computers.end()) {
        vci->second.vc->Reset();
    }
}

void VComputerSystem::AddComponent(const id_t entity_id, std::shared_ptr<ComponentBase> component) { }

void VComputerSystem::Notify(const KeyboardEvent* key_event) {
    switch (key_event->action) {
    case KeyboardEvent::KEY_DOWN:
        this->gkeyb->SendKeyEvent(key_event->scancode, key_event->key, computer::gkeyboard::KEY_MODS::KEY_MOD_NONE);
    default:
        break;
    }
}

} // namespace trillek


#include "hardware/cpu.hpp"
#include "trillek-game.hpp"
#include "components/component.hpp"
#include "components/system-component.hpp"
#include "systems/resource-system.hpp"
#include "systems/vcomputer-system.hpp"
#include "event-queue.hpp"
#include "interaction.hpp"
#include "logging.hpp"

#include "vcomputer.hpp"
#include "tr3200/tr3200.hpp"
#include "auxiliar.hpp"
#include "devices/gkeyb.hpp"
#include "devices/tda.hpp"

namespace trillek {
namespace hw {

bool Computer::Initialize(const std::vector<Property> &properties) {
    using namespace component;
    id_t entity_id = 0;
    std::string cputype;
    std::string romfile;
    uint32_t cpuclock = 100000;
    std::size_t memory = 128*1024;
    for (const Property& p : properties) {
        std::string name = p.GetName();
        if(name == "entity_id") {
            entity_id = p.Get<uint32_t>();
        }
        else if(name == "cpu") {
            cputype = p.Get<std::string>();
        }
        else if(name == "rate") {
            cpuclock = p.Get<uint32_t>();
        }
        else if(name == "ram" || name == "memory") {
            memory = p.Get<uint32_t>() * 1024;
        }
        else if(name == "rom") {
            romfile = p.Get<std::string>();
        }
    }
    std::unique_ptr<computer::ICPU> cpu;
    if(cputype == "TR3200") {
        cpu.reset(new computer::TR3200(cpuclock));
    }
    else {
        LOGMSGC(ERROR) << "Unsupported CPU Type";
        return false;
    }
    this->vc.reset(new computer::VComputer(memory));
    if(cpu) {
        vc->SetCPU(std::move(cpu));
    }
    if(!LoadROMFile(romfile)) {
        return false;
    }

    if(!Has<Component::Interactable>(entity_id)) {
        game.GetSystemComponent().Insert<Component::Interactable>(entity_id, Interaction(entity_id));
    }
    auto& act = game.GetSystemComponent().Get<Component::Interactable>(entity_id);
    act.AddAction(Action::IA_POWER, (uint32_t)Component::VComputer);

    return true;
}

bool Computer::LoadROMFile(const std::string& fname) {
    int size = computer::LoadROM(fname, rom.get());
    if (size < 0) {
        LOGMSGC(ERROR) << "failed reading the ROM file " << fname;
        return false;
    }
    rom_size = size;
    vc->SetROM(rom.get(), rom_size);
    return true;
}

void Computer::SetDevice(uint32_t slot, std::shared_ptr<computer::IDevice> device) {
    this->vc->AddDevice(slot, device);
    this->devices.push_back(device);
}

void Computer::RemoveDevice(uint32_t slot) {
    auto dev = this->vc->GetDevice(slot);
    for (auto itr = this->devices.begin(); itr != this->devices.end(); ++itr) {
        if ((*itr) == dev) {
            this->devices.erase(itr);
            break;
        }
    }
    this->vc->RmDevice(slot);
}

void Computer::PowerOn() {
    if(!vc) return;
    vc->On();
}

void Computer::PowerOff() {
    if(!vc) return;
    vc->Off();
}

void Computer::ComputerReset() {
    if(!vc) return;
    vc->Reset();
}

bool VHardware::LinkDevice() {
    using namespace component;
    if(entity_link > 0 && Has<component::Component::VComputer>(entity_link)) {
        auto& vcr = game.GetSystemComponent().Get<Component::VComputer>(entity_link);
        vcr.SetDevice(this->slot, this->device);
        this->linked = true;
        LOGMSG(INFO) << "Hardware: Successfully linked " << entity_id;
        return true;
    }
    else {
        this->linked = false;
        return false;
    }
}

void VHardware::QueueLinkDevice() {
    event::QueueEvent(HardwareAction(HardwareAction::ATTACH, entity_id));
}

bool VDisplay::Initialize(const std::vector<Property> &properties) {
    using namespace component;
    uint32_t instid = 0;
    entity_link = 0;
    slot = 2;

    std::string modelname("TDA");
    std::string mfgname;
    for (const Property& p : properties) {
        std::string name = p.GetName();
        if(name == "entity_id") {
            entity_id = p.Get<uint32_t>();
        }
        else if(name == "instance_id") {
            instid = p.Get<uint32_t>();
        }
        else if(name == "link_id") {
            entity_link = p.Get<uint32_t>();
        }
        else if(name == "slot") {
            slot = p.Get<uint32_t>();
        }
        else if(name == "model") {
            modelname = p.Get<std::string>();
        }
        else if(name == "make") {
            mfgname = p.Get<std::string>();
        }
    }
    if(modelname == "TDA") {
        device.reset(new computer::tda::TDADev());
    }
    else {
        LOGMSG(ERROR) << "VDisplay: Invalid model name";
        return false;
    }
    std::stringstream resname;
    resname << entity_id << "%" << instid;
    auto pixa = resource::ResourceMap::Get<resource::PixelBuffer>(resname.str());
    if(!pixa) {
        return false;
    }
    pixa->Create(320, 240, 8, resource::ImageColorMode::COLOR_RGBA);
    pixa->meta.push_back(Property("wrap", std::string("clamp")));
    pixa->meta.push_back(Property("mip", true));
    pixa->meta.push_back(Property("mag-filter", std::string("linear")));
    pixa->meta.push_back(Property("min-filter", std::string("linear-mip-linear")));
    this->surface = pixa;

    QueueLinkDevice();

    if(!Has<Component::Interactable>(entity_id)) {
        game.GetSystemComponent().Insert<Component::Interactable>(entity_id, Interaction(entity_id));
    }
    auto& act = game.GetSystemComponent().Get<Component::Interactable>(entity_id);
    act.AddAction(Action::IA_POWER, (uint32_t)Component::VDisplay);

    return true;
}

void VKeyboard::Notify(const unsigned int entity_id, const KeyboardEvent* data) {

}

bool VKeyboard::Initialize(const std::vector<Property> &properties) {
    using namespace component;
    slot = 1;

    std::string modelname("generic");
    std::string mfgname;
    for (const Property& p : properties) {
        std::string name = p.GetName();
        if(name == "entity_id") {
            entity_id = p.Get<uint32_t>();
        }
        else if(name == "link_id") {
            entity_link = p.Get<uint32_t>();
        }
        else if(name == "slot") {
            slot = p.Get<uint32_t>();
        }
        else if(name == "model") {
            modelname = p.Get<std::string>();
        }
        else if(name == "make") {
            mfgname = p.Get<std::string>();
        }
    }

    if(modelname == "generic") {
        device.reset(new computer::gkeyboard::GKeyboardDev());
        keybapi = GENERIC;
    }
    else {
        LOGMSG(ERROR) << "VKeyboard: Invalid model name";
        return false;
    }

    QueueLinkDevice();

    if(!Has<Component::Interactable>(entity_id)) {
        game.GetSystemComponent().Insert<Component::Interactable>(entity_id, Interaction(entity_id));
    }
    auto& act = game.GetSystemComponent().Get<Component::Interactable>(entity_id);
    act.AddAction(Action::IA_USE);

    return true;
}

} // namespace hw
} // namespace trillek


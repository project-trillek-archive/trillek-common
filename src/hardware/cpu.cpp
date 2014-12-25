
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

void Computer::DoPowerOn() {
    if(!vc) return;
    vc->On();
}

void Computer::DoPowerOff() {
    if(!vc) return;
    vc->Off();
}

void Computer::ComputerReset() {
    if(!vc) return;
    vc->Reset();
}

bool VHardware::LinkDevice() {
    using namespace component;
    if(this->linked) return true;
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

void VHardware::QueueLinkDevice(component::Component c) {
    event::QueueEvent(HardwareAction(HardwareAction::ATTACH, c, entity_id));
}

class VDisplayAPI_TDA final : public VDisplayAPI {
public:
    VDisplayAPI_TDA() : framecount(0) {}
    ~VDisplayAPI_TDA() {}

    void ResetState() override {
        this->framecount = 0;
        std::memset(&screen, 0, sizeof(computer::tda::TDAScreen));
    }

    void ScreenUpdate(computer::IDevice* dev, resource::PixelBuffer* surface) override {
        ((computer::tda::TDADev*)dev)->DumpScreen(screen);
        computer::tda::TDAtoRGBATexture(screen, (DWord*)surface->LockWrite(), this->framecount);
        surface->UnlockWrite();
        surface->Invalidate();
    }

    unsigned framecount;
    computer::tda::TDAScreen screen;
};

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
        devapi.reset(new VDisplayAPI_TDA());
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

    devapi->ResetState();
    QueueLinkDevice(Component::VDisplay);

    if(!Has<Component::Interactable>(entity_id)) {
        game.GetSystemComponent().Insert<Component::Interactable>(entity_id, Interaction(entity_id));
    }
    auto& act = game.GetSystemComponent().Get<Component::Interactable>(entity_id);
    act.AddAction(Action::IA_POWER, (uint32_t)Component::VDisplay);

    return true;
}

void VDisplay::ScreenUpdate() {
    using namespace component;

    switch(mode) {
    case hw::VDisplay::DISP_OFF:
        break;
    case hw::VDisplay::DISP_ON:
        if( (!Has<Component::VComputer>(entity_link))
            || (!Get<Component::VComputer>(entity_link).IsPowered())
            || (!devapi)) {
            mode = hw::VDisplay::DISP_ON_CLEAR;
        }
        devapi->ScreenUpdate(device.get(), surface.get());
        break;
    case hw::VDisplay::DISP_ON_CLEAR:
        if( Has<Component::VComputer>(entity_link)
            && Get<Component::VComputer>(entity_link).IsPowered() ) {
            mode = hw::VDisplay::DISP_ON;
        }
        else {
            uint32_t* data = (uint32_t*)surface->LockWrite();
            uint32_t clr = 0xFFFF0000;
            uint32_t sz = surface->Width() * surface->Height();
            for(uint32_t i = 0; i < sz; i++) {
                *(data++) = clr;
            }
            surface->UnlockWrite();
            surface->Invalidate();
            if(devapi) {
                devapi->ResetState();
            }
            mode = hw::VDisplay::DISP_ON_NOVID;
        }
        break;
    case hw::VDisplay::DISP_ON_NOVID:
        if( Has<Component::VComputer>(entity_link)
            && Get<Component::VComputer>(entity_link).IsPowered() ) {
            mode = hw::VDisplay::DISP_ON;
        }
        break;
    case hw::VDisplay::DISP_TO_OFF:
        {
            uint32_t* data = (uint32_t*)surface->LockWrite();
            uint32_t clr = 0x00000000;
            uint32_t sz = surface->Width() * surface->Height();
            for(uint32_t i = 0; i < sz; i++) {
                *(data++) = clr;
            }
            surface->UnlockWrite();
            surface->Invalidate();
        }
        break;
    }
}

static const uint8_t keytranslation_generic[] = {
    0x1b, 0xd, 0x9, 0x8, 0x10, 0x5, 0x15, 0x14, 0x13, 0x12, /* 256-265 */
    0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, /* 266-290 */
    0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, /* 291-305 */
    0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, /* 306-330 */
    0,0,0,0,0, 0,0,0,0, /* 331-339 */
    0xE, 0xF, 0x6, 0xE, 0xF, 0x6 /* 340-346 */
};
void VKeyGeneric::TranslateKeyDown(computer::IDevice* keyb, int scan, int mods) {
    lastscan = scan;
    if(scan > 255 && scan <= 346) {
        uint8_t k = keytranslation_generic[scan - 256];
        if(k) {
            ((computer::gkeyboard::GKeyboardDev*)keyb)
                ->EnforceSendKeyEvent((uint16_t)scan, k, mods & 0x7);
        }
    }
}
void VKeyGeneric::TranslateKeyUp(computer::IDevice* keyb, int scan, int mods) {

}
void VKeyGeneric::TranslateChar(computer::IDevice* keyb, int keycode, int mods) {
    ((computer::gkeyboard::GKeyboardDev*)keyb)
        ->EnforceSendKeyEvent((uint16_t)lastscan, (uint8_t)keycode, mods & 0x7);
}

void VKeyboard::Notify(const KeyboardEvent* data) {
    switch (data->action) {
    case KeyboardEvent::KEY_DOWN:
        keybapi->TranslateKeyDown(this->device.get(), data->key, data->mods);
        break;
    case KeyboardEvent::KEY_CHAR:
        keybapi->TranslateChar(this->device.get(), data->key, data->mods);
        break;
    case KeyboardEvent::KEY_UP:
        keybapi->TranslateKeyUp(this->device.get(), data->key, data->mods);
        break;
    default:
        break;
    }
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
        keybapi.reset(new VKeyGeneric());
    }
    else {
        LOGMSG(ERROR) << "VKeyboard: Invalid model name";
        return false;
    }

    QueueLinkDevice(Component::VKeyboard);

    if(!Has<Component::Interactable>(entity_id)) {
        game.GetSystemComponent().Insert<Component::Interactable>(entity_id, Interaction(entity_id));
    }
    auto& act = game.GetSystemComponent().Get<Component::Interactable>(entity_id);
    act.AddAction(Action::IA_USE, (uint32_t)Component::VKeyboard);

    return true;
}
void VKeyboard::SetActive(bool sactive) {
    if(sactive && !active) {
        active = true;
        event::Dispatcher<KeyboardEvent>::GetInstance()->Subscribe(this);
    }
    else if(!sactive && active) {
        active = false;
        event::Dispatcher<KeyboardEvent>::GetInstance()->Unsubscribe(this);
    }
}

} // namespace hw
} // namespace trillek


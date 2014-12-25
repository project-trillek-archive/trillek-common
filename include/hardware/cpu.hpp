#ifndef CPU_HPP_INCLUDED
#define CPU_HPP_INCLUDED

#include <memory>
#include "components/component.hpp"
#include "systems/dispatcher.hpp"
#include "resources/pixel-buffer.hpp"
#include "interaction.hpp"
#include "os-event.hpp"
#include "power.hpp"

#include "vcomputer.hpp"

namespace trillek {
namespace hw {

/**
 * Class for VComputer components
 */
class Computer final : public PoweredDevice, public ComponentBase {
public:
    Computer() : rom(new Byte[32 * 1024]), rom_size(0) { }
    Computer(Computer&& v) : rom_size(v.rom_size), rom(std::move(v.rom)),
        devices(std::move(v.devices)), vc(std::move(v.vc)) { }
    Computer(const Computer&) = delete;
    ~Computer() {
        if (this->vc) {
            this->vc->Off();
        }
    }

    bool Initialize(const std::vector<Property> &properties) override;

    bool LoadROMFile(const std::string& fname);
    void SetDevice(uint32_t slot, std::shared_ptr<computer::IDevice> device);
    void RemoveDevice(uint32_t slot);

    /** Turns the computer on.
     */
    void DoPowerOn() override;

    /** Turns the computer off.
     */
    void DoPowerOff() override;

    /** Reset the computer.
     */
    void ComputerReset();

    std::unique_ptr<Byte[]> rom;
    size_t rom_size;
    std::unique_ptr<computer::VComputer> vc;
    std::list<std::shared_ptr<computer::IDevice>> devices;
};

class VHardware {
public:
    VHardware() : linked(false), slot(0),
        entity_id(0), entity_link(0) { }
    virtual ~VHardware() {}
    VHardware(VHardware&& v) :
        device(std::move(v.device)),
        entity_id(v.entity_id),
        entity_link(v.entity_link),
        slot(v.slot),
        linked(v.linked) { }

    std::shared_ptr<computer::IDevice> device;

    bool LinkDevice();
    void QueueLinkDevice(component::Component c);
protected:
    id_t entity_id;
    id_t entity_link;
    uint32_t slot;
    bool linked;
};

/**
 * Class for abstracted display client backends
 */
class VDisplayAPI {
public:
    VDisplayAPI() {}
    virtual ~VDisplayAPI() {}
    virtual void ResetState() = 0;
    virtual void ScreenUpdate(computer::IDevice*, resource::PixelBuffer*) = 0;
};

/**
 * Class for computer displays
 */
class VDisplay final : public VHardware,
        public PoweredDevice, public ComponentBase {
public:
    VDisplay() : sid(~0), mode(DISP_OFF) { }
    VDisplay(VDisplay&& v) : VHardware::VHardware(std::forward<VHardware>(v)),
        surface(std::move(v.surface)), sid(v.sid), mode(v.mode) { }
    VDisplay(const VDisplay&) = delete;
    ~VDisplay() {
    }

    bool Initialize(const std::vector<Property> &properties) override;

    void ScreenUpdate();

    std::shared_ptr<resource::PixelBuffer> surface;
    std::unique_ptr<VDisplayAPI> devapi;
    uint32_t sid;
protected:
    /** Turns the display on.
     */
    void DoPowerOn() override { mode = DISP_ON_CLEAR; }
    /** Turns the display off.
     */
    void DoPowerOff() override { mode = DISP_TO_OFF; }
    enum DisplayState : uint32_t {
        DISP_OFF,
        DISP_ON,
        DISP_ON_CLEAR,
        DISP_ON_NOVID,
        DISP_TO_OFF
    };
    DisplayState mode;
};

class VKeyAPI {
public:
    VKeyAPI() {}
    virtual ~VKeyAPI() {}
    virtual void TranslateKeyDown(computer::IDevice* keyb, int scan, int mods) = 0;
    virtual void TranslateKeyUp(computer::IDevice* keyb, int scan, int mods) = 0;
    virtual void TranslateChar(computer::IDevice* keyb, int keycode, int mods) = 0;
};

class VKeyGeneric final : public VKeyAPI {
public:
    VKeyGeneric() { lastscan = 0; }
    ~VKeyGeneric() {}
    void TranslateKeyDown(computer::IDevice* keyb, int scan, int mods);
    void TranslateKeyUp(computer::IDevice* keyb, int scan, int mods);
    void TranslateChar(computer::IDevice* keyb, int keycode, int mods);

    int lastscan;
};

/**
 * Class for keyboards
 */
class VKeyboard final : public VHardware, public ComponentBase,
        public event::Subscriber<KeyboardEvent> {
public:
    VKeyboard() : keybapi(), VHardware::VHardware(), active(false) { }
    VKeyboard(VKeyboard&& v) : keybapi(std::move(v.keybapi)),
            VHardware::VHardware(std::forward<VHardware>(v)) {
        this->component_type_id = v.component_type_id; // ComponentBase
        active = v.active;
    }
    VKeyboard(const VKeyboard&) = delete;
    ~VKeyboard() {
    }

    bool Initialize(const std::vector<Property> &properties) override;
    void SetActive(bool active);
    bool IsActive() const { return active; }

    void Notify(const KeyboardEvent* data) override;

private:
    std::unique_ptr<VKeyAPI> keybapi;
    bool active;
};

} // namespace hw
} // namespace trillek

#endif

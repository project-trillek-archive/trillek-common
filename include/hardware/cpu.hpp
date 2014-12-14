#ifndef CPU_HPP_INCLUDED
#define CPU_HPP_INCLUDED

#include <memory>
#include "components/component.hpp"
#include "resources/pixel-buffer.hpp"
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
    void PowerOn() override;

    /** Turns the computer off.
     */
    void PowerOff() override;

    /** Reset the computer.
     */
    void ComputerReset();

    std::unique_ptr<Byte[]> rom;
    size_t rom_size;
    std::unique_ptr<computer::VComputer> vc;
    std::list<std::shared_ptr<computer::IDevice>> devices;
};

/**
 * Class for computer displays
 */
class VDisplay final : public PoweredDevice, public ComponentBase {
public:
    VDisplay() { }
    VDisplay(VDisplay&& v) { }
    VDisplay(const VDisplay&) = delete;
    ~VDisplay() {

    }

    /** Turns the display on.
     */
    void PowerOn() override {}
    /** Turns the display off.
     */
    void PowerOff() override {}

    bool Initialize(const std::vector<Property> &properties) override;

    std::shared_ptr<resource::PixelBuffer> surface;
    std::shared_ptr<computer::IDevice> device;
private:
    id_t entity_link;
    uint32_t slot;
    bool linked;
};

/**
 * Class for keyboards
 */
class VKeyboard final : public ComponentBase {
public:
    VKeyboard() : keybapi(GENERIC), entity_link(0), linked(false) { }
    VKeyboard(VKeyboard&& v) : keyb(std::move(v.keyb)) {
        this->component_type_id = v.component_type_id; // ComponentBase
        keybapi = v.keybapi;
        entity_link = v.entity_link;
        slot = v.slot;
        linked = v.linked;
    }
    VKeyboard(const VKeyboard&) = delete;
    ~VKeyboard() {
    }

    bool Initialize(const std::vector<Property> &properties) override;

    enum VKeyAPI : uint32_t {
        GENERIC
    };
    std::shared_ptr<computer::IDevice> keyb;
private:
    VKeyAPI keybapi;
    id_t entity_link;
    uint32_t slot;
    bool linked;
};

} // namespace hw
} // namespace trillek

#endif

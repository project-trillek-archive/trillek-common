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
    void QueueLinkDevice();
protected:
    id_t entity_id;
    id_t entity_link;
    uint32_t slot;
    bool linked;
};

/**
 * Class for computer displays
 */
class VDisplay final : public VHardware, public PoweredDevice, public ComponentBase {
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

};

/**
 * Class for keyboards
 */
class VKeyboard final : public VHardware, public ComponentBase {
public:
    VKeyboard() : keybapi(GENERIC), VHardware::VHardware() { }
    VKeyboard(VKeyboard&& v) : VHardware::VHardware(std::forward<VHardware>(v)) {
        this->component_type_id = v.component_type_id; // ComponentBase
        keybapi = v.keybapi;
    }
    VKeyboard(const VKeyboard&) = delete;
    ~VKeyboard() {
    }

    bool Initialize(const std::vector<Property> &properties) override;

    enum VKeyAPI : uint32_t {
        GENERIC
    };
private:
    VKeyAPI keybapi;
};

} // namespace hw
} // namespace trillek

#endif

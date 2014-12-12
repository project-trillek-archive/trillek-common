#ifndef CPU_HPP_INCLUDED
#define CPU_HPP_INCLUDED

#include <memory>
#include "vcomputer.hpp"
#include "power.hpp"

namespace trillek {
namespace hw {

/**
 * Class for VComputer objects
 */
class Computer final : public PoweredDevice {
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
    std::unique_ptr<Byte[]> rom;
    size_t rom_size;
    std::unique_ptr<computer::VComputer> vc;
    std::list<std::shared_ptr<computer::IDevice>> devices;
};

} // namespace hw


} // namespace trillek

#endif

#ifndef POWER_HPP_INCLUDED
#define POWER_HPP_INCLUDED

namespace trillek {
namespace hw {

/**
 * Interface class for devices that connect to power
 * A Device can both draw power and provide power.
 */
class PoweredDevice {
public:
    PoweredDevice() {}
    virtual ~PoweredDevice() {}

private:
};

} // namespace hw
} // namespace trillek

#endif

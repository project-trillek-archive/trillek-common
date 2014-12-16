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
    PoweredDevice() : power(POWER_OFF) {}
    virtual ~PoweredDevice() {}

    void PowerToggle() {
        if(IsOn()) {
            PowerOff();
        }
        else {
            PowerOn();
        }
    }
    void PowerOn() {
        if(power == POWER_OFF) {
            power = POWER_ON;
            DoPowerOn();
        }
    }
    void PowerOff() {
        if(power != POWER_OFF) {
            power = POWER_OFF;
            DoPowerOff();
        }
    }

    enum PowerState {
        POWER_OFF,
        POWER_ON,
        POWER_ON_FAIL,
    };
    bool IsOn() const { return power != POWER_OFF; }
    bool IsPowered() const { return power == POWER_ON; }
protected:
    virtual void DoPowerOn() = 0;
    virtual void DoPowerOff() = 0;

private:
    PowerState power;
};

} // namespace hw
} // namespace trillek

#endif

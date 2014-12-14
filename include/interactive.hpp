#ifndef INTERACTIVE_HPP_INCLUDED
#define INTERACTIVE_HPP_INCLUDED

#include <vector>
#include "trillek.hpp"
#include "components/component.hpp"

namespace trillek {

enum class Action : uint32_t {
    IA_USE,
    IA_POWER_ON,
    IA_POWER_OFF,
};

struct InteractEvent {
    InteractEvent(Action a) : act(a) {}
    Action act;
};

class Interaction final : public ComponentBase {
public:
    Interaction() {}
    ~Interaction() {}

    bool Initialize(const std::vector<Property> &properties);

    void TriggerAction(size_t num) const;
    std::string GetActionText(size_t num) const;

    size_t AddAction(Action);
private:
    std::vector<Action> triggers;
};

} // namespace trillek

#endif

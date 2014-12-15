#ifndef INTERACTIVE_HPP_INCLUDED
#define INTERACTIVE_HPP_INCLUDED

#include <vector>
#include "trillek.hpp"
#include "components/component.hpp"

namespace trillek {

enum class Action : uint32_t {
    IA_INVALID = 0,
    IA_FUNCTION = 1,
    IA_USE,
    IA_MOVE,
    IA_POWER,
};

struct ActionText {
    static std::map<Action, std::map<int, std::string>> txtmap;
    static std::map<std::string, Action> actmap;
    static void Register(Action, int locale, const std::string& txt);
    static const std::string& Get(Action, int locale);
    static const std::string& Get(Action);
    static Action GetAction(const std::string&);
    static bool Exists(Action, int locale);
    static bool Exists(const std::string&);

    static void RegisterStatic();
};

struct InteractEvent {
    InteractEvent(Action a, id_t e, uint32_t n) :
        act(a), entity(e), num(n) {}
    Action act;
    id_t entity;
    uint32_t num;
};

class Interaction final : public ComponentBase {
public:
    Interaction() : entity_id(0) {}
    Interaction(id_t entity) : entity_id(entity) {}
    ~Interaction() {}

    bool Initialize(const std::vector<Property> &properties);

    void TriggerAction(size_t num) const;
    std::string GetActionText(size_t num) const;
    InteractEvent GetAction(size_t num) const;

    uint32_t AddAction(Action);
    void AddAction(Action, uint32_t sn);
    void AddAction(Action, uint32_t sn, id_t entity);
    size_t ActionCount() const { return triggers.size(); }

private:
    id_t entity_id;
    std::vector<InteractEvent> triggers;
};

} // namespace trillek

#endif

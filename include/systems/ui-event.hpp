#ifndef UI_EVENT_HPP_INCLUDED
#define UI_EVENT_HPP_INCLUDED

#include "trillek.hpp"
#include "type-id.hpp"

#include <string>

namespace trillek {

struct UIControlEvent {
    enum UICONTROL : uint32_t { DOC_LOAD, DOC_UNLOAD, FONT_LOAD };
    UICONTROL type;
    uint32_t number;
    std::string parameter;
};

class UIEventHandler {
public:
    virtual void AddUIEventType(uint32_t event_id, const std::string& event_class, const std::string& event_value) = 0;
    virtual void UINotify(uint32_t event_id, const std::string& element_id) = 0;
    virtual void RemoveUIEvent(uint32_t event_id) = 0;
};

} // namespace trillek

#endif

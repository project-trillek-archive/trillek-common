#ifndef COMPONENT_ENUM_HPP_INCLUDED
#define COMPONENT_ENUM_HPP_INCLUDED

#define TRILLEK_MAKE_COMPONENT(enumerator,name,type,container) \
    namespace component {\
    template<> struct type_trait<Component::enumerator> { typedef type value_type; };\
    typedef type enumerator##_type;\
    \
    template<> struct container_type_trait<Component::enumerator> { typedef container container_type; };\
    }\
    \
    namespace reflection {\
    template <>\
    inline const char* GetTypeName<std::integral_constant<component::Component,component::Component::enumerator>>()\
                { return name; };\
    template <>\
    inline unsigned int GetTypeID<std::integral_constant<component::Component,component::Component::enumerator>>()\
                { return static_cast<uint32_t>(component::Component::enumerator); };\
    }

namespace trillek {

class Property;
class Transform;
class Interaction;

namespace physics {
class Collidable;
}

namespace hw {
class Computer;
class VDisplay;
class VKeyboard;
}

namespace component {

class Container;
class System;
class Shared;
class SystemValue;

enum class Component : uint32_t {
    Velocity = 1,               // instant displacement
    VelocityMax,                // maximum instant displacement
    ReferenceFrame,             // id of the parent entity for the transform
    IsReferenceFrame,           // true if this entity has at least a child
    CombinedVelocity,           // ?
    Collidable,                 // wrapper object for body
    OxygenRate,                 // Oxygen rate associated with an entity (room...)
    Health,                     // Health level
    Immune,                     // true = Immune
    Movable,                    // true = Movable, Moving can be changed by players
    Moving,                     // true = Actively moving
    MoveOffset,                 // transform representing the offset to another object
    GraphicTransform,           // transform used to display the entity
    GameTransform,              // last confirmed transform
    Interactable,               // Has interactions
    VComputer,                  // Virtual computer and CPU
    VKeyboard,                  // Keyboard for virtual computers
    VDisplay,                   // Display for a virtual computer
};

template<Component C> struct type_trait;
template<Component C> struct container_type_trait;

} // namespace component

TRILLEK_MAKE_COMPONENT(Collidable,"collidable",trillek::physics::Collidable,System)
TRILLEK_MAKE_COMPONENT(Velocity,"velocity",trillek::physics::VelocityStruct,Shared)
TRILLEK_MAKE_COMPONENT(VelocityMax,"velocity-max",trillek::physics::VelocityMaxStruct,Shared)
TRILLEK_MAKE_COMPONENT(ReferenceFrame,"reference-frame",id_t,SystemValue)
TRILLEK_MAKE_COMPONENT(IsReferenceFrame,"is-reference-frame",bool,SystemValue)
TRILLEK_MAKE_COMPONENT(CombinedVelocity,"combined-velocity",trillek::physics::VelocityStruct,System)
TRILLEK_MAKE_COMPONENT(OxygenRate,"oxygen-rate",float_t,SystemValue)
TRILLEK_MAKE_COMPONENT(Health,"health",uint32_t,SystemValue)
TRILLEK_MAKE_COMPONENT(Immune,"immune",bool,SystemValue)
TRILLEK_MAKE_COMPONENT(Movable,"movable",bool,SystemValue)
TRILLEK_MAKE_COMPONENT(Moving,"moving",bool,SystemValue)
TRILLEK_MAKE_COMPONENT(MoveOffset,"move-offset",trillek::Transform,System)
TRILLEK_MAKE_COMPONENT(GraphicTransform,"graphic-transform",trillek::Transform, Shared)
TRILLEK_MAKE_COMPONENT(GameTransform,"game-transform",trillek::Transform, Shared)
TRILLEK_MAKE_COMPONENT(Interactable,"interaction",trillek::Interaction, System)
TRILLEK_MAKE_COMPONENT(VComputer,"trillek-computer",trillek::hw::Computer, System)
TRILLEK_MAKE_COMPONENT(VKeyboard,"keyboard",trillek::hw::VKeyboard, System)
TRILLEK_MAKE_COMPONENT(VDisplay,"display",trillek::hw::VDisplay, System)

} // namespace trillek

#endif // COMPONENT_ENUM_HPP_INCLUDED

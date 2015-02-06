#ifndef COMPONENT_ENUM_HPP_INCLUDED
#define COMPONENT_ENUM_HPP_INCLUDED

#define TRILLEK_MAKE_COMPONENT(enumerator,name,ctype,ccontainer) \
    namespace component {\
    template<> struct type_trait<Component::enumerator> { typedef ctype value_type; };\
    typedef ctype enumerator##_type;\
    \
    template<> struct container_type_trait<static_cast<typename std::underlying_type<Component>::type>(Component::enumerator)> {\
        typedef ccontainer<Component::enumerator> container_type;\
        \
        static ContainerBase* find(typename std::underlying_type<Component>::type component_id) {\
            return component_id == static_cast<typename std::underlying_type<Component>::type>(Component::enumerator) ?\
                &ContainerRef<container_type>::container\
                : container_type_trait<static_cast<typename std::underlying_type<Component>::type>(Component::enumerator)+1>::find(component_id);\
        }\
    };\
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

namespace graphics {
class LightBase;
class Renderable;
class CameraBase;
class SixDOFCamera;
}

namespace physics {
class Collidable;
struct VelocityStruct;
struct VelocityMaxStruct;
}

namespace hw {
class Computer;
class VDisplay;
class VKeyboard;
}

namespace component {

class Container;
template<Component C> class System;
template<Component C> class Shared;
template<Component C> class SystemValue;

enum class Component : uint32_t {
    Velocity = 1,               // instant displacement
    VelocityMax,                // maximum instant displacement
    ReferenceFrame,             // id of the parent entity for the transform
    IsReferenceFrame,           // true if this entity has at least a child
    CombinedVelocity,           // ?
    Collidable,                 // wrapper object for body
    Renderable,                 // entities with a rendered mesh
    Light,                      // light source
    Camera,                     // camera state and transform
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
    END                         // Keep this entry at the end
};

// A class to hold a container reference
template<class T>
struct ContainerRef {
    static T container;
};


template<class T>
T ContainerRef<T>::container;

template<Component C> struct type_trait;

template<typename std::underlying_type<Component>::type> struct container_type_trait;

struct ContainerBase {
    virtual ~ContainerBase() {}
};

} // namespace component
} // namespace trillek

#endif // COMPONENT_ENUM_HPP_INCLUDED

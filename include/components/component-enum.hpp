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

namespace physics {
class Collidable;
struct VelocityStruct;
struct VelocityMaxStruct;
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
    Collidable,                 // wrapper object for body TODO:make a true component
    OxygenRate,                 // Oxygen rate associated with an entity (room...)
    Health,                     // Health level
    Immune,                     // true = Immune
    GraphicTransform,           // transform used to display the entity
    GameTransform,              // last confirmed transform
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

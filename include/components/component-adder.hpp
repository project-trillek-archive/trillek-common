#ifndef COMPONENT_ADDER_HPP_INCLUDED
#define COMPONENT_ADDER_HPP_INCLUDED

#include "component.hpp"

namespace trillek {

using namespace component;

template<Component type, typename std::enable_if<!std::is_base_of<ComponentBase, typename type_trait<type>::value_type>::value>::type* = nullptr>
static std::shared_ptr<component::Container> CreateComponent(
        id_t entity_id, const std::vector<Property> &properties) {
    auto type_id = static_cast<uint32_t>(type);
    auto sharedcomp = component::Initialize<type>(properties);
    if (sharedcomp) {
        return std::move(sharedcomp);
    }
    LOGMSG(ERROR) << "Error while initializing component "
                    << reflection::GetTypeName<std::integral_constant<Component,type>>()
                    << " for entity id #" << entity_id;
    return nullptr;
}

template<Component type, class T = typename type_trait<type>::value_type, typename std::enable_if<std::is_base_of<ComponentBase, typename type_trait<type>::value_type>::value>::type* = nullptr>
static std::shared_ptr<component::Container> CreateComponent(
    id_t entity_id, const std::vector<Property> &properties) {
    auto type_id = static_cast<uint32_t>(type);
    auto ret = component::Create<type>(T());
    if (component::Get<type>(ret)->Initialize(properties)) {
        return std::move(ret);
    }
    LOGMSG(ERROR) << "Error while initializing class component "
        << reflection::GetTypeName<std::integral_constant<Component,type>>() << " for entity id #" << entity_id;
    return nullptr;
}


template<template <Component> class S,Component C>
struct ComponentAdder {
    bool Create(id_t entity_id, const std::vector<Property> &properties) {
        auto comp = CreateComponent<C>(entity_id, properties);

        if (comp) {
            LOGMSG(DEBUG) << "Adding component "
                << reflection::GetTypeName<std::integral_constant<Component,C>>()
                << " to entity #" << entity_id;
            Insert<C>(entity_id, std::move(comp));
            return true;
        }
        return false;
    }
};

template<Component C>
class ComponentAdder<SystemValue,C> {
public:
    bool Create(id_t entity_id, const std::vector<Property> &properties) {
        bool result = false;
        auto comp = component::Initialize<C>(result, properties);
        if (!result) {
            LOGMSGC(ERROR) << "Error while initializing component "
                            << reflection::GetTypeName<std::integral_constant<Component,C>>()
                            << " for entity id #" << entity_id;
            return false;
        }

        LOGMSG(DEBUG) << "Adding component "
            << reflection::GetTypeName<std::integral_constant<Component,C>>()
            << " to entity #" << entity_id;
        Insert<C>(entity_id, std::move(comp));
        return true;
    }
};
}

#endif // COMPONENT_ADDER_HPP_INCLUDED

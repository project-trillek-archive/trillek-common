#ifndef SYSTEM_COMPONENT_HPP_INCLUDED
#define SYSTEM_COMPONENT_HPP_INCLUDED

#include <map>
#include "bitmap.hpp"

namespace trillek { namespace component {

template<Component type>
class SystemContainer {
public:
    typedef std::map<id_t, std::shared_ptr<Container>,std::less<id_t>,
        TrillekAllocator<std::pair<const id_t,std::shared_ptr<Container>>>> container_type;

    static container_type container;
    static BitMap<uint32_t> bitmap;
};

template<Component type>
typename SystemContainer<type>::container_type SystemContainer<type>::container;

template<Component C>
BitMap<uint32_t> SystemContainer<C>::bitmap;

template<Component C>
class System final : public ContainerBase {
public:
    System() {};
    ~System() {};

    typename type_trait<C>::value_type& Get(id_t entity_id) {
        return *component::Get<C>(Map().at(entity_id));
    }

    std::shared_ptr<Container> GetContainer(id_t entity_id) {
        return Map().at(entity_id);
    }

    std::shared_ptr<typename type_trait<C>::value_type> GetSharedPtr(id_t entity_id) {
        const auto& ptr = Map().at(entity_id);
        return component::Get<C>(ptr);
    }

    bool Has(id_t entity_id) {
        return Bitmap().at(entity_id);
    }

    template<class V>
    void Insert(id_t entity_id, V&& value, typename std::enable_if<!util::is_shared_ptr<typename std::decay<V>::type>::value>::type* = 0) {
        Map().insert(std::make_pair(std::move(entity_id), component::Create<C>(std::forward<V>(value))));
        LOGMSG(DEBUG) << "system inserting component " << reflection::GetTypeName<std::integral_constant<Component,C>>() << " for entity #" << entity_id;
        SystemContainer<C>::bitmap[entity_id] = true;
    }

    template<class V>
    void Insert(id_t entity_id, V&& value, typename std::enable_if<util::is_shared_ptr<typename std::decay<V>::type>::value>::type* = 0) {
        Map().insert(std::make_pair(std::move(entity_id), std::forward<V>(value)));
        LOGMSG(DEBUG) << "system inserting component " << reflection::GetTypeName<std::integral_constant<Component,C>>() << " for entity #" << entity_id;
        SystemContainer<C>::bitmap[entity_id] = true;
    }

    template<class V>
    void Update(id_t entity_id, V&& value, typename std::enable_if<!util::is_shared_ptr<typename std::decay<V>::type>::value>::type* = 0) {
        Map().at(entity_id) = component::Create<C>(std::forward<V>(value));
    }

    template<class V>
    void Update(id_t entity_id, V&& value, typename std::enable_if<util::is_shared_ptr<typename std::decay<V>::type>::value>::type* = 0) {
        Map().at(entity_id) = std::forward<V>(value);
    }

    void Remove(id_t entity_id) {
        Map().erase(entity_id);
        SystemContainer<C>::bitmap.at(entity_id) = false;
    }

    typename SystemContainer<C>::container_type& Map() {
        return SystemContainer<C>::container;
    }

    const BitMap<uint32_t>& Bitmap() {
        return SystemContainer<C>::bitmap;
    }
};

} // namespace component
} // namespace trillek


#endif // SYSTEM_COMPONENT_HPP_INCLUDED

#ifndef SYSTEM_COMPONENT_VALUE_HPP_INCLUDED
#define SYSTEM_COMPONENT_VALUE_HPP_INCLUDED

#include <map>
#include "bitmap.hpp"

namespace trillek { namespace component {

template<Component C,class T>
class SystemValueContainer {
public:
    typedef std::map<id_t, T,std::less<id_t>,
        TrillekAllocator<std::pair<const id_t,T>>> container_type;

    static container_type container;
    static BitMap<uint32_t> bitmap;
};

template<Component C, class T>
typename SystemValueContainer<C,T>::container_type SystemValueContainer<C,T>::container;

template<Component C, class T>
BitMap<uint32_t> SystemValueContainer<C,T>::bitmap;

template<Component C>
class SystemValueContainer<C,bool> {
public:
    typedef BitMap<uint32_t> container_type;

    static container_type container;
    static BitMap<uint32_t>& bitmap;
};

template<Component C>
typename SystemValueContainer<C,bool>::container_type SystemValueContainer<C,bool>::container;

template<Component C>
BitMap<uint32_t>& SystemValueContainer<C,bool>::bitmap = SystemValueContainer<C,bool>::container;

template<Component C>
class SystemValue final : public ContainerBase {
public:
    SystemValue() {};
    ~SystemValue() {};

    template<Component D=C>
    typename type_trait<C>::value_type& Get(id_t entity_id, typename std::enable_if<!std::is_same<typename type_trait<D>::value_type,bool>::value>::type* = 0) {
        return Map().at(entity_id);
    }

    // bool specialization
    template<Component D=C>
    typename type_trait<C>::value_type Get(id_t entity_id, typename std::enable_if<std::is_same<typename type_trait<D>::value_type,bool>::value>::type* = 0) {
        return Map().at(entity_id);
    }

    bool Has(id_t entity_id) {
        return Bitmap().at(entity_id);
    }

    template<class V,Component D=C>
    void Insert(id_t entity_id, V&& value, typename std::enable_if<!std::is_same<typename type_trait<D>::value_type,bool>::value>::type* = 0) {
        Update(entity_id, std::forward<V>(value));
        SystemValueContainer<C,typename type_trait<C>::value_type>::bitmap[entity_id] = true;
    }

    // bool specialization
    template<class V,Component D=C>
    void Insert(id_t entity_id, V&& value, typename std::enable_if<std::is_same<typename type_trait<D>::value_type,bool>::value>::type* = 0) {
        Update(entity_id, std::forward<V>(value));
    }

    template<class V>
    void Update(id_t entity_id, V&& value) {
        (Map())[entity_id] = std::forward<V>(value);
    }

    template<Component D=C>
    void Remove(id_t entity_id, typename std::enable_if<!std::is_same<typename type_trait<D>::value_type,bool>::value>::type* = 0) {
        Map().erase(entity_id);
        SystemValueContainer<C,typename type_trait<C>::value_type>::bitmap[entity_id] = false;
    }

    // bool specialization
    template<Component D=C>
    void Remove(id_t entity_id, typename std::enable_if<std::is_same<typename type_trait<D>::value_type,bool>::value>::type* = 0) {
        Map().erase(entity_id);
    }

    typename SystemValueContainer<C,typename type_trait<C>::value_type>::container_type& Map() {
        return SystemValueContainer<C,typename type_trait<C>::value_type>::container;
    }

    const BitMap<uint32_t>& Bitmap() {
        return SystemValueContainer<C,typename type_trait<C>::value_type>::bitmap;
    }

};

} // namespace component
} // namespace trillek



#endif // SYSTEM_COMPONENT_VALUE_HPP_INCLUDED

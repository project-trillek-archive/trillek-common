#ifndef SHARED_COMPONENT_HPP_INCLUDED
#define SHARED_COMPONENT_HPP_INCLUDED

#include "systems/rewindable-map.hpp"

namespace trillek { namespace component {

template<Component C,class T>
class SharedContainer {
public:
    static RewindableMap<id_t, std::shared_ptr<const Container>,frame_tp,30> container;
};

template<Component C,class T>
RewindableMap<id_t, std::shared_ptr<const Container>,frame_tp,30> SharedContainer<C,T>::container;

template<Component C>
class SharedContainer<C,bool> {
public:
    static RewindableMap<id_t, bool,frame_tp,30> container;
};

template<Component C>
RewindableMap<id_t, bool,frame_tp,30> SharedContainer<C,bool>::container;

template<Component C>
class Shared final : public ContainerBase {
public:
    Shared() {};
    ~Shared() {};

    const typename type_trait<C>::value_type& Get(id_t entity_id) {
        return *component::Get<C>(Map().Map().at(entity_id));
    }

    std::shared_ptr<const Container> GetConstContainer(id_t entity_id) {
        return Map().Map().at(entity_id);
    }

    std::shared_ptr<const typename type_trait<C>::value_type> GetSharedPtr(id_t entity_id) {
        const auto& ptr = std::const_pointer_cast<Container>(Map().Map().at(entity_id));
        return component::Get<C,const typename type_trait<C>::value_type>(ptr);
    }

    bool Has(id_t entity_id) {
        return Map().Map().count(entity_id);
    }

    template<class V>
    void Insert(id_t entity_id, V&& value, typename std::enable_if<!util::is_shared_ptr<typename std::decay<V>::type>::value>::type* = 0) {
        Map().Insert(entity_id, component::CreateConst<C>(std::forward<V>(value)));
    }


    template<class V>
    void Insert(id_t entity_id, V&& value, typename std::enable_if<util::is_shared_ptr<typename std::decay<V>::type>::value>::type* = 0) {
        Map().Insert(entity_id, std::forward<V>(value));
    }

    template<class V>
    void Update(id_t entity_id, V&& value, typename std::enable_if<!util::is_shared_ptr<typename std::decay<V>::type>::value>::type* = 0) {
        Map().Update(entity_id, component::CreateConst<C>(std::forward<V>(value)));
    }

    template<class V>
    void Update(id_t entity_id, V&& value, typename std::enable_if<util::is_shared_ptr<typename std::decay<V>::type>::value>::type* = 0) {
        Map().Update(entity_id, std::forward<V>(value));
    }

    void Remove(id_t entity_id) {
        Map().Remove(entity_id);
    }


    void Commit(frame_tp frame) {
        Map().Commit(frame);
    }

    const BitMap<uint32_t>& GetLastPositiveBitMap() {
        return Map().GetLastPositiveBitMap();
    }

    const BitMap<uint32_t>& Bitmap() {
        return Map().Bitmap();
    }

    auto Map() -> decltype((SharedContainer<C,typename type_trait<C>::value_type>::container)) {
        return SharedContainer<C,typename type_trait<C>::value_type>::container;
    }

    auto GetLastPositiveCommit() -> decltype(this->Map().GetLastPositiveCommit()) {
        return Map().GetLastPositiveCommit();
    }
};

} // namespace component
} // namespace trillek

#endif // SHARED_COMPONENT_HPP_INCLUDED

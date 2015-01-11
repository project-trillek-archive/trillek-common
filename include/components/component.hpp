#ifndef COMPONENT_HPP_INCLUDED
#define COMPONENT_HPP_INCLUDED

#include <vector>
#include "systems/physics.hpp"
#include "bitmap.hpp"
#include "components/component-enum.hpp"
#include "components/component-container.hpp"
#include "components/shared-component.hpp"
#include "components/system-component.hpp"
#include "components/system-component-value.hpp"

namespace trillek {

namespace component {

template<> struct type_trait<Component::END> { typedef char value_type; };

template<> struct container_type_trait<static_cast<typename std::underlying_type<Component>::type>(Component::END)> {
    typedef SystemValue<Component::END> container_type;

    static ContainerBase* find(typename std::underlying_type<Component>::type component_id) {
        assert(false && "Could not find component");
        return nullptr;
    }
};
} // namespace component

// use this macro in reverse order of the enum
TRILLEK_MAKE_COMPONENT(GameTransform,"game-transform",trillek::Transform, Shared)
TRILLEK_MAKE_COMPONENT(GraphicTransform,"graphic-transform",trillek::Transform, Shared)
TRILLEK_MAKE_COMPONENT(Immune,"immune",bool,SystemValue)
TRILLEK_MAKE_COMPONENT(Health,"health",uint32_t,SystemValue)
TRILLEK_MAKE_COMPONENT(OxygenRate,"oxygen-rate",float_t,SystemValue)
TRILLEK_MAKE_COMPONENT(Collidable,"collidable",trillek::physics::Collidable,System)
TRILLEK_MAKE_COMPONENT(CombinedVelocity,"combined-velocity",trillek::physics::VelocityStruct,System)
TRILLEK_MAKE_COMPONENT(IsReferenceFrame,"is-reference-frame",bool,SystemValue)
TRILLEK_MAKE_COMPONENT(ReferenceFrame,"reference-frame",id_t,SystemValue)
TRILLEK_MAKE_COMPONENT(VelocityMax,"velocity-max",trillek::physics::VelocityMaxStruct,Shared)
TRILLEK_MAKE_COMPONENT(Velocity,"velocity",trillek::physics::VelocityStruct,Shared)

class ComponentBase {
public:

    ComponentBase() : component_type_id(0) { }
    ~ComponentBase() { }

    /**
     * \brief Returns a component with the specified name.
     *
     * \param[in] const std::vector<Property> &properties The creation properties for the component.
     * \return bool True if initialization finished with no errors.
     */
    virtual bool Initialize(const std::vector<Property> &properties) { return false; };

    unsigned int component_type_id;
};

namespace component {

/** \brief Default initialization function
 *
 * May be specialized in component.cpp
 *
 * \param properties the properties
 * \return the component created
 *
 */
template<Component C>
std::shared_ptr<Container> Initialize(const std::vector<Property> &properties) {
    return component::Create<C>(typename type_trait<C>::value_type());
}

/** \brief Default initialization function for values
 *
 * May be specialized in component.cpp
 *
 * result is set to true if initialization was OK.
 *
 * \param result a placeholder for the result
 * \param properties the properties
 * \return an empty value
 *
 */
template<Component C>
typename type_trait<C>::value_type Initialize(bool& result, const std::vector<Property> &properties) {
    result = true;
    return typename type_trait<C>::value_type();
}

/** \brief Apply a function to all entities in the bitmap
 *
 * \param bitmap the bitmap
 * \param operation the function executed
 *
 */
template<class T>
static void OnTrue(const BitMap<T>& bitmap, const std::function<void(id_t)>& operation) {
    // TODO replace 10000 by the number of entities in the game.
    auto end = bitmap.DefaultValue() ? std::max(bitmap.size(), size_t(10000)) : bitmap.size();
    for (auto i = bitmap.enumerator(10000); *i < end; ++i) {
        operation(*i);
    }
}

/** \brief Get the components container
 *
 * Use this to make a copy of all components.
 *
 * \return A reference on the raw container
 *
 */
template<Component C>
static typename container_type_trait<static_cast<typename std::underlying_type<Component>::type>(C)>::container_type& GetRawContainer() {
    return ContainerRef<typename container_type_trait<static_cast<typename std::underlying_type<Component>::type>(C)>::container_type>::container;
}

/** \brief Get the components container at runtime
 *
 * The object returned must be upcast to the real type using RTTI
 *
 * \return A reference on the raw container
 *
 */
static ContainerBase* GetRawContainer(typename std::underlying_type<Component>::type component_id) {
    return container_type_trait<1>::find(component_id);
}

/** \brief Return the component value
 *
 * The pointer (if any) is dereferenced. You may prefer GetContainer() to get a copy of the pointer.
 *
 * \param entity_id the id of the entity
 * \return A reference to the value
 *
 */
template<Component C>
static const typename type_trait<C>::value_type& Get(id_t entity_id, typename std::enable_if<!std::is_same<typename type_trait<C>::value_type,bool>::value>::type* = 0) {
    return GetRawContainer<C>().Get(entity_id);
}

/** \brief Return the component value. bool version.
 *
 * \param entity_id the id of the entity
 * \return the component value (bool)
 *
 */
template<Component C>
static typename type_trait<C>::value_type Get(id_t entity_id, typename std::enable_if<std::is_same<typename type_trait<C>::value_type,bool>::value>::type* = 0) {
    return GetRawContainer<C>().Get(entity_id);
}

/** \brief Return the pointer of the component value
 *
 * The returned value can be copied to another component
 *
 * This function is not defined for SystemValue components.
 *
 * \param entity_id the entity id
 * \return std::shared_ptr<Container> the pointer of the component
 *
 */
template<Component C>
std::shared_ptr<Container> GetContainer(id_t entity_id) {
    return GetRawContainer<C>().GetContainer(entity_id);
}

/** \brief Return the pointer of the component value. const version
 *
 * The returned value can be copied to another component
 *
 * This function is not defined for SystemValue components.
 *
 * \param entity_id the entity id
 * \return std::shared_ptr<const Container> the pointer of the component
 *
 */
template<Component C>
std::shared_ptr<const Container> GetConstContainer(id_t entity_id) {
    return GetRawContainer<C>().GetConstContainer(entity_id);
}

/** \brief Return a pointer cast to the type of the component
 *
 * \param entity_id the entity id
 * \return a shared pointer of the component type
 *
 */
template<Component C>
std::shared_ptr<const typename type_trait<C>::value_type> GetSharedPtr(id_t entity_id) {
    return GetRawContainer<C>().GetSharedPtr(entity_id);
}

/** \brief Return a pointer cast to the const type of the component
 *
 * \param entity_id the entity id
 * \return a shared pointer of the component const type
 *
 */
template<Component C>
std::shared_ptr<const typename type_trait<C>::value_type> GetConstSharedPtr(id_t entity_id) {
    return GetRawContainer<C>().GetSharedPtr(entity_id);
}

/** \brief Store a new component
 *
 * \param entity_id the entity id
 * \param value the value to store
 *
 */
template<Component C, class V>
static void Insert(id_t entity_id, V&& value) {
    GetRawContainer<C>().Insert(entity_id, std::forward<V>(value));
}

/** \brief Modify the value of an existing component
 *
 * \param entity_id the entity id
 * \param value the new value
 *
 */
template<Component C, class V>
static void Update(id_t entity_id, V&& value) {
    GetRawContainer<C>().Update(entity_id, std::forward<V>(value));
}

/** \brief Remove a component
 *
 * \param entity_id the entity id
 *
 */
template<Component C>
static void Remove(id_t entity_id) {
    GetRawContainer<C>().Remove(entity_id);
}

/** \brief Tell if a component exists
 *
 * \param entity_id the entity id
 * \return true if the component exists, false otherwise
 *
 */
template<Component C>
static bool Has(id_t entity_id) {
    return GetRawContainer<C>().Has(entity_id);
}

/** \brief Returns the bitmap associated with the component
 *
 * The bitmap will return true for each entity id that has the component.
 *
 * \return const BitMap<uint32_t>& the bitmap
 *
 */
template<Component C>
static const BitMap<uint32_t>& Bitmap() {
    return GetRawContainer<C>().Bitmap();
}

/** \brief Commit the data in the work space
 *
 * For shared component, this actually publishes the component updates.
 *
 * \param frame the frame number to tag the commit with
 *
 */
template<Component C>
static void Commit(frame_tp frame) {
    GetRawContainer<C>().Commit(frame);
}

/** \brief Get the updates from the last frame
 *
 * For shared component, this actually gets the component updates.
 *
 *
 */
template<Component C>
static const std::map<id_t,const typename type_trait<C>::value_type, std::less<id_t>,
            TrillekAllocator<std::pair<const id_t,typename type_trait<C>::value_type>>>&
                                                         GetLastPositiveCommit() {
    return GetRawContainer<C>().GetLastPositiveCommit();
}

/** \brief Get the bitmap associated with the updates from the last frame
 *
 * For shared component, this actually gets the bitmap of the updates.
 *
 *
 */
template<Component C>
static const BitMap<uint32_t>& GetLastPositiveBitMap() {
    return GetRawContainer<C>().GetLastPositiveBitMap();
}

/** \brief Return a bitmap of component comparison
 *
 * The bitmap returns true for each entity verifying 'value < n'
 *
 * \param n a value to compare
 * \return BitMap<uint32_t> a bitmap for the comparison
 *
 */
template<Component C, class T>
static BitMap<uint32_t> Lower(const T& n) {
    BitMap<uint32_t> ret;
    OnTrue(Bitmap<C>(),
        [&](id_t id) {
            if (Get<C>(id) < n) {
                ret[id] = true;
            }
        }
    );
    return ret;
}

/** \brief Return a bitmap of component comparison
 *
 * The bitmap returns true for each entity verifying 'value <= n'
 *
 * \param n a value to compare
 * \return BitMap<uint32_t> a bitmap for the comparison
 *
 */
template<Component C, class T>
static BitMap<uint32_t> LowerOrEqual(const T& n) {
    BitMap<uint32_t> ret;
    OnTrue(Bitmap<C>(),
        [&](id_t id) {
            if (Get<C>(id) <= n) {
                ret[id] = true;
            }
        }
    );
    return ret;
}

/** \brief Return a bitmap of component comparison
 *
 * The bitmap returns true for each entity verifying 'value > n'
 *
 * \param n a value to compare
 * \return BitMap<uint32_t> a bitmap for the comparison
 *
 */
template<Component C, class T>
static BitMap<uint32_t> Greater(const T& n) {
    BitMap<uint32_t> ret;
    OnTrue(Bitmap<C>(),
        [&](id_t id) {
            if (Get<C>(id) > n) {
                ret[id] = true;
            }
        }
    );
    return ret;
}

/** \brief Return a bitmap of component comparison
 *
 * The bitmap returns true for each entity verifying 'value >= n'
 *
 * \param n a value to compare
 * \return BitMap<uint32_t> a bitmap for the comparison
 *
 */
template<Component C, class T>
static BitMap<uint32_t> GreaterOrEqual(const T& n) {
    BitMap<uint32_t> ret;
    OnTrue(Bitmap<C>(),
        [&](id_t id) {
            if (Get<C>(id) >= n) {
                ret[id] = true;
            }
        }
    );
    return ret;
}

/** \brief Return a bitmap of component comparison
 *
 * The bitmap returns true for each entity verifying 'value == n'
 *
 * \param n a value to compare
 * \return BitMap<uint32_t> a bitmap for the comparison
 *
 */
template<Component C, class T>
static BitMap<uint32_t> Equal(const T& n) {
    BitMap<uint32_t> ret;
    OnTrue(Bitmap<C>(),
        [&](id_t id) {
            if (Get<C>(id) == n) {
                ret[id] = true;
            }
        }
    );
    return ret;
}

/** \brief Return a bitmap of component comparison
 *
 * The bitmap returns true for each entity verifying 'value != n'
 *
 * \param n a value to compare
 * \return BitMap<uint32_t> a bitmap for the comparison
 *
 */
template<Component C, class T>
static BitMap<uint32_t> NotEqual(const T& n) {
    BitMap<uint32_t> ret;
    OnTrue(Bitmap<C>(),
        [&](id_t id) {
            if (Get<C>(id) != n) {
                ret[id] = true;
            }
        }
    );
    return ret;
}

/** \brief Return a bitmap of component comparison
 *
 * The bitmap returns true for each entity verifying 'value<C1> < value<C2>'
 *
 * \return BitMap<uint32_t> a bitmap for the comparison
 *
 */
template<Component C1, Component C2>
static BitMap<uint32_t> Lower() {
    BitMap<uint32_t> ret;
    OnTrue(Bitmap<C1>() & Bitmap<C2>(),
        [&](id_t id) {
            if (Get<C1>(id) < Get<C2>(id)) {
                ret[id] = true;
            }
        }
    );
    return ret;
}

/** \brief Return a bitmap of component comparison
 *
 * The bitmap returns true for each entity verifying 'value<C1> <= value<C2>'
 *
 * \return BitMap<uint32_t> a bitmap for the comparison
 *
 */
template<Component C1, Component C2>
static BitMap<uint32_t> LowerOrEqual() {
    BitMap<uint32_t> ret;
    OnTrue(Bitmap<C1>() & Bitmap<C2>(),
        [&](id_t id) {
            if (Get<C1>(id) <= Get<C2>(id)) {
                ret[id] = true;
            }
        }
    );
    return ret;
}

/** \brief Return a bitmap of component comparison
 *
 * The bitmap returns true for each entity verifying 'value<C1> > value<C2>'
 *
 * \return BitMap<uint32_t> a bitmap for the comparison
 *
 */
template<Component C1, Component C2>
static BitMap<uint32_t> Greater() {
    BitMap<uint32_t> ret;
    OnTrue(Bitmap<C1>() & Bitmap<C2>(),
        [&](id_t id) {
            if (Get<C1>(id) > Get<C2>(id)) {
                ret[id] = true;
            }
        }
    );
    return ret;
}

/** \brief Return a bitmap of component comparison
 *
 * The bitmap returns true for each entity verifying 'value<C1> >= value<C2>'
 *
 * \return BitMap<uint32_t> a bitmap for the comparison
 *
 */
template<Component C1, Component C2>
static BitMap<uint32_t> GeaterOrEqual() {
    BitMap<uint32_t> ret;
    OnTrue(Bitmap<C1>() & Bitmap<C2>(),
        [&](id_t id) {
            if (Get<C1>(id) >= Get<C2>(id)) {
                ret[id] = true;
            }
        }
    );
    return ret;
}

/** \brief Return a bitmap of component comparison
 *
 * The bitmap returns true for each entity verifying 'value<C1> == value<C2>'
 *
 * \return BitMap<uint32_t> a bitmap for the comparison
 *
 */
template<Component C1, Component C2>
static BitMap<uint32_t> Equal() {
    BitMap<uint32_t> ret;
    OnTrue(Bitmap<C1>() & Bitmap<C2>(),
        [&](id_t id) {
            if (Get<C1>(id) == Get<C2>(id)) {
                ret[id] = true;
            }
        }
    );
    return ret;
}

/** \brief Return a bitmap of component comparison
 *
 * The bitmap returns true for each entity verifying 'value<C1> != value<C2>'
 *
 * \return BitMap<uint32_t> a bitmap for the comparison
 *
 */
template<Component C1, Component C2>
static BitMap<uint32_t> NotEqual() {
    BitMap<uint32_t> ret;
    OnTrue(Bitmap<C1>() & Bitmap<C2>(),
        [&](id_t id) {
            if (Get<C1>(id) != Get<C2>(id)) {
                ret[id] = true;
            }
        }
    );
    return ret;
}

/** \brief Add a constant to all components
 *
 * \param n a value to add
 *
 */
template<Component C, class T>
static void Add(const T& n) {
    OnTrue(Bitmap<C>(),
        [&](id_t id) {
            Update<C>(id, Get<C>(id) + n);
        }
    );
}

/** \brief Add a constant to the components matching a bitmap
 *
 * \param n the constant to add
 * \param bitmap the bitmap to match
 *
 */
template<Component C, class T>
static void Add(const T& n, const BitMap<uint32_t>& bitmap) {
    OnTrue(bitmap,
        [&](id_t id) {
            Update<C>(id, Get<C>(id) + n);
        }
    );
}

/** \brief Multiply all components by a constant
 *
 * \param n a multiplication factor
 *
 */
template<Component C, class T>
static void Multiply(const T& n) {
    OnTrue(Bitmap<C>(),
        [&](id_t id) {
            Update<C>(id, Get<C>(id) * n);
        }
    );
}

/** \brief Multiply the components matching a bitmap by a constant
 *
 * \param n the multiplication factor
 * \param bitmap the bitmap to match
 *
 */
template<Component C, class T>
static void Multiply(const T& n, const BitMap<uint32_t>& bitmap) {
    OnTrue(bitmap,
        [&](id_t id) {
            Update<C>(id, Get<C>(id) * n);
        }
    );
}

/** \brief Divide all components by a constant
 *
 * \param n a value that will divide the components
 *
 */
template<Component C, class T>
static void Divide(const T& n) {
    OnTrue(Bitmap<C>(),
        [&](id_t id) {
            Update<C>(id, Get<C>(id) / n);
        }
    );
}

/** \brief Divide the components matching a bitmap by a constant
 *
 * \param n the constant to divide the components by
 * \param bitmap the bitmap to match
 *
 */
template<Component C, class T>
static void Divide(const T& n, const BitMap<uint32_t>& bitmap) {
    OnTrue(bitmap,
        [&](id_t id) {
            Update<C>(id, Get<C>(id) / n);
        }
    );
}

} // namespace component
} // namespace trillek

#endif

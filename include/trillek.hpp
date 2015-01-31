#ifndef TRILLEK_H_INCLUDED
#define TRILLEK_H_INCLUDED

#include <stdint.h>
#include <list>
#include <memory>

// useful variable
#define GCC_VERSION (__GNUC__ * 10000 \
                               + __GNUC_MINOR__ * 100 \
                               + __GNUC_PATCHLEVEL__)

// Visual Studio 2015 will have noexcept keyword
// Until then we remove it
#ifndef _MSC_VER
#define NOEXCEPT noexcept
#else
#define NOEXCEPT
#endif

// make_unique will be in C++14. Implemented here since we're using C++11.
// VS2013 already implements it, GCC 4.9 will implement it
// TODO: remove it when using GCC 4.9 and -std=c++1y
#if defined(__GNUC__) && ((GCC_VERSION < 40900) || __cplusplus <= 201103L)
#include <memory>

template<typename T, typename... Args>
std::unique_ptr<T> make_unique(Args&&... args) {
    return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}
#else
using std::make_unique;
#endif

template<typename T, typename Alloc, typename... Args>
std::unique_ptr<T> allocate_unique(Alloc& a, Args&&... args) {
    T* ptr = a.allocate(1);
    try {
        a.construct(ptr, std::forward<Args>(args)...);
        auto deleter = [a](T* p) {
            a.destroy(p);
            a.deallocate(p, 1);
        };
        return std::unique_ptr<T>(ptr, std::move(deleter));
    }
    catch (const std::bad_alloc&) {
        a.deallocate(ptr, 1);
        throw;
    }
}

namespace trillek {

// type of an entity #id
typedef uint32_t id_t;

template<class T, class Alloc>
class TrillekAllocator;

// type of a list
template<class U,class Alloc=std::allocator<U>>
using trillek_list = std::list<U, TrillekAllocator<U,Alloc>>;

/** \brief Type of component
 *
 * DYNAMIC: The component is passed to the system and then stored in the
 * ComponentFactory container
 * SYSTEM : The component is passed to the system that must store it.
 * SHARED : The component is passed to SharedComponent and is stored there
 *
 * Only SHARED components can be shared between systems in different threads.
 */
enum ComponentType { DYNAMIC, SYSTEM, SHARED };

namespace component {
enum class Component : uint32_t;
}

namespace reflection {

// Template methods that can be used for reflection.
// To use them just call GetTypeName<MyType>() to retrieve the name for the specified type.
template<class TYPE> const char* GetTypeName(void) { return "UNKNOWN"; }
template<class TYPE> unsigned int GetTypeID(void) { return ~0; }
// function to get the name of the handler of a packet type
template <uint32_t MAJOR,uint32_t MINOR> const char* GetNetworkHandler(void) {}

} // End of reflection
} // End fo trillek

#endif // TRILLEK_H_INCLUDED

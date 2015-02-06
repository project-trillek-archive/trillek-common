#ifndef TRILLEKALLOCATOR_HPP_INCLUDED
#define TRILLEKALLOCATOR_HPP_INCLUDED

#include <cstddef>
#include "trillek.hpp"
#include <iostream>

extern size_t gAllocatedSize;

namespace trillek {

template<class T, class Alloc = std::allocator<T>>
class TrillekAllocator {
    template<class U,class AnyAlloc>
    friend class TrillekAllocator;
public:
    typedef size_t size_type;
    typedef ptrdiff_t difference_type;
    typedef T* pointer;
    typedef const T* const_pointer;
    typedef T& reference;
    typedef const T& const_reference;
    typedef T value_type;

    typedef typename Alloc::propagate_on_container_copy_assignment propagate_on_container_copy_assignment;
    typedef typename Alloc::propagate_on_container_move_assignment propagate_on_container_move_assignment;
    typedef typename Alloc::propagate_on_container_swap propagate_on_container_swap;

    /// Default constructor
    TrillekAllocator(Alloc* alloc) NOEXCEPT : alloc(alloc) {}
    /// Copy constructor
    TrillekAllocator(const TrillekAllocator& other) NOEXCEPT : alloc(other.alloc) {}
    /// Move constructor
    TrillekAllocator(TrillekAllocator&& other) NOEXCEPT :  alloc(std::move(other.alloc)) {}
    /// Copy constructor with another type
    template<typename U>
    TrillekAllocator(const TrillekAllocator<U,Alloc>& other) NOEXCEPT : alloc(other.alloc) {}
    /// Move constructor with another type
    template<typename U>
    TrillekAllocator(TrillekAllocator<U,Alloc>&& other) NOEXCEPT : alloc(std::move(other.alloc)) {}

    /// Destructor
    ~TrillekAllocator() {}

    /// Copy
    TrillekAllocator<T,Alloc>& operator=(TrillekAllocator other) NOEXCEPT {
        swap(other);
        return *this;
    }

    /// Copy with another type
    template<typename U>
    TrillekAllocator<T,Alloc>& operator=(TrillekAllocator<U,Alloc> other) NOEXCEPT {
        swap(other);
        return *this;
    }

    /// swap
    template<class U>
    void swap(TrillekAllocator<U,Alloc>& other) NOEXCEPT {
        using std::swap;
        swap(this->alloc, other.alloc);
    }

    bool operator==(const TrillekAllocator& lhs) NOEXCEPT {
        return *this->alloc == *lhs.alloc;
    }

    bool operator!=(const TrillekAllocator& lhs) const NOEXCEPT {
        return *this->alloc != *lhs.alloc;
    }

    /// Get address of reference
    pointer address(reference x) const {
        return &x;
    }
    /// Get const address of const reference
    const_pointer address(const_reference x) const {
        return &x;
    }

    /// Allocate memory
    pointer allocate(size_type n, const void* = 0) {
        size_type size = n * sizeof(value_type);
        gAllocatedSize += size;
        return static_cast<pointer>(alloc->allocate(size));
    }

    /// Deallocate memory
    void deallocate(void* p, size_type n) {
        size_type size = n * sizeof(T);
        gAllocatedSize -= size;
        alloc->deallocate(static_cast<pointer>(p), size);
    }

    /// Call constructor
    void construct(pointer p, const T& val) {
        // Placement new
        new ((T*)p) T(val);
    }

    /// Call constructor with more arguments
    template<typename U, typename... Args>
    void construct(U* p, Args&&... args) {
        // Placement new
        ::new((void*)p) U(std::forward<Args>(args)...);
    }

    /// Call the destructor of p
    void destroy(pointer p) {
        p->~T();
    }
    /// Call the destructor of p of type U
    template<typename U>
    void destroy(U* p) {
        p->~U();
    }

    /// Get the max allocation size
    size_type max_size() const {
        return alloc->max_size();
    }

    /// A struct to rebind the allocator to another allocator of type U
    template<typename U>
    struct rebind {
        typedef TrillekAllocator<U,Alloc> other;
    };

    // called by copy assignment operator of containers only if
    // propagate_on_container_copy_assignment == true
    // && alloc this != alloc other (i.e other can't deallocate objects allocated by this)
    // && always_equal == false
    TrillekAllocator<T,Alloc> select_on_container_copy_construction() {
        auto copy = std::make_shared<Alloc>();
        return TrillekAllocator<T,Alloc>(std::move(copy));
    }

private:
    typename Alloc::self_raw_pointer_type alloc;
};

template<class T>
class TrillekAllocator<T,std::allocator<T>> : public std::allocator_traits<std::allocator<T>> {
public:
    typedef T& reference;
    typedef const T& const_reference;

    /// Default constructor
    TrillekAllocator() NOEXCEPT { }
    /// Copy constructor
    TrillekAllocator(const TrillekAllocator&) NOEXCEPT {}
    /// Copy constructor with another type
    template<typename U>
    TrillekAllocator(const TrillekAllocator<U,std::allocator<U>>&) NOEXCEPT {}
    /// Destructor
    ~TrillekAllocator() { }
    /// Copy
    TrillekAllocator<T,std::allocator<T>>& operator=(const TrillekAllocator&) NOEXCEPT {
        return *this;
    }

    /// Allocate memory
    T* allocate(size_t n) {
        auto size = n * sizeof(T);
        gAllocatedSize += size;
        return static_cast<T*>(::operator new(size));
    }

    /// Deallocate memory
    void deallocate(T* p, size_t n) {
        gAllocatedSize -= n * sizeof(T);
        ::operator delete(p);
    }

    /// Call constructor
    void construct(T* p, const T& val) {
        // Placement new
        new ((T*)p) T(val);
    }

    /// Call constructor with more arguments
    template<typename U, typename... Args>
    void construct(U* p, Args&&... args) {
        // Placement new
        ::new((void*)p) U(std::forward<Args>(args)...);
    }

    /// Call the destructor of p
    void destroy(T* p) {
        p->~T();
    }
    /// Call the destructor of p of type U
    template<typename U>
    void destroy(U* p) {
        p->~U();
    }

    /// A struct to rebind the allocator to another allocator of type U
    template<typename U>
    struct rebind {
        typedef TrillekAllocator<U,std::allocator<U>> other;
    };

};

} // namespace trillek

#endif // TRILLEKALLOCATOR_HPP_INCLUDED

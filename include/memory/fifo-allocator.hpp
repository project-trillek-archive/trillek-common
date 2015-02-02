#ifndef FIFOALLOCATOR_HPP_INCLUDED
#define FIFOALLOCATOR_HPP_INCLUDED

#include <new>

namespace trillek { namespace memory {

/** \brief FIFO allocator
 *
 * This class sequentially allocate memory in a chain of fixed size buffer.
 * A sliding window is moved forward during time and is split when reaching the end of the buffer.
 * Allocations and deallocations must be performed in the same order.
 *
 * 1 or 2 vectors of data are available for each buffer.
 *
 * If the allocation is not possible in the current buffer, the request is transferred to the next buffer
 * in the chain. A new buffer is created on-the-fly if necessary.
 *
 * size: size of the buffer
 * Alignment: alignment of pointers. Default is 16.
 */
template<size_t size, size_t Alignment = 16>
class FIFOAllocator {
public:
    typedef std::true_type propagate_on_container_copy_assignment;
    typedef std::true_type propagate_on_container_move_assignment;
    typedef std::true_type propagate_on_container_swap;

    typedef FIFOAllocator* self_raw_pointer_type;

    FIFOAllocator() {
        auto aligned_size = Align(size);
        buffer.reserve(aligned_size + Alignment);
        buffer_start = Align(buffer.data());
        lower_bound = buffer_start;
        upper_bound = lower_bound;
        buffer_end = lower_bound + aligned_size;
    }

    // Copy
    FIFOAllocator(const FIFOAllocator& other) :
        buffer(other.buffer) {
        auto aligned_size = Align(size);
        buffer.reserve(aligned_size + Alignment);
        buffer_start = Align(this->buffer.data());
        lower_bound = this->buffer_start;
        upper_bound = this->lower_bound;
        buffer_end = this->lower_bound + aligned_size;
        if (other.next_buffer) {
            this->next_buffer(make_unique<FIFOAllocator<size,Alignment>>(*other.next_buffer));
        }
    }

    // Copy assignment
    FIFOAllocator& operator=(const FIFOAllocator& other) {
        buffer = other.buffer;
        auto aligned_size = Align(size);
        buffer.reserve(aligned_size + Alignment);
        buffer_start = Align(this->buffer.data());
        lower_bound = buffer_start;
        upper_bound = lower_bound;
        buffer_end = lower_bound + aligned_size;
        if (other.next_buffer) {
            this->next_buffer = make_unique<FIFOAllocator<size,Alignment>>(*other.next_buffer);
        }
        return *this;
    }

    void* allocate(size_t n) {
/*        std::cout << "   >>> allocate lower_bound = " << std::dec << lower_bound - Align(buffer->data()) << std::endl;
        std::cout << "   >>> allocate upper_bound = " << std::dec << upper_bound - Align(buffer->data()) << std::endl;
        std::cout << "+++allocate n = " << std::dec << n << std::endl;
*/        auto aligned_n = Align(n);
        auto new_upper_bound = upper_bound + aligned_n;
//        std::cout << "+++allocate new upper_bound = " << std::dec << new_upper_bound - Align(buffer->data()) << std::endl;
        if ( n > Align(size) || new_upper_bound < upper_bound) {
            // integer overflow or buffer too small
            throw std::bad_alloc();
        }
        void* ret;
        if (lower_bound <= upper_bound) {
            if (new_upper_bound > buffer_end) {
                // above the buffer end, try to allocate at the buffer first address
                new_upper_bound = buffer_start + aligned_n;
                if (new_upper_bound > lower_bound) {
                    // requested size is too big, so we allocate memory in next buffer
                    if (! next_buffer) {
                        this->next_buffer = make_unique<FIFOAllocator<size,Alignment>>();
                    }
                    return next_buffer->allocate(n);
                }
                ret = buffer_start;
                last_address = upper_bound;
            }
            else {
                // we can allocate space
                ret = upper_bound;
            }
        }
        else {
            if (new_upper_bound > lower_bound) {
                // requested size is too big, so we allocate memory in next buffer
                if (! next_buffer) {
                    this->next_buffer = make_unique<FIFOAllocator<size,Alignment>>();
                }
                return next_buffer->allocate(n);
            }
            // we can allocate space
            ret = upper_bound;
        }
        upper_bound = new_upper_bound;
//        std::cout << "   OOO return " << std::hex << static_cast<void*>(ret) << std::endl;
        return ret;
    }

    void deallocate(void* p, size_t n) {
        if (p < buffer_start || p >= buffer_end) {
            if (next_buffer) {
                next_buffer->deallocate(p, n);
            }
            else {
                LOGMSG(ERROR) << "CircularBufferAllocator: Trying to deallocate a bad pointer.";
            }
            return;
        }
        if (p != lower_bound) {
            LOGMSG(ERROR) << "CircularBufferAllocator: Deallocating in wrong order. The allocator is in an invalid state. Fix it !";
            LOGMSG(ERROR) << "Note: Instantiating object on the stack in reverse order before allocating memory may solve the issue.";
        }
/*        std::cout << "   >>> deallocate lower_bound = " << std::dec << lower_bound - Align(buffer->data()) << std::endl;
        std::cout << "   >>> deallocate upper_bound = " << std::dec << upper_bound - Align(buffer->data()) << std::endl;
        std::cout << "---deallocate n = " << std::dec << n << std::endl;
*/        auto aligned_n = Align(n);
        auto new_lower_bound = static_cast<char*>(p) + aligned_n;
//        std::cout << "   >>> allocate new lower_bound = " << std::dec << new_lower_bound - Align(buffer->data()) << std::endl;
        assert(new_lower_bound <= buffer_end);
        if (lower_bound < upper_bound) {
            if (new_lower_bound > lower_bound) {
                lower_bound = std::min(new_lower_bound, upper_bound);
            }
        }
        else {
            lower_bound = (new_lower_bound >= last_address) ? std::min(buffer_start + (new_lower_bound - last_address), upper_bound) : new_lower_bound;
        }
        if (lower_bound == upper_bound) {
            lower_bound = buffer_start;
            upper_bound = lower_bound;
        }
//        std::cout << "---deallocate new lower_bound = " << std::dec << lower_bound - Align(buffer->data()) << std::endl;
    }

    bool operator==(const FIFOAllocator& lhs) {
        return this == &lhs;
    }

    bool operator!=(const FIFOAllocator& lhs) {
        return this != &lhs;
    }

    /// Get the max allocation size
    size_t max_size() const {
        return size_t(-1);
    }

private:
    template<class T>
    T Align(T ptr) const {
        return Align(ptr, typename std::conditional<(Alignment > 1),std::true_type,std::false_type>::type());
    }

    template<class T>
    T Align(T ptr, std::true_type) const {
        return reinterpret_cast<T>((reinterpret_cast<size_t>(ptr) | (Alignment - 1)) + 1);
    }

    template<class T>
    T Align(T ptr, std::false_type) const {
        return ptr;
    }

    std::unique_ptr<FIFOAllocator> next_buffer;
    std::vector<char> buffer;
    char* buffer_start;
    char* buffer_end;
    char* last_address;
    char* lower_bound;
    char* upper_bound;
};

} // memory
} // trillek

#endif // FIFOALLOCATOR_HPP_INCLUDED

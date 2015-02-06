#ifndef STREAM_ALLOCATOR_HPP_INCLUDED
#define STREAM_ALLOCATOR_HPP_INCLUDED

namespace trillek { namespace memory {

/** \brief FIFO allocator in a contiguous segment
 *
 * This allocator must be wrapped in TrillekAllocator.
 *
 * This class sequentially allocate memory in a fixed size buffer.
 * A sliding window is moved forward during time and is moved when reaching the end of the buffer.
 * Deallocations are asynchronous: they are cumulated and actually performed when gc() is called.
 *
 * Since it is a FIFO, gc() may be called only when deallocate() was called for every block preceeding
 * the most recent block deallocated.
 *
 * for example:
 *  TrillekAllocator<Widget> alloc_widget(fifo);
 *  TrillekAllocator<Gadget> alloc_gadget(fifo);
 *  auto a = std::allocate_shared<Widget>(alloc_widget);
 *  auto b = std::allocate_shared<Gadget>(alloc_gadget);
 *  b.reset();  // deallocate b (call destructor)
 *  // gc(); incorrect since 'a' is not deallocated.
 *  a.reset();  // deallocate a (call destructor)
 *  fifo.gc(); // actually deallocates a and b.
 *
 * the buffer of data is available by calling data().
 *
 * NOTE FOR CONTAINERS
 *
 * Only the most recent container can be filled using this allocator.
 * Capacity must be reserved during construction or just after.
 * Dynamic resizing is not supported.
 * Move constructor and assignment are disabled.
 * Copy is allowed, but the allocator is not propagated during assignment. Implementers must copy the allocator manually.
 *
 * Containers that are not the most recent are in undefined states.
 *
 * Typical usage with containers is to construct a container as smart pointer, reserve the exact capacity,
 * fill it with data (More containers can be added by repeating these steps) and access the buffer.
 *
 * When the containers are no more used, reset the smart pointers to deallocate memory in the order you created them.
 *
 * size: size of the buffer
 */
template<size_t size>
class StreamAllocator {
public:
    // 'const' qualified to have a build-time error when trying to move a container
    typedef StreamAllocator* const self_raw_pointer_type;

    // propagating the container on copy is disabled because of the 'const' qualifier above: we do not want to
    // trigger a build-time error here.
    typedef std::false_type propagate_on_container_copy_assignment;
    // propagating the container on move are enabled to trigger a build-time error.
    typedef std::true_type propagate_on_container_move_assignment;
    typedef std::true_type propagate_on_container_swap;

    StreamAllocator() : deallocated_size(0) {
        buffer.reserve(size);
        buffer_start = buffer.data();
        lower_bound = buffer_start;
        upper_bound = lower_bound;
        buffer_end = lower_bound + size;
    }

    // Copy
    StreamAllocator(const StreamAllocator& other) :
        buffer(other.buffer), deallocated_size(other.deallocated_size) {
        buffer.reserve(size);
        buffer_start = this->buffer.data();
        lower_bound = this->buffer_start;
        upper_bound = this->lower_bound;
        buffer_end = this->lower_bound + size;
    }

    // Move
    StreamAllocator(StreamAllocator&& other) NOEXCEPT = default;

    // Copy and move assignment, no allocation since size is the same
    StreamAllocator& operator=(StreamAllocator other) NOEXCEPT {
        swap(*this,other);
        return *this;
    }

    void swap(StreamAllocator& other) NOEXCEPT {
        using std::swap;
        swap(this->buffer, other.buffer);
        swap(this->buffer_start, other.buffer_start);
        swap(this->lower_bound, other.lower_bound);
        swap(this->upper_bound, other.upper_bound);
        swap(this->buffer_end, other.buffer_end);
        swap(this->deallocated_size, other.deallocated_size);
    }

    void* allocate(size_t n) {
        std::cout << "   >>> allocate lower_bound = " << std::dec << lower_bound - buffer_start << std::endl;
        std::cout << "   >>> allocate upper_bound = " << std::dec << upper_bound - buffer_start << std::endl;
        std::cout << "+++allocate n = " << std::dec << n << std::endl;
        auto new_upper_bound = upper_bound + n;
        std::cout << "+++allocate new upper_bound = " << std::dec << new_upper_bound - buffer_start << std::endl;
        if ( n > size || new_upper_bound < upper_bound) {
            // integer overflow or buffer too small
            throw std::bad_alloc();
        }
        void* ret;
        if (new_upper_bound > buffer_end) {
            // above the buffer end, move the data at the beginning of the buffer
            size_t data_size = upper_bound - lower_bound;
            std::memmove(buffer_start, lower_bound, data_size);
            lower_bound = buffer_start;
            upper_bound = buffer_start + data_size;
            new_upper_bound = upper_bound + n;
            if (new_upper_bound > buffer_end) {
                throw std::bad_alloc();
            }
           std::cout << "+++allocate copy to start, lower_bound = " << std::dec << lower_bound - buffer_start << std::endl;
           std::cout << "+++allocate copy to start, lower_bound = " << std::hex << (void*)lower_bound << std::endl;
           std::cout << "+++allocate copy to start, upper_bound = " << std::dec << upper_bound - buffer_start << std::endl;
           std::cout << "+++allocate copy to start, upper_bound = " << std::hex << (void*)upper_bound << std::endl;
           std::cout << "+++allocate copy to start, size = " << std::dec << data_size << std::endl;
        }
        ret = upper_bound;
        upper_bound = new_upper_bound;
        std::cout << "   OOO return " << std::hex << static_cast<void*>(ret) << std::endl;
        return ret;
    }

    void deallocate(void* p, size_t n) {
        if (p < buffer_start || p >= buffer_end) {
            LOGMSG(ERROR) << "StreamAllocator: Trying to deallocate a bad pointer.";
            return;
        }
        deallocated_size += n;
    }

    void gc() {
        std::cout << "   >>> deallocate lower_bound = " << std::dec << lower_bound - buffer_start << std::endl;
        std::cout << "   >>> deallocate upper_bound = " << std::dec << upper_bound - buffer_start << std::endl;
        std::cout << "---deallocate n = " << std::dec << deallocated_size << std::endl;
        auto new_lower_bound = lower_bound + deallocated_size;
        std::cout << "   >>> allocate new lower_bound = " << std::dec << new_lower_bound - buffer_start << std::endl;
        assert(new_lower_bound <= buffer_end);
        if (new_lower_bound > lower_bound) {
            lower_bound = new_lower_bound;
        }
        if (lower_bound >= upper_bound) {
            lower_bound = buffer_start;
            upper_bound = lower_bound;
        }
        deallocated_size = 0;
        std::cout << "---deallocate new lower_bound = " << std::dec << lower_bound - buffer_start << std::endl;
    }

    std::pair<char*,size_t> data() {
        return std::make_pair(lower_bound, upper_bound - lower_bound);
    }

    bool operator==(const StreamAllocator& lhs) {
        return this == &lhs;
    }

    bool operator!=(const StreamAllocator& lhs) {
        return this != &lhs;
    }

    /// Get the max allocation size
    size_t max_size() const {
        return size;
    }

private:
    std::vector<char> buffer;
    char* buffer_start;
    char* buffer_end;
    char* lower_bound;
    char* upper_bound;
    size_t deallocated_size;
};

} // memory
} // trillek

#endif // STREAM_ALLOCATOR_HPP_INCLUDED

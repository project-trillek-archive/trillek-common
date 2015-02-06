#ifndef UDP_RELIABLE_MESSAGE_H_INCLUDED
#define UDP_RELIABLE_MESSAGE_H_INCLUDED

#include "controllers/network/message.hpp"
#include "memory/stream-allocator.hpp"

// the size of the buffer to send reliable messages
#define UDP_RELIABLE_BUFFER_SIZE            2048L

namespace trillek { namespace network {

class UDPReliableMessage final : public Message {
public:
    typedef memory::StreamAllocator<UDP_RELIABLE_BUFFER_SIZE> raw_allocator_type;
    typedef TrillekAllocator<char,raw_allocator_type> allocator_type;
    typedef std::vector<char,allocator_type> vector_type;

    UDPReliableMessage(vector_type buffer, size_t size, const ConnectionData* cnxd = nullptr, int fd = -1)
            : Message(buffer.data(), size, cnxd), buffer(std::move(buffer)) {}

    ~UDPReliableMessage() {}
#if 0
     /** \brief Send a message to a client using UDP
     *
     * \param id id_t the id of the entity to which the message must be sent
     * \param major unsigned char the major code of the message
     * \param minor unsigned char the minor code of the message
     *
     */
    void Send(id_t id, unsigned char major, unsigned char minor, uint64_t timestamp);
#endif
    /** \brief Send a message to a server using UDP
     *
     * \param major unsigned char the major code of the message
     * \param minor unsigned char the minor code of the message
     *
     */
    void Send(unsigned char major, unsigned char minor, uint64_t timestamp);

    static allocator_type GetAllocator();

private:
    // The buffer
    const vector_type buffer;
};
} // network
} // trillek

#endif // UDP_RELIABLE_MESSAGE_H_INCLUDED

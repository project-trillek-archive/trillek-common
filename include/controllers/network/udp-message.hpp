#ifndef UDP_MESSAGE_HPP_INCLUDED
#define UDP_MESSAGE_HPP_INCLUDED


#include "controllers/network/message.hpp"

namespace trillek { namespace network {

class UDPMessage final : public Message {
public:
    typedef TrillekAllocator<char> allocator_type;
    typedef std::vector<char,allocator_type> vector_type;

    UDPMessage(vector_type&& buffer, size_t size, const ConnectionData* cnxd = nullptr, int fd = -1)
            : Message(buffer.data(), size, cnxd), buffer(std::move(buffer)) {}

    ~UDPMessage() {}
#if 0
     /** \brief Send a message to a client using UDP
     *
     * \param id id_t the id of the entity to which the message must be sent
     * \param major unsigned char the major code of the message
     * \param minor unsigned char the minor code of the message
     *
     */
    virtual void Send(id_t id, unsigned char major, unsigned char minor, uint64_t timestamp) final;
#endif
    /** \brief Send a message to a server using UDP
     *
     * \param major unsigned char the major code of the message
     * \param minor unsigned char the minor code of the message
     *
     */
    virtual void Send(unsigned char major, unsigned char minor, uint64_t timestamp) final;

    static allocator_type GetAllocator() {
        return allocator_type();
    }

private:
    // The pointer on the buffer
    const vector_type buffer;
};
} // network
} // trillek
#endif // UDP_MESSAGE_HPP_INCLUDED

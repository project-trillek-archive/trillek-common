#ifndef TCP_MESSAGE_HPP_INCLUDED
#define TCP_MESSAGE_HPP_INCLUDED

#include "controllers/network/message.hpp"

namespace trillek { namespace network {

class TCPMessage final : public Message {
public:
    typedef TrillekAllocator<char> allocator_type;
    typedef std::vector<char,allocator_type> vector_type;

    TCPMessage(vector_type&& buffer, size_t size, const ConnectionData* cnxd = nullptr, int fd = -1)
            : Message(buffer.data(), size, cnxd), buffer(std::move(buffer)) {}

    ~TCPMessage() {}
#if 0
    /** \brief Send a message to a client using TCP
     *
     * \param id id_t the id of the entity to which the message must be sent
     * \param major unsigned char the major code of the message
     * \param minor unsigned char the minor code of the message
     *
     */
    void Send(id_t id, unsigned char major, unsigned char minor);
#endif
    /** \brief Send a message to a server using TCP
     *
     * \param major unsigned char the major code of the message
     * \param minor unsigned char the minor code of the message
     *
     */
    void Send(unsigned char major, unsigned char minor);

    static allocator_type GetAllocator() {
        return allocator_type();
    }

private:
    // The buffer
    const vector_type buffer;
};
} // network
} // trillek

#endif // TCP_MESSAGE_HPP_INCLUDED

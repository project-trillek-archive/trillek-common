#ifndef MESSAGE_UNAUTHENTICATED_HPP_INCLUDED
#define MESSAGE_UNAUTHENTICATED_HPP_INCLUDED

#include "controllers/network/message.hpp"

namespace trillek { namespace network {

class MessageUnauthenticated : public Message {
public:
    MessageUnauthenticated(const std::shared_ptr<std::vector<char,TrillekAllocator<char>>>& buffer,
                size_t index = 0, size_t size = 0, const ConnectionData* cnxd = nullptr, int fd = -1)
        : Message(buffer, index, size, cnxd), cx_data(cnxd), fd(fd) {}

    /** \brief File Descriptor getter
     *
     * \return the file descriptor
     *
     */
    int FileDescriptor() const { return fd; }

    const ConnectionData* CxData() { return cx_data; }
private:
    const ConnectionData* const cx_data;
    const int fd;
};

} // network
} // trillek
#endif // MESSAGE_UNAUTHENTICATED_HPP_INCLUDED

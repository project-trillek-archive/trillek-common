#ifndef MESSAGE_UNAUTHENTICATED_HPP_INCLUDED
#define MESSAGE_UNAUTHENTICATED_HPP_INCLUDED

#include "controllers/network/message.hpp"

namespace trillek { namespace network {

class MessageUnauthenticated : public Message {
public:
    static std::shared_ptr<MessageUnauthenticated> NewReceivedMessage(size_t size, const ConnectionData* cnxd = nullptr, int fd = -1) {
        char* ptr = TrillekAllocator<char>().allocate(size);
        auto buffer = std::shared_ptr<char>(ptr);
        return std::allocate_shared<MessageUnauthenticated>(TrillekAllocator<MessageUnauthenticated>(), std::move(buffer), size, cnxd, fd);
    }

    MessageUnauthenticated(std::shared_ptr<char>&& buffer, size_t size, const ConnectionData* cnxd, int fd)
            : Message(std::move(buffer), size, cnxd), cx_data(cnxd), fd(fd) {}

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

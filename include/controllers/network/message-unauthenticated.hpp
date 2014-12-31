#ifndef MESSAGE_UNAUTHENTICATED_HPP_INCLUDED
#define MESSAGE_UNAUTHENTICATED_HPP_INCLUDED

#include "controllers/network/message.hpp"

namespace trillek { namespace network {

class MessageUnauthenticated : public Message {
public:
    MessageUnauthenticated(const std::shared_ptr<std::vector<char,TrillekAllocator<char>>>& buffer,
                size_t index, size_t size, const ConnectionData* cnxd, const int fd)
        : Message(buffer, index, size, cnxd, fd), cx_data(cnxd) {}

    const ConnectionData* CxData() { return cx_data; }
private:
    const ConnectionData* const cx_data;
};

} // network
} // trillek
#endif // MESSAGE_UNAUTHENTICATED_HPP_INCLUDED

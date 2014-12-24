#ifndef MESSAGE_UNAUTHENTICATED_HPP_INCLUDED
#define MESSAGE_UNAUTHENTICATED_HPP_INCLUDED

#include "controllers/network/message.hpp"

namespace trillek { namespace network {

class MessageUnauthenticated : public Message {
public:
    MessageUnauthenticated(const ConnectionData* cnxd, const int fd)
        : Message(cnxd, fd), cx_data(cnxd) {}

    const ConnectionData* CxData() { return cx_data; }
private:
    const ConnectionData* const cx_data;
};

} // network
} // trillek
#endif // MESSAGE_UNAUTHENTICATED_HPP_INCLUDED

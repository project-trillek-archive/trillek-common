#ifndef NETWORKNODE_H_INCLUDED
#define NETWORKNODE_H_INCLUDED

#include "map-array.hpp"
#include "controllers/network/TCPConnection.hpp"

namespace trillek { namespace network {
class NetworkNode final {
public:
    NetworkNode() {};
    ~NetworkNode() {};

    static void AddEntity(const id_t id, TCPConnection&& cnx) {
        fd_map[id] = cnx.get_handle();
        connection_map[id] = std::move(cnx);
    }

    static void RemoveEntity(const id_t id) {
        fd_map.clear(id);
        connection_map.clear(id);
    }

    static socket_t GetFileDescriptor(const id_t id) {
        return fd_map.at(id);
    }

private:
    static MapArray<TCPConnection> connection_map;
    static MapArray<socket_t> fd_map;
};
} // network
} // trillek

#endif // NETWORKNODE_H_INCLUDED

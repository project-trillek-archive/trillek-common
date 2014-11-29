#include "composites/network-node.hpp"

namespace trillek { namespace network {
MapArray<TCPConnection> NetworkNode::connection_map;
MapArray<socket_t> NetworkNode::fd_map;
}
}

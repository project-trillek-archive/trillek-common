#ifndef NETWORK_NODE_DATA_HPP_INCLUDED
#define NETWORK_NODE_DATA_HPP_INCLUDED

#include "controllers/network/network_common.hpp"

namespace trillek { namespace network {

class NetworkNodeData {
public:
    NetworkNodeData(NetworkAddress remote) :
        _addr(std::move(remote)),
        _id(0) {}

    NetworkNodeData(const id_t id,
        std::function<bool(const unsigned char*,const unsigned char*,size_t,uint64_t)>&& verifier,
        NetworkAddress remote)
        : _id(id), _verifier(std::move(verifier)), _addr(std::move(remote)) {}

    /** \brief Return the verifier associated to this socket
     *
     * \return const std::function<bool(const unsigned char*,const unsigned char*,size_t)>* const
     *
     */
    const std::function<bool(const unsigned char*,const unsigned char*,size_t,uint64_t)>& Verifier() const {
        return _verifier;
    }

    /** \brief Return the id of the entity to which this socket is attached
     *
     * \return id_t the id
     *
     */
    id_t Id() const { return _id; }

    const NetworkAddress& GetRemoteAddress() const { return _addr; }

private:
    const NetworkAddress _addr;
    const id_t _id;
    const std::function<bool(const unsigned char*,const unsigned char*,size_t,uint64_t)> _verifier;
};
} // network
} // trillek
#endif // NETWORK_NODE_DATA_HPP_INCLUDED

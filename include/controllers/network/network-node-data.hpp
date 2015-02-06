#ifndef NETWORK_NODE_DATA_HPP_INCLUDED
#define NETWORK_NODE_DATA_HPP_INCLUDED

#include "controllers/network/network_common.hpp"
#include "memory/stream-allocator.hpp"

namespace trillek { namespace network {

class NetworkNodeData {
public:
    // constructor used by the client in connect() and by the server with new connections
    NetworkNodeData(NetworkAddress remote, uint64_t timestamp) :
        _addr(std::move(remote)),
        _id(0),
        _timestamp(timestamp) {}

    // constructor used by the authentication handler on server side to set cnx_data
    NetworkNodeData(const id_t id,
        std::function<bool(const unsigned char*,const unsigned char*,size_t,uint64_t)>&& verifier,
        NetworkAddress remote, uint64_t timestamp)
        : _id(id), _verifier(std::move(verifier)), _addr(std::move(remote)), _timestamp(timestamp),
        udp_counter(0) {}

    // constructor used by the authentication handler on server side to set the Networknode component
    NetworkNodeData(const id_t id,
        std::function<bool(const unsigned char*,const unsigned char*,size_t,uint64_t)>&& verifier,
        std::function<void(unsigned char*,const unsigned char*,size_t,uint64_t)>&& hasher_udp,
        std::function<void(unsigned char*,const unsigned char*,size_t,uint64_t)>&& hasher_tcp,
            NetworkAddress remote, uint64_t timestamp)
        : _id(id), _verifier(std::move(verifier)), _hasher_udp(std::move(hasher_udp)), _hasher_tcp(std::move(hasher_tcp)),
            _addr(std::move(remote)), _timestamp(timestamp), udp_counter(0),
             udp_reliable_buffer(make_unique<memory::StreamAllocator<4096>>()) {}

    /** \brief Return the verifier associated to this socket
     *
     * \return const std::function<bool(const unsigned char*,const unsigned char*,size_t)>* const
     *
     */
    const std::function<bool(const unsigned char*,const unsigned char*,size_t,uint64_t)>& Verifier() const {
        return _verifier;
    }

    /** \brief Return the UDP hasher associated to this socket
     *
     * \return const std::function<void(unsigned char*,const unsigned char*,size_t,uint64_t)>* const
     *
     */
    const std::function<void(unsigned char*,const unsigned char*,size_t,uint64_t)>& Hasher_UDP() const {
        return _hasher_udp;
    }

    /** \brief Return the TCP hasher associated to this socket
     *
     * \return const std::function<void(unsigned char*,const unsigned char*,size_t,uint64_t)>* const
     *
     */
    const std::function<void(unsigned char*,const unsigned char*,size_t,uint64_t)>& Hasher_TCP() const {
        return _hasher_tcp;
    }

    /** \brief Return the id of the entity to which this socket is attached
     *
     * \return id_t the id
     *
     */
    id_t Id() const { return _id; }

    /** \brief Get the timestamp of the message
     *
     * \return uint64_t the timestamp
     *
     */
    uint64_t Timestamp() const { return _timestamp; }

    /** \brief Increment and return the counter value
     *
     * \return std::uint_least16_t the counter
     *
     */
    std::uint_least16_t UDPCounter() {
        return ++udp_counter;
    }

    memory::StreamAllocator<4096>* UDPReliableBuffer() { return udp_reliable_buffer.get(); }

    const NetworkAddress& GetRemoteAddress() const { return _addr; }

private:
    std::unique_ptr<memory::StreamAllocator<4096>> udp_reliable_buffer;
    const NetworkAddress _addr;
    const std::function<bool(const unsigned char*,const unsigned char*,size_t,uint64_t)> _verifier;
    const std::function<void(unsigned char*,const unsigned char*,size_t,uint64_t)> _hasher_udp;
    const std::function<void(unsigned char*,const unsigned char*,size_t,uint64_t)> _hasher_tcp;
    const uint64_t _timestamp;
    const id_t _id;
    uint_least16_t udp_counter;
};
} // network
} // trillek
#endif // NETWORK_NODE_DATA_HPP_INCLUDED

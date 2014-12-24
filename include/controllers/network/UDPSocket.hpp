#pragma once

#include "controllers/network/network_common.hpp"


namespace trillek { namespace network {
    /** \brief Adapted from Netport
     *
     * see https://github.com/Meisaka/NetPort/
     */
	class UDPSocket {
	public:
		UDPSocket();
		~UDPSocket();

		bool init(ADDRTYPE af);
		bool bind(NetworkAddress &);

		bool is_bound() const;

		int send_to(const NetworkAddress &, const char *, size_t);
		int send_to(const NetworkAddress &, const std::string &);
		int recv_from(NetworkAddress &, char *, size_t);
		int recv_from(NetworkAddress &, std::string &);
        bool connect(const NetworkAddress &remote);
		socket_t get_handle() const { return handle; }
	private:
		bool bound;
		NetworkAddress laddr;
		socket_t handle;
	};
} // network
} // trillek

#pragma once

#include "controllers/network/network_common.hpp"

namespace trillek { namespace network {
	class TCPConnection final {
	public:
		TCPConnection(void);
		TCPConnection(const TCPConnection&) = delete;
		TCPConnection & operator=(const TCPConnection&) = delete;
		TCPConnection(TCPConnection &&);
		TCPConnection & operator=(TCPConnection &&);
		~TCPConnection(void);

		TCPConnection(socket_t, int); // depricated

		// these are more for specialized / internal use
		// existing connected socket
		TCPConnection(socket_t, const NetworkAddress &la, const NetworkAddress &ra);

		bool init(ADDRTYPE); // new tcp socket (unbound)
		bool bind(const NetworkAddress &); // bind a socket
		bool listen(int); // listen on socket

		bool init(const NetworkAddress &); // new tcp socket (unbound)
		bool connect(const NetworkAddress &); // connect, makes new socket if none exists
		bool listen(const NetworkAddress &, int); // bind and listen

		/* Enable or disable TCP keepalive */
		bool set_keepalive(bool);

		/* Enable or disable TCP keepalives and set options
		* options:
		* enable
		* time - before sending keepalives (seconds)
		* interval - time between keepalives (seconds)
		* probes - maximum number of keepalive to send before dropping the connection (Linux specific)
		*/
		bool set_keepalive(bool, unsigned long, unsigned long, unsigned long);

		// true disables nagle
		bool set_nodelay(bool enable);

		// accepts new connections
		TCPConnection accept();

		int send(const char *, int);
		int send(const std::string &);
		int recv(char *, int);
		int recv(std::string &, int);

		// address of remote (connected sockets)
		const NetworkAddress& remote() const { return raddr; }
		socket_t get_handle() const { return handle; }

		operator bool() const;
	private:
		NetworkAddress laddr;
		NetworkAddress raddr;
		socket_t handle;
	};
} // network
} // trillek

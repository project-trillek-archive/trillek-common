
#include "controllers/network/network_os.h"
#include "controllers/network/UDPSocket.hpp"

namespace trillek { namespace network {
	UDPSocket::UDPSocket()  : bound(false) { }
	UDPSocket::~UDPSocket() {
	}

	bool UDPSocket::init(ADDRTYPE afn) {
		if(handle) {
			close(handle);
		}
		this->laddr.af = afn;
		socket_t sc;
		sc = socket(afn, SOCK_DGRAM, IPPROTO_UDP);
		if(INVALID_SOCKET == sc) {
			return false;
		}
		handle = sc;
		return true;
	}

	bool UDPSocket::bind(NetworkAddress & local) {
		if(!handle) { return false; }
		long v = 1;
		if(setsockopt(handle, SOL_SOCKET, SO_REUSEADDR, (char*)&v, sizeof(long))) {
			return false;
		}
		if(::bind(handle, (struct sockaddr*)&local.addr, local.length())) {
			return false;
		}
		laddr = local;
		bound = true;
		return true;
	}
	bool UDPSocket::is_bound() const {
		return this->bound;
	}

	int UDPSocket::send_to(const NetworkAddress & r, const char * b, size_t l) {
		if(!handle) { return -1; }
		return sendto(handle, b, l, 0, (struct sockaddr*)&r.addr, r.length());
	}
	int UDPSocket::send_to(const NetworkAddress & r, const std::string & s) {
		if(!handle) { return -1; }
		return sendto(handle, s.data(), s.length(), 0, (struct sockaddr*)&r.addr, r.length());
	}
	int UDPSocket::recv_from(NetworkAddress & r, char * b, size_t l) {
		if(!handle) { return -1; }
		r.af = this->laddr.af;
		socklen_t x = r.length();
		int i = recvfrom(handle, b, l, 0, (struct sockaddr*)&r.addr, &x);
		if(i < 0) {
#ifdef WIN32
			if(WSAGetLastError() == WSAEWOULDBLOCK) {
#else
			if(errno == EAGAIN || errno == EWOULDBLOCK) {
#endif
				return 0;
			}
#ifdef WIN32
			if(WSAGetLastError() == WSAEMSGSIZE) {
				return l;
			}
#endif
		}
		return i;
	}
	int UDPSocket::recv_from(NetworkAddress & r, std::string & s) {
		if(!handle) { return -1; }
		r.af = this->laddr.af;
		socklen_t x = r.length();
		char f[9000]; // MTU of most networks is limited to 1500, so this should be enough space ;)
		int i = recvfrom(handle, f, 9000, 0, (struct sockaddr*)&r.addr, &x);
		if(i > 0) {
			s.assign(f, i);
		} else if(i < 0) {
#ifdef WIN32
			if(WSAGetLastError() == WSAEWOULDBLOCK) {
#else
			if(errno == EAGAIN || errno == EWOULDBLOCK) {
#endif
				return 0;
			}
#ifdef WIN32
			if(WSAGetLastError() == WSAEMSGSIZE) {
				return 9000;
			}
#endif
		}
		return i;
	}

    bool UDPSocket::connect(const NetworkAddress &remote)
	{
		if(!handle) {
			return false;
		}
		if(::connect(handle, (struct sockaddr*)&remote.addr, remote.length())) {
			return false;
		}
		return true;
	}

} // network
} // trillek

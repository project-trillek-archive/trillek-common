#include "controllers/network/TCPConnection.hpp"
#include "controllers/network/network_os.h"

namespace trillek { namespace network {
	TCPConnection::TCPConnection(void) {}

	TCPConnection::TCPConnection(socket_t hndl, int afn)
		: handle(hndl)
	{
	}

	TCPConnection::TCPConnection(TCPConnection && rc)
	{
        handle = rc.handle;
		laddr = rc.laddr;
		raddr = rc.raddr;
	}
	TCPConnection & TCPConnection::operator=(TCPConnection && rc)
	{
        handle = rc.handle;
		laddr = rc.laddr;
		raddr = rc.raddr;
		return *this;
	}

	TCPConnection::TCPConnection(socket_t hndl, const NetworkAddress &la, const NetworkAddress &ra)
		: handle(hndl)
	{
		laddr = la;
		raddr = ra;
	}

	bool TCPConnection::set_nodelay(bool enable)
	{
		unsigned long i = (enable ? 1 : 0);
		if(handle == INVALID_SOCKET) { return false; }
		//if(setsockopt(handle, SOL_SOCKET, TCP_NODELAY, (char*)&i, sizeof(unsigned long))) {
		if(setsockopt(handle, IPPROTO_TCP, TCP_NODELAY, (char*)&i, sizeof(unsigned long))) {
			return false;
		}
		return true;
	}

	bool TCPConnection::set_keepalive(bool enable)
	{
		unsigned long i = (enable ? 1 : 0);
		if(handle == INVALID_SOCKET) { return false; }
		if(setsockopt(handle, SOL_SOCKET, SO_KEEPALIVE, (char*)&i, sizeof(unsigned long))) {
			return false;
		}
		return true;
	}

	bool TCPConnection::set_keepalive(bool enable, unsigned long time, unsigned long intvl, unsigned long probes)
	{
		unsigned long i = (enable ? 1 : 0);
		if(handle == INVALID_SOCKET) { return false; }
		if(setsockopt(handle, SOL_SOCKET, SO_KEEPALIVE, (char*)&i, sizeof(unsigned long))) {
			return false;
		}
#ifdef WIN32
		tcp_keepalive tcpka_set;
		tcpka_set.onoff = i;
		// these values are expected in milliseconds
		tcpka_set.keepaliveinterval = intvl * 1000;
		tcpka_set.keepalivetime = time * 1000;
		// probes is not settable on Windows

		if(WSAIoctl(handle, SIO_KEEPALIVE_VALS, &tcpka_set, sizeof(tcp_keepalive), 0, 0, 0, 0, 0)) {
			return false;
		}
#else
#ifdef LINUX
		// These options are Linux specific (not portable)
		if(setsockopt(handle, SOL_TCP, TCP_KEEPIDLE, &time, sizeof(unsigned long))) {
			return false;
		}
		if(setsockopt(handle, SOL_TCP, TCP_KEEPCNT, &probes, sizeof(unsigned long))) {
			return false;
		}
		if(setsockopt(handle, SOL_TCP, TCP_KEEPINTVL, &intvl, sizeof(unsigned long))) {
			return false;
		}
#endif
#endif
		return true;
	}

	int TCPConnection::send(const char * buf, int buflen)
	{
		int i;
		if(handle == INVALID_SOCKET) { return -1; }
		i = ::send(handle, buf, buflen, 0);
		if(i < 0) {
			close(handle);
		}
		return i;
	}
	int TCPConnection::send(const std::string &s)
	{
		int i;
		if(handle == INVALID_SOCKET) { return -1; }
		i = ::send(handle, s.data(), s.length(), 0);
		if(i < 0) {
			close(handle);
		}
		return i;
	}

	int TCPConnection::recv(char * buf, int buflen)
	{
		int i;
		if(handle == INVALID_SOCKET) { return -1; }
		i = network::recv(handle, buf, buflen);
		if(i < 0) {
			close(handle);
		}
		return i;
	}
	int TCPConnection::recv(std::string &s, int buflen)
	{
		if(handle == INVALID_SOCKET) { return -1; }
		char *h = new char[buflen];
		int i = TCPConnection::recv(h, buflen);
		if(i > 0) {
			s.assign(h, i);
		}
		delete h;
		return i;
	}

	TCPConnection TCPConnection::accept()
	{
		TCPConnection rv;
		socket_t h;
		socklen_t sas = sizeof(sockaddr);
		address radd;
		h = ::accept(this->handle, (struct sockaddr*)&radd.addr, &sas);
		if(h == INVALID_SOCKET) {
			return rv;
		}
		return TCPConnection(h, this->laddr, radd);
	}

	bool TCPConnection::init(ADDRTYPE afn)
	{
		if(handle != INVALID_SOCKET) {
			close(handle);
		}
		this->laddr.af = afn;
		socket_t sc;
		sc = socket(afn, SOCK_STREAM, IPPROTO_TCP);
		if(INVALID_SOCKET == sc) {
			return false;
		}

		handle = sc;
		return true;
	}
	bool TCPConnection::init(const NetworkAddress &type)
	{
		if(handle != INVALID_SOCKET) {
			close(handle);
		}
		this->laddr.af = type.af;
		socket_t sc;
		sc = socket(type.af, SOCK_STREAM, IPPROTO_TCP);
		if(INVALID_SOCKET == sc) {
			return false;
		}

		handle = sc;
		return true;
	}

	bool TCPConnection::bind(const NetworkAddress &local)
	{
		if(INVALID_SOCKET == handle) {
			if(!init(local.af)) {
				return false;
			}
		}
		long v = 1;
		if(setsockopt(handle, SOL_SOCKET, SO_REUSEADDR, (char*)&v, sizeof(long))) {
			return false;
		}
		if(::bind(handle, (struct sockaddr*)&local.addr, local.length())) {
			return false;
		}
		laddr = local;
		return true;
	}
	bool TCPConnection::connect(const NetworkAddress &remote)
	{
		if(handle == INVALID_SOCKET) {
			if(!init(remote.af)) {
				return false;
			}
		}
		long v = 1;
		if(setsockopt(handle, SOL_SOCKET, SO_REUSEADDR, (char*)&v, sizeof(long))) {
			return false;
		}
		if(::connect(handle, (struct sockaddr*)&remote.addr, remote.length())) {
			return false;
		}
		raddr = remote;
		return true;
	}

	bool TCPConnection::listen(const NetworkAddress &local, int queue)
	{
		if(handle == INVALID_SOCKET) {
			return false;
		}
		if(::listen(handle, queue)) {
			return false;
		}
		return true;
	}

	bool TCPConnection::listen(int queue)
	{
		if(handle == INVALID_SOCKET) { return false; }
		if(::listen(handle, queue)) {
			return false;
		}
		return true;
	}

    TCPConnection::operator bool() const {
        return handle != INVALID_SOCKET;
    }



	TCPConnection::~TCPConnection(void)
	{
	}
} // namespace network
} // namespace trillek

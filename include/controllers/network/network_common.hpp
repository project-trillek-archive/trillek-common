#ifndef INCL_NETWORKCOM
#define INCL_NETWORKCOM

#include <string>

namespace trillek { namespace network {
#ifdef WIN32
#if defined(_WIN64)
	typedef unsigned __int64 socket_t;
#else
	typedef unsigned int socket_t;
#endif
#else
	typedef int socket_t;
#endif
	enum ADDRTYPE : unsigned short {
		NETA_UNDEF= 0,
		NETA_IPv4 = 2,
		NETA_IPv6 = 23,
	};

	struct net_sockaddr {
		unsigned short sa_family;
		char sa_data[48];
	};

	/* send data to a native socket */
	int send(socket_t handle, const char *, int);
	/* receive data from a native socket */
	int recv(socket_t handle, char *, int);
	/* close a native socket handle */
	void close(socket_t &handle);

    void set_nonblocking(socket_t handle, bool enable);

	typedef struct NetworkAddress {
	public:
		struct net_sockaddr addr;
		ADDRTYPE af;
		int type;
		int proto;
		NetworkAddress();
		NetworkAddress(const std::string &host);
		NetworkAddress(const std::string &host, unsigned short p);
		NetworkAddress(const std::string &host, const std::string &service);

		void resolve(const std::string &host);
		void resolve(const std::string &host, const std::string &service);
		void port(unsigned short p);
		void ip4(const char *);
		void ip4(const std::string &);
		void ip4(const char *, unsigned short p);
		void ip4(const std::string &, unsigned short p);
		void ip4(unsigned char i1, unsigned char i2, unsigned char i3, unsigned char i4);
		void ip4(unsigned long i);

		void ip6(const std::string &);
		void ip6(const std::string &, unsigned short p);
		std::string to_string() const;
		int length() const;
	} address;

} // network
} // trillek
#endif

#ifndef MESSAGE_H_INCLUDED
#define MESSAGE_H_INCLUDED

#include <cstdint>
#include <memory>
#include <vector>
#include <cstring>
#include <assert.h>
#include "controllers/network/packet-handler.hpp"
#include "controllers/network/network_common.hpp"
#include "trillek.hpp"
#include "logging.hpp"

// size of the VMAc tag
#define VMAC_SIZE			8
// size of the ESIGN tag
#define ESIGN_SIZE          24
// size of the counter
#define COUNTER_SIZE        4

// TODO : Convert integers to Big Endian before sending them to the network

// Version MAJOR codes
// unauthenticated messages: everyone can send them
#define TEST_MSG			0		// Reserved
#define NET_MSG 		1		// low level messages for authentication and disconnect
#define SERVER_MSG		2		// Server public information
#define PUBPLAYER_MSG	3		// Player public profile
#define	DLBINARY_MSG	4		// for binary download
#define	ASSETS_MSG		5		// for public assets download
#define	RESERVED_6		6
#define	RESERVED_7		7
// authenticated messages: only authenticated users can send and receive this
#define	PROFILE_MSG		8		// player private profile
#define WORLD_MSG		9		// world data
#define GAME_MSG		10		// game data
#define SOCIAL_MSG		11		// chat/mail
#define PASSETS_MSG		11		// private assets
#define CPU_MSG			12		// for CPU...
// Feel free to add or modify this list

// Minor codes
// TEST_MSG
#define TEST_MSG_TCP 0
#define TEST_MSG_UDP 1
#define UDP_ECHO 2

#define IS_RESTRICTED(x)	((x >> 3) != 0)

namespace trillek { namespace network {

class ConnectionData;
class NetworkNodeData;

/** \brief The header of the message, without the preceding length
 */
// Prevent insertion of padding bytes
#pragma pack(push)
#pragma pack(1)
struct msg_hdr {
    uint16_t flags;
    unsigned char type_major;
    unsigned char type_minor;
    uint64_t timestamp;
};
#pragma pack(pop)

struct msg_tail_stoc {
    unsigned char tag[VMAC_SIZE];
};

/** \brief The header of the frame, i.e the length
 */
struct Frame_hdr {
    uint32_t length;
};

/** \brief The header including length
 */
// Prevent insertion of padding bytes
#pragma pack(push)
#pragma pack(1)
struct Frame {
    Frame_hdr fheader;
    msg_hdr mheader;
};
#pragma pack(pop)

class Message {
public:
    friend class Authentication;
    friend class NetworkController;
    friend class Frame_req;
    friend class TrillekAllocator<Message>;
    friend void packet_handler::PacketHandler::Process<NET_MSG,5>() const;

    static std::shared_ptr<Message> NewUDPReliableMessage(id_t id, size_t size) {
        return NewTCPMessage(id, size);
    }

    static std::shared_ptr<Message> NewUDPMessage(id_t id, size_t size) {
        return NewTCPMessage(id, size);
    }

    static std::shared_ptr<Message> NewTCPMessage(id_t id, size_t size) {
        char* ptr = TrillekAllocator<char>().allocate(size);
        auto buffer = std::shared_ptr<char>(ptr);
        return std::allocate_shared<Message,TrillekAllocator<Message>>(TrillekAllocator<Message>(), std::move(buffer), size);
    }

    virtual ~Message() {}

    // Copy constructor and assignment are deleted to remain zero-copy
    Message(Message&) = delete;
    Message& operator=(Message&) = delete;

    // Move constructor
    Message(Message&& m) :
        data(std::move(m.data)),
        data_size(std::move(m.data_size)),
        index (std::move(m.index)),
        node_data (std::move(m.node_data)) {};

    template<class U>
    Message& operator<<(const U& in) {
        append(&in, sizeof(U));
        return *this;
    }

    /** \brief Send a message to a client using UDP
     *
     * \param id id_t the id of the entity to which the message must be sent
     * \param major unsigned char the major code of the message
     * \param minor unsigned char the minor code of the message
     *
     */
    void SendUDP(id_t id, unsigned char major, unsigned char minor, uint64_t timestamp);

    /** \brief Send a message to a client using TCP
     *
     * \param id id_t the id of the entity to which the message must be sent
     * \param major unsigned char the major code of the message
     * \param minor unsigned char the minor code of the message
     *
     */
    void SendTCP(id_t id, unsigned char major, unsigned char minor);

    /** \brief Send a message to a server using TCP
     *
     * \param major unsigned char the major code of the message
     * \param minor unsigned char the minor code of the message
     *
     */
    void SendTCP(unsigned char major, unsigned char minor);

    /** \brief Send a message to a server using UDP
     *
     * \param major unsigned char the major code of the message
     * \param minor unsigned char the minor code of the message
     *
     */
    void SendUDP(unsigned char major, unsigned char minor, uint64_t timestamp);

    /** \brief Maps the body of the message to the structure of
     * the packet of type T
     *
     * The index is updated to point to the byte after T. There is no dynamic
     * allocation of buffer
     *
     * \return U* pointer to the packet T
     *
     */
    template<class V, class U=V>
    U* Content() {
        Resize(sizeof(V));
        return reinterpret_cast<U*>(Body());
    }

    /** \brief Return the entity id attached to this message
     *
     * \return id_t the id
     *
     */
    id_t GetId() const;

    /** \brief Get the timestamp of the message
     *
     * \return uint64_t the timestamp
     *
     */
    uint64_t Timestamp() { return Header()->timestamp; }

protected:
    /** \brief Constructor
     *
     * \param buffer the buffer where to put the data
     * \param index the index in the buffer where to start to write data
     * \param size the number of bytes allocated in the buffer for this message
     * \param cnxd (internal) an object used by the network
     * \param fd (internal) the file descriptor
     *
     */
    Message(std::shared_ptr<char>&& buffer, size_t size, const ConnectionData* cnxd = nullptr);

private:
    static std::shared_ptr<Message> NewReceivedMessage(size_t size, const ConnectionData* cnxd = nullptr, int fd = -1) {
        char* ptr = TrillekAllocator<char>().allocate(size);
        auto buffer = std::shared_ptr<char>(ptr);
        return std::allocate_shared<Message>(TrillekAllocator<Message>(), std::move(buffer), size, cnxd);
    }

    /** \brief Send data to the network using a socket (client version)
     *
     * \param fd the socket to use
     * \param major the major code
     * \param minor the minor code
     * \param the cryptotag generator
     * \param tagptr pointer on the tag
     * \param tag_size size of the cryptotag
     *
     */
    void Send(int fd, unsigned char major, unsigned char minor,
        const std::function<void(unsigned char*,const unsigned char*,size_t,uint64_t)>& hasher,
        unsigned char* tagptr,
        unsigned int tag_size);

    /** \brief Send data to the network using a network address (client version)
     *
     * \param address the address to use
     * \param major the major code
     * \param minor the minor code
     * \param the cryptotag generator
     * \param tagptr pointer on the tag
     * \param tag_size size of the cryptotag
     *
     */
    void Send(const NetworkAddress& address, unsigned char major, unsigned char minor,
        const std::function<void(unsigned char*,const unsigned char*,size_t,uint64_t)>& hasher,
        unsigned char* tagptr,
        unsigned int tag_size);

    /** \brief Send data to the network using a socket (server version)
     *
     * \param fd the socket to use
     * \param major the major code
     * \param minor the minor code
     * \param the cryptotag generator
     * \param tagptr pointer on the tag
     * \param tag_size size of the cryptotag
     *
     */
    void Send(int fd, unsigned char major, unsigned char minor,
        const std::function<void(unsigned char*,const unsigned char*,size_t)>& hasher,
        unsigned char* tagptr,
        unsigned int tag_size);

    /** \brief Send data to the network using a network address (server version)
     *
     * \param address the address to use
     * \param major the major code
     * \param minor the minor code
     * \param the cryptotag generator
     * \param tagptr pointer on the tag
     * \param tag_size size of the cryptotag
     *
     */
    void Send(const NetworkAddress& address, unsigned char major, unsigned char minor,
        const std::function<void(unsigned char*,const unsigned char*,size_t)>& hasher,
        unsigned char* tagptr,
        unsigned int tag_size);

    /** \brief Send data to the network using a socket
     *
     * The packet is not authentified.
     *
     * \param fd the socket to use
     * \param major the major code
     * \param minor the minor code
     *
     */
    void SendMessageNoVMAC(int fd, uint8_t major, uint8_t minor) {
        if(fd < 0) {
            return;
        }
        auto header = Header();
        header->type_major = major;
        header->type_minor = minor;
        FrameHeader()->length = index - sizeof(Frame_hdr);

        if (send(fd, reinterpret_cast<char*>(FrameHeader()), index) <= 0) {
            LOGMSG(ERROR) << "could not send frame with no tag to fd = " << fd ;
        }

    }

    /** \brief Return a pointer on the header of the message
     *
     * \return msg_hdr* the pointer on msg_hdr
     *
     */
    msg_hdr* Header() {
        return reinterpret_cast<msg_hdr*>(data.get() + sizeof(Frame_hdr));
    }

    /** \brief Return a pointer on the frame, beginning at the length field
     *
     * \return Frame_hdr* the pointer
     *
     */
    Frame_hdr* FrameHeader() {
        return reinterpret_cast<Frame_hdr*>(data.get());
    }

    /** \brief Update the index to remove the VMAC tag
     *
     */
    void RemoveVMACTag() { index -= VMAC_SIZE; };


    /** \brief Update the index to remove the tail of the message (client only)
     *
     */
    void RemoveTailClient() { index -= sizeof(msg_tail_stoc); };

    /** \brief Return a pointer on the body, i.e the data after the header
     *
     * \return char* the pointer
     *
     */
    char* Body() {
        return (data.get() + sizeof(Frame));
    }

    /** \brief Return the size of the body of the message
     *
     * \return size_t the size
     *
     */
    size_t BodySize() {
        return (Tail<unsigned char*>() - reinterpret_cast<unsigned char*>(Body()));
    }

    /** \brief Return a pointer at the index position
     *
     * \return char* the pointer
     *
     */
    template<class T>
    T Tail() {
        return reinterpret_cast<T>(data.get() + index);
    }

    /** \brief Set the index to a new position.
     *
     *  0 is the first byte of the frame.
     *
     * \param new_position size_t new position.
     *
     */
    void SetIndexPosition(size_t new_position) {
        assert(new_position <= data_size);
        index = new_position;
    }

    /** \brief Set the index to a new position.
     *
     *  0 is the first byte of the body.
     *
     * \param new_position size_t new position.
     *
     */
    void Resize(size_t new_position) {
        if (index < new_position + sizeof(Frame)) {
            index = new_position + sizeof(Frame);
        }
        assert(index <= data_size);
    }

    /** \brief Append data to the message using the copy method of the object
     *
     * The data is appended starting at the index position
     *
     * \param data const T& the data
     *
     */
    template<class T>
    void append(const T& data) {
        Resize(BodySize() + sizeof(T));
        *reinterpret_cast<T*>(Tail<unsigned char*>() - sizeof(T)) = data;
    }

    /** \brief Append data to the message using memcpy function
     *
     * The data is appended starting at the index position
     *
     * \param in const void* the pointer on the data
     * \param sizeBytes std::size_t the number of bytes to append
     *
     */
    void append(const void* in, std::size_t sizeBytes) {
        Resize(BodySize() + sizeBytes);
        std::memcpy(Tail<unsigned char*>() - sizeBytes, in, sizeBytes);
    }

    /** \brief Return the position of the index
     *
     * \return size_t the position of the index
     *
     */
    size_t PacketSize() const { return index; }

    /** \brief Return the size available in the allocated buffer
     *
     * \return size_t the size
     *
     */
    size_t BufferSize() const { return data_size; }

    /** \brief NodeData instance setter
     *
     * \param nodedata the instance to store
     *
     */
    void SetNodeData(std::shared_ptr<NetworkNodeData> nodedata) { this->node_data = std::move(nodedata); }

    /** \brief Set the timestamp
     *
     * \param timestamp uint64_t the timestamp
     *
     */
    void SetTimestamp(uint64_t timestamp) { Header()->timestamp = timestamp; }

    /** \brief NodeData instance getter
     *
     * \return the instance
     *
     */
    const std::shared_ptr<NetworkNodeData> NodeData() const { return node_data; }

    // The pointer on the buffer
    std::shared_ptr<char> data;
    // the number of bytes allocated
    size_t data_size;
    // the position where data will be appended
    size_t index;
    // an instance of NodeData used during message distribution
    std::shared_ptr<NetworkNodeData> node_data;
};

// Declare specialized template functions.
template<> Message& Message::operator<<(const std::string& in);
template<> Message& Message::operator<<(const std::vector<unsigned char>& in);
} // network
} // trillek

#endif // MESSAGE_H_INCLUDED

#ifndef VMAC_DATAGRAM_HASHER_HPP_INCLUDED
#define VMAC_DATAGRAM_HASHER_HPP_INCLUDED

#include <functional>
#include <memory>
#include "vmac.h"
#include "aes.h"
#include "integer.h"

namespace trillek { namespace network { namespace cryptography {

using namespace std::placeholders;

/** \brief Provide a stateless VMAC hasher
 *
 */
class VMAC_DatagramHasher final : public std::enable_shared_from_this<VMAC_DatagramHasher> {

public:

    typedef std::shared_ptr<CryptoPP::FixedSizeAlignedSecBlock<byte,16>> buffer;

    /** \brief Constructor
     *
     * \param key std::unique_ptr<std::vector<byte>>& The secret key
     *
     */
    VMAC_DatagramHasher(buffer&& key);
    VMAC_DatagramHasher() {};
    ~VMAC_DatagramHasher() {};

    VMAC_DatagramHasher(VMAC_DatagramHasher&) = delete;
    VMAC_DatagramHasher& operator=(const VMAC_DatagramHasher&) = delete;

    /** \brief Get a function object containing the hasher
     *
     * \return The function to hash a message
     *
     */
    std::function<void(unsigned char*,const unsigned char*,size_t,uint64_t)> Hasher() {
        return std::bind(&cryptography::VMAC_DatagramHasher::CalculateDigest, this->shared_from_this(), _1, _2, _3, _4);
    }

    /** \brief Get a function object containing the verifier
     *
     * \return The function to verify a message
     *
     */
    std::function<bool(const unsigned char*,const unsigned char*,size_t,uint64_t)> Verifier() {
        return std::bind(&cryptography::VMAC_DatagramHasher::Verify, this->shared_from_this(), _1, _2, _3, _4);
    }

private:

    /** \brief Verify a digest
     *
     * This function is thread-safe
     *
     * \param digest const byte* the digest to verify (8 bytes)
     * \param message const byte* the message associated with the digest
     * \param len int the length of the message
     * \param nonce uint64_t the nonce
     * \return bool true if the verification is positive, false otherwise
     *
     */
    bool Verify(const byte* digest, const byte* message, size_t len, uint64_t nonce);

    /** \brief Compute a digest of 8 bytes
     *
     * This function is thread-safe
     *
     * \param digest byte* the 8-byte buffer to fill
     * \param message const byte* the message to hash
     * \param len int the length of the message (in bytes)
     * \param nonce uint64_t the nonce
     *
     */
    void CalculateDigest(byte* digest, const byte* message, size_t len, uint64_t nonce);

    const buffer _key;
    CryptoPP::VMAC<CryptoPP::AES,64> _hasher;
};
} // cryptography
} // network
} // trillek


#endif // VMAC_DATAGRAM_HASHER_HPP_INCLUDED

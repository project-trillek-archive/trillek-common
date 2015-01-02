#include "controllers/network/VMAC-datagram-hasher.hpp"

//#include <cstring>
//#include <iostream>
#include "algparam.h"
#include "argnames.h"

using namespace CryptoPP;

namespace trillek { namespace network { namespace cryptography {
VMAC_DatagramHasher::VMAC_DatagramHasher(buffer&& key)
    : _key(std::move(key)) {
    uint64_t nonce = 0;
    _hasher.SetKey(_key->data(), _key->size(),
        MakeParameters(Name::IV(), ConstByteArrayParameter(reinterpret_cast<byte*>(&nonce), 8, false), false));
};

bool VMAC_DatagramHasher::Verify(const byte* digest, const byte* message, size_t len, uint64_t nonce) {
    CryptoPP::Integer _nonce(CryptoPP::Integer::POSITIVE, static_cast<CryptoPP::lword>(nonce));
    byte noncebuffer[8];
/*     Integer vmac, k, m;
    vmac.Decode(digest, 8);
    k.Decode(_key->data(), 16);
    m.Decode(message, len);
    std::cout << ">>Verify VMAC" << std::endl;
    std::cout << "   VMAC is " << std::hex << vmac << std::endl;
    std::cout << "   nonce is " << _nonce << std::endl;
    std::cout << "   key is " << std::hex << k << std::endl;
    std::cout << "   message is " << std::hex << m << std::endl;
    std::cout << "   len is " << len << std::endl;
*/
    _nonce.Encode(noncebuffer, 8);
    _hasher.Resynchronize(noncebuffer, 8);
    if (_hasher.VerifyDigest(digest, message, len)) {
//        std::cout << ">>VMAC OK" << std::endl;
        return true;
    }
//    std::cout << ">>VMAC FAILED" << std::endl;
    return false;
}

void VMAC_DatagramHasher::CalculateDigest(byte* digest, const byte* message, size_t len, uint64_t nonce) {
    CryptoPP::Integer _nonce(CryptoPP::Integer::POSITIVE, static_cast<CryptoPP::lword>(nonce));
    byte noncebuffer[8];
    _nonce.Encode(noncebuffer, 8);
    _hasher.Resynchronize(noncebuffer, 8);
    _hasher.CalculateDigest(digest, message, len);
/*    _hasher.Resynchronize(noncebuffer, 8);
    if (_hasher.VerifyDigest(digest, message, len)) {
        std::cout << ">>VMAC OK" << std::endl;
    }
    else {
        std::cout << ">>VMAC FAILED" << std::endl;
    }
     Integer vmac, nonc, k, m;
    vmac.Decode(digest, 8);
    k.Decode(_key->data(), 16);
    m.Decode(message, len);
    std::cout << ">>Compute VMAC" << std::endl;
    std::cout << "   VMAC is " << std::hex << vmac << std::endl;
    std::cout << "   nonce is " << _nonce << std::endl;
    std::cout << "   key is " << std::hex << k << std::endl;
    std::cout << "   message is " << std::hex << m << std::endl;
    std::cout << "   len is " << len << std::endl;
*/
}
} // cryptography
} // network
} // trillek

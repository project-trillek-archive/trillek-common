#include "controllers/network/VMAC-stream-hasher.hpp"

//#include <cstring>
//#include <iostream>
#include "algparam.h"
#include "argnames.h"

using namespace CryptoPP;

namespace trillek { namespace network { namespace cryptography {
VMAC_StreamHasher::VMAC_StreamHasher(buffer&& key, const byte* nonce)
    : _key(std::move(key)), _nonce(nonce, 8) {
    _hasher.SetKey(_key->data(), _key->size(),
        MakeParameters(Name::IV(), ConstByteArrayParameter(nonce, 8, false), false));
};

bool VMAC_StreamHasher::Verify(const byte* digest, const byte* message, size_t len) {
    byte nonce[8];
    ++_nonce;
/*     Integer vmac, nonc, k, m;
    vmac.Decode(digest, 8);
    nonc.Decode(nonce, 8);
    k.Decode(_key->data(), 16);
    m.Decode(message, len);
    std::cout << ">>Verify VMAC" << std::endl;
    std::cout << "   VMAC is " << std::hex << vmac << std::endl;
    std::cout << "   nonce is " << _nonce << std::endl;
    std::cout << "   key is " << std::hex << k << std::endl;
    std::cout << "   message is " << std::hex << m << std::endl;
*/
    _nonce.Encode(nonce, 8);
    _hasher.Resynchronize(nonce, 8);
    if (_hasher.VerifyDigest(digest, message, len)) {
//        std::cout << ">>VMAC OK" << std::endl;
        return true;
    }
    // if check failed, reset the counter to previous state
    --_nonce;
//    std::cout << ">>VMAC FAILED" << std::endl;
    return false;
}

void VMAC_StreamHasher::CalculateDigest(byte* digest, const byte* message, size_t len) {
    byte nonce[8];
    ++_nonce;
    _nonce.Encode(nonce, 8);
    _hasher.Resynchronize(nonce, 8);
    _hasher.CalculateDigest(digest, message, len);
    /* Integer vmac, nonc, k, m;
    vmac.Decode(digest, 8);
    nonc.Decode(nonce, 8);
    k.Decode(_key->data(), 16);
    m.Decode(message, len);
    std::cout << ">>Compute VMAC" << std::endl;
    std::cout << "VMAC is " << std::hex << vmac << std::endl;
    std::cout << "nonce is " << std::hex << nonc << std::endl;
    std::cout << "key is " << std::hex << k << std::endl;
    std::cout << "message is " << std::hex << m << std::endl;
    */
}
} // cryptography
} // network
} // trillek

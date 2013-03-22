
#include <authenticator.hpp>
#include <iostream>
#include "Poco/SHA1Engine.h"
#include "Poco/DigestEngine.h"

using namespace std;

Authenticator::Authenticator(int seed)
{
    if (seed > 0) {
        _prng.seed(seed);
    } else {
        _prng.seed();
    }
    this->genNonce();
}

const uint32_t Authenticator::genNonce()
{
    _nonce = _prng.next();
    return _nonce;
}

bool Authenticator::authenticate(string ident, string secret) const
{
    string password = "pippero";

    Poco::SHA1Engine sha1;
    sha1.update((char*)&_nonce, 4);
    sha1.update(password);

    return (Poco::DigestEngine::digestToHex(sha1.digest()) == secret);
}

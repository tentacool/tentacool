
#include <authenticator.hpp>

#include <iostream>
#include <cassert>
#include <stdio.h>
#include <arpa/inet.h>

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
//    _nonce = _prng.next();
    _nonce = 1234;
    return _nonce;
}

bool Authenticator::authenticate(string ident, string secret)
{
    // TODO
    string password = "pippero";

    Poco::DigestEngine::Digest d(secret.begin(), secret.end());

    Poco::SHA1Engine sha1;
    sha1.update((unsigned char*)&_nonce, 4);
    sha1.update(password.data(), password.size());

    string d_str(Poco::DigestEngine::digestToHex(d));
    string sha1_str(Poco::DigestEngine::digestToHex(sha1.digest()));

    return (d_str == sha1_str);
}

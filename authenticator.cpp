#include "authenticator.hpp"
#include <iostream>
#include <stdio.h>
#include <Poco/SHA1Engine.h>
#include <Poco/DigestEngine.h>

using namespace std;

Authenticator::Authenticator(int seed): _nonce(0)
{
    if(seed > 0){
        _prng.seed(seed);
    }else{
        _prng.seed();
    }
    this->genNonce();
}
Authenticator::~Authenticator()
{
}
const uint32_t Authenticator::genNonce()
{
    _nonce = _prng.next();
    return _nonce;
}

const uint32_t Authenticator::nonce() const
{
    return _nonce;
}

bool Authenticator::authenticate(string hash, string password)
{
    Poco::DigestEngine::Digest d(hash.begin(), hash.end());
    Poco::SHA1Engine sha1;
    sha1.update((unsigned char*)&_nonce, 4);
    sha1.update(password.data(), password.size());

    string d_str(Poco::DigestEngine::digestToHex(d));
    string sha1_str(Poco::DigestEngine::digestToHex(sha1.digest()));
    return (d_str == sha1_str);
}

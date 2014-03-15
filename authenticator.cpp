
#include "Authenticator.hpp"

#include <iostream>
#include <stdio.h>
#include "DataManager.hpp"
#include "Poco/SHA1Engine.h"
#include "Poco/DigestEngine.h"

using namespace std;

Authenticator::Authenticator(int seed): _nonce(_prng.next()/*1234*/)
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
	//_nonce = _prng.next();
    return _nonce;
}

bool Authenticator::authenticate(string ident, string hash, string password)
{
	/*! Authenticate a Client by name and the hash received by the brocker
	 *
	 */
    //string password = "none";

	Poco::DigestEngine::Digest d(hash.begin(), hash.end());

    Poco::SHA1Engine sha1;
    sha1.update((unsigned char*)&_nonce, 4);
    sha1.update(password.data(), password.size());

    string d_str(Poco::DigestEngine::digestToHex(d));
    string sha1_str(Poco::DigestEngine::digestToHex(sha1.digest()));
    return (d_str == sha1_str);
}


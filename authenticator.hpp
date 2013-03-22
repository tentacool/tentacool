
#ifndef __AUTHENTICATOR_HPP__
#define __AUTHENTICATOR_HPP__

#include <string>
#include <stdint.h>
#include "Poco/Random.h"

class Authenticator {
    Poco::Random _prng;
    uint32_t _nonce;
public:
    // Constructor
    // @param seed The seed to use. Default value means to generate it
    //             internally (random)
    Authenticator(int seed = 0);

    // Gets a nonce for the challenge. This starts the authentication routine
    // for the client
    const uint32_t genNonce();

    // Nonce getter
    const uint32_t nonce() const { return _nonce; }

    // The authentication method.
    // @param ident The username for the authentication
    // @param secret The hashed password
    bool authenticate(std::string ident, std::string secret) const;
};

#endif

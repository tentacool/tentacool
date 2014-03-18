
#ifndef __AUTHENTICATOR_HPP__
#define __AUTHENTICATOR_HPP__

#include <string>
#include <stdint.h>
#include "Poco/Random.h"

class Authenticator {
	/*! This class have to authenticate an user by challenge authentication (SHA1 of the password and a nonce)*/
    Poco::Random _prng;
    uint32_t _nonce;
public:
    /// Constructor
    /// @param seed The seed to use. Default value means to generate it
    ///             internally (random)
    Authenticator(int seed = 0);

    /// Gets a nonce for the challenge. This starts the authentication routine
    /// for the client
    const uint32_t genNonce();

    /// Nonce getter
    const uint32_t nonce() const { return _nonce; }

    /// The authentication method.
    /// @param hash The hashed password
    /// @param password The user password
    bool authenticate( std::string hash, std::string password);
};

#endif

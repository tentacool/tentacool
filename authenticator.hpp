#ifndef __AUTHENTICATOR__
#define __AUTHENTICATOR__

#include <stdint.h>
#include "Poco/Random.h"

//! Authenticate an user by challenge authentication (SHA1 of the password and a nonce)
class Authenticator{
    Poco::Random _prng;
    uint32_t _nonce;
public:
    //! Constructor
    //! \param seed The seed to use. Default value means to generate it
    //!             internally (random)
    Authenticator(int seed = 0);

    ~Authenticator();

    //! Gets a nonce for the challenge
    const uint32_t genNonce();

    //! Nonce getter
    const uint32_t nonce() const;

    //! The authentication method.
    //! \param hash The hashed password
    //! \param password The user password
    bool authenticate(std::string hash, std::string password);
};
#endif

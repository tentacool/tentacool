//
// Copyright 2013
// Dario Lombardo <lomato@gmail.com>
// Sebastiano Di Paola <sebastiano.dipaola@gmail.com>
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
//

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

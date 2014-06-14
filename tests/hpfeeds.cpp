#include "hpfeeds.hpp"
#include <algorithm>
#include <iostream>
#include <Poco/SHA1Engine.h>
#include <Poco/DigestEngine.h>

using namespace std;

hpf_msg hpf_new()
{
    hpf_msg msg {0x05, 0x00, 0x00, 0x00, 0xff};
    return msg;
}

hpf_msg hpf_error(const string err)
{
    hpf_msg msg = hpf_new();
    msg.at(msg.size() - 1) = OP_ERROR;
    hpf_add_payload(msg, reinterpret_cast<const u_char*>(err.data()),
                                                                err.length());
    return msg;
}

hpf_msg hpf_info(u_int32_t nonce, string fbname)
{
    hpf_msg msg = hpf_new();
    msg.at(msg.size() - 1) = OP_INFO;
    hpf_add_chunk(msg, reinterpret_cast<const u_char*>(fbname.data()),
                                                               fbname.length());
    hpf_add_payload(msg, reinterpret_cast<const u_char*>(&nonce),
                                                            sizeof(uint32_t));
    return msg;
}

hpf_msg hpf_auth(uint32_t nonce, string ident, string secret)
{
    hpf_msg msg = hpf_new();
    msg.at(msg.size() - 1) = OP_AUTH;
    //Preparing the hash
    Poco::SHA1Engine sha1;
    Poco::DigestEngine::Digest hash_d;
    sha1.update(reinterpret_cast<u_char*>(&nonce), 4);
    sha1.update(secret.data(), secret.size());
    hash_d = sha1.digest();
    string hash(hash_d.begin(),hash_d.end());

    hpf_add_chunk(msg, reinterpret_cast<const u_char*>(ident.data()),
                                                                ident.length());
    hpf_add_payload(msg, reinterpret_cast<const u_char*>(hash.data()),
                                                                 hash.length());
    return msg;
}

hpf_msg hpf_subscribe(string ident, string channel)
{
    hpf_msg msg = hpf_new();
    msg.at(msg.size() - 1) = OP_SUBSCRIBE;
    hpf_add_chunk(msg, reinterpret_cast<const u_char*>(ident.data()),
                                                                ident.length());
    hpf_add_payload(msg, reinterpret_cast<const u_char*>(channel.data()),
                                                              channel.length());
    return msg;
}

hpf_msg hpf_publish(string ident, string channel, u_char *data,
                                                    uint32_t data_len)
{
    if(data == NULL || data_len == 0)
        throw Poco::Exception("Publish: Invalid Input");
    hpf_msg msg = hpf_new();
    msg.at(msg.size() - 1) = OP_PUBLISH;
    hpf_add_chunk(msg, reinterpret_cast<const u_char*>(ident.data()),
                                                                ident.length());
    hpf_add_chunk(msg, reinterpret_cast<const u_char*>(channel.data()),
                                                              channel.length());
    hpf_add_payload(msg, data, data_len);
    return msg;
}

void hpf_add_chunk(hpf_msg& m, const u_char *data, uint32_t len)
{
    u_char chunk_length;
    uint32_t old_size = (uint32_t) m.size();
    chunk_length = len < 0xff ? len : 0xff;
    m.resize(m.size() + chunk_length + 1);
    ///////////  BYTE ORDERING  - BIG ENDIAN //////////
    const uint32_t ns = (uint32_t) m.size();
    uint8_t new_size[sizeof(uint32_t)];
    new_size[3] = (ns & 0x000000ff);
    new_size[2] = (ns & 0x0000ff00) >> 8;
    new_size[1] = (ns & 0x00ff0000) >> 16;
    new_size[0] = (ns & 0xff000000) >> 24;
    m.at(old_size) = chunk_length; //set chunk length
    copy(data, data + len, m.end() - len);  //copy chunk
    copy(new_size, new_size + sizeof(uint32_t), m.begin());  //set length
}

void hpf_add_payload(hpf_msg &m, const u_char *data, uint32_t len)
{
    m.resize(m.size() + len);
    ///////////  BYTE ORDERING //////////
    const uint32_t ns = (uint32_t) m.size();
    uint8_t new_size[sizeof(uint32_t)];
    new_size[3] = (ns & 0x000000ff);
    new_size[2] = (ns & 0x0000ff00) >> 8;
    new_size[1] = (ns & 0x00ff0000) >> 16;
    new_size[0] = (ns & 0xff000000) >> 24;
    copy(data, data + len, m.end() - len); //copy payload
    copy(new_size, new_size + sizeof(uint32_t), m.begin()); //set length
}

uint32_t hpf_getsize(hpf_msg& m)
{
    return m.size();
}

uint8_t hpf_gettype(hpf_msg& m)
{
    return m.at(sizeof(uint32_t));
}

hpf_chunk_t *hpf_msg_get_chunk(u_char *data, size_t len)
{
    hpf_chunk_t *c;
    if (!data || !len)
        throw Poco::Exception("Invalid data input!");
    c = reinterpret_cast<hpf_chunk_t*>(data);
    // incomplete chunk?
    if (c->len > len + 1)
        throw Poco::Exception("Invalid data input!");
    return c;
}

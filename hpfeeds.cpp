#include "hpfeeds.hpp"
#include <algorithm>
#include <iostream>
#include <Poco/SHA1Engine.h>
#include <Poco/DigestEngine.h>
#include <Poco/NumberFormatter.h>

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
    //Preparing the hash to use
    Poco::SHA1Engine sha1;
    Poco::DigestEngine::Digest hash_d;
    sha1.update((unsigned char*)&nonce, 4);
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
    try{
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
    }catch(Poco::Exception& e){e.displayText();}
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

////////////////// OLD ///////////////

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

/*
hpf_msg_t *hpf_msg_new()
{
    hpf_msg_t *msg;
    msg = (hpf_msg_t *) calloc(1, sizeof(hpf_msg_t ));
    msg->hdr.msglen = htonl(sizeof(msg->hdr));
    return msg;
}

void hpf_msg_delete(hpf_msg_t *m)
{
    if (m) free(m);
    return;
}

u_int32_t hpf_msg_getsize(hpf_msg_t *m)
{
    return ntohl(m->hdr.msglen);
}

u_int32_t hpf_msg_gettype(hpf_msg_t *m)
{
    return m->hdr.opcode;
}

void hpf_msg_add_payload(hpf_msg_t **m, const u_char *data, size_t len)
{
    if (!m || !data || !len)
        throw Poco::Exception("Invalid data input!");
    *m = reinterpret_cast<hpf_msg_t*>(realloc(*m, ntohl((*m)->hdr.msglen) +
                                                                        len));
    if (!*m)
        throw Poco::Exception("Memory allocation fault");
    memcpy((reinterpret_cast<u_char *>(*m)) + ntohl((*m)->hdr.msglen),
                                                                    data, len);
    (*m)->hdr.msglen = htonl(ntohl((*m)->hdr.msglen) + len);
}

void hpf_msg_add_chunk(hpf_msg_t **m, const u_char *data, size_t len)
{
    u_char chunk_length;
    if (!m || !data || !len)
        throw Poco::Exception("Invalid data input!");
    chunk_length = len < 0xff ? len : 0xff;
    *m = reinterpret_cast<hpf_msg_t*>(realloc(*m, ntohl((*m)->hdr.msglen) +
                                                            chunk_length + 1));
    if (!*m)
        throw Poco::Exception("Memory allocation fault");
    (reinterpret_cast<u_char*>(*m))[ntohl((*m)->hdr.msglen)] = chunk_length;
    memcpy((reinterpret_cast<u_char *>(*m)) + ntohl((*m)->hdr.msglen) + 1,
                                                           data, chunk_length);
    (*m)->hdr.msglen = htonl(ntohl((*m)->hdr.msglen) + 1 + chunk_length);
}

hpf_msg_t *hpf_msg_error(const string err)
{
    hpf_msg_t *msg = hpf_msg_new();
    if (!msg)
        throw Poco::Exception("Memory allocation fault");
    msg->hdr.opcode = OP_ERROR;
    hpf_msg_add_payload(&msg, reinterpret_cast<const u_char*>(err.data()),
                                                                err.length());
    return msg;
}

hpf_msg_t *hpf_msg_info(u_int32_t nonce, string fbname)
{
    hpf_msg_t *msg;
    msg = hpf_msg_new();
    if (!msg)
        throw Poco::Exception("Memory allocation fault");
    msg->hdr.opcode = OP_INFO;
    hpf_msg_add_chunk(&msg, reinterpret_cast<const u_char*>(fbname.data()),
                                                               fbname.length());
    hpf_msg_add_payload(&msg, reinterpret_cast<u_char*>(&nonce),
                                                            sizeof(u_int32_t));
    return msg;
}
hpf_msg_t *hpf_msg_publish(string ident, string channel, u_char *data,
                                                               size_t data_len)
{
    hpf_msg_t *msg;
    msg = hpf_msg_new();
    if (!msg)
        throw Poco::Exception("Memory allocation fault");
    msg->hdr.opcode = OP_PUBLISH;
    hpf_msg_add_chunk(&msg, reinterpret_cast<const u_char*>(ident.data()),
                                                                ident.length());
    hpf_msg_add_chunk(&msg, reinterpret_cast<const u_char*>(channel.data()),
                                                              channel.length());
    try {
        hpf_msg_add_payload(&msg, data, data_len);
    } catch (Poco::Exception& e){
        hpf_msg_delete(msg);
        throw Poco::Exception("Memory allocation fault");
    }
    return msg;
}

hpf_msg_t *hpf_msg_subscribe(string ident, string channel)
{
    hpf_msg_t *msg;
    msg = hpf_msg_new();
    if (!msg)
        throw Poco::Exception("Memory allocation fault");
    msg->hdr.opcode = OP_SUBSCRIBE;
    hpf_msg_add_chunk(&msg, reinterpret_cast<const u_char*>(ident.data()),
                                                                ident.length());

    try {
        hpf_msg_add_payload(&msg, 
            reinterpret_cast<const u_char*>(channel.data()), channel.length());
    } catch (Poco::Exception& e){
        hpf_msg_delete(msg);
        throw Poco::Exception("Memory allocation fault");
    }
    return msg;
}
hpf_msg_t *hpf_msg_auth(u_int32_t nonce, string ident, string secret)
{
    hpf_msg_t *msg;
    msg = hpf_msg_new();
    if (!msg)
        throw Poco::Exception("Memory allocation fault");
    msg->hdr.opcode = OP_AUTH;

    //Preparing the hash to use
    Poco::SHA1Engine sha1;
    Poco::DigestEngine::Digest hash_d;
    sha1.update((unsigned char*)&nonce, 4);
    sha1.update(secret.data(), secret.size());
    hash_d = sha1.digest();
    string hash(hash_d.begin(),hash_d.end());

    hpf_msg_add_chunk(&msg, reinterpret_cast<const u_char*>(ident.data()),
                                                                ident.length());
    hpf_msg_add_payload(&msg, reinterpret_cast<const u_char*>(hash.data()),
                                                                 hash.length());
    return msg;
}
*/

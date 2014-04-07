#ifndef __HPFEEDS__
#define __HPFEEDS__

#include <arpa/inet.h>
#include <cstring>
#include <string>
#include <iostream>
#include <Poco/Exception.h>

#define OP_ERROR 0
#define OP_INFO 1
#define OP_AUTH 2
#define OP_PUBLISH 3
#define OP_SUBSCRIBE 4

using namespace std;

//! Struct representing an hpfeeds message
typedef struct {
    struct {
        u_int32_t msglen;   //!< Total message lenght
        u_int8_t opcode;    //!< Message OP_CODE
    }  __attribute__((__packed__)) hdr;
    u_char data[]; //!< Data payload
} hpf_msg_t;

//! Struct representing a chunk of data carried by a hpfeeds message
typedef struct {
    u_char len;     //!< Chunk lenght
    u_char data[];  //!< Data payload
} hpf_chunk_t;

hpf_msg_t *hpf_msg_new()
{
    hpf_msg_t *msg;
    msg = new hpf_msg_t;
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
    *m = reinterpret_cast<hpf_msg_t*>(realloc(*m, ntohl((*m)->hdr.msglen) + len));
    if (!*m)
        throw Poco::Exception("Memory allocation fault");
    memcpy((reinterpret_cast<u_char *>(*m)) + ntohl((*m)->hdr.msglen), data, len);
    (*m)->hdr.msglen = htonl(ntohl((*m)->hdr.msglen) + len);
}

void hpf_msg_add_chunk(hpf_msg_t **m, const u_char *data, size_t len)
{
    u_char chunk_length;
    if (!m || !data || !len)
        throw Poco::Exception("Invalid data input!");
    chunk_length = len < 0xff ? len : 0xff;
    *m = reinterpret_cast<hpf_msg_t*>(realloc(*m, ntohl((*m)->hdr.msglen) + chunk_length + 1));
    if (!*m)
        throw Poco::Exception("Memory allocation fault");
    (reinterpret_cast<u_char*>(*m))[ntohl((*m)->hdr.msglen)] = chunk_length;
    memcpy((reinterpret_cast<u_char *>(*m)) + ntohl((*m)->hdr.msglen) + 1, data, chunk_length);
    (*m)->hdr.msglen = htonl(ntohl((*m)->hdr.msglen) + 1 + chunk_length);
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

hpf_msg_t *hpf_msg_error(const string err)
{
    hpf_msg_t *msg;
    msg = hpf_msg_new();
    if (!msg)
        throw Poco::Exception("Memory allocation fault");
    msg->hdr.opcode = OP_ERROR;
    hpf_msg_add_payload(&msg, reinterpret_cast<const u_char*>(err.data()), err.length());
    return msg;
}

hpf_msg_t *hpf_msg_info(u_int32_t nonce, string fbname)
{
    hpf_msg_t *msg;
    msg = hpf_msg_new();
    if (!msg)
        throw Poco::Exception("Memory allocation fault");
    msg->hdr.opcode = OP_INFO;
    hpf_msg_add_chunk(&msg, reinterpret_cast<const u_char*>(fbname.data()), fbname.length());
    hpf_msg_add_payload(&msg, reinterpret_cast<u_char*>(&nonce), sizeof(u_int32_t));
    return msg;
}
hpf_msg_t *hpf_msg_publish(string ident, string channel, u_char *data, size_t data_len)
{
    hpf_msg_t *msg;
    msg = hpf_msg_new();
    if (!msg)
        throw Poco::Exception("Memory allocation fault");
    msg->hdr.opcode = OP_PUBLISH;
    hpf_msg_add_chunk(&msg, reinterpret_cast<const u_char*>(ident.data()), ident.length());
    hpf_msg_add_chunk(&msg, reinterpret_cast<const u_char*>(channel.data()), channel.length());
    hpf_msg_add_payload(&msg, data, data_len);
    return msg;
}
#endif /* __HPFEEDS__ */

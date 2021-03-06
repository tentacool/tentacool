#ifndef __HPFEEDS__
#define __HPFEEDS__

#include <arpa/inet.h>
#include <cstring>
#include <cstdlib>
#include <string>
#include <vector>
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

typedef vector<u_char> hpf_msg; //!< The hpfeeds message

hpf_msg hpf_new();
hpf_msg hpf_error(const string err);
hpf_msg hpf_info(u_int32_t nonce, string fbname);
hpf_msg hpf_auth(uint32_t nonce, string ident, string secret);
hpf_msg hpf_subscribe(string ident, string channel);
hpf_msg hpf_publish(string ident, string channel, u_char *data,
                                                    uint32_t data_len);
void hpf_add_chunk(hpf_msg&m, const u_char *data, uint32_t len);
void hpf_add_payload(hpf_msg& m, const u_char *data, uint32_t len);
uint32_t hpf_getsize(hpf_msg& m);
uint8_t hpf_gettype(hpf_msg& m);

//! TODO
hpf_chunk_t *hpf_msg_get_chunk(u_char *data, size_t len);

#endif

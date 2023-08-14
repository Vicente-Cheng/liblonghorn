#ifndef LONGHORN_RPC_PROTOCOL_HEADER
#define LONGHORN_RPC_PROTOCOL_HEADER

#include <pthread.h>
#include <time.h>

#include "lib/uthash.h"
#include "lib/utlist.h"

#define MAGIC_VERSION 0x1b01 // LongHorn01

struct MessageHeader {
        uint16_t        MagicVersion; // 2 bytes
        uint32_t        Seq; // 4 bytes
        uint16_t        Type; // 2 bytes
        uint64_t        Offset; // 8 bytes
        uint32_t        Size; // 4 bytes
        uint32_t        DataLength; // 4 bytes
} __attribute__((packed));

struct MessageWithData {
        struct MessageHeader *header;
        void *data;
};

struct Message {
        uint16_t        MagicVersion;
        uint32_t        Seq;
        uint16_t        Type;
        int64_t         Offset;
        uint32_t        Size;
        uint32_t        DataLength;
        void*           Data;

	pthread_cond_t  cond;
	pthread_mutex_t mutex;

        UT_hash_handle  hh;

        struct Message *next, *prev;
};

enum uint32_t {
	TypeRead,
	TypeWrite,
	TypeResponse,
	TypeError,
	TypeEOF,
	TypeClose,
	TypePing,
	TypeUnmap
};

int send_msg(int fd, struct Message *msg, void *header, ssize_t header_size);
int receive_msg(int fd, struct Message *msg, uint8_t *header, int header_size);

#endif

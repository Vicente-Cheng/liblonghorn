#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <endian.h>

#include "log/log.h"
#include "longhorn_rpc_protocol.h"

static ssize_t read_full(int fd, void *buf, ssize_t len) {
        ssize_t nread = 0;
        ssize_t ret;

        while (nread < len) {
                ret = read(fd, buf + nread, len - nread);
                if (ret < 0) {
                        if (errno == EINTR) {
                                continue;
                        }
                        return ret;
                } else if (ret == 0) {
                        return nread;
                }
                nread += ret;
        }

        return nread;
}

static ssize_t write_full(int fd, void *buf, ssize_t len) {
        ssize_t nwrote = 0;
        ssize_t ret;

        while (nwrote < len) {
                ret = write(fd, buf + nwrote, len - nwrote);
                if (ret < 0) {
                        log_error("write() error, errno: %d", errno);
                        if (errno == EINTR) {
                                continue;
                        }
                        return ret;
                }
                nwrote += ret;
        }

        return nwrote;
}

int send_msg(int fd, struct Message *msg, void *header, ssize_t size) {
        ssize_t n = 0;

        n = write_full(fd, header, size);
        if (n != size) {
                log_error("fail to write header\n");
                return -EINVAL;
        }

	if (msg->DataLength != 0) {
	        n = write_full(fd, msg->Data, msg->DataLength);
                if (n != msg->DataLength) {
	                log_error("fail to write data, wrote %zd; expected %u", n, msg->DataLength);
                        return -EINVAL;
	        }
        }
        return 0;
}

static int read_header(int fd, struct Message *msg, uint8_t *header, int header_size) {
        uint64_t Offset;
        int offset = 0, n = 0;

        n = read_full(fd, header, header_size);
        if (n != header_size) {
                log_error("fail to read header\n");
		return -EINVAL;
        }

        msg->MagicVersion = le16toh(*((uint16_t *)(header)));
        offset += sizeof(msg->MagicVersion);

        if (msg->MagicVersion != MAGIC_VERSION) {
                log_error("wrong magic version 0x%x, expected 0x%x\n",
                                msg->MagicVersion, MAGIC_VERSION);
                return -EINVAL;
        }

        msg->Seq = le32toh(*((uint32_t *)(header + offset)));
        offset += sizeof(msg->Seq);

        msg->Type = le16toh(*((uint16_t *)(header + offset)));
        offset += sizeof(msg->Type);

        Offset = le64toh(*((uint64_t *)(header + offset)));
        msg->Offset = *( (int64_t *) &Offset);
        offset += sizeof(msg->Offset);

        msg->Size = le32toh(*((uint32_t *)(header + offset)));
        offset += sizeof(msg->Size);

        msg->DataLength = le32toh(*((uint32_t *)(header + offset)));
        offset += sizeof(msg->DataLength);

        return offset;
}

// Caller needs to release msg->Data
int receive_msg(int fd, struct Message *msg, uint8_t *header, int header_size) {
	ssize_t n;

        bzero(msg, sizeof(struct Message));

        // There is only one thread reading the response, and socket is
        // full-duplex, so no need to lock
        n = read_header(fd, msg, header, header_size);
        if (n != header_size) {
                log_error("fail to read header, except: %d, actual: %d\n", header_size, n);
                return -EINVAL;
        }

	if (msg->DataLength > 0) {
		msg->Data = malloc(msg->DataLength);
                if (msg->Data == NULL) {
                        perror("cannot allocate memory for data");
                        return -EINVAL;
                }
		n = read_full(fd, msg->Data, msg->DataLength);
		if (n != msg->DataLength) {
                        log_error("Cannot read full from fd, %u vs %zd\n",
                                msg->DataLength, n);
			free(msg->Data);
			return -EINVAL;
		}
	}
	return 0;
}

#ifndef _207LAYER_H
#define _207LAYER_H

//struct union definition of packet header

void die (char *s);
void check_for_error(int ret, char* s);

// TCP functions

// packet header

typedef union {
    char full_20_byte[20];
    // assuming little endian other byte order is swapped
    struct {
        unsigned short int source_port;
        unsigned short int dest_port;
        unsigned int       seq_num;
        unsigned int       ack_num;
        unsigned int       data_offset : 4;
        unsigned int       reserved    : 3;
        unsigned int       ns_flag     : 1;
        unsigned int       cwr_flag    : 1;
        unsigned int       ece_flag    : 1;
        unsigned int       urg_flag    : 1;
        unsigned int       ack_flag    : 1;
        unsigned int       psh_flag    : 1;
        unsigned int       rst_flag    : 1;
        unsigned int       syn_flag    : 1;
        unsigned int       fin_flag    : 1;
        unsigned short int window_size;
        unsigned short int checksum;
        unsigned short int urg_ptr;
    }__attribute__((packed));
} packet_header;

// client specific functions
// int connect(int sockfd, struct sockaddr *serv_addr, int addrlen); 

// server specific functions
// int bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
// int listen(int sockfd, int backlog);
// int accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen);

// common functions
// ssize_t recv(int sockfd, void *buf, size_t len, int flags);
// ssize_t send(int sockfd, const void *buf, size_t len, int flags);
// int close(int fd);

#endif 
